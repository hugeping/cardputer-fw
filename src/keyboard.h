#ifndef __KEYBOARD_H_INCLUDED
#define __KEYBOARD_H_INCLUDED
#include "../external.h"

enum keycode_t {
	KEY_ALT = 128,
	KEY_BS,
	KEY_CTRL,
	KEY_DEL,
	KEY_DOWN,
	KEY_ENTER,
	KEY_ESC,
	KEY_FN,
	KEY_LEFT,
	KEY_MENU,
	KEY_OPT,
	KEY_RIGHT,
	KEY_SHIFT,
	KEY_SPACE,
	KEY_TAB,
	KEY_UP,
};

struct keysym_t {
	uint8_t code;
	struct layout {
		const char *sym;
		const char *shift_sym;
		uint8_t fn_code;
	} sym[2];
};

static keysym_t row0[] = {
	{ 1, { {"`", "~", KEY_ESC}, { "ё" ,"Ё", KEY_ESC} } },
	{ 2, { {"1", "!"}, { "1" ,"!"} } },
	{ 3, { {"2", "@"}, { "2" ,"\""} } },
	{ 4, { {"3", "#"}, { "3" ,"#"} } },
	{ 5, { {"4", "$"}, { "4" ,";"} } },
	{ 6, { {"5", "%"}, { "5" ,"%"} } },
	{ 7, { {"6", "^"}, { "6" ,":"} } },
	{ 8, { {"7", "&"}, { "7" ,"?"} } },
	{ 9, { {"8", "*"}, { "8" ,"*"} } },
	{ 10, { {"9", "("}, { "9" ,"("} } },
	{ 11, { {"0", ")"}, { "0" ,")"} } },
	{ 12, { {"-", "_"}, { "-" ,"_"} } },
	{ 13, { {"=", "+"}, { "=" ,"+"} } },
	{ KEY_BS, { {NULL, NULL, KEY_DEL}, { NULL, NULL, KEY_DEL} } },
};

static keysym_t row1[] = {
	{ KEY_TAB, { {}, {} } },
	{ 16, { {"q", "Q"}, { "й" ,"Й"} } },
	{ 17, { {"w", "W"}, { "ц" ,"Ц"} } },
	{ 18, { {"e", "E"}, { "у" ,"У"} } },
	{ 19, { {"r", "R"}, { "к" ,"К"} } },
	{ 20, { {"t", "T"}, { "е" ,"Е"} } },
	{ 21, { {"y", "Y"}, { "н" ,"Н"} } },
	{ 22, { {"u", "U"}, { "г" ,"Г"} } },
	{ 23, { {"i", "I"}, { "ш" ,"Ш"} } },
	{ 24, { {"o", "O"}, { "щ" ,"Щ"} } },
	{ 25, { {"p", "P"}, { "з" ,"З"} } },
	{ 26, { {"[", "{"}, { "х" ,"Х"} } },
	{ 27, { {"]", "}"}, { "ъ" ,"Ъ"} } },
	{ 28, { {"\\", "|"}, { "\\" ,"/"} } },
};

static keysym_t row2[] = {
	{ KEY_FN, { {}, {} } },
	{ KEY_SHIFT, { {}, {} } },
	{ 31, { {"a", "A"}, { "ф" ,"Ф"} } },
	{ 32, { {"s", "S"}, { "ы" ,"Ы"} } },
	{ 33, { {"d", "D"}, { "в" ,"В"} } },
	{ 34, { {"f", "F"}, { "а" ,"А"} } },
	{ 35, { {"g", "G"}, { "п" ,"П"} } },
	{ 36, { {"h", "H"}, { "р" ,"Р"} } },
	{ 37, { {"j", "J"}, { "о" ,"О"} } },
	{ 38, { {"k", "K"}, { "л" ,"Л"} } },
	{ 39, { {"l", "L"}, { "д" ,"Д"} } },
	{ 40, { {";", ":", KEY_UP}, { "ж" ,"Ж", KEY_UP} } },
	{ 41, { {"'", "\""}, { "э" ,"Э"} } },
	{ KEY_ENTER, { {}, {} } },
};

static keysym_t row3[] = {
	{ KEY_CTRL, { {}, {} } },
	{ KEY_OPT, { {}, {} } },
	{ KEY_ALT, { {}, {} } },
	{ 46, { {"z", "Z"}, { "я" ,"Я"} } },
	{ 47, { {"x", "X"}, { "ч" ,"Ч"} } },
	{ 48, { {"c", "C"}, { "с" ,"С"} } },
	{ 49, { {"v", "V"}, { "м" ,"М"} } },
	{ 50, { {"b", "B"}, { "и" ,"И"} } },
	{ 51, { {"n", "N"}, { "т" ,"Т"} } },
	{ 52, { {"m", "M"}, { "ь" ,"Ь"} } },
	{ 53, { {",", "<", KEY_LEFT}, { "б" ,"Б", KEY_LEFT} } },
	{ 54, { {".", ">", KEY_DOWN}, { "ю" ,"Ю", KEY_DOWN} } },
	{ 55, { {"/", "?", KEY_RIGHT}, { "." ,",", KEY_RIGHT} } },
	{ KEY_SPACE, { { " ", " ", KEY_MENU}, { " ", " ", KEY_MENU} } },
};

class Keyboard
{
	static const int fifo_len = 8;
	static const int poll_filter = 8;

	uint8_t layout = 0;
	static uint8_t state[4][14];
	struct {
		int start;
		int end;
		struct {
			uint8_t code;
			const char *sym;
		} keys[fifo_len];
	} inp_fifo = { 0, 0, { }};
	long repeat_ms = 0;
	long release_ms = 0;
	uint32_t old_poll = 0;
	uint8_t poll_nr = poll_filter;
	uint8_t repeat_code = 0;
	uint8_t release_code = 0;
	void process_event(uint8_t state[4][14], uint8_t key, bool press);
	uint32_t poll_keys(uint8_t out[4][14]);
	void layout_toggle();
	bool fn_mode = false;
	bool shift_mode = false;
public:
	void set_fn_mode(bool m) { fn_mode = m; };
	void set_shift_mode(bool m) { shift_mode = m; };
	bool get_fn(void) { return state[2][0] ^ fn_mode; };
	bool get_shift(void) { return state[2][1] ^ shift_mode; };
	bool get_ctrl(void) { return state[3][0]; };
	bool get_alt(void) { return state[3][2]; };
	bool get_layout(void) { return layout; };
	Keyboard();
	void setup();
	uint8_t input(const char **sym = NULL);
	void inp_add(uint8_t code, const char *sym);
	void poll();
};
#endif
