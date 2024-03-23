#include "../external.h"
#include "../internal.h"

void
Menu::start()
{
	resume();
	sel = 0;
}

void
Menu::resume()
{
	kbd.set_fn_mode(true);
	App::start();
	show();
}

Menu::Menu(Screen &screen, Keyboard &keys, int sz) :
	scr(screen), kbd(keys), size(sz)
{
	list = new String[size];
}

Menu::Menu(Screen &screen, Keyboard &keys, const char *items[]) :
	scr(screen), kbd(keys)
{
	for (int i=0; items[i]; i++) {
		size ++;
		len ++;
	}
	list = new String[size];
	for (int i=0; i < len; i++)
		list[i] = String(items[i]);
}

void Menu::reset()
{
	len = 0;
	sel = 0;
}

void Menu::set(int nr, const char *text)
{
	if (nr >= size)
		return;
	list[nr] = String(text);
}

void Menu::append(const char *text)
{
	if (len >= size)
		return;
	list[len] = String(text);
	len ++;
}

void
Menu::clear()
{
	scr.text_clear(x, y, w, h);
}
void
Menu::show()
{
	int hh = h;
	int yy = y;
	if (h <= 0)
		return;
	clear();
	int nr = (sel / hh)*hh;
	for (int pos = nr; pos < len && yy < y + h; pos ++) {
		const char *text = list[pos].c_str();
		if (pos == sel) {
			scr.text_clear(x, yy, w, 1, FG, scr.color(0, 0, 255));
			scr.text_scrolled(x, yy, text, w, sel_offset);
		} else {
			scr.text_clear(x, yy, w, 1, FG, BG);
			scr.text(x, yy, text);
		}
		yy ++;
	}
	Serial.println("Show menu");
	scr.text_update();
}

int
Menu::process()
{
	uint8_t c;
	int ret = APP_NOP;
	bool dirty = false;
	while ((c = kbd.input()) && ret == APP_NOP) {
		switch(c){
		case KEY_LEFT:
			sel --;
			dirty = true;
			break;
		case KEY_UP:
			sel -= h - 1;
			dirty = true;
			break;
		case KEY_RIGHT:
			sel ++;
			dirty = true;
			break;
		case KEY_DOWN:
			sel += h - 1;
			dirty = true;
			break;
		case KEY_BS:
		case KEY_ESC:
			ret = APP_EXIT;
			break;
		case KEY_ENTER:
			ret = sel;
			break;
		default:
			break;
		}
		sel = max(0, sel);
		sel = min(sel, len - 1);
	}
	if (!dirty && len > 0) {
		static long otick = 0;
		if (scr.text_scroll(list[sel].c_str(), w, &otick)) {
			dirty = true;
			sel_offset ++;
		}
	} else
		sel_offset = 0;
	if (dirty)
		show();
	return ret;
}


Menu::~Menu()
{
	delete(list);
}
