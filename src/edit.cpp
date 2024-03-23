#include "../external.h"
#include "../internal.h"

static char *text_buf = NULL;
static int text_size = -1;

Edit::Edit(Screen &screen, Keyboard &keys, int sz) : scr(screen), kbd(keys), size(sz)
{
	buf = new codepoint_t[sz+1];
	set("");
}

Edit::Edit(Screen &screen, Keyboard &keys, const char *text) : scr(screen), kbd(keys)
{
	size = utf8::len(text);
	buf = new codepoint_t[size+1];
	set(text);
}

Edit::~Edit()
{
	delete(buf);
	free(text_buf);
	text_buf = NULL;
	text_size = 0;
}

const char *
Edit::text()
{
	int sz = 0;
	for (int i = 0; i < len; i ++)
		sz += utf8::from_codepoint(buf[i]);
	if (sz > text_size) {
		if (text_buf)
			free(text_buf);
		text_buf = (char*)malloc(sz + 1);
		text_size = sz;
	}
	char *ptr = text_buf;
	for (int i = 0; i < len; i ++)
		ptr += utf8::from_codepoint(buf[i], ptr);
	*ptr = 0;
	return text_buf;
}

void
Edit::set(const char *text)
{
	const char *ptr = text;
	codepoint_t cp;
	int i = 0;
	while (*ptr && i < size) {
		ptr = utf8::to_codepoint(ptr, &cp);
		buf[i++] = cp;
	}
	buf[i] = 0;
	len = i;
	cur = 0;
}

void
Edit::up()
{
	int skip = w;
	while (cur>0 && skip --) {
		cur --;
		if (buf[cur] == '\n')
			break;
	}
}

void
Edit::down()
{
	cur = utf8::fmt_down(buf, cur, len, w);
}

void
Edit::visible()
{
	if (cur_visible)
		return;
	if (oneline) {
		if (cur < off)
			off = cur - 1;
		else
			off = cur - w + 1;
		off = max(0, off);
		off = min(off, len);
		show();
		return;
	}
	if (cur < off)
		off = utf8::fmt_up(buf, off, w);
	else
		off = utf8::fmt_down(buf, off, len, w);
	show();
}

int
Edit::process()
{
	uint8_t c;
	codepoint_t cp;
	const char *sym = NULL;
	int ret = APP_NOP;
	bool dirty = false;
	while ((c = kbd.input(&sym)) && ret == APP_NOP) {
		if (c == KEY_ENTER && !oneline) {
			sym = "\n";
		}
		if (sym && len < size) {
			memmove(&buf[cur+1], &buf[cur], sizeof(codepoint_t)*(len-cur+1));
			utf8::to_codepoint(sym, &cp);
			buf[cur] = cp;
			cur ++;
			len ++;
			dirty = true;
			c = -1;
			ret = 0;
		}
		switch(c){
		case KEY_DOWN:
			if (oneline) {
				ret = c;
				break;
			}
			down();
			dirty = true;
			break;
		case KEY_UP:
			if (oneline) {
				ret = c;
				break;
			}
			up();
			dirty = true;
			break;
		case KEY_LEFT:
			cur --;
			dirty = true;
			break;
		case KEY_RIGHT:
			cur ++;
			dirty = true;
			break;
		case KEY_BS:
			if (cur > 0 && len >0) {
				memmove(&buf[cur-1], &buf[cur], sizeof(codepoint_t)*(len-cur+1));
				cur --;
				len --;
				dirty = true;
			}
			break;
		case KEY_ESC:
			ret = APP_EXIT;
			break;
		case KEY_ENTER:
			ret = c;
			break;
		default:
			break;
		}
	}
	cur = max(0, cur);
	cur = min(cur, len);
	if (dirty) {
		show();
		visible();
	}
	return ret;
}

void
Edit::start()
{
	kbd.set_fn_mode(false);
	App::start();
	show();
}

void
Edit::show()
{
	int hh = h;
	int yy = 0;
	int xx = 0;
	cur_visible = false;
	scr.text_clear(x, y, w, h);
	for (int pos = off; pos <= len && yy < h;) {
		codepoint_t cp = buf[pos];
		int ox = xx + x;
		int oy = yy + y;
		bool atcur = (cur == pos); // && active;
		cur_visible |= atcur;
		scr.text_glyph(ox, oy, 0, 0, (atcur)?FG:BG);
		if (utf8::fmt_next(buf, &pos, len, w, &xx, &yy))
			scr.text_glyph(ox, oy, cp,
				(atcur)?BG:FG, (atcur)?FG:BG);
	}
	scr.text_update();
}
