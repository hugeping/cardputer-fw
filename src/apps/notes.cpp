#include "../../external.h"
#include "../../internal.h"

Notes::Notes(Screen &screen, Keyboard &keys) : scr(screen), kbd(keys),
        m_toc(screen, keys, toc_max)
{
        m_toc.geom(0, 0, COLS-1, ROWS);
}

void
Notes::menu()
{
	char item[65];
	char sym[5];

	m_toc.reset();
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
		}
	}
	if (toc_nr > 0) {
		edit->pause();
		push(&m_toc);
	}
}

int
Notes::process()
{
	int m = App::process();
	if (m == APP_NOP)
		return m;
	if (app == &m_toc) {
		if (m == APP_EXIT)
			edit->resume();
		else if (m >= 0) {
			int off = toc[m];
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