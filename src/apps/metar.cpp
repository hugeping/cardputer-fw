#include "../../external.h"
#include "../../internal.h"

Metar::Metar(Screen &scr, Keyboard &kbd) :
	view(scr, kbd),
	e_icao(scr, kbd, 32),
	scr(scr)
{
	e_icao.geom(0, 1, COLS-1, 1);
	e_icao.oneline = true;
	view.geom(0, 0, COLS-1, ROWS);
}

void
Metar::icao()
{
	view.pause();
	scr.text_clear(0, 0, COLS-1, 1, 0xffff,
		scr.color(0, 128, 128));
	scr.text(0, 0, "ICAO");
	push(&e_icao);
}

int
Metar::process()
{
	int m = App::process();
	if (m == KEY_TAB && app == &view) {
		icao();
		return APP_NOP;
	}
	if (m == KEY_ENTER && app == &e_icao) {
		e_icao.stop();
		view.resume();
		get();
		return APP_NOP;
	}
	if (m == APP_EXIT && app == &e_icao) {
		view.resume();
		return APP_NOP;
	}
	if (m == APP_EXIT) {
		prefs.begin("METAR", false);
		prefs.putString("ICAO", e_icao.text());
		prefs.end();
		return m;
	}
	return APP_NOP;
}

void
Metar::get()
{
	char r[256];
	HTTPClient https;
	client.setInsecure();

	sprintf(r, "https://aviationweather.gov/api/data/metar?ids=%s", e_icao.text());
	if (!https.begin(client, r) || https.GET() != HTTP_CODE_OK) {
		view.set("Can not get METAR information...");
	} else {
		view.set(https.getString().c_str());
	}
	https.end();

	sprintf(r, "https://aviationweather.gov/api/data/taf?ids=%s", e_icao.text());
	if (!https.begin(client, r) || https.GET() != HTTP_CODE_OK) {
		view.append("Can not get TAF information...");
	} else {
		view.append(https.getString().c_str());
	}
	view.show();
	client.stop();
}

void
Metar::start()
{
	char buf[256];

	prefs.begin("METAR", true);
	e_icao.set(prefs.getString("ICAO").c_str());
	prefs.end();

	App::start();
	push(&view);
	if (!e_icao.text()[0]) {
		icao();
	} else
		get();
}
