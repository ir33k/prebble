#include <pebble.h>

static Window      *s_win;      // Main window
static Layer       *s_bg_layer;	// Background

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

static void bg_update(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Draw the background
  graphics_context_set_fill_color(ctx, GColorRed);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  draw_pattern_lines(ctx, bounds, GColorBlack);
}

static void win_load(Window *win) {
  Layer *root_layer = window_get_root_layer(win);
  GRect root_bounds = layer_get_bounds(root_layer);

  // Background
  s_bg_layer = layer_create(root_bounds);
  layer_set_update_proc(s_bg_layer, bg_update);
  layer_add_child(root_layer, s_bg_layer);
}

static void win_unload(Window *win) {
  layer_destroy(s_bg_layer);
}

static void init(void) {
  s_win = window_create();
  WindowHandlers win_h = { win_load, 0, 0, win_unload };
  window_set_window_handlers(s_win, win_h);
  window_stack_push(s_win, true);
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
