#include <pebble.h>

static int          s_is24h;           // None 0 when 24h enabled
static Window      *s_win;             // Main window
static Layer       *s_main_bg_layer;   // Color background
static Layer       *s_text_bg_layer;   // Time and date background
static TextLayer   *s_text_time_layer; // Time
static GFont        s_text_time_font;  // 
static TextLayer   *s_text_date_layer; // Date
static GFont        s_text_date_font;  // 

int abs(int x) {
  return x < 0 ? x*-1 : x;
}

static void draw_pattern_lines(GContext *ctx, GRect bounds, GColor color) {
  int16_t y;
  int16_t beg = bounds.origin.y - bounds.size.w;
  int16_t end = bounds.origin.y + bounds.size.h;
  int16_t step = abs(bounds.size.w - bounds.size.h);

  graphics_context_set_antialiased(ctx, false);
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_stroke_width(ctx, 1);

  for (y = beg; y < end; y += step) {
    graphics_draw_line(ctx,
                       GPoint(bounds.origin.x, y),
                       GPoint(bounds.origin.x + bounds.size.w,
                              y + bounds.size.w));
  }
}

static void main_bg_layer_update(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  graphics_context_set_fill_color(ctx, GColorRed);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  draw_pattern_lines(ctx, bounds, GColorBlack);
}

static void text_bg_layer_update(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
}

static void time_set(struct tm *time) {
  static char buf[8];
  strftime(buf, sizeof(buf), s_is24h ? "%H:%M" : "%I:%M", time);
  text_layer_set_text(s_text_time_layer, buf);
}

static void date_set(struct tm *time) {
  static char buf[16];
  // https://sourceware.org/newlib/libc.html#strftime
  strftime(buf, sizeof(buf), "%B %e", time);
  text_layer_set_text(s_text_date_layer, buf);
}

// TickHandler that runs after each minute.
static void on_min(struct tm *time, TimeUnits change) {
  time_set(time);
  if (change & DAY_UNIT) {
    date_set(time);
  }
}

static void win_load(Window *win) {
  Layer *layer = window_get_root_layer(win);
  GRect rect = layer_get_bounds(layer);

  // Settings
  s_is24h = clock_is_24h_style(); // Might change, better update
  // Main background
  s_main_bg_layer = layer_create(rect);
  layer_set_update_proc(s_main_bg_layer, main_bg_layer_update);
  layer_add_child(layer, s_main_bg_layer);
  // Text background for time and date
  GRect text_rect = GRect(rect.origin.x,
                          rect.origin.y + rect.size.h/2,
                          rect.size.w,
                          rect.size.h/2);
  s_text_bg_layer = layer_create(text_rect);
  layer_set_update_proc(s_text_bg_layer, text_bg_layer_update);
  layer_add_child(layer, s_text_bg_layer);
  // Time
  GRect time_rect = GRect(text_rect.origin.x,
                          text_rect.origin.y + 16,
                          text_rect.size.w,
                          32);
  s_text_time_layer = text_layer_create(time_rect);
  text_layer_set_background_color(s_text_time_layer, GColorClear);
  text_layer_set_text_color(s_text_time_layer, GColorBlack);
  text_layer_set_font(s_text_time_layer, s_text_time_font);
  text_layer_set_text_alignment(s_text_time_layer, GTextAlignmentCenter);
  layer_add_child(layer, text_layer_get_layer(s_text_time_layer));
  // Date
  GRect date_rect = GRect(time_rect.origin.x,
                          time_rect.origin.y + time_rect.size.h,
                          time_rect.size.w,
                          18);
  s_text_date_layer = text_layer_create(date_rect);
  text_layer_set_background_color(s_text_date_layer, GColorClear);
  text_layer_set_text_color(s_text_date_layer, GColorBlack);
  text_layer_set_font(s_text_date_layer,  s_text_date_font);
  text_layer_set_text_alignment(s_text_date_layer, GTextAlignmentCenter);
  layer_add_child(layer, text_layer_get_layer(s_text_date_layer));
}

static void win_unload(Window *win) {
  layer_destroy(s_main_bg_layer);
  layer_destroy(s_text_bg_layer);
  text_layer_destroy(s_text_time_layer);
  text_layer_destroy(s_text_date_layer);
}

static void init(void) {
  // Settings
  s_is24h = clock_is_24h_style();
  // Fonts
  s_text_time_font = fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS);
  s_text_date_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  // Window
  s_win = window_create();
  WindowHandlers win_h = { win_load, 0, 0, win_unload };
  window_set_window_handlers(s_win, win_h);
  window_stack_push(s_win, true);
  // Time and date.  Update time and date even before first tick by
  // calling onmin() tick handler manually.  Required because first
  // tick happen after first minute ends and we want to show correct
  // time from the start.
  time_t timestamp = time(NULL);
  on_min(localtime(&timestamp), MINUTE_UNIT | DAY_UNIT);
  tick_timer_service_subscribe(MINUTE_UNIT, on_min);
}

static void deinit(void) {
  window_destroy(s_win);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
  return 0;
}
