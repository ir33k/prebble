#include <pebble.h>

static bool        s_is24h;		   /* True when 24h */
static Window     *s_win;		   /* Main window */
static Layer      *s_bg_main_layer;	   /* Color background */
static Layer      *s_bg_text_layer;	   /* Time and date BG */
static TextLayer  *s_time_layer;	   /* Digital time */
static GFont       s_time_font;		   /* Digital time font */
static TextLayer  *s_date_layer;	   /* Date */
static GFont       s_date_font;		   /* Date font */
static Layer      *s_analog_layer;	   /* Analog watch image */
static GDrawCommandImage    *s_analog_img; /* Static image */
static GDrawCommandSequence *s_analog_seq; /* Animated image */
static uint32_t    s_analog_num;	   /* Seq frames count */
static uint32_t    s_index;		   /* Seq animation index*/
static Layer      *s_hands_layer;	   /* Analog watch hands */

/* Get absolute value of X. */
int
abs(int x)
{
	return x < 0 ? x*-1 : x;
}

static void
next_frame_handler(void *ctx)
{
	layer_mark_dirty(s_analog_layer);
	if (s_index++ < s_analog_num) {
		app_timer_register(12, next_frame_handler, NULL);
	}
}

static void
draw_pattern_lines(GContext *ctx, GRect bounds, GColor color)
{
	int16_t y;
	int16_t x = bounds.origin.x;
	int16_t step = 24;
	int16_t beg = bounds.origin.y - step * (bounds.size.w / step);
	int16_t end = bounds.origin.y + bounds.size.h;

	graphics_context_set_antialiased(ctx, false);
	graphics_context_set_stroke_color(ctx, color);
	graphics_context_set_stroke_width(ctx, 1);

	for (y = beg; y < end; y += step) {
		graphics_draw_line(ctx,
				   GPoint(x, y),
				   GPoint(x + bounds.size.w,
					  y + bounds.size.w));
	}
}

static void
bg_text_layer_update(Layer *layer, GContext *ctx)
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

static void
analog_layer_update(Layer *layer, GContext *ctx)
{
	GDrawCommandFrame *frame = gdraw_command_sequence_get_frame_by_index(s_analog_seq, s_index);

	if (frame) {
		gdraw_command_frame_draw(ctx, s_analog_seq, frame, GPoint(0, 0));
	} else {
		graphics_context_set_antialiased(ctx, false);
		gdraw_command_image_draw(ctx, s_analog_img, GPoint(0, 0));
	}
}

static void
hands_layer_update(Layer *layer, GContext *ctx)
{
	int32_t angle;
	GRect inset;
	GPoint mid, point;
	GRect bounds = layer_get_bounds(layer);
	time_t timestamp = time(NULL);
	struct tm *time = localtime(&timestamp);  

	graphics_context_set_antialiased(ctx, true);

	/* Middle circle. */
	mid = grect_center_point(&bounds);
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_circle(ctx, mid, 2);

	/* Minute hand. */
	inset = grect_inset(bounds, GEdgeInsets(2));
	angle = DEG_TO_TRIGANGLE(360*time->tm_min/60);
	point = gpoint_from_polar(inset, GOvalScaleModeFillCircle, angle);
	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_context_set_stroke_width(ctx, 6);
	graphics_draw_line(ctx, mid, point);
	graphics_fill_circle(ctx, point, 2);
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_context_set_stroke_width(ctx, 4);
	graphics_draw_line(ctx, mid, point);
	graphics_fill_circle(ctx, point, 2);

	/* Hour hand. */
	inset = grect_inset(bounds, GEdgeInsets(14));
	angle = DEG_TO_TRIGANGLE(360*(time->tm_hour%12)/12) +
		DEG_TO_TRIGANGLE(360/12*time->tm_min/60);
	point = gpoint_from_polar(inset, GOvalScaleModeFitCircle, angle);
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_context_set_stroke_width(ctx, 4);
	graphics_draw_line(ctx, mid, point);
	graphics_fill_circle(ctx, point, 2);
}

