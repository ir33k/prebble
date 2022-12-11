#include <pebble.h>

// TODO(irek): Add desciptions of chosen options in clay settings.
// TODO(irek): New gif and images in store.

#define CONF_KEY    1           // Clay config persist storage key

enum bg {                       // Possible background types
	BG_NUL = 0,             // No background, results in white
	BG_PLAIN,               // Use plain background color
	BG_BATT                 // Color depends on battery level
};
enum fg {                       // Background pattern AKA foreground
	FG_NUL = 0,             // No foreground pattern
	FG_LINES,               // Use lines pattern
	FG_DOTS                 // Use dots pattern
};
enum vibe {                     // Possible vibrations
	VIBE_NUL = 0,           // No vibrations
	VIBE_SHORT,             // Short pulse
	VIBE_LONG,              // Long pulse
	VIBE_DOUBLE             // Double pulse
};
struct clay {                   // Struct for all Clay settings
	enum bg     bg_type;    // Background type
	GColor      bg_color;   // Background color
	enum fg     fg_type;    // Foregroud AKA background pattern
	GColor      fg_color;   // Foreground color
	bool        fg_bt;      // Hide pattern on BT disconnect
	enum vibe   vibe_bt;    // Vibration on BT disconnect
	enum vibe   vibe_h;     // Vibration on each hour
	char        date[16];   // Date format
};

static bool         s_24h;      // True when 24h, false when 12h
static Layer       *s_bg;       // Background for color & pattern
static Layer       *s_text;     // Background of time & date
static TextLayer   *s_time;     // Digital time
static TextLayer   *s_date;     // Date below digital time
static GFont        s_font[2];  // Time and date fonts
static Layer       *s_analog;   // Analog sequence animation
static GDrawCommandSequence *s_seqv; // Sequence value, image frames
static uint32_t     s_seqc;     // Seq frames count
static uint32_t     s_seqi;     // Seq animation index
static Layer       *s_hands;    // Analog watch hands
static struct clay  s_conf;     // Configuration AKA Clay settings

// Vibrate using one of predefined patterns TYPE.
static void
vibe(enum vibe type)
{
	switch (type) {
	case VIBE_SHORT:  vibes_short_pulse();  break;
	case VIBE_LONG:   vibes_long_pulse();   break;
	case VIBE_DOUBLE: vibes_double_pulse(); break;
	case VIBE_NUL:    break;
	}
}

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
	uint16_t gap = 28;
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

// Draw patter of small dots of given COLOR with CTX in RECT area.
static void
draw_pattern_dots(GContext *ctx, GRect rect, GColor color)
{
	int32_t x, y;
	uint16_t gap = rect.size.w / 7;

	for (x = rect.origin.x + gap/2; x < rect.size.w; x += gap)
	for (y = rect.origin.y + gap/2; y < rect.size.h; y += gap) {
		graphics_context_set_fill_color(ctx, color);
		graphics_fill_circle(ctx, GPoint(x, y), 1);
	}
}

static void
text_update(Layer *layer, GContext *ctx)
{
	GRect bounds = layer_get_bounds(layer);

#ifdef PBL_RECT
	// In case of white background we want to have black outline
	// around text layer to separate it from background.
	if (gcolor_equal(s_conf.bg_color, GColorWhite)) {
		graphics_context_set_fill_color(ctx, GColorBlack);
		graphics_fill_rect(ctx, bounds, 6,
				   GCornerTopLeft | GCornerTopRight);
		bounds.origin.y += 1;
	}
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, bounds, 6, GCornerTopLeft | GCornerTopRight);
#else
	GPoint center = GPoint(bounds.origin.x + bounds.size.w/2,
			       bounds.origin.y + bounds.size.h);
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_circle(ctx, center, bounds.size.h-1);
	// In case of white background we want to have black outline
	// around text layer to separate it from background.
	if (gcolor_equal(s_conf.bg_color, GColorWhite)) {
		graphics_context_set_stroke_color(ctx, GColorBlack);
		graphics_context_set_stroke_width(ctx, 1);
		graphics_draw_circle(ctx, center, bounds.size.h);
	}
#endif
}

