#include "../../external.h"
#include "../../internal.h"

Settings::Settings(Screen &screen, Keyboard &keys) : scr(screen), kbd(keys),
	m_main(screen, keys, 16)
{
	m_main.append("Brightness");
	m_main.geom(0, 1, COLS-1, ROWS-1);
}

void
Settings::show_bright()
{
	char fmt[256];
	if (mode == brightness)
		sprintf(fmt, "Brightness [%d]", set_br);
	else
		sprintf(fmt, "Brightness");
	m_main.set(0, fmt);
	m_main.show();
	scr.text_update();
}

void
Settings::setup()
{
	prefs.begin("settings", true);
	set_br = prefs.getInt("brightness", -1);
	if (set_br < 0)
		set_br = 128;
	prefs.end();
	set_br = min(set_br, 255);
	set_br = max(set_br, 5);
	scr.tft.setBrightness(set_br);
}

int
Settings::process()
{
	if (mode == brightness) {
		uint8_t c;
		while ((c = kbd.input())) {
			if (c == KEY_ESC || c == KEY_BS || c == KEY_ENTER) {
				mode = none;
				show_bright();
				prefs.begin("settings", false);
				prefs.putInt("brightness", set_br);
				prefs.end();
				return APP_NOP;
			}
			if (c == KEY_DOWN || c == KEY_LEFT)
				set_br -= (c == KEY_DOWN)?10:1;
			else if (c == KEY_UP || c == KEY_RIGHT)
				set_br += (c == KEY_UP)?10:1;
			set_br = max(1, set_br);
			set_br = min(255, set_br);
			scr.tft.setBrightness(set_br);
			show_bright();
		}
		return APP_NOP;
	}
	int m = App::process();
	if (app == &m_main) {
		if (m == APP_EXIT)
			return m;
		switch(m) {
		case 0:
			mode = brightness;
			show_bright();
			break;
		}
	}
	return APP_NOP;
}

void
Settings::start()
{
	App::start();
	scr.text_clear(0, 0, COLS-1, 1, FG, scr.color(0, 128, 128));
	scr.text(0, 0, "Settings");
	push(&m_main);
}
