class Edit : public App {
	Screen &scr;
	Keyboard &kbd;
	boolean cur_visible;
	void visible();
public:
	codepoint_t *buf;
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

	uint32_t hash();
	void start();
	void resume() { App::resume(); kbd.set_fn_mode(false); show(); };
	void up();
	void down();
	void set(const char *text);
	int process();
	const char *text();
	void show();
};
