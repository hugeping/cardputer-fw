#include "screen.h"
Screen::Screen()
{
}

void
Screen::setup()
{
	tft.begin();
	memset(screens, 0xff, COLS*ROWS*sizeof(cell_t)*2);
	tft.setRotation(1);
	tft.fillRect(0, 0, W, H, color(128, 128, 0));
	text_clear();
}


cell_t *
Screen::get_screen()
{
	return (cell_t*)&screens[0];
}

cell_t *
Screen::get_old_screen()
{
	return (cell_t*)&screens[1];
}

const uint8_t*
Screen::lookup_glyph(codepoint_t cp)
{
	const uint16_t *ptr = font->map;
	while (ptr[2]) {
		if (cp >= ptr[1] && cp < ptr[1] + ptr[2])
			return font->data + ptr[0] + (cp-ptr[1])*font->h;
		ptr += 3;
	}
	return NULL;
}

void
Screen::draw_glyph(int x, int y, codepoint_t cp, color_t fg, color_t bg)
{
	int yy, xx;
	const uint8_t *p = lookup_glyph(cp);
	if (!p) {
		tft.fillRect(x, y, font->w, font->h, bg);
		return;
	}
	tft.setWindow(x, y, x + font->w - 1, y + font->h - 1);
	tft.startWrite();
	for (yy = 0; yy < font->h; yy++) {
		uint8_t b = pgm_read_byte(p);
		for (xx = 0; xx < font->w; xx++) {
//			tft.fillRect(x + xx, y + yy, 1, 1, (b&1)?fg:bg);
			tft.setColor((b&1)?fg:bg);
			tft.writeFillRect(x + xx, y + yy, 1, 1);
			b >>= 1;
		}
		p ++;
	}
	tft.endWrite();
}

void
Screen::draw_text(int x, int y, const char *str, color_t fg, color_t bg)
{
	const char *ptr = str;
	codepoint_t cp = 0;
	int ox = x;
	tft.startWrite();
	do {
		ptr = utf8::to_codepoint(ptr, &cp);
		if (cp == '\n') {
			y = y + font->h;
			x = ox;
		} else {
			if (x < W-font->w)
				draw_glyph(x, y, cp, fg, bg);
			x += font->w;
		}
	} while(*ptr && y < H);
	tft.endWrite();
}

void
Screen::update_cell(int x, int y, cell_t *cell)
{
	x *= font->w;
	y *= font->h;
	if (!cell->glyph)
		tft.fillRect(text_xoff + x, text_yoff + y,
			font->w, font->h, cell->bg);
	else
		draw_glyph(text_xoff + x, text_yoff + y,
			cell->glyph, cell->fg, cell->bg);
}

void
Screen::text_clear(int xx, int yy, int w, int h, color_t fg, color_t bg)
{
	cell_t *line = get_screen() + yy * cols;
	for (int y = yy; y < yy + h; y ++) {
		for (int x = xx; x < xx + w; x ++) {
			line[x] = { .glyph = 0, .bg = bg, .fg = fg };
		}
		line += cols;
	}
}

void
Screen::text_scrolled(int x, int y, const char *t, int w, int offset)
{
	int n = utf8::len(t) - w;
	if (n > 0) {
		n = n - abs(n - (offset)%(2*n));
		t = utf8::index(t, n);
	}
	text(x, y, t);
}

bool
Screen::text_scroll(const char *str, int w, long *otick)
{
	bool dirty = false;
	int ll = utf8::len(str);
	int tick = (millis()-*otick) > 150;
	if (ll > w && tick) {
		*otick = millis();
		dirty = true;
	}
	return dirty;
}

void
Screen::text_glyph(int x, int y, codepoint_t cp, color_t fg, color_t bg)
{
	cell_t *c = get_screen() + y*COLS;
	c[x].glyph = cp;
	c[x].fg = fg;
	c[x].bg = bg;
}

void
Screen::text(int x, int y, const char *str, bool brk)
{
	const char *ptr = str;
	codepoint_t cp = 0;
	int ox = x;
	cell_t *c = get_screen() + y*cols;
	do {
		ptr = utf8::to_codepoint(ptr, &cp);
		if (cp == '\n') {
			y ++;
			c += cols;
			x = ox;
		} else {
			if (brk && x >= cols) {
				x = ox;
				y ++;
				c += cols;
			}
			if (x < cols && y < rows) {
				c[x].glyph = cp;
				x ++;
			}
		}
	} while(*ptr && y < rows);
}
void

Screen::text_update(bool force)
{
	cell_t *line = get_screen();
	cell_t *oline = get_old_screen();
	tft.startWrite();
	for (int y = 0; y < rows; y ++) {
		for (int x = 0; x < cols; x ++) {
			if (force || line[x].glyph != oline[x].glyph ||
				line[x].fg != oline[x].fg ||
				line[x].bg != oline[x].bg) {
					update_cell(x, y, &line[x]);
					oline[x] = line[x];
			}
		}
		line += cols;
		oline += cols;
	}
	tft.endWrite();
}

color_t
Screen::color(byte r, byte g, byte b)
{
	return (( r & 0xF8 ) << 8) |
		(( g & 0xFC) << 3) |
		((b & 0xF8) >> 3);
}
