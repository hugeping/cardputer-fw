class Python : public App {
	Term t_repl;
	char heap[8 * 1024];
	char *line = NULL;
public:
	int process();
	Python(Screen &scr, Keyboard &kbd);
	~Python();
	void start();
	void resume();
};
