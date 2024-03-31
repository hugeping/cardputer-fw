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
	KEY_PGUP,
	KEY_PGDOWN,
};

struct keysym_t {
	uint8_t code;
	uint8_t ctrl_code;
	struct layout {
		const char *sym;
		const char *shift_sym;
		uint8_t fn_code;
	} sym[2];
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
