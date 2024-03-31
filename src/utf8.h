#ifndef __UTF8_H_INCLUDED
#define __UTF8_H_INCLUDED
#include <stdint.h>
#include <stddef.h>
typedef uint16_t codepoint_t;
class utf8
{
public:
	static const char *to_codepoint(const char *p, codepoint_t *dst = NULL);
	static int from_codepoint(codepoint_t cp, char *out = NULL);
	static size_t len(const char *p);
	static const char *index(const char *p, int i);
	static int prev_line(codepoint_t *buf, int off);
	static int start_line(codepoint_t *buf, int off);
	static bool fmt_next(codepoint_t *buf, int *off, int len, int w, int *x, int *y);
	static int fmt_up(codepoint_t *buf, int off, int w);
	static int fmt_down(codepoint_t *buf, int off, int len, int w);
	static bool is_space(codepoint_t cp);
	static void rem_left(codepoint_t *edit, int *cur, int *len);
	static void insert(codepoint_t *edit, int *cur, int *len, codepoint_t cp);
};

#endif
