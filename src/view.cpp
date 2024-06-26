#include "../external.h"
#include "../internal.h"

#define WRAP_BY_WORDS

View::View(Screen &screen, Keyboard &keys) :
	scr(screen), kbd(keys)
{
}

View::~View()
{
	reset();
}

void
View::set(const char *text)
{
	reset();
	append(text);
	cur = begin;
}

void
View::down(int nr)
{
	if (visible)
		return;
	for (int i = 0; i<nr; i++) {
		if (cur && cur->next)
			cur = cur->next;
		else
			break;
	}
}

void
View::up(int nr)
{
	for (int i = 0; i<nr; i++) {
		if (cur && cur->prev)
			cur = cur->prev;
		else
			break;
	}
}

int
View::process()
{
	uint8_t c;
	const char *sym = NULL;
	int ret = APP_NOP;
	bool dirty = false;
	while ((c = kbd.input(&sym)) && ret == APP_NOP) {
		switch(c){
		case KEY_DOWN:
			down();
			dirty = true;
			break;
		case KEY_PGDOWN:
		case KEY_RIGHT:
			down(h);
			dirty = true;
			break;
		case KEY_UP:
			up();
			dirty = true;
			break;
		case KEY_PGUP:
		case KEY_LEFT:
			up(h);
			dirty = true;
			break;
		case KEY_ESC:
			ret = APP_EXIT;
			Serial.println("View exit");
			break;
		case KEY_BS:
		case KEY_DEL:
		case KEY_TAB:
		case KEY_ENTER:
			ret = c;
			break;
		default:
			break;
		}
	}
	if (dirty)
		show();
	return ret;
}

void
View::free_line(struct line_t *ln)
{
	while (ln) {
		struct line_t *p = ln;
		ln = ln->next;
		free(p);
	}
}

void
View::reset()
{
	free_line(begin);
	begin = NULL;
	cur = NULL;
	last = NULL;
	lines_nr = 0;
}

bool
View::fmt_next(codepoint_t *buf, int last, int *off, int len, int *xx, int *yy)
{
	codepoint_t cp = buf[*off];
	if (cp == '\n') {
		(*yy) ++;
		*xx = 0;
		(*off) ++;
		return false;
	}
#ifdef WRAP_BY_WORDS
	if (wrap_by_words) {
		int begin = utf8::start_line(buf + last, *off - last) + last;
		bool words = false;

		for (int i = begin; i < *off; i ++) {
			if (utf8::is_space(buf[i])) {
				words = true;
				break;
			}
		}

		for (int i = 0; words && *off + i < len; i ++) {
			if (*xx + i >= w) {
				*xx = 0;
				(*yy) ++;
				return false;
			}
			cp = buf[*off + i];
			if (utf8::is_space(cp) || cp == '\n')
				break;
		}
	}
#endif
	(*xx) ++;
	(*off) ++;
	if (*xx >= w) {
		*xx = 0;
		(*yy) ++;
	}
	return true;
}

struct View::line_t *
View::alloc_line(const void *text, int size)
{
	int sz = size;
	codepoint_t *buf;
	if (size < 0)
		sz = utf8::len((const char*)text);
	codepoint_t cp;
	struct line_t *ln = (struct line_t*)malloc(sizeof(struct line_t)
		+ (sz+1)*sizeof(codepoint_t));
	int i;
	if (!ln)
		return NULL;
	struct line_t *first = ln;
	ln->next = NULL;
	ln->prev = NULL;
	ln->len = 0;
	ln->chunks = 1;
	int *chunks = &ln->chunks;
	buf = (codepoint_t *)(ln + 1);
	ln->buf = buf;
	if (size >= 0) {
		for(i = 0; i<size; i++)
			buf[i] = ((codepoint_t*)text)[i];
	} else {
		const char *p = (const char*)text;
		for(i = 0; i<sz && *p; i++) {
			p = utf8::to_codepoint(p, &cp);
			buf[i] = cp;
		}
	}
	int xx = 0;
	int yy = 0;
	i = 0;
	int last_i = 0;
	while (i < sz) {
		int oy = yy;
		fmt_next(buf, last_i, &i, sz, &xx, &yy);
		if (yy > oy || i == sz) {
			ln->len = i - last_i;
			ln->next = NULL;
			last_i = i;
			if (i < sz) {
				struct line_t *nl = (struct line_t*)malloc(sizeof(struct line_t));
				if (!nl) {
					free_line(ln);
					return NULL;
				}
				nl->chunks = 0;
				nl->buf = buf + i;
				nl->prev = ln;
				ln->next = nl;
				ln = nl;
				(*chunks) ++;
			}
		}
	}
	return first;
}

void
View::append(const void *text, int sz)
{
	struct line_t *ln = alloc_line(text, sz);
	if (!ln)
		return;

	lines_nr += ln->chunks;
	if (!begin) {
		begin = ln;
		ln->prev = NULL;
		cur = ln;
		last = ln;
	} else {
		struct line_t *p = last;
		while (p->next) p = p->next;
		p->next = ln;
		ln->prev = p;
		last = ln;
	}
}

void
View::trim_head(int nr)
{
	if (lines_nr <= nr)
		return;
	nr = lines_nr - nr;
	struct line_t *pos = begin;
	while (pos && nr > 0) {
		int chunks = pos->chunks;
		struct line_t *p = pos;
		pos = pos->next;
		free(p);
		nr --;
		lines_nr --;
		if (chunks)
			chunks --;
		while (chunks --) {
			p = pos;
			pos = pos->next;
			free(p);
			nr --;
			lines_nr --;
		}
		begin = pos;
	}
	cur = begin;
	if (cur)
		cur->prev = NULL;
}

void
View::tail()
{
	struct line_t *pos;
	for(pos = cur; pos && pos->next; pos = pos->next);
	if (!pos)
		return;
	for (int hh = h; pos && pos->prev && --hh > 0; pos = pos->prev);
	cur = pos;
}

void
View::show()
{
	int hh = h;
	int yy = y;
	int xx = x;
	scr.text_clear(x, y, w, h);
	struct line_t *pos = cur;
	for (;pos && hh -- > 0; pos = pos->next) {
		for (int i = 0; i < pos->len && i < w; i ++) {
			scr.text_clear(xx + i, yy, 1, 1);
			scr.text_glyph(xx + i, yy, pos->buf[i]);
		}
		yy ++;
	}
	visible = !pos;
	scr.text_update();
}
