class Wifilist : public App {
	static const int wifi_nr = 5;
	struct AP {
		String ssid;
		String pass;
	};
	AP wifi_ap[wifi_nr];
	bool wifi_dirty = false;
	Edit e_pass;
	Menu m_cancel;
	Menu m_wifi;
	View v_info;
	Screen &scr;
	Preferences prefs;
	void wifi_push(int i);
	String wifi_get(String ssid);
	void wifi_set(String ssid, String pass);
	int wifi_lookup(String ssid);
	void wifi_save();
public:
	Wifilist(Screen &scr, Keyboard &kbd);
	void background();
	void setup();
	void resume() { start(); };
	void start();
	int process();
};
