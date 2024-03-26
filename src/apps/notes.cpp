#include "../../external.h"
#include "../../internal.h"

Notes::Notes(Screen &screen, Keyboard &keys) : scr(screen), kbd(keys),
        m_toc(screen, keys, toc_max),
        v_server(screen, keys)
{
        m_toc.geom(0, 0, COLS-1, ROWS);
        v_server.geom(0, 0, COLS-1, ROWS);
}

void
Notes::menu()
{
	char item[65];
	char sym[5];

	m_toc.reset();
	m_toc.append("[Server]");
	codepoint_t *buf = edit->buf;
	bool fill = false;
	int off;
	toc_nr = 0;
	for (int i = 0; i < edit->len; i++) {
		if (fill) {
			if (strlen(item) >= 60 || buf[i] == '\n' ||
				i == edit->len - 1) {
				if (item[0]) {
					m_toc.append(item);
					toc[toc_nr++] = off;
				}
				fill = false;
				continue;
			}
			sym[utf8::from_codepoint(buf[i], sym)] = 0;
			strcat(item, sym);
		} else if (buf[i] == '#' && (!i || buf[i-1] =='\n')) {
			fill = true;
			item[0] = 0;
			off = i;
			while (i < edit->len-1 && buf[i+1] == ' ')
				i++;
		}
	}
	edit->pause();
	push(&m_toc);
}
void
Notes::server()
{
	if (!srv)
		return;
	WiFiClient cli = srv->available();
	if (cli.connected()||cli.available()) {
		Serial.println("Connection on port 1000");
		cli.write(edit->text());
		char *buf = (char *)malloc(65536);
		int i = 0;
		while ((cli.connected()||cli.available())
			&& i < 65535) {
			if (cli.available())
				buf[i++] = cli.read();
		}
		if (i > 0) {
			Serial.println("Readed: "+String(i)+ " byte(s)");
			buf[i] = 0;
			edit->set(buf);
		}
		free(buf);
		cli.stop();
		return;
	}
}
void
Notes::start_server()
{
	char fmt[512];
	m_toc.stop();
	if (WiFi.status() != WL_CONNECTED) {
		v_server.set("No WiFi connection");
		push(&v_server);
		srv = NULL;
		return;
	}
	srv = new WiFiServer(1000);
	srv->begin();
	sprintf(fmt, "Get:\nnc %s 1000\n\nExchange:\nnc -N %s 1000 < todo.txt",
		WiFi.localIP().toString().c_str(),
		WiFi.localIP().toString().c_str());
	v_server.set(fmt);
	push(&v_server);
}

int
Notes::process()
{
	int m = App::process();
	if (v_server.is_active()) {
		server();
		return APP_NOP;
	}
	if (m == APP_NOP)
		return m;

	if (m == APP_EXIT && app == &v_server) {
		if (srv) {
			srv->end();
			delete(srv);
		}
		edit->resume();
		return APP_NOP;
	}

	if (app == &m_toc) {
		if (m == APP_EXIT)
			edit->resume();
		else if (m == 0) { //server
			start_server();
		} else if (m >= 1) {
			int off = toc[m-1];
			m_toc.stop();
			edit->off = off;
			edit->cur = off;
			edit->resume();
		}
		return APP_NOP;
	} else if (m == KEY_TAB) {
		menu();
		return APP_NOP;
	} else if (m == APP_EXIT) {
		uint32_t n_hash = edit->hash();
		if (n_hash == hash && cur == edit->cur && off == edit->off)
			return m;
		prefs.begin("notes", false);
		if (n_hash != hash) {
			hash = n_hash;
			prefs.putString("text", edit->text());
		}
		if (edit->off != off) {
			off = edit->off;
			prefs.putInt("off", off);
		}
		if (edit->cur != cur) {
			cur = edit->cur;
			prefs.putInt("cur", cur);
		}
		prefs.end();
		return m;
	}
	return m;
}

void
Notes::start()
{
	App::start();
	if (!edit) {
		prefs.begin("notes", true);
		String text = prefs.getString("text");
		off = prefs.getInt("off", 0);
		cur = prefs.getInt("cur", 0);
		prefs.end();
		int len = utf8::len(text.c_str()) + 4096;
		edit = new Edit(scr, kbd, len);
		edit->geom(0, 0, COLS-1, ROWS);
		edit->set(text.c_str());
		if (cur > edit->len || off > edit->len)
			cur = off = 0;
		edit->cur = cur;
		edit->off = off;
		hash = edit->hash();
	}
	push(edit);
}

Notes::~Notes()
{
	if (edit)
		delete edit;
}