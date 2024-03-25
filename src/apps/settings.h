class Settings : public App {
	enum settings_mode {
		none = 0,
		brightness,
	};
	Preferences prefs;
	Screen &scr;
	Keyboard &kbd;
	Menu m_main;
	settings_mode mode = none;
	void show_bright();
public:
	int set_br = 128;
	Settings(Screen &scr, Keyboard &kbd);
	int process();
	void setup();
	void start();
};
