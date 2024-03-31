#define MAX_LINKS 256
class Gemini : public App {
	Screen &scr;
	Keyboard &kbd;
	Edit e_addr;
	Edit e_input;
	Menu m_links;
	View view;
	View v_status;
	WiFiClientSecure client;
	char status[3];
	char meta[1025];
	String links[MAX_LINKS];
	int links_nr = 0;
	char *server = NULL;
	char *last_url = NULL;
	static const int hist_max = 16;
	String history[hist_max] = {};
	int hist_size = 0;
	int hist_pos = 0;
	bool req(const char *req);
	void body();
	void input(const char *msg);
	bool reqURI(const char *uri, bool history);
	void message(const char *msg);
	void menu_reset();
	Preferences prefs;
public:
	int process();
	bool request(const char *uri, bool hist = true);
	Gemini(Screen &scr, Keyboard &kbd);
	~Gemini() { free(server); free(last_url); };
	void start();
};
