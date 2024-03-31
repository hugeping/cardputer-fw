#include "../../external.h"
#include "../../internal.h"

extern "C" {
	#include "../mp/port/micropython_embed.h"
	#include "../mp/py/repl.h"
}

static Term *g_repl = NULL;

static codepoint_t bucket[COLS-1];
static int bucket_len = 0;
static void

bucket_flush()
{
	if (!bucket_len)
		return;
	g_repl->append((const char*)bucket, bucket_len);
	bucket_len = 0;
}

extern "C" void mp_write(const char *str, int len)
{
	if (!len)
		return;
	char *nstr = (char*)alloca(len + 1);
	memcpy(nstr, str, len);
	nstr[len] = 0;
	len = utf8::len(nstr);
	Serial.println(String(len));
	for (int i = 0; i < len; i++) {
		codepoint_t cp;
		nstr = (char*)utf8::to_codepoint((const char*)nstr, &cp);
		if (cp == '\r')
			continue;
		bucket[bucket_len++] = cp;
		if (cp == '\n' || bucket_len >= COLS-1)
			bucket_flush();
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
	if (m == KEY_TAB) {
		const char *compl_str = NULL;
		ssize_t compl_len;

		char *l = t_repl.getinp();
		String p;
		t_repl.remove_last();
		compl_len = mp_repl_autocomplete(l, strlen(l),
			&mp_plat_print, &compl_str);

		bucket_flush();

		if (compl_len > 0) {
			char *s = (char*)alloca(compl_len + 1);
			memcpy(s, compl_str, compl_len);
			s[compl_len] = 0;
			p = String(l) + String(s);
		} else {
			Serial.println(String(compl_len));
			p = String(l);
		}
		t_repl.inp_append(p.c_str());
		free(l);
		t_repl.tail();
		t_repl.show();
	} else if (m == KEY_ENTER) {
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
			bucket_flush();
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