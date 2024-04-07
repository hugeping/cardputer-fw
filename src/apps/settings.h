class Settings : public App {
	enum settings_mode {
		none = 0,
		brightness,
		keysound,
	};
	Preferences prefs;
	Screen &scr;
	Keyboard &kbd;
	Menu m_main;
	settings_mode mode = none;
	void show_bright();
	void show_keysound();
public:
	int set_br = 128;
	bool set_keysound = false;
	Settings(Screen &scr, Keyboard &kbd);
	int process();
	void setup();
	void start();
};
