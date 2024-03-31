class Term : public View {
	void replace_last(const codepoint_t *cp, int size = 0);
	void replace_last(const char *str);

	static const int inp_size = 256;
	static const int hist_max = 32;
	String hist[hist_max] = {};
	int hist_size = 0;
	int hist_pos = 0;
	int hist_top = 0;
	int inp_pfx = 0;
public:
	int inp_cur = 0;
	int inp_len = 0;

	String pfx;

	codepoint_t input[inp_size+1];
	char *getinp();

	virtual void start() {
		View::start();
		kbd.set_fn_mode(false);
	};
	void prompt(const char *str = NULL);
	void remove_last();
	void inp_append(const char *text);
//	void append(const char *text, int sz = -1);
	Term(Screen &scr, Keyboard &kbd);
	~Term();
	int process();
	void show();
};
