#include "../external.h"
#include "../internal.h"

Term::Term(Screen &screen, Keyboard &keys) : View(screen, keys)
{
	wrap_by_words = false;
	keys.set_fn_mode(false);
}

char*
Term::getinp()
{
	int sz = 0;
	for (int i = inp_pfx; i < inp_len; i ++)
		sz += utf8::from_codepoint(input[i]);
	char *ptr = (char*)malloc(sz + 1);
        char *p = ptr;
        for (int i = inp_pfx; i < inp_len; i ++)
        	p += utf8::from_codepoint(input[i], p);
        *p = 0;
	return ptr;
}

void
Term::prompt(const char *str)
{
	inp_pfx = (str)?utf8::len(str):0;
        int i = 0;
	pfx = String(str);
        while (str && *str) {
                codepoint_t cp;
                str = utf8::to_codepoint(str, &cp);
                input[i++] = cp;
        }
        inp_cur = inp_pfx;
        inp_len = inp_pfx;
//	remove_last();
	View::append((const char*)input, inp_len);
}

Term::~Term()
{
}

int
Term::process()
{
	uint8_t c;
	const char *sym = NULL;
	int ret = APP_NOP;
	bool dirty = false;
	while ((c = kbd.input(&sym)) && ret == APP_NOP && !dirty) {
		if (sym && inp_len < inp_size) {
			codepoint_t cp;
			utf8::to_codepoint(sym, &cp);
			utf8::insert(input, &inp_cur, &inp_len, cp);
			dirty = true;
			replace_last(input, inp_len);
			tail();
			c = -1;
		}
		switch(c){
		case KEY_BS:
			if (inp_cur > inp_pfx && inp_len > 0) {
				utf8::rem_left(input, &inp_cur, &inp_len);
				dirty = true;
				replace_last(input, inp_len);
				tail();
			}
			break;
		case KEY_PGUP:
			up();
			dirty = true;
			break;
		case KEY_PGDOWN:
			down();
			dirty = true;
			break;
		case KEY_LEFT:
			inp_cur --;
			dirty = true;
			break;
		case KEY_RIGHT:
			inp_cur ++;
			dirty = true;
			break;
		case KEY_ESC:
			ret = APP_EXIT;
			break;
		case KEY_DOWN:
			if (hist_pos > 0) {
				hist_pos--;
				String h;
				if (!hist_pos)
					h = "";
				else
					h = hist[(hist_top-hist_pos)%hist_max];
				h = pfx + h;
				replace_last(h.c_str());
				inp_cur = inp_len = utf8::len(h.c_str());
				dirty = true;
			}
			break;
		case KEY_UP:
			Serial.println(hist_size);
			Serial.println(hist_pos);
			if (hist_size > hist_pos) {
				hist_pos++;
				String h = hist[(hist_top-hist_pos)%hist_max];
				h = pfx + h;
				replace_last(h.c_str());
				inp_cur = inp_len = utf8::len(h.c_str());
				dirty = true;
			}
			break;
		case KEY_ENTER:
			{
				char *h = getinp();
				if (h[0] && (!hist_size || strcmp(h, hist[hist_top-1].c_str()))) {
					remove_last();
					append((const char*)input, inp_len);
					hist[(hist_top++)%hist_max] = String(h);
					hist_size = min(hist_size + 1, hist_max);
					free(h);
				}
				hist_pos = 0;
			}
			tail();
			dirty = true;
			ret = c;
			break;
		case KEY_TAB:
			ret = c;
			break;
		default:
			break;
		}
	}
	inp_cur = max(inp_pfx, inp_cur);
	inp_cur = min(inp_cur, inp_len);
	if (dirty)
		show();
	return ret;
}

void
Term::remove_last()
{
	if (!last)
		return;
	if (last->prev)
		last->prev->next = NULL;
	else
		begin = NULL;
	struct line_t *ln = last;
	for (last = last->prev; last && !last->chunks; last = last->prev);
	lines_nr -= ln->chunks;
	free_line(ln);
}

void
Term::replace_last(const codepoint_t *text, int size)
{
	remove_last();
	View::append((const char*)text, size);
}

void
Term::replace_last(const char *text)
{
	remove_last();
	View::append((const char*)text);
}

/*
void
Term::append(const char *text, int sz)
{
	remove_last();
	View::append(text, sz);
//	View::append((const char*)input, inp_len);
}
*/

void
Term::show()
{
	View::show();

	int hh = h;
	int yy = y;
	int xx = x;

	struct line_t *pos = cur;

	for (;pos && hh > 0; pos = pos->next) {
		if (pos == last)
			break;
		yy ++;
		hh --;
	}
	if (pos != last)
		return;
	int c = 0;
	for (;pos && hh -- > 0; pos = pos->next) {
		for (int i = 0; i < pos->len + !(pos->next); i ++) {
			if (c == inp_cur) {
				if (i == pos->len && i >= w)
					scr.text_clear(xx, yy + 1, 1, 1, BG, FG);
				else
					scr.text_clear(xx + i, yy, 1, 1, BG, FG);
				if (i < w && i < pos->len)
					scr.text_glyph(xx + i, yy, pos->buf[i], BG, FG);
				scr.text_update();
				return;
			}
			c ++;
		}
		yy ++;
	}
}
