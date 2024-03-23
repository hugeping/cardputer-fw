class Menu : public App {
	String *list;
	Screen &scr;
	Keyboard &kbd;
	int sel_offset = 0;
public:
	int size = 0;
	int len = 0;
	int sel = 0;

	int w = COLS;
	int h = ROWS;
	int x = 0;
	int y = 0;
	void start();
	void resume();
	void clear();
	void geom(int x, int y, int w, int h) {
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
	};
	Menu(Screen &scr, Keyboard &kbd, int size);
	Menu(Screen &scr, Keyboard &kbd, const char *items[]);
	~Menu();
	const char *selected() { return list[sel].c_str(); };
	void append(const char *text);
	void set(int nr, const char *text);
	void reset();
	int process();
	void show();
};
