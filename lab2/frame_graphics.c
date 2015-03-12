/*
 * frame_graphics.c
 *
 *  Created on: Aug 8, 2012
 *      Author: Gene Bogdanov
 */

#include "frame_graphics.h"

void FillFrame(unsigned level)
{
	unsigned long fill = level & 0xf;
	unsigned long *p_fill = (unsigned long *)g_pucFrame;
	int i;
	fill |= fill << 4;
	fill |= fill << 8;
	fill |= fill << 16;
	for (i = FRAME_SIZE_X * FRAME_SIZE_Y / 32; i; i--) {
		*(p_fill++) = fill;
		*(p_fill++) = fill;
		*(p_fill++) = fill;
		*(p_fill++) = fill;
	}
}

void DrawPoint(int x, int y, unsigned level)
{
	int index;
	if ((x >= 0) && (x < FRAME_SIZE_X) && (y >= 0) && (y < FRAME_SIZE_Y)) { // check coordinate validity
		index = y * (FRAME_SIZE_X / 2) + x / 2;
		if (x & 1) {
			g_pucFrame[index] =	(g_pucFrame[index] & 0xf0) | (level & 0xf);
		}
		else {
			g_pucFrame[index] =	(g_pucFrame[index] & 0x0f) | ((level & 0xf) << 4);
		}
	}
}

void DrawLine(int x1, int y1, int x2, int y2, unsigned level)
{
	int u, v, d1x, max, d1y, n, s, d2x, d2y, x, y, i;

	if (x2 < x1) {
		u = x2;
		x2 = x1;
		x1 = u;
		u = y2;
		y2 = y1;
		y1 = u;
	}
	u = x2 - x1;
	v = y2 - y1;
	x = x1;
	y = y1;
	if (u) {
		d1x = 1;
		max = u;
	}
	else {
		d1x = 0;
		max = 0;
	}
	if (v > 0) {
		d1y = 1;
		n = v;
	}
	else if (v < 0) {
		d1y = -1;
		n = -v;
	}
	else {
		d1y = 0;
		n = 0;
	}
	if (max < n) {
		/* swap */
		d2x = max;
		max = n;
		n = d2x;

		d2x = 0;
		d2y = d1y;
	}
	else {
		d2x = d1x;
		d2y = 0;
	}
	s = max >> 1;
	for (i = 0; i <= max; i++ ) {
		DrawPoint(x, y, level);
		s += n;
		if (s >= max)
		{
			s -= max;
			x += d1x;
			y += d1y;
		}
		else {
			x += d2x;
			y += d2y;
		}
	}
}

int DrawChar(int x, int y, char ascii, unsigned level, int proportional)
{
	const unsigned char *lookup = g_pucFont[ascii - 32]; //get bitmap for character
	const unsigned char *lookup_end = lookup + 5;
	unsigned temp;
	int blank = 0, nonblank = 0;  // counters
	int i;

	if (ascii < 32 || ascii > 126)
		return 0; // skip non-printing characters
	if (x >= 0) { // check coordinate validity
		for (; (x < FRAME_SIZE_X) && (lookup != lookup_end); lookup++) {
			temp = *lookup;
			if (!proportional || temp) { // not a blank line
				if (temp) {
					for (i = 0; i < 8; i++) { // draw the line
						if (temp & 1) DrawPoint(x, y + i, level);
						temp >>= 1;
					}
				}
				x++;
				nonblank++;
				blank = 0;
			}
			else {  // blank line
				if (nonblank) {
					nonblank++;
					blank++;
				}
			}
		}
	}
	return ((nonblank == 0) ? 3 : (nonblank - blank + 1)); // space is 3 pixels wide
}

int DrawString(int x, int y, const char *str, unsigned level, int proportional)
{
	int x0 = x;
	for (; (*str != '\0') && (x < FRAME_SIZE_X); str++) {
		x += DrawChar(x, y, *str, level, proportional);
	}
	return (x - x0);
}