static void
bg_main_layer_update(Layer *layer, GContext *ctx)
{
	GRect bounds = layer_get_bounds(layer);

	graphics_context_set_fill_color(ctx, PBL_IF_BW_ELSE(GColorLightGray, GColorRed));
	graphics_fill_rect(ctx, bounds, 0, GCornerNone);
	draw_pattern_lines(ctx, bounds, GColorBlack);
}

/* Run analog clock sequence animation with initial DELAY in ms. */
void
analog_anim_run(uint32_t delay)
{
	s_index = 0;
	app_timer_register(delay, next_frame_handler, NULL);
}

static void
time_set(struct tm *time)
{
	static char buf[16];
	/* https://sourceware.org/newlib/libc.html#strftime */
	strftime(buf, sizeof(buf), s_is24h ? "%H:%M" : "%I:%M %p", time);
	text_layer_set_text(s_time_layer, buf);
}

static void
date_set(struct tm *time)
{
	static char buf[16];
	/* https://sourceware.org/newlib/libc.html#strftime */
	strftime(buf, sizeof(buf), PBL_IF_ROUND_ELSE("%a %m.%d", "%A %m.%d"), time);
	text_layer_set_text(s_date_layer, buf);
}

/* TickHandler that runs after each minute. */
static void
on_min(struct tm *time, TimeUnits change)
{
	if (change & DAY_UNIT) {
		date_set(time);
	}
	if (change & MINUTE_UNIT) {
		analog_anim_run(0);
	}
	time_set(time);
	layer_mark_dirty(s_hands_layer);
}

static void
unobstructed_change(AnimationProgress progress, void *ctx)
{
	struct Layer *layer = window_get_root_layer(s_win);
	GRect  bounds = layer_get_bounds(layer);
	GRect ubounds = layer_get_unobstructed_bounds(layer);
	bool obstructed = !grect_equal(&bounds, &ubounds);

	layer_set_hidden(s_bg_text_layer, obstructed);
}

