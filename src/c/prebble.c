#include <pebble.h>

static Window      *s_win;           // Main window
static Layer       *s_bg_layer;      // Background for color & pattern

// Time & date
static Layer       *s_text_layer;    // Background of time & date
static TextLayer   *s_time_layer;
static GFont        s_time_font;
static bool         s_time_24h;	     // True when 24h is enabled
static TextLayer   *s_date_layer;
static GFont        s_date_font;

// Analog
static Layer       *s_analog_layer;  // Layer for sequence animation
static GDrawCommandSequence *s_seqv; // Sequence value, image frames
static uint32_t     s_seqc;          // Seq frames count
static uint32_t     s_seqi;          // Seq animation index
static Layer       *s_hands_layer;   // Analog watch hands

// Draw patter of 45 deg diagonal lines of given COLOR with CTX in
// RECT area.
static void
draw_pattern_lines(GContext *ctx, GRect rect, GColor color)
{
	// Y and X are starting points of lines.  Y starting position
	// is iterated from top BEG to bottom END.  GAP determinate
	// spacing between each line.  P1 and P2 are convenient vars.
	// 2 magic numbers, on in GAP second in value calculation of
	// BEG.  Both are adjusted to avoid overlaping lines with
	// analog clock image diagonal lines which makes them look
	// thicker than they are on Aplite gray background.
	int16_t y, x = rect.origin.x;
	int16_t gap = 28;
	int16_t beg = rect.origin.y - gap*(rect.size.w/gap) + 16;
	int16_t end = rect.origin.y + rect.size.h;
	GPoint p1, p2;

	graphics_context_set_antialiased(ctx, false);
	graphics_context_set_stroke_color(ctx, color);
	graphics_context_set_stroke_width(ctx, 1);
	for (y = beg; y < end; y += gap) {
		p1 = GPoint(x, y);
		p2 = GPoint(x + rect.size.w, y + rect.size.w);
		graphics_draw_line(ctx, p1, p2);
	}
}

static void
text_layer_update(Layer *layer, GContext *ctx)
{
	GRect bounds = layer_get_bounds(layer);

	graphics_context_set_fill_color(ctx, GColorWhite);
#ifdef PBL_RECT
	graphics_fill_rect(ctx, bounds, 6, GCornerTopLeft | GCornerTopRight);
#else
	graphics_fill_circle(ctx,
			     GPoint(bounds.origin.x + bounds.size.w/2,
				    bounds.origin.y + bounds.size.h),
			     bounds.size.h);
#endif
}

// Start s_analog_layer sequence animation.
static void
seq_anim(void *_ctx)
{
	// Advance to next frame if possible, else end animation.
	if (++s_seqi < s_seqc) {
		app_timer_register(12, seq_anim, NULL);
	} else {
		s_seqi = 0;
	}
	layer_mark_dirty(s_analog_layer);
}

static void
analog_layer_update(Layer *_l, GContext *ctx)
{
	GDrawCommandFrame *frame = gdraw_command_sequence_get_frame_by_index(s_seqv, s_seqi);

	// This should never happen.
	if (!frame) {
		APP_LOG(APP_LOG_LEVEL_ERROR, "No frame: %lu", s_seqi);
		return;
	}
	// Disable AA when static image is drawn, when s_seqi is 0.
	graphics_context_set_antialiased(ctx, s_seqi);
	gdraw_command_frame_draw(ctx, s_seqv, frame, GPoint(0, 0));
}