static void CirclePlot(int x0, int y0, int x, int y, unsigned level)
{
	DrawPoint(x0 + x, y0 + y, level);
	DrawPoint(x0 + x, y0 - y, level);
	DrawPoint(x0 - x, y0 + y, level);
	DrawPoint(x0 - x, y0 - y, level);
	DrawPoint(x0 + y, y0 + x, level);
	DrawPoint(x0 + y, y0 - x, level);
	DrawPoint(x0 - y, y0 + x, level);
	DrawPoint(x0 - y, y0 - x, level);
}

void DrawCircle(int x0, int y0, int r, unsigned level)
{
	int x = 0;
	int y = r;
	int dx = y;

	while (y > x) {
		CirclePlot(x0, y0, x, y, level);
		dx = dx + (x << 1) + 1;
		if (((dx >= 0) && (dx >= (y << 1))) || ((dx < 0) && (-dx >= (y << 1)))) {
			dx = dx - (y << 1) + 1;
			y--;
		}
		x++;
	}
	if (x == y)
		CirclePlot(x0, y0, x, y, level);
}

void DrawFilledRectangle(int x1, int y1, int x2, int y2, unsigned level)
{
	unsigned long *p_fill;
	int i, j, temp;
	int x8start, x8count;
	unsigned long fill, fill_head, fill_tail, mask_head, mask_tail;
	static const unsigned long mask_right[8] = { 0x00000000,
			0x000000f0, 0x000000ff, 0x0000f0ff, 0x0000ffff,
			0x00f0ffff, 0x00ffffff, 0xf0ffffff
	};
	static const unsigned long mask_left[8] = { 0x00000000,
			0x0f000000, 0xff000000, 0xff0f0000, 0xffff0000,
			0xffff0f00, 0xffffff00, 0xffffff0f
	};

	if (x1 > x2) { //arrange the coordinates
		temp = x1; //swap values
		x1 = x2;
		x2 = temp;
	}

	if (x1 < 0) // make sure x-coordinates fall within the buffer
		x1 = 0;
	if (x2 >= FRAME_SIZE_X)
		x2 = FRAME_SIZE_X - 1;
	if (x1 > x2) return; // invalid rectangle

	if (y1 > y2) {
		temp = y1;
		y1 = y2;
		y2 = temp;
	}

	if (y1 < 0)
		y1 = 0;
	if (y2 >= FRAME_SIZE_Y)
		y2 = FRAME_SIZE_Y - 1;
	if (y1 > y2) return;

	fill = level & 0xf;
	fill |= fill << 4;
	fill |= fill << 8;
	fill |= fill << 16;

	x8start = ((x1 - 1) >> 3) + 1; 			// starting horizontal 8-pixel block
	x8count = ((x2 + 1) >> 3) - x8start; 	// number of 8-pixel blocks to fill horizontally
	mask_head = mask_left[(x8start << 3) - x1];
	mask_tail = mask_right[(x2 + 1) & 0x7];
	fill_head = fill & mask_head;
	fill_tail = fill & mask_tail;

	if (x8count < 0) { // rectangle fits in an 8-pixel block
		mask_head &= mask_tail;
		fill_head = fill & mask_head;
		p_fill = (unsigned long *)g_pucFrame + y1 * (FRAME_SIZE_X >> 3) + (x1 >> 3);
		for (j = y2 - y1 + 1; j > 0; j--) {
			*p_fill = (*p_fill & ~mask_head) | fill_head;
			p_fill += FRAME_SIZE_X >> 3;
		}
	}
	else if (x8count == 0) { // rectangle spans two incomplete 8-pixel blocks
		p_fill = (unsigned long *)g_pucFrame + y1 * (FRAME_SIZE_X >> 3) + x8start;
		for (j = y2 - y1 + 1; j > 0; j--) {
			if (mask_head)
				*(p_fill - 1) = (*(p_fill - 1) & ~mask_head) | fill_head;
			if (mask_tail)
				*p_fill = (*p_fill & ~mask_tail) | fill_tail;
			p_fill += FRAME_SIZE_X >> 3;
		}
	}
	else {	// rectangle contains full 8-pixel blocks
		for (j = y1; j <= y2; j++) {
			p_fill = (unsigned long *)g_pucFrame + j * (FRAME_SIZE_X >> 3) + x8start;
			if (mask_head)
				*(p_fill - 1) = (*(p_fill - 1) & ~mask_head) | fill_head;
			for (i = x8count; i > 0; i--) *(p_fill++) = fill;
			if (mask_tail)
				*p_fill = (*p_fill & ~mask_tail) | fill_tail;
		}
	}
}

