class Metar : public App {
	View view;
	Edit e_icao;
	Preferences prefs;
	WiFiClientSecure client;
	Screen &scr;
	void icao();
	void get();
public:
	int process();
	Metar(Screen &scr, Keyboard &kbd);
	void start();
};