static void
hands_layer_update(Layer *layer, GContext *ctx)
{
	int32_t angle;
	GRect inset, bounds = layer_get_bounds(layer);
	GPoint point, mid = grect_center_point(&bounds);
	time_t timestamp = time(NULL);
	struct tm *time = localtime(&timestamp);

	graphics_context_set_antialiased(ctx, true);

	// Minute hand.
	inset = grect_inset(bounds, GEdgeInsets(2));
	angle = DEG_TO_TRIGANGLE(360*time->tm_min/60);
	point = gpoint_from_polar(inset, GOvalScaleModeFillCircle, angle);
	// Draw white border/outline around black line.
	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_context_set_stroke_width(ctx, 8);
	graphics_draw_line(ctx, mid, point);
	graphics_fill_circle(ctx, point, 2);
	// Draw actuall black line.
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_context_set_stroke_width(ctx, 4);
	graphics_draw_line(ctx, mid, point);
	graphics_fill_circle(ctx, point, 2);

	// Hour hand.
	inset = grect_inset(bounds, GEdgeInsets(14));
	angle = DEG_TO_TRIGANGLE(360*(time->tm_hour%12)/12) +
		DEG_TO_TRIGANGLE(360/12*time->tm_min/60);
	point = gpoint_from_polar(inset, GOvalScaleModeFitCircle, angle);
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_context_set_stroke_width(ctx, 4);
	graphics_draw_line(ctx, mid, point);
	graphics_fill_circle(ctx, point, 2);

	// Middle circle.
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_circle(ctx, mid, 2);
}

static void
bg_main_layer_update(Layer *layer, GContext *ctx)
{
	GRect bounds = layer_get_bounds(layer);
	GColor bg_color = PBL_IF_BW_ELSE(GColorLightGray, GColorRed);
	GColor fg_color = GColorBlack;

	graphics_context_set_fill_color(ctx, bg_color);
	graphics_fill_rect(ctx, bounds, 0, GCornerNone);
	draw_pattern_lines(ctx, bounds, fg_color);
}

static void
time_set(struct tm *time)
{
	static char buf[16];
	const char *fmt = s_time_24h ? "%H:%M" : "%I:%M %p";

	strftime(buf, sizeof(buf), fmt, time);
	text_layer_set_text(s_time_layer, buf);
}

static void
date_set(struct tm *time)
{
	static char buf[16];
	const char *fmt = PBL_IF_ROUND_ELSE("%a %m.%d", "%A %m.%d");

	strftime(buf, sizeof(buf), fmt, time);
	text_layer_set_text(s_date_layer, buf);
}

static void
unobstructed_change(AnimationProgress _p, void *_ctx)
{
	Layer *layer  = window_get_root_layer(s_win);
	GRect  bounds = layer_get_bounds(layer);
	GRect ubounds = layer_get_unobstructed_bounds(layer);

	layer_set_hidden(s_text_layer, !grect_equal(&bounds, &ubounds));
}

// Animate LAYER with BEG starting position using slide-up animation
// that starts after DELAY ms and takes DURATION ms to complete.
static void
anim_slideup(Layer *layer, GRect beg, uint32_t delay, uint32_t duration)
{
	GRect end = beg;
	PropertyAnimation *prop;
	Animation *anim;

	end.origin.y -= end.size.h;
	prop = property_animation_create_layer_frame(layer, &beg, &end);
	anim = property_animation_get_animation(prop);

	animation_set_curve(anim, AnimationCurveDefault);
	animation_set_delay(anim, delay);
	animation_set_duration(anim, duration);
	animation_schedule(anim);
}


// TickHandler that runs after each minute.
static void
onmin(struct tm *time, TimeUnits change)
{
	if (change & DAY_UNIT) {
		date_set(time);
	}
	if (change & MINUTE_UNIT) {
		seq_anim(NULL);
	}
	time_set(time);
	layer_mark_dirty(s_hands_layer);
}

