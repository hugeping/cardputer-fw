class Wifilist : public App {
	Edit e_pass;
	Menu m_cancel;
	Menu m_wifi;
	View v_info;
	Screen &scr;
	Preferences prefs;
public:

	Wifilist(Screen &scr, Keyboard &kbd);
	void setup();
	void resume() { start(); };
	void start();
	int process();
};