static void
win_load(Window *win)
{
	Layer *layer  = window_get_root_layer(win);
	GRect  bounds = layer_get_bounds(layer);

	/* Settings that might change so better update. */
	s_is24h = clock_is_24h_style();

	/* Unobstructed area (quick view). */
	UnobstructedAreaHandlers uareah = { 0, unobstructed_change, 0 };
	unobstructed_area_service_subscribe(uareah, NULL);

	/* Main background. */
	GRect main_rect = GRect(bounds.origin.x,
				bounds.origin.y,
				bounds.size.w,
				bounds.size.h);
	s_bg_main_layer = layer_create(main_rect);
	layer_set_update_proc(s_bg_main_layer, bg_main_layer_update);
	layer_add_child(layer, s_bg_main_layer);

	/* Text background for time and date. */
	GRect text_rect_end = GRect(bounds.origin.x,
				    bounds.origin.y + bounds.size.h/2,
				    bounds.size.w,
				    bounds.size.h/2);
	GRect text_rect_beg = GRect(text_rect_end.origin.x,
				    bounds.origin.y + bounds.size.h,
				    text_rect_end.size.w,
				    text_rect_end.size.h);
	s_bg_text_layer = layer_create(text_rect_beg);
	layer_set_update_proc(s_bg_text_layer, bg_text_layer_update);
	layer_add_child(layer, s_bg_text_layer);
	/* Animation. */
	PropertyAnimation *text_anim_prop = property_animation_create_layer_frame(s_bg_text_layer, &text_rect_beg, &text_rect_end);
	Animation *text_anim = property_animation_get_animation(text_anim_prop);
	animation_set_curve(text_anim, AnimationCurveDefault);
	animation_set_delay(text_anim, 20);
	animation_set_duration(text_anim, 400);
	animation_schedule(text_anim);
	/* Time. */
	GRect time_rect = GRect(0, 18, text_rect_end.size.w, 32);
	s_time_layer = text_layer_create(time_rect);
	text_layer_set_background_color(s_time_layer, GColorClear);
	text_layer_set_text_color(s_time_layer, GColorBlack);
	text_layer_set_font(s_time_layer, s_time_font);
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	layer_add_child(s_bg_text_layer, text_layer_get_layer(s_time_layer));
	/* Date. */
	GRect date_rect = GRect(time_rect.origin.x,
				time_rect.origin.y + time_rect.size.h - 4,
				time_rect.size.w,
				22);
	s_date_layer = text_layer_create(date_rect);
	text_layer_set_background_color(s_date_layer, GColorClear);
	text_layer_set_text_color(s_date_layer, GColorBlack);
	text_layer_set_font(s_date_layer,  s_date_font);
	text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
	layer_add_child(s_bg_text_layer, text_layer_get_layer(s_date_layer));

	/* Analog image. */
	GSize analog_bounds = gdraw_command_image_get_bounds_size(s_analog_img);
	/* -1 in width and height is necessary to have size that have
         * center pixel.  When analog_rect is used to create layer for
         * analog hands it will pass size that can have that middle
         * pixel making gpoint_from_polar return points that produce
         * straight lines. */
	GRect analog_rect = GRect(bounds.origin.x + (bounds.size.w - analog_bounds.w)/2,
				  bounds.origin.y - PBL_IF_ROUND_ELSE(8, 12),
				  analog_bounds.w - 1,
				  analog_bounds.h - 1);
	s_analog_layer = layer_create(analog_rect);
	layer_set_update_proc(s_analog_layer, analog_layer_update);
	layer_add_child(layer, s_analog_layer);
	/* Analog hands. */
	analog_rect.origin.y = 0;
	analog_rect.origin.x = 0;
	GRect hands_rect = grect_inset(analog_rect, GEdgeInsets(40));
	s_hands_layer = layer_create(hands_rect);
	layer_set_update_proc(s_hands_layer, hands_layer_update);
	layer_add_child(s_analog_layer, s_hands_layer);

	/* Force to run logic that handles unobstructed area. */
	unobstructed_change(0, NULL);
}

static void
win_unload(Window *win)
{
	layer_destroy(s_bg_main_layer);
	layer_destroy(s_bg_text_layer);
	layer_destroy(s_analog_layer);
	layer_destroy(s_hands_layer);
	text_layer_destroy(s_time_layer);
	text_layer_destroy(s_date_layer);
}

static void
init(void)
{
	/* Settings. */
	s_is24h = clock_is_24h_style();

	/* Fonts. */
	s_time_font = fonts_get_system_font(FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM);
	s_date_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);

	/* PDC images. */
	s_analog_img = gdraw_command_image_create_with_resource(RESOURCE_ID_ANALOG);
	s_analog_seq = gdraw_command_sequence_create_with_resource(RESOURCE_ID_ANALOG_SEQ);
	s_analog_num = gdraw_command_sequence_get_num_frames(s_analog_seq);

	/* Window. */
	s_win = window_create();
	WindowHandlers win_h = { win_load, 0, 0, win_unload };
	window_set_window_handlers(s_win, win_h);
	window_stack_push(s_win, true);

	/* Time and date.  Update time and date even before first tick
	 * by calling onmin() tick handler manually.  Required because
	 * first tick happen after first minute ends and we want to
	 * show correct time from the start. */
	time_t timestamp = time(NULL);
	on_min(localtime(&timestamp), MINUTE_UNIT | DAY_UNIT);
	tick_timer_service_subscribe(MINUTE_UNIT, on_min);
}

static void
deinit(void)
{
	window_destroy(s_win);
	gdraw_command_image_destroy(s_analog_img);
	gdraw_command_sequence_destroy(s_analog_seq);
}

int
main(void)
{
	init();
	app_event_loop();
	deinit();
	return 0;
}
