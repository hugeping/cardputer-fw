class View : public App {
	struct line_t;

	struct line_t {
		struct line_t *next;
		struct line_t *prev;
		codepoint_t *buf;
		int len;
		int chunks;
	};

	struct line_t *begin = NULL;
	struct line_t *cur = NULL;
	int lines_nr = 0;
	Screen &scr;
	Keyboard &kbd;
	bool fmt_next(codepoint_t *buf, int last, int *off, int len, int *xx, int *yy);
public:
	boolean visible = false;

	int x = 0;
	int y = 0;
	int w = COLS;
	int h = ROWS;

	View(Screen &scr, Keyboard &kbd);

	~View();
	void start() {
		App::start();
		show();
	};
	void resume() {
		App::start();
		show();
	};
	void geom(int x, int y, int w, int h) {
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
	};
	void trim_head(int nr);
	void tail();
	void append(const char *text);
	void set(const char *text);
	void reset();
	void up(int nr = 1);
	void down(int nr = 1);
	int process();
	void show();
};
