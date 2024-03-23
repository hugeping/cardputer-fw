#define MAX_LINKS 128
class Gemini : public App {
	View view;
	Menu m_links;
	View v_status;
	Edit e_input;
	Edit e_addr;
	WiFiClientSecure &client;
	char status[3];
	char meta[1025];
	String links[MAX_LINKS];
	int links_nr = 0;
	char *server = NULL;
	char *last_url = NULL;
	static const int hist_max = 16;
	String history[hist_max];
	int hist_size;
	int hist_pos;
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
	Gemini(Screen &scr, Keyboard &kbd, WiFiClientSecure &c);
	~Gemini() { free(server); free(last_url); };
	void start();
};
