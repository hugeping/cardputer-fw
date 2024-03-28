#ifndef __SCREEN_H_INCLUDED
#define __SCREEN_H_INCLUDED
#include <M5GFX.h>
//#include <TFT_eSPI.h>
#include "utf8.h"

#define W 240
#define H 135

typedef uint16_t color_t;

#include "../font8x15.h"
#define COLS (W/FONT_W)
#define ROWS (H/FONT_H)
#define FG 0xfffd
#define BG 0 //0x18e3

typedef struct {
	codepoint_t glyph;
	color_t bg;
	color_t fg;
} cell_t;

class Screen
{
	const uint8_t* lookup_glyph(codepoint_t cp);
	int text_xoff = 0;
	int text_yoff = 0;
	cell_t *get_screen();
	cell_t *get_old_screen();
	void update_cell(int x, int y, cell_t *cell);
public:
	int cols = COLS;
	int rows = ROWS;

	M5GFX tft;
//	TFT_eSPI tft;
	Screen();
	void setup();
	void draw_text(int x, int y, const char *str,
		color_t fg = FG, color_t bg = BG);

	void draw_glyph(int x, int y, codepoint_t cp,
		color_t fg = FG, color_t bg = BG);

	bool text_scroll(const char *str, int w, long *otick);
	void text_scrolled(int x, int y, const char *text, int w, int offset);
	int text(int tx, int ty, const char *str,
		bool brk = false, int maxw = COLS, int maxh = ROWS);
	void off2xy(int off, int *x, int *y) { if (y) *y = off / cols; if (x) *x = off % cols; };
	void text_glyph(int tx, int ty, codepoint_t cp,
		color_t fg = FG, color_t bg = BG);

	void text_clear(int tx = 0, int ty = 0, int w = COLS, int h = ROWS,
		color_t fg = FG, color_t bg = BG);

	void text_update(bool force = false);
	static color_t color(byte r, byte g, byte b);
};

#endif