// Start s_analog sequence animation.
static void
seq_anim(void *_ctx)
{
	// Advance to next frame if possible, else end animation.
	if (++s_seqi < s_seqc) {
		app_timer_register(12, seq_anim, NULL);
	} else {
		s_seqi = 0;
	}
	layer_mark_dirty(s_analog);
}

static void
analog_update(Layer *_l, GContext *ctx)
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
hands_update(Layer *layer, GContext *ctx)
{
	int32_t angle;
	GRect inset, bounds = layer_get_bounds(layer);
	GPoint point, mid = grect_center_point(&bounds);
	time_t timestamp = time(NULL);
	struct tm *time = localtime(&timestamp);

	graphics_context_set_antialiased(ctx, true);
	// Minute hand.
	inset = grect_inset(bounds, GEdgeInsets(4));
	angle = DEG_TO_TRIGANGLE(360*time->tm_min/60);
	point = gpoint_from_polar(inset, GOvalScaleModeFillCircle, angle);
	// Draw white border/outline around minute hand.
	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_context_set_stroke_width(ctx, 6);
	graphics_draw_line(ctx, mid, point);
	graphics_fill_circle(ctx, point, 3);
	// I only work in black, and sometime very very dark gray.
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_context_set_fill_color(ctx, GColorBlack);
	// Draw actuall black minute hand.
	graphics_context_set_stroke_width(ctx, 4);
	graphics_draw_line(ctx, mid, point);
	graphics_fill_circle(ctx, point, 2);
	// Hour hand.
	inset = grect_inset(bounds, GEdgeInsets(16));
	angle = DEG_TO_TRIGANGLE(360*(time->tm_hour%12)/12) +
		DEG_TO_TRIGANGLE(360/12*time->tm_min/60);
	point = gpoint_from_polar(inset, GOvalScaleModeFitCircle, angle);
	graphics_draw_line(ctx, mid, point);
	graphics_fill_circle(ctx, point, 2);
	// Middle circle.
	graphics_fill_circle(ctx, mid, 2);
}

static void
bg_update(Layer *layer, GContext *ctx)
{
	GRect bounds = layer_get_bounds(layer);

	graphics_context_set_fill_color(ctx, s_conf.bg_color);
	graphics_fill_rect(ctx, bounds, 0, GCornerNone);
	if (!s_conf.fg_bt || connection_service_peek_pebble_app_connection()) {
		switch (s_conf.fg_type) {
		case FG_LINES:
			draw_pattern_lines(ctx, bounds, s_conf.fg_color);
			break;
		case FG_DOTS:
			draw_pattern_dots(ctx, bounds, s_conf.fg_color);
			break;
		case FG_NUL:
			break;
		}
	}
}

static void
time_set(struct tm *time)
{
	static char buf[16];
	strftime(buf, sizeof(buf), s_24h ? "%H:%M" : "%I:%M %p", time);
	text_layer_set_text(s_time, buf);
}

static void
date_set(struct tm *time)
{
	static char buf[16];
	strftime(buf, sizeof(buf), s_conf.date, time);
	text_layer_set_text(s_date, buf);
}

