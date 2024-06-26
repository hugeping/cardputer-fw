#include "external.h"
#include "internal.h"
/*
Libraries:
https://github.com/m5stack/M5GFX

*/
Sound snd = Sound();
Screen scr = Screen();
Keyboard kbd = Keyboard(snd);
Settings settings = Settings(scr, kbd);

Gemini *gemini = NULL;
Irc *irc = NULL;
Notes *notes = NULL;
Python *python = NULL;
Metar *metar = NULL;

Menu main_menu(scr, kbd, (const char *[]){ "WiFi", "Notes", "Gemini", "IRC", "Python", "METAR", "Settings", NULL });
Wifilist wifi(scr, kbd);

App app;

void
setup()
{
	Serial.begin(115200);
	adc_read_init();
	settings.setup();
	scr.setup();
	kbd.setup();
	wifi.setup();
	snd.setup();

	Serial.println("Started");
	main_menu.geom(0, 0, COLS-1, ROWS);
	app.push(&main_menu);
}

static long bat_ms = 0;

void
battery()
{
	float bat = adc_read_get_value() * 2 / 1000;
	int pcnt = 0;
//	Serial.println("Bat: " + String(bat));
	scr.tft.fillRect(W - FONT_W + 1, FONT_H + 1, FONT_W - 2, FONT_H - 2,
		scr.color(128, 128, 128));
	scr.tft.fillRect(W - FONT_W + 2, FONT_H + 2, FONT_W - 4, FONT_H - 4,
		scr.color(0, 255, 0));
	if (bat >= 4.12)
		pcnt = 100;
	else if (bat >= 3.88)
		pcnt = 75;
	else if (bat >= 3.61)
		pcnt = 50;
	else if (bat >= 3.40)
		pcnt = 25;
	pcnt = (100 - pcnt)*(FONT_H - 4)/100;
	scr.tft.fillRect(W - FONT_W + 2, FONT_H + 2, FONT_W - 4, pcnt, BG);
}

void
status()
{
	const uint32_t bg = scr.color(0x33, 0x33, 0x33);
	if (millis() - bat_ms > 1000) {
		bat_ms = millis();
		battery();
	}
	int x = COLS-1;
	scr.text_clear(x, 0, 1, ROWS, FG, bg);
	int status = WiFi.status();
	if (status == WL_CONNECTED) {
		scr.text_glyph(x, 0, 1, FG, bg);
	} else
		scr.text_glyph(x, 0, ' ', FG, bg);

	if (irc)
		scr.text_glyph(x, ROWS-5, '#', 0xffff, bg);
	if (kbd.get_ctrl())
		scr.text_glyph(x, ROWS-4, '^', 0xffff, bg);
	else if (kbd.get_alt())
		scr.text_glyph(x, ROWS-4, '@',  0xffff, bg);

	if (kbd.get_shift())
		scr.text_glyph(x, ROWS-3, 'A', 0xffff, scr.color(0, 0, 255));
	else
		scr.text_glyph(x, ROWS-3, 'a',  0xffff, bg);

	if (kbd.get_fn())
		scr.text_glyph(x, ROWS-2, 2, 0xffff, scr.color(0xff, 0x80, 0));
	else
		scr.text_glyph(x, ROWS-2, 2, 0xffff, bg);

	if (kbd.get_layout())
		scr.text_glyph(x, ROWS-1, 0x44f, bg, scr.color(0, 255, 0));
	else
		scr.text_glyph(x, ROWS-1, 0x44f, scr.color(0, 255, 0), bg);
	scr.text_update();
//	scr.tft.fillRect(W - FONT_W, 0, 1, H, scr.color(128, 128, 128));
}

void
loop()
{
	kbd.poll();
	scr.tft.startWrite();
	int m = app.process();
	if (main_menu.is_active() && m >= 0) {
		main_menu.pause();
		switch (m) {
		case 0:
			app.push(&wifi);
			break;
		case 1:
			if (!notes)
				notes = new Notes(scr, kbd);
			app.push(notes);
			break;
		case 2:
			if (!gemini)
				gemini = new Gemini(scr, kbd);
			app.push(gemini);
			break;
		case 3:
			if (!irc) {
				irc = new Irc(scr, kbd);
				app.push(irc);
			} else {
				Serial.println("Resume irc");
				irc->resume();
			}
			break;
		case 4:
			if (!python) {
				python = new Python(scr, kbd);
				app.push(python);
			} else {
				Serial.println("Resume repl");
				python->resume();
			}
			break;
		case 5:
			metar = new Metar(scr, kbd);
			app.push(metar);
			break;
		case 6:
			app.push(&settings);
			break;
		}
	} else if (m == APP_EXIT) {
		if (app.app == &main_menu) {
			app.push(&main_menu);
		} else {
			main_menu.resume();
			if (app.app == gemini) {
				delete(gemini);
				gemini = NULL;
			} else if (app.app == irc) {
				delete(irc);
				irc = NULL;
			} else if (app.app == notes) {
				delete(notes);
				notes = NULL;
			} else if (app.app == python) {
				delete(python);
				python = NULL;
			} else if (app.app == metar) {
				delete(metar);
				metar = NULL;
			}
		}
	} else if (m == APP_BG) {
		Serial.println("App goes bg");
		main_menu.resume();
	}
	status();
	scr.tft.endWrite();
}
