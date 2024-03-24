class Edit : public App {
	codepoint_t *buf;
	Screen &scr;
	Keyboard &kbd;
	boolean cur_visible;
	void visible();
public:
	boolean oneline = false;
	int x = 0;
	int y = 0;
	int w = COLS;
	int h = ROWS;
	int off = 0;
	int cur = 0;
	int size = 0;
	int len = 0;
	void geom(int x, int y, int w, int h) {
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
	}
	Edit(Screen &scr, Keyboard &kbd, int size);
	Edit(Screen &scr, Keyboard &kbd, const char *text);
	~Edit();

	void start();
	void resume() { App::resume(); show(); };
	void up();
	void down();
	void set(const char *text);
	int process();
	const char *text();
	void show();
};
