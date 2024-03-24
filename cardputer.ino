#include "external.h"
#include "internal.h"

WiFiClientSecure sslClient;

Screen scr = Screen();
Keyboard kbd = Keyboard();

Gemini *gemini = NULL;
Menu main_menu(scr, kbd, (const char *[]){ "WiFi", "Edit", "Gemini", NULL });
Wifilist wifi(scr, kbd);

Edit *edit = new Edit(scr, kbd, 2048);

App app;

void
setup()
{
	Serial.begin(115200);
	sslClient.setInsecure();
	adc_read_init();
	scr.setup();
	kbd.setup();
	wifi.setup();
	Serial.println("Started");
	main_menu.geom(0, 0, COLS-1, ROWS);
	edit->geom(0, 0, COLS-1, ROWS);
	app.push(&main_menu);
}

static long bat_ms = 0;

void
battery()
{
	float bat = adc_read_get_value() * 2 / 1000;
	int pcnt = 0;
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
	const uint32_t bg = 0;
	if (millis() - bat_ms > 1000) {
		bat_ms = millis();
		battery();
	}
	int x = COLS-1;
	scr.text_clear(x, 0, 1, ROWS, FG, bg);
	int status = WiFi.status();
	if (status == WL_CONNECTED)
		scr.text_glyph(x, 0, 1, FG, bg);
	else
		scr.text_glyph(x, 0, ' ', FG, bg);

	if (kbd.get_ctrl())
		scr.text_glyph(x, ROWS-4, '^', 0xffff, bg);
	else if (kbd.get_alt())
		scr.text_glyph(x, ROWS-4, '@',  0xffff, bg);

	if (kbd.get_shift())
		scr.text_glyph(x, ROWS-3, 'A', 0xffff, scr.color(0, 0, 255));
	else
		scr.text_glyph(x, ROWS-3, 'a',  scr.color(0, 0, 255), bg);

	if (kbd.get_fn())
		scr.text_glyph(x, ROWS-2, 2, 0xffff, scr.color(0xff, 0x80, 0));
	else
		scr.text_glyph(x, ROWS-2, 2, scr.color(0xff, 0x80, 0), bg);

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
			app.push(edit);
			break;
		case 2:
			if (!gemini)
				gemini = new Gemini(scr, kbd, sslClient);
			app.push(gemini);
			break;
		}
	} else if (m == APP_EXIT) {
		if (app.app == &main_menu) {
			app.push(&main_menu);
		} else {
			main_menu.resume();
			if (gemini) {
				gemini->stop();
				delete(gemini);
				gemini = NULL;
			}
		}
	}
	status();
	scr.tft.endWrite();
}
