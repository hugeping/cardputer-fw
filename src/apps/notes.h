class Notes : public App {
	Screen &scr;
	static int const toc_max = 256;
	int toc[toc_max];
	int toc_nr = 0;
	int off = 0;
	int cur = 0;
	Keyboard &kbd;
	Menu m_toc;
	Edit *edit = NULL;
	uint32_t hash = 0;
	Preferences prefs;
public:
	void menu();
	Notes(Screen &scr, Keyboard &kbd);
	~Notes();
	int process();
	void start();
};
