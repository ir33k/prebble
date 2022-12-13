// Draw uniform dithering pattern.

#ifndef PATTERN_H
#define PATTERN_H

#include "math.h"
#include "fbuf.h"

static const uint8_t pattern__dither_map[8][8] = {
	{   0, 128,  32, 160,   8, 136,  40, 168 },
	{ 192,  64, 224,  96, 200,  72, 232, 104 },
	{  48, 176,  16, 144,  56, 184,  24, 152 },
	{ 240, 112, 208,  80, 248, 120, 216,  88 },
	{  12, 140,  44, 172,   4, 132,  36, 164 },
	{ 204,  76, 236, 108, 196,  68, 228, 100 },
	{  60, 188,  28, 156,  52, 180,  20, 148 },
	{ 252, 124, 220,  92, 244, 116, 212,  84 }
};

// Draw dithering pattern in CTX context RECT boundry in given COLOR.
// Density of dighering depends on VALUE (0 - 255).
static void
pattern_dither(GContext *ctx, GRect rect, GColor color, uint8_t value)
{
	if (value <= 0) {
		return;
	}
	if (value >= 255) {
		graphics_context_set_fill_color(ctx, color);
		graphics_fill_rect(ctx, rect, 0, GCornerNone);
		return;
	}

	struct fbuf_each_ctx fb = {0};

	while (fbuf_each(&fb, ctx, rect)) {
		if (value > pattern__dither_map[fb.y%8][fb.x%8]) {
			fbuf_set_pixel(fb.info, GPoint(fb.x, fb.y), color);
		}
	}
}

// Draw patter of 45 deg diagonal lines of given COLOR with CTX in
// RECT area.
static void
pattern_lines(GContext *ctx, GRect rect, GColor color)
{
	uint16_t gap = 28;
	uint16_t xoffset = 16;
	struct fbuf_each_ctx fb = {0};

	while (fbuf_each(&fb, ctx, rect)) {
		if ((fb.x - fb.y + xoffset) % gap == 0) {
			fbuf_set_pixel(fb.info, GPoint(fb.x, fb.y), color);
			fb.x += gap-1; // Skip to next valid X
		}
	}
}

// Draw patter of small dots of given COLOR with CTX in RECT area.
static void
pattern_dots(GContext *ctx, GRect rect, GColor color)
{
	int32_t x, y;
	uint16_t gap = rect.size.w / 7;

	for (x = rect.origin.x + gap/2; x < rect.size.w; x += gap)
	for (y = rect.origin.y + gap/2; y < rect.size.h; y += gap) {
		graphics_context_set_fill_color(ctx, color);
		graphics_fill_circle(ctx, GPoint(x, y), 1);
	}
}
#endif	/* PATTERN_H */
