/*
 * frame_graphics.h
 *
 *  Created on: Aug 8, 2012
 *      Author: Gene Bogdanov
 */

#ifndef FRAME_GRAPHICS_H_
#define FRAME_GRAPHICS_H_

// local frame buffer dimensions in pixels
#define FRAME_SIZE_X 128
#define FRAME_SIZE_Y 96

// local frame buffer
// copy this to the OLED after drawing the complete frame
extern unsigned char g_pucFrame[(FRAME_SIZE_X / 2) * FRAME_SIZE_Y];

// 5x8 ASCII font bitmaps (starting with " ")
extern const unsigned char g_pucFont[95][5];

// in all functions level = 0...15 specifies pixel brightness

// fill the entire frame with the specified pixel brightness
void FillFrame(unsigned level);

// draw point at x,y in the frame
// coordinates checked for validity, and nothing is done if out of range
void DrawPoint(int x, int y, unsigned level);

// draw line from (x1,y1) to (x2,y2)
// coordinates may lie outside the frame; only the visible portion is drawn
void DrawLine(int x1, int y1, int x2, int y2, unsigned level);

// draw ASCII character using the 5x8 font
// coordinates x,y are for the top left corner of the character
// proportional: 0 = fixed-width characters (6 pixels wide)
// 				 1 = reduce character width by skipping blank vertical lines
// returns the visible width of the character in pixels
int DrawChar(int x, int y, char ascii, unsigned level, int proportional);

// draw null-terminated ASCII string to the frame
// x,y are at the upper left corner
// proportional: 0 = fixed-width characters (6 pixels wide)
// 			     1 = reduced character width (takes up less space)
// returns the visible width of the drawn string in pixels
int DrawString(int x, int y, const char *str, unsigned level, int proportional);

// draw circle centered at x0,y0 with radius r
void DrawCircle(int x0, int y0, int r, unsigned level);

// draw filled rectangle with opposite corners (x1,y1) and (x2,y2)
// coordinates may lie outside the frame; only the visible portion is drawn
// this function is faster than DrawLine() for horizontal and vertical lines
void DrawFilledRectangle(int x1, int y1, int x2, int y2, unsigned level);

#endif /* FRAME_GRAPHICS_H_ */