// global variables
unsigned char g_pucFrame[(FRAME_SIZE_X / 2) * FRAME_SIZE_Y];
const unsigned char g_pucFont[95][5] =
{
    { 0x00, 0x00, 0x00, 0x00, 0x00 }, // " "
    { 0x00, 0x00, 0x4f, 0x00, 0x00 }, // !
    { 0x00, 0x07, 0x00, 0x07, 0x00 }, // "
    { 0x14, 0x7f, 0x14, 0x7f, 0x14 }, // #
    { 0x24, 0x2a, 0x7f, 0x2a, 0x12 }, // $
    { 0x23, 0x13, 0x08, 0x64, 0x62 }, // %
    { 0x36, 0x49, 0x55, 0x22, 0x50 }, // &
    { 0x00, 0x05, 0x03, 0x00, 0x00 }, // '
    { 0x00, 0x1c, 0x22, 0x41, 0x00 }, // (
    { 0x00, 0x41, 0x22, 0x1c, 0x00 }, // )
    { 0x14, 0x08, 0x3e, 0x08, 0x14 }, // *
    { 0x08, 0x08, 0x3e, 0x08, 0x08 }, // +
    { 0x00, 0x50, 0x30, 0x00, 0x00 }, // ,
    { 0x08, 0x08, 0x08, 0x08, 0x08 }, // -
    { 0x00, 0x60, 0x60, 0x00, 0x00 }, // .
    { 0x20, 0x10, 0x08, 0x04, 0x02 }, // /
    { 0x3e, 0x51, 0x49, 0x45, 0x3e }, // 0
    { 0x00, 0x42, 0x7f, 0x40, 0x00 }, // 1
    { 0x42, 0x61, 0x51, 0x49, 0x46 }, // 2
    { 0x21, 0x41, 0x45, 0x4b, 0x31 }, // 3
    { 0x18, 0x14, 0x12, 0x7f, 0x10 }, // 4
    { 0x27, 0x45, 0x45, 0x45, 0x39 }, // 5
    { 0x3c, 0x4a, 0x49, 0x49, 0x30 }, // 6
    { 0x01, 0x71, 0x09, 0x05, 0x03 }, // 7
    { 0x36, 0x49, 0x49, 0x49, 0x36 }, // 8
    { 0x06, 0x49, 0x49, 0x29, 0x1e }, // 9
    { 0x00, 0x36, 0x36, 0x00, 0x00 }, // :
    { 0x00, 0x56, 0x36, 0x00, 0x00 }, // ;
    { 0x08, 0x14, 0x22, 0x41, 0x00 }, // <
    { 0x14, 0x14, 0x14, 0x14, 0x14 }, // =
    { 0x00, 0x41, 0x22, 0x14, 0x08 }, // >
    { 0x02, 0x01, 0x51, 0x09, 0x06 }, // ?
    { 0x32, 0x49, 0x79, 0x41, 0x3e }, // @
    { 0x7e, 0x11, 0x11, 0x11, 0x7e }, // A
    { 0x7f, 0x49, 0x49, 0x49, 0x36 }, // B
    { 0x3e, 0x41, 0x41, 0x41, 0x22 }, // C
    { 0x7f, 0x41, 0x41, 0x22, 0x1c }, // D
    { 0x7f, 0x49, 0x49, 0x49, 0x41 }, // E
    { 0x7f, 0x09, 0x09, 0x09, 0x01 }, // F
    { 0x3e, 0x41, 0x49, 0x49, 0x7a }, // G
    { 0x7f, 0x08, 0x08, 0x08, 0x7f }, // H
    { 0x00, 0x41, 0x7f, 0x41, 0x00 }, // I
    { 0x20, 0x40, 0x41, 0x3f, 0x01 }, // J
    { 0x7f, 0x08, 0x14, 0x22, 0x41 }, // K
    { 0x7f, 0x40, 0x40, 0x40, 0x40 }, // L
    { 0x7f, 0x02, 0x0c, 0x02, 0x7f }, // M
    { 0x7f, 0x04, 0x08, 0x10, 0x7f }, // N
    { 0x3e, 0x41, 0x41, 0x41, 0x3e }, // O
    { 0x7f, 0x09, 0x09, 0x09, 0x06 }, // P
    { 0x3e, 0x41, 0x51, 0x21, 0x5e }, // Q
    { 0x7f, 0x09, 0x19, 0x29, 0x46 }, // R
    { 0x46, 0x49, 0x49, 0x49, 0x31 }, // S
    { 0x01, 0x01, 0x7f, 0x01, 0x01 }, // T
    { 0x3f, 0x40, 0x40, 0x40, 0x3f }, // U
    { 0x1f, 0x20, 0x40, 0x20, 0x1f }, // V
    { 0x3f, 0x40, 0x38, 0x40, 0x3f }, // W
    { 0x63, 0x14, 0x08, 0x14, 0x63 }, // X
    { 0x07, 0x08, 0x70, 0x08, 0x07 }, // Y
    { 0x61, 0x51, 0x49, 0x45, 0x43 }, // Z
    { 0x00, 0x7f, 0x41, 0x41, 0x00 }, // [
    { 0x02, 0x04, 0x08, 0x10, 0x20 }, // "\"
    { 0x00, 0x41, 0x41, 0x7f, 0x00 }, // ]
    { 0x04, 0x02, 0x01, 0x02, 0x04 }, // ^
    { 0x40, 0x40, 0x40, 0x40, 0x40 }, // _
    { 0x00, 0x01, 0x02, 0x04, 0x00 }, // `
    { 0x20, 0x54, 0x54, 0x54, 0x78 }, // a
    { 0x7f, 0x48, 0x44, 0x44, 0x38 }, // b
    { 0x38, 0x44, 0x44, 0x44, 0x20 }, // c
    { 0x38, 0x44, 0x44, 0x48, 0x7f }, // d
    { 0x38, 0x54, 0x54, 0x54, 0x18 }, // e
    { 0x08, 0x7e, 0x09, 0x01, 0x02 }, // f
    { 0x0c, 0x52, 0x52, 0x52, 0x3e }, // g
    { 0x7f, 0x08, 0x04, 0x04, 0x78 }, // h
    { 0x00, 0x44, 0x7d, 0x40, 0x00 }, // i
    { 0x20, 0x40, 0x44, 0x3d, 0x00 }, // j
    { 0x7f, 0x10, 0x28, 0x44, 0x00 }, // k
    { 0x00, 0x41, 0x7f, 0x40, 0x00 }, // l
    { 0x7c, 0x04, 0x18, 0x04, 0x78 }, // m
    { 0x7c, 0x08, 0x04, 0x04, 0x78 }, // n
    { 0x38, 0x44, 0x44, 0x44, 0x38 }, // o
    { 0x7c, 0x14, 0x14, 0x14, 0x08 }, // p
    { 0x08, 0x14, 0x14, 0x18, 0x7c }, // q
    { 0x7c, 0x08, 0x04, 0x04, 0x08 }, // r
    { 0x48, 0x54, 0x54, 0x54, 0x20 }, // s
    { 0x04, 0x3f, 0x44, 0x40, 0x20 }, // t
    { 0x3c, 0x40, 0x40, 0x20, 0x7c }, // u
    { 0x1c, 0x20, 0x40, 0x20, 0x1c }, // v
    { 0x3c, 0x40, 0x30, 0x40, 0x3c }, // w
    { 0x44, 0x28, 0x10, 0x28, 0x44 }, // x
    { 0x0c, 0x50, 0x50, 0x50, 0x3c }, // y
    { 0x44, 0x64, 0x54, 0x4c, 0x44 }, // z
    { 0x00, 0x08, 0x36, 0x41, 0x00 }, // {
    { 0x00, 0x00, 0x7f, 0x00, 0x00 }, // |
    { 0x00, 0x41, 0x36, 0x08, 0x00 }, // }
    { 0x02, 0x01, 0x02, 0x04, 0x02 }  // ~
};
