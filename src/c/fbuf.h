// Functions for working with frame buffer.
// Source: https://developer.rebble.io/developer.pebble.com/guides/graphics-and-animations/framebuffer-graphics/index.html#getting-and-setting-pixels

#ifndef FBUF_H
#define FBUF_H

struct fbuf_each_ctx {          // fbuf_each() context
	GBitmap *fb;
	GBitmapDataRowInfo info;
	int16_t x, y;           // Current x,y pixel
};

// Set VALUE (0 or 1) in single BIT of BYTE.
// Only for BW displays.
#if defined(PBL_BW)
static void
fbuf__byte_set_bit(uint8_t *byte, uint8_t bit, uint8_t value)
{
	*byte ^= (-value ^ *byte) & (1 << bit);
}
#endif

// In given INFO Graphical Context set POINT pixel to COLOR.
static void
fbuf_set_pixel(GBitmapDataRowInfo info, GPoint point, GColor color)
{
#if defined(PBL_COLOR)
	memset(&info.data[point.x], color.argb, 1);
#elif defined(PBL_BW)
	uint8_t byte  = point.x / 8;
	uint8_t bit   = point.x % 8;
	uint8_t value = gcolor_equal(color, GColorWhite) ? 1 : 0;
	fbuf__byte_set_bit(&info.data[byte], bit, value);
#endif
}

// Iterate over each CTX pixel in BOUND boundary.  On each call FB
// context is updated with current fb.x fb.y pixel position and on
// each row fb.info gets updated.  Function return non 0 value as long
// as there are more pixels to get.  On last pixel frame buffer is
// released so it's important to call function untill it returns 0.
static int
fbuf_each(struct fbuf_each_ctx *fb, GContext *ctx, GRect bound)
{
	// Init.
	if (fb->fb == 0) {
		fb->fb   = graphics_capture_frame_buffer(ctx);
		fb->y    = bound.origin.y;
		fb->info = gbitmap_get_data_row_info(fb->fb, fb->y);
		fb->x    = MAX(fb->info.min_x, bound.origin.x) - 1;
	}
	// Update x,y on each call.
	fb->x += 1;
	if (fb->x > MIN(fb->info.max_x, bound.size.w)) {
		fb->y   += 1;
		fb->info = gbitmap_get_data_row_info(fb->fb, fb->y);
		fb->x    = MAX(fb->info.min_x, bound.origin.x);
	}
	// End.
	if (fb->y >= bound.size.h) {
		graphics_release_frame_buffer(ctx, fb->fb);
		fb->fb = 0;
		return 0;       // End here
	}
	return 1;               // Continue
}
#endif	/* FBUF_H */