static void
win_load(Window *win)
{
	Layer *layer = window_get_root_layer(win);
	GRect rect, bounds = layer_get_bounds(layer);

	// Settings that might change so better update.
	s_time_24h = clock_is_24h_style();

	// Main background.
	rect = bounds;
	s_bg_layer = layer_create(rect);
	layer_set_update_proc(s_bg_layer, bg_main_layer_update);
	layer_add_child(layer, s_bg_layer);

	// Text background for time and date.
	rect.origin.y += rect.size.h;
	rect.size.h = rect.size.h/2 + PBL_IF_ROUND_ELSE(8, 0);
	s_text_layer = layer_create(rect);
	layer_set_update_proc(s_text_layer, text_layer_update);
	layer_add_child(layer, s_text_layer);
	anim_slideup(s_text_layer, rect, 20, 400);

	// Time.
	rect.origin.x = 0;
	rect.origin.y = PBL_IF_ROUND_ELSE(26, 18);
	rect.size.h = 32;
	s_time_layer = text_layer_create(rect);
	text_layer_set_background_color(s_time_layer, GColorClear);
	text_layer_set_text_color(s_time_layer, GColorBlack);
	text_layer_set_font(s_time_layer, s_time_font);
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	layer_add_child(s_text_layer, text_layer_get_layer(s_time_layer));

	// Date.
	rect.origin.y += rect.size.h - 4;
	rect.size.h = 22;
	s_date_layer = text_layer_create(rect);
	text_layer_set_background_color(s_date_layer, GColorClear);
	text_layer_set_text_color(s_date_layer, GColorBlack);
	text_layer_set_font(s_date_layer,  s_date_font);
	text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
	layer_add_child(s_text_layer, text_layer_get_layer(s_date_layer));

	// Analog image.
	GSize seq_bounds = gdraw_command_sequence_get_bounds_size(s_seqv);
	rect = bounds;
	rect.origin.x += (bounds.size.w - seq_bounds.w)/2;
	rect.origin.y -= PBL_IF_ROUND_ELSE(8, 12);
	// -1 in width and height is necessary to have size that have
	// center pixel.  When analog_rect is used to create layer for
	// analog hands it will pass size that can have that middle
	// pixel making gpoint_from_polar return points that produce
	// straight lines.
	rect.size.w = seq_bounds.w - 1;
	rect.size.h = seq_bounds.h - 1;
	s_analog_layer = layer_create(rect);
	layer_set_update_proc(s_analog_layer, analog_layer_update);
	layer_add_child(layer, s_analog_layer);

	// Analog hands.
	rect.origin.y = 0;
	rect.origin.x = 0;
	rect = grect_inset(rect, GEdgeInsets(40));
	s_hands_layer = layer_create(rect);
	layer_set_update_proc(s_hands_layer, hands_layer_update);
	layer_add_child(s_analog_layer, s_hands_layer);

	// Unobstructed area (quick view).
	UnobstructedAreaHandlers uareah = { 0, unobstructed_change, 0 };
	unobstructed_area_service_subscribe(uareah, NULL);
	unobstructed_change(0, NULL);
}

static void
win_unload(Window *_win)
{
	layer_destroy(s_bg_layer);
	layer_destroy(s_text_layer);
	layer_destroy(s_analog_layer);
	layer_destroy(s_hands_layer);
	text_layer_destroy(s_time_layer);
	text_layer_destroy(s_date_layer);
}

int
main(void)
{
	// Initialize settings.
	s_time_24h = clock_is_24h_style();

	// Get fonts.
	s_time_font = fonts_get_system_font(FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM);
	s_date_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);

	// Setup PDC sequence animation image of analog watch.
	s_seqv = gdraw_command_sequence_create_with_resource(RESOURCE_ID_SEQ);
	s_seqc = gdraw_command_sequence_get_num_frames(s_seqv);
	s_seqi = 0;

	// Make main window.
	s_win = window_create();
	WindowHandlers win_h = { win_load, 0, 0, win_unload };
	window_set_window_handlers(s_win, win_h);
	window_stack_push(s_win, true);

	// Time and date.  Update time and date even before first tick
	// by calling onmin() tick handler manually.  Required because
	// first tick happen after first minute ends and we want to
	// show correct time even before that.
	time_t timestamp = time(NULL);
	onmin(localtime(&timestamp), DAY_UNIT);
	tick_timer_service_subscribe(MINUTE_UNIT, onmin);

	// Watchface event loop.
	app_event_loop();

	// Destroy resources.
	window_destroy(s_win);
	gdraw_command_sequence_destroy(s_seqv);

	return 0;
}
