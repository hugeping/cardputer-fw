#include "../../external.h"
#include "../../internal.h"

extern "C" {
	#include "../mp/port/micropython_embed.h"
	#include "../mp/py/repl.h"
}

static Term *g_repl = NULL;

static codepoint_t bucket[COLS-1];
static int bucket_len = 0;

extern "C" void mp_write(const char *str, int len)
{
	Serial.write(str, len);
//	Serial.println(str);
	for (int i = 0; i < len; i++) {
		codepoint_t cp;
		str = utf8::to_codepoint(str, &cp);
		if (cp == '\r')
			continue;
		bucket[bucket_len++] = cp;
		if (cp == '\n' || bucket_len >= COLS-1) {
			g_repl->append((const char*)bucket, bucket_len);
			bucket_len = 0;
		}
	}
}

Python::Python(Screen &scr, Keyboard &kbd) :
	t_repl(scr, kbd)
{
	t_repl.geom(0, 0, COLS-1, ROWS);
	g_repl = &t_repl;
}

static char *
strjoin(const char *s1, int sep_char, const char *s2)
{
	int l1 = strlen(s1);
	int l2 = strlen(s2);
	char *s = (char*)malloc(l1 + l2 + 2);
	memcpy(s, s1, l1);
	if (sep_char != 0) {
		s[l1] = sep_char;
		l1 += 1;
	}
	memcpy(s + l1, s2, l2);
	s[l1 + l2] = 0;
	return s;
}

int
Python::process()
{
	int m = App::process();
	if (m == APP_NOP || m == APP_EXIT)
		return m;
	if (m == KEY_ENTER) {
		char *l = t_repl.getinp();
		if (!line)
			line = l;
		else {
			char *nl = strjoin(line, '\n', l);
			free(line);
			line = nl;
		}
		if (mp_repl_continue_with_input(l)) { /* next */
			t_repl.prompt(". ");
			t_repl.tail();
			t_repl.show();
		} else {
			Serial.println(line);
			mp_embed_exec_str(line);
			free(line);
			line = NULL;
			if (bucket_len > 0) {
				g_repl->append((const char*)bucket, bucket_len);
				bucket_len = 0;
			}
			t_repl.prompt("> ");
			t_repl.tail();
			t_repl.show();
		}
	}
	return APP_NOP;
}

Python::~Python()
{
	mp_embed_deinit();
}

void
Python::start()
{
	mp_embed_init(&heap[0], sizeof(heap));
	App::start();
	t_repl.reset();
	t_repl.prompt("> ");
	push(&t_repl);
}

void
Python::resume()
{
	App::start();
	push(&t_repl);
}