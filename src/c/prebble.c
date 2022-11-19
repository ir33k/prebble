#include <pebble.h>

static Window      *s_win;	// Main window
static BitmapLayer *s_bg_layer;	// Background
static GBitmap     *s_bg_bitmap;

static void win_load(Window *win) {
  Layer *root_layer = window_get_root_layer(win);
  GRect root_rect = layer_get_bounds(root_layer);

  s_bg_layer = bitmap_layer_create(root_rect);
  bitmap_layer_set_bitmap(s_bg_layer, s_bg_bitmap);
  layer_add_child(root_layer, bitmap_layer_get_layer(s_bg_layer));
}

static void win_unload(Window *win) {
  gbitmap_destroy(s_bg_bitmap);
}

static void init(void) {
  s_bg_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMG_WF);
  // Window
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
