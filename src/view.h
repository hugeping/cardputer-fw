class View : public App {
protected:
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
	struct line_t *last = NULL;
	int lines_nr = 0;
	Screen &scr;
	Keyboard &kbd;
	void free_line(struct line_t *beign);
	struct line_t *alloc_line(const void *text, int sz = -1);
	bool fmt_next(codepoint_t *buf, int last, int *off, int len, int *xx, int *yy);
public:
	bool wrap_by_words = true;
	boolean visible = false;

	int x = 0;
	int y = 0;
	int w = COLS;
	int h = ROWS;

	View(Screen &scr, Keyboard &kbd);

	virtual ~View();
	virtual void start() {
		App::start();
		show();
		kbd.set_fn_mode(true);
	};
	virtual void resume() {
		start();
	};
	void geom(int x, int y, int w, int h) {
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
	};
	void trim_head(int nr);
	void tail();
	void append(const void *text, int size = -1);
	void append(const char *text) {
		append((void*)text, -1);
	}
	void append(const codepoint_t *text, int size) {
		append((void*)text, size);
	}
	void set(const char *text);
	void reset();
	void up(int nr = 1);
	void down(int nr = 1);
	virtual int process();
	virtual void show();
};
