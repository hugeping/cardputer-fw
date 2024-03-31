#include "../../external.h"
#include "../../internal.h"

static const char *wifi_cancel_menu[] = { "Ok", "Disconnect", NULL };
Wifilist::Wifilist(Screen &screen, Keyboard &kbd) :
	scr(screen),
	m_wifi(screen, kbd, 64),
	m_cancel(screen, kbd, wifi_cancel_menu),
	e_pass(screen, kbd, 128),
	v_info(screen, kbd)
{
	e_pass.oneline = true;
	e_pass.set("");
	m_wifi.geom(0, 0, COLS - 1, ROWS - 2);
	v_info.geom(0, 0, COLS - 1, ROWS - 2);
	e_pass.geom(0, 1, COLS - 1, 1);
	m_cancel.geom(0, ROWS-2, COLS - 1, 2);
}

void
Wifilist::wifi_push(int i)
{
	if (!i)
		return;
	AP ap = wifi_ap[0];
	wifi_ap[0] = wifi_ap[i];
	wifi_ap[i] = ap;
	wifi_dirty = true;
}

int
Wifilist::wifi_lookup(String ssid)
{
	for (int i = 0; i < wifi_nr; i++) {
		if (wifi_ap[i].ssid == ssid) {
			return i;
		}
	}
	return -1;
}

void Wifilist::wifi_save()
{
	if (!wifi_dirty)
		return;
	Serial.println("WiFi save prefs");
	prefs.begin("wifi", false);
	for (int i = 0; i < wifi_nr; i++) {
		char fmt[64];
		sprintf(fmt, "ssid%d", i);
		prefs.putString(fmt, wifi_ap[i].ssid);
		sprintf(fmt, "passwd%d", i);
		prefs.putString(fmt, wifi_ap[i].pass);
	}
	prefs.end();
	wifi_dirty = false;
}

void
Wifilist::wifi_set(String ssid, String pass)
{
	WiFi.begin(m_wifi.selected(), e_pass.text());

	prefs.begin("wifi", false);
	prefs.putString("ssid", ssid);
	prefs.putString("passwd", pass);
	prefs.end();

	int i = wifi_lookup(ssid);
	if (i >= 0) {
		if (wifi_ap[i].pass != pass) {
			wifi_ap[i].pass = pass;
			wifi_push(i);
		}
		return;
	}
	for (i = wifi_nr-1; i > 0; i --)
		wifi_ap[i] = wifi_ap[i-1];
	wifi_ap[0].ssid = ssid;
	wifi_ap[0].pass = pass;
	wifi_dirty = true;
}

String
Wifilist::wifi_get(String ssid)
{
	int i = wifi_lookup(ssid);
	if (i < 0)
		return "";
	wifi_push(i);
	return wifi_ap[0].pass;
}

void
Wifilist::setup()
{
	prefs.begin("wifi", true);
	String ssid = prefs.getString("ssid");
	String passwd = prefs.getString("passwd");
	if (ssid != "" && passwd != "")
		WiFi.begin(ssid, passwd);
	for (int i = 0; i < wifi_nr; i++) {
		char fmt[64];
		sprintf(fmt, "ssid%d", i);
		wifi_ap[i].ssid = prefs.getString(fmt);
		sprintf(fmt, "passwd%d", i);
		wifi_ap[i].pass = prefs.getString(fmt);
	}
	prefs.end();
}

void
Wifilist::background()
{
	if (WiFi.status() == WL_CONNECTED) {
		Serial.println("WiFi online");
		wifi_save();
		stop();
	}
}

int
Wifilist::process()
{
	int m = App::process();
	if (app == &m_wifi && m >= 0) {
		m_wifi.pause();
		m_wifi.clear();
		scr.text(0, 0, "Password");
		e_pass.set(wifi_get(m_wifi.selected()).c_str());
		push(&e_pass);
	} else if (app == &m_cancel) {
		if (m == KEY_ESC || m == 0) {
			return APP_EXIT;
		}
		if (m == 1) { /* cancel */
			WiFi.disconnect(true);
			prefs.begin("wifi", false);
			prefs.remove("ssid");
			prefs.remove("passwd");
			prefs.end();
			return APP_EXIT;
		}
	} else if (app == &e_pass) {
		if (m == APP_EXIT) {
			e_pass.stop();
			m_wifi.resume();
			return APP_NOP;
		}
		if (m == KEY_ENTER) {
			wifi_set(m_wifi.selected(), e_pass.text());
			m_wifi.stop();
			e_pass.stop();
			return APP_BG;
		}
	}
	return m;
}

void
Wifilist::start()
{
	App::start();
	static const char *statuses[] = { "Idle",
		"No SSID", "SCAN", "CONNECTED", "FAILED",
		"LOST", "DISCONNECTED" };
	int status = WiFi.status();
	if (status != WL_NO_SHIELD) {
		char fmt[128];
		sprintf(fmt, "SSID: %s\nIP: %s\nStatus: %s",
			WiFi.SSID().c_str(),
			WiFi.localIP().toString().c_str(),
			statuses[status]);
		v_info.set(fmt);
		v_info.show();
		m_cancel.sel = 0;
		push(&m_cancel);
		return;
	}
	WiFi.disconnect(true);
	m_wifi.reset();
	m_wifi.append("Scanning...");
	m_wifi.show();

	int nr = WiFi.scanNetworks();
	m_wifi.reset();
	for (int net = 0; net < nr; net ++)
		m_wifi.append(WiFi.SSID(net).c_str());
	push(&m_wifi);
}
