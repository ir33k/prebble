#ifndef MATH_H
#define MATH_H

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Normalize V value with VMIN minimum possible value and VMAX maximum
// possible value to fit in range from MIN to MAX.
int
normal(int v, int vmin, int vmax, int min, int max)
{
	return ((float)(v-vmin) / (float)(vmax-vmin)) * (float)(max-min) + min;
}
#endif	/* MATH_H */
