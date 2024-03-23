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
	scr.setup();
	kbd.setup();
	wifi.setup();
	Serial.println("Started");
	main_menu.geom(0, 0, COLS-1, ROWS);
	edit->geom(0, 0, COLS-1, ROWS);
	app.push(&main_menu);
}
void
status()
{
	const uint32_t bg = 0;
	int x = COLS-1;
	scr.text_clear(x, 0, 1, ROWS, FG, bg);
	int status = WiFi.status();
	if (status == WL_CONNECTED)
		scr.text_glyph(x, 0, '!', FG, bg);
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
		scr.text_glyph(x, ROWS-2, 'f', 0xffff, scr.color(0xff, 0x80, 0));
	else
		scr.text_glyph(x, ROWS-2, 'f', scr.color(0xff, 0x80, 0), bg);

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
//	scr.tft.startWrite();
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
		main_menu.resume();
		if (gemini) {
			gemini->stop();
			delete(gemini);
			gemini = NULL;
		}
	}
	status();
//	scr.tft.endWrite();
}