static void
unobstructed_change(AnimationProgress _p, void *win)
{
	Layer *layer  = window_get_root_layer((Window *) win);
	GRect  bounds = layer_get_bounds(layer);
	GRect ubounds = layer_get_unobstructed_bounds(layer);

	// TODO(irek): Do it with animations.
	layer_set_hidden(s_text, !grect_equal(&bounds, &ubounds));
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

static void
win_load(Window *win)
{
	Layer *layer = window_get_root_layer(win);
	GRect rect, bounds = layer_get_bounds(layer);

	// Settings that might change so better update.
	s_24h = clock_is_24h_style();

	// Main background.
	rect = bounds;
	s_bg = layer_create(rect);
	layer_set_update_proc(s_bg, bg_update);
	layer_add_child(layer, s_bg);

	// Text background for time and date.
	rect.origin.y += rect.size.h;
	rect.size.h = rect.size.h/2 + PBL_IF_ROUND_ELSE(8, 0);
	s_text = layer_create(rect);
	layer_set_update_proc(s_text, text_update);
	layer_add_child(layer, s_text);
	anim_slideup(s_text, rect, 20, 400);

	// Time.
	rect.origin.x = 0;
	rect.origin.y = PBL_IF_ROUND_ELSE(26, 18);
	rect.size.h = 32;
	s_time = text_layer_create(rect);
	text_layer_set_background_color(s_time, GColorClear);
	text_layer_set_text_color(s_time, GColorBlack);
	text_layer_set_font(s_time, s_font[0]);
	text_layer_set_text_alignment(s_time, GTextAlignmentCenter);
	layer_add_child(s_text, text_layer_get_layer(s_time));

	// Date.
	rect.origin.y += rect.size.h - 4;
	rect.size.h = 22;
	s_date = text_layer_create(rect);
	text_layer_set_background_color(s_date, GColorClear);
	text_layer_set_text_color(s_date, GColorBlack);
	text_layer_set_font(s_date, s_font[1]);
	text_layer_set_text_alignment(s_date, GTextAlignmentCenter);
	layer_add_child(s_text, text_layer_get_layer(s_date));

	// Analog animated sequence image.
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
	s_analog = layer_create(rect);
	layer_set_update_proc(s_analog, analog_update);
	layer_add_child(layer, s_analog);

	// Analog hands.
	rect.origin.y = 0;
	rect.origin.x = 0;
	rect = grect_inset(rect, GEdgeInsets(38));
	s_hands = layer_create(rect);
	layer_set_update_proc(s_hands, hands_update);
	layer_add_child(s_analog, s_hands);

	// Unobstructed area (quick view).
	UnobstructedAreaHandlers uareah = { 0, unobstructed_change, 0 };
	unobstructed_area_service_subscribe(uareah, win);
	unobstructed_change(0, win);
}

static void
win_unload(Window *_win)
{
	layer_destroy(s_bg);
	layer_destroy(s_text);
	layer_destroy(s_analog);
	layer_destroy(s_hands);
	text_layer_destroy(s_time);
	text_layer_destroy(s_date);
}

// Handle Bluetooth state change.
static void
bluetooth(bool _connected)
{
	layer_mark_dirty(s_bg);
	vibe(s_conf.vibe_bt);
}

// Handle battery state change.  Used when s_conf.bg_type is set to
// BG_BAT to define backgorund color based on battery charge status.
static void
battery(BatteryChargeState state)
{
	// TODO(irek): Add more colors along with colors desciption on
	// settings page.
#ifdef PBL_BW
	if (state.charge_percent > 30) {
		s_conf.bg_color = GColorLightGray;
	} else {
		s_conf.bg_color = GColorWhite;
	}
#else
	if (state.charge_percent > 60) {
		s_conf.bg_color = GColorIslamicGreen;
	} else if (state.charge_percent > 30) {
		s_conf.bg_color = GColorCobaltBlue;
	} else {
		s_conf.bg_color = GColorRed;
	}
#endif
	layer_mark_dirty(s_bg);
	// s_text layer reacts to white background by adding black
	// border to separate itself from background.
	layer_mark_dirty(s_text);
}


// TickHandler, runs on each minute.
static void
onmin(struct tm *time, TimeUnits change)
{
	if (change & MINUTE_UNIT) {
		seq_anim(NULL);
	}
	if (change & HOUR_UNIT) {
		vibe(s_conf.vibe_h);
	}
	if (change & DAY_UNIT) {
		date_set(time);
	}
	time_set(time);
	layer_mark_dirty(s_hands);
}

static void
conf_load(struct clay *conf)
{
	// Apply defaults first then load old values if possible.
	conf->bg_type  = BG_PLAIN;
	conf->bg_color = PBL_IF_BW_ELSE(GColorLightGray, GColorRed);
	conf->fg_type  = FG_LINES;
	conf->fg_color = GColorBlack;
	conf->fg_bt    = true;
	conf->vibe_bt  = VIBE_NUL;
	conf->vibe_h   = VIBE_NUL;
	conf->date[0]  = 0;     // Empty string force default format
	persist_read_data(CONF_KEY, conf, sizeof(struct clay));
}

static void
conf_save(struct clay *conf)
{
	persist_write_data(CONF_KEY, conf, sizeof(struct clay));
}

static void
conf_apply(struct clay *conf)
{
	// Set background color.
	switch (conf->bg_type) {
	case BG_PLAIN:
		break;
	case BG_BATT:
		battery_state_service_subscribe(battery);
		battery(battery_state_service_peek());
		break;
	case BG_NUL:
		conf->bg_color = GColorWhite;
		break;
	}
	if (conf->bg_type != BG_BATT) {
		battery_state_service_unsubscribe();
	}

	// React to Bluetooth connection changes only if necessary.
	if (conf->fg_bt || conf->vibe_bt) {
		connection_service_subscribe((ConnectionHandlers) { bluetooth, 0 });
	} else {
		connection_service_unsubscribe();
	}

	// Use default date format when custom format is empty.
	if (conf->date[0] == 0) {
		strcpy(conf->date, PBL_IF_ROUND_ELSE("%a %m.%d", "%A %m.%d"));
	}

	// Redraw layers.
	layer_mark_dirty(s_bg);
	layer_mark_dirty(s_text);

	// Force date update.
	time_t timestamp = time(NULL);
	onmin(localtime(&timestamp), DAY_UNIT);
}

static void
conf_onmsg(DictionaryIterator *di, void *ctx)
{
	struct clay *conf = (struct clay *) ctx;
	Tuple *tuple;

	if ((tuple = dict_find(di, MESSAGE_KEY_BGTYPE))) {
		conf->bg_type = atoi(tuple->value->cstring);
	}
	if ((tuple = dict_find(di, MESSAGE_KEY_BGCOLOR))) {
		conf->bg_color = GColorFromHEX(tuple->value->int32);
	}
	if ((tuple = dict_find(di, MESSAGE_KEY_FGTYPE))) {
		conf->fg_type = atoi(tuple->value->cstring);
	}
	if ((tuple = dict_find(di, MESSAGE_KEY_FGCOLOR))) {
		conf->fg_color = GColorFromHEX(tuple->value->int32);
	}
	if ((tuple = dict_find(di, MESSAGE_KEY_FGBT))) {
		conf->fg_bt = tuple->value->int8;
	}
	if ((tuple = dict_find(di, MESSAGE_KEY_VIBEBT))) {
		conf->vibe_bt = atoi(tuple->value->cstring);
	}
	if ((tuple = dict_find(di, MESSAGE_KEY_VIBEH))) {
		conf->vibe_h = atoi(tuple->value->cstring);
	}
	if ((tuple = dict_find(di, MESSAGE_KEY_DATE)) &&
	    strlen(tuple->value->cstring)) {
		strcpy(conf->date, tuple->value->cstring);
	}
	conf_save(conf);
	conf_apply(conf);
}

int
main(void)
{
	// Initialize settings.
	s_24h = clock_is_24h_style();

	// Get fonts.
	s_font[0] = fonts_get_system_font(FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM);
	s_font[1] = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);

	// Setup PDC sequence animation image of analog watch.
	s_seqv = gdraw_command_sequence_create_with_resource(RESOURCE_ID_SEQ);
	s_seqc = gdraw_command_sequence_get_num_frames(s_seqv);
	s_seqi = 0;

	// Create main window.
	Window *win = window_create();
	WindowHandlers win_h = { win_load, 0, 0, win_unload };
	window_set_window_handlers(win, win_h);
	window_stack_push(win, true);

	// Time and date.  Update time and date even before first tick
	// by calling onmin() tick handler manually.  Required because
	// first tick happen after first minute ends and we want to
	// show correct time even before that.
	time_t timestamp = time(NULL);
	onmin(localtime(&timestamp), DAY_UNIT);
	tick_timer_service_subscribe(MINUTE_UNIT, onmin);

	// Conf (watchface settings page) init and setup.
	conf_load(&s_conf);
	conf_apply(&s_conf);
	app_message_set_context(&s_conf);
	app_message_register_inbox_received(conf_onmsg);
	app_message_open(dict_calc_buffer_size(8, 16), 0);

	// Watchface event loop.
	app_event_loop();

	// Destroy resources.
	window_destroy(win);
	gdraw_command_sequence_destroy(s_seqv);

	return 0;
}
