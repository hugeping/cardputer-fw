#include <string.h>
#include "utf8.h"
bool
utf8::is_space(codepoint_t cp)
{
	return cp == ' ' || cp == '\t';
}
int
utf8::start_line(codepoint_t *buf, int off)
{
	while (off--) {
		if (buf[off] == '\n')
			return off+1;
	}
	return off + 1;
}

int
utf8::prev_line(codepoint_t *buf, int off)
{
	off = start_line(buf, off);
	if (!off)
		return off;
	return start_line(buf, off - 1);
}

int
utf8::fmt_up(codepoint_t *buf, int off, int w)
{
	int noff = prev_line(buf, off);
	int xx = 0;
	int yy = 0;
	int lastoff = noff;
	while (noff < off) {
		int oy = yy;
		utf8::fmt_next(buf, &noff, off, w, &xx, &yy);
		if (yy != oy) {
			if (noff < off)
				lastoff = noff;
		}
	}
	return lastoff;
}
int
utf8::fmt_down(codepoint_t *buf, int off, int len, int skip)
{
	while (off < len && --skip) {
		if (buf[off++] == '\n')
			break;
	}
	return off;
}
bool
utf8::fmt_next(codepoint_t *buf, int *off, int len, int w, int *x, int *y)
{
	codepoint_t cp = buf[*off];
	(*off) ++;
	if (cp == '\n') {
		(*y) ++;
		*x = 0;
		return false;
	}
	(*x) ++;
	if (*x >= w) {
		//(*off) --;
		*x = 0;
		(*y) ++;
		return true;
	}
	return true;
}
int
utf8::from_codepoint(codepoint_t cp, char *out)
{
	if (cp <= 0x7F) {
		if (out) {
			out[0] = (char) cp;
		}
		return 1;
	} else if (cp <= 0x07FF) {
		// 2-byte unicode
		if (out) {
			out[0] = (char) (((cp >> 6) & 0x1F) | 0xC0);
			out[1] = (char) (((cp >> 0) & 0x3F) | 0x80);
		}
		return 2;
	}
#if 0
	else if (cp <= 0xFFFF) {
		// 3-byte unicode
		if (out) {
			out[0] = (char) (((cp >> 12) & 0x0F) | 0xE0);
			out[1] = (char) (((cp >>  6) & 0x3F) | 0x80);
			out[2] = (char) (((cp >>  0) & 0x3F) | 0x80);
		}
		return 3;
	} else if (cp <= 0x10FFFF) {
		// 4-byte unicode
		if (out) {
			out[0] = (char) (((cp >> 18) & 0x07) | 0xF0);
			out[1] = (char) (((cp >> 12) & 0x3F) | 0x80);
			out[2] = (char) (((cp >>  6) & 0x3F) | 0x80);
			out[3] = (char) (((cp >>  0) & 0x3F) | 0x80);
		}
		return 4;
	}
#endif
	return -1;
}
const char*
utf8::to_codepoint(const char *p, codepoint_t *dst)
{
	unsigned res, n;
	switch (*p & 0xf0) {
		case 0xf0 :  res = *p & 0x07;  n = 3;  break;
		case 0xe0 :  res = *p & 0x0f;  n = 2;  break;
		case 0xd0 :
		case 0xc0 :  res = *p & 0x1f;  n = 1;  break;
		default   :  res = *p;         n = 0;  break;
	}
	while (n-- && *p)
		res = (res << 6) | (*(++p) & 0x3f);
	if (dst)
		*dst = res;
	return p + 1;
}

size_t
utf8::len(const char *p)
{
	size_t sz = 0;
	while (*p) {
		p = to_codepoint(p);
		sz ++;
	}
	return sz;
}

const char *
utf8::index(const char *p, int pos)
{
	while (*p && pos --)
		p = to_codepoint(p);
	return p;
}

void
utf8::insert(codepoint_t *edit, int *cur, int *len, codepoint_t cp)
{
	memmove(&edit[*cur+1], &edit[*cur],
		sizeof(codepoint_t)*(*len-(*cur)+1));
	edit[*cur] = cp;
	(*cur) ++;
	(*len) ++;
}

void
utf8::rem_left(codepoint_t *edit, int *cur, int *len)
{

	memmove(&edit[*cur-1], &edit[*cur],
		sizeof(codepoint_t)*(*len-(*cur)+1));
	(*cur) --;
	(*len) --;
}