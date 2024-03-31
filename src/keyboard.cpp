#include "keyboard.h"

static keysym_t row0[] = {
	{ 1, 0, { {"`", "~", KEY_ESC}, { "ё" ,"Ё", KEY_ESC} } },
	{ 2, 0, { {"1", "!", 0 }, { "1" ,"!", 0 } } },
	{ 3, 0, { {"2", "@", 0 }, { "2" ,"\"", 0 } } },
	{ 4, 0, { {"3", "#", 0 }, { "3" ,"#", 0 } } },
	{ 5, 0, { {"4", "$", 0 }, { "4" ,";", 0 } } },
	{ 6, 0, { {"5", "%", 0 }, { "5" ,"%", 0 } } },
	{ 7, 0, { {"6", "^", 0 }, { "6" ,":", 0 } } },
	{ 8, 0, { {"7", "&", 0 }, { "7" ,"?", 0 } } },
	{ 9, 0, { {"8", "*", 0 }, { "8" ,"*", 0 } } },
	{ 10, 0, { {"9", "(", 0 }, { "9" ,"(", 0 } } },
	{ 11, 0, { {"0", ")", 0 }, { "0" ,")", 0 } } },
	{ 12, 0, { {"-", "_", 0 }, { "-" ,"_", 0 } } },
	{ 13, 0, { {"=", "+", 0 }, { "=" ,"+", 0 } } },
	{ KEY_BS, 0, { {NULL, NULL, KEY_DEL}, { NULL, NULL, KEY_DEL} } },
};

static keysym_t row1[] = {
	{ KEY_TAB, 0, { {}, {} } },
	{ 16, 0, { {"q", "Q", 0}, { "й" ,"Й", 0} } },
	{ 17, 0, { {"w", "W", 0}, { "ц" ,"Ц", 0} } },
	{ 18, 0, { {"e", "E", 0}, { "у" ,"У", 0} } },
	{ 19, 0, { {"r", "R", 0}, { "к" ,"К", 0} } },
	{ 20, 0, { {"t", "T", 0}, { "е" ,"Е", 0} } },
	{ 21, 0, { {"y", "Y", 0}, { "н" ,"Н", 0} } },
	{ 22, 0, { {"u", "U", 0}, { "г" ,"Г", 0} } },
	{ 23, 0, { {"i", "I", 0}, { "ш" ,"Ш", 0} } },
	{ 24, 0, { {"o", "O", 0}, { "щ" ,"Щ", 0} } },
	{ 25, 0, { {"p", "P", 0}, { "з" ,"З", 0} } },
	{ 26, 0, { {"[", "{", 0}, { "х" ,"Х", 0} } },
	{ 27, 0, { {"]", "}", 0}, { "ъ" ,"Ъ", 0} } },
	{ 28, 0, { {"\\", "|", 0}, { "\\" ,"/", 0} } },
};

static keysym_t row2[] = {
	{ KEY_FN, 0, { {}, {} } },
	{ KEY_SHIFT, 0, { {}, {} } },
	{ 31, 0, { {"a", "A", 0}, { "ф" ,"Ф", 0} } },
	{ 32, 0, { {"s", "S", 0}, { "ы" ,"Ы", 0} } },
	{ 33, 0, { {"d", "D", 0}, { "в" ,"В", 0} } },
	{ 34, 0, { {"f", "F", 0}, { "а" ,"А", 0} } },
	{ 35, 0, { {"g", "G", 0}, { "п" ,"П", 0} } },
	{ 36, 0, { {"h", "H", 0}, { "р" ,"Р", 0} } },
	{ 37, 0, { {"j", "J", 0}, { "о" ,"О", 0} } },
	{ 38, 0, { {"k", "K", 0}, { "л" ,"Л", 0} } },
	{ 39, 0, { {"l", "L", 0}, { "д" ,"Д", 0} } },
	{ 40, KEY_PGUP, { {";", ":", KEY_UP}, { "ж" ,"Ж", KEY_UP} } },
	{ 41, 0, { {"'", "\"", 0}, { "э" ,"Э", 0} } },
	{ KEY_ENTER, 0, { {}, {} } },
};

static keysym_t row3[] = {
	{ KEY_CTRL, 0, { {}, {} } },
	{ KEY_OPT, 0, { {}, {} } },
	{ KEY_ALT, 0, { {}, {} } },
	{ 46, 0, { {"z", "Z", 0}, { "я" ,"Я", 0} } },
	{ 47, 0, { {"x", "X", 0}, { "ч" ,"Ч", 0} } },
	{ 48, 0, { {"c", "C", 0}, { "с" ,"С", 0} } },
	{ 49, 0, { {"v", "V", 0}, { "м" ,"М", 0} } },
	{ 50, 0, { {"b", "B", 0}, { "и" ,"И", 0} } },
	{ 51, 0, { {"n", "N", 0}, { "т" ,"Т", 0} } },
	{ 52, 0, { {"m", "M", 0}, { "ь" ,"Ь", 0} } },
	{ 53, 0, { {",", "<", KEY_LEFT}, { "б" ,"Б", KEY_LEFT} } },
	{ 54, KEY_PGDOWN, { {".", ">", KEY_DOWN}, { "ю" ,"Ю", KEY_DOWN} } },
	{ 55, 0, { {"/", "?", KEY_RIGHT}, { "." ,",", KEY_RIGHT} } },
	{ KEY_SPACE, 0, { { " ", " ", KEY_MENU}, { " ", " ", KEY_MENU} } },
};


static const keysym_t *keymap[] =  { row0, row1, row2, row3 };
uint8_t Keyboard::state[4][14];


Keyboard::Keyboard()
{
	inp_fifo = { 0, 0, { }};
}

static const uint8_t out_list[3] = {8, 9, 11};
static const uint8_t in_list[7]  = {13, 15, 3, 4, 5, 6, 7};
static void set_output(uint8_t output)
{
	output &= 0x7;
	gpio_set_level((gpio_num_t)out_list[0], output & 0x1);
	gpio_set_level((gpio_num_t)out_list[1], output & 0x2);
	gpio_set_level((gpio_num_t)out_list[2], output & 0x4);
}

static uint8_t get_input()
{
	uint8_t ret = 0;
	for (int i = 0; i < 7; i++) {
		uint8_t v = (gpio_get_level((gpio_num_t)in_list[i]) == 1) ? 0x00 : 0x01;
		ret |= (v << i);
	}
	return ret;
}

void
Keyboard::setup()
{
	gpio_set_direction((gpio_num_t)0, GPIO_MODE_INPUT);
        gpio_set_pull_mode((gpio_num_t)0, GPIO_PULLUP_ONLY);

	for (int i = 0; i < 3; i ++) {
		gpio_num_t pin = (gpio_num_t)out_list[i];
		gpio_reset_pin(pin);
	        gpio_set_direction(pin, GPIO_MODE_OUTPUT);
        	gpio_set_pull_mode(pin, GPIO_PULLUP_PULLDOWN);
        	gpio_set_level(pin, 0);
	}
	for (int i = 0; i < 7; i ++) {
		gpio_num_t pin = (gpio_num_t)in_list[i];
		gpio_reset_pin(pin);
	        gpio_set_direction(pin, GPIO_MODE_INPUT);
        	gpio_set_pull_mode(pin, GPIO_PULLUP_ONLY);
	}
	set_output(0);
}

void
Keyboard::inp_add(uint8_t code, const char *sym)
{
	int end = inp_fifo.end;
	inp_fifo.keys[end].code = code;
	inp_fifo.keys[end].sym = sym;
	end = (end + 1) % fifo_len;
	inp_fifo.end = end;
}

uint8_t
Keyboard::input(const char **sym)
{
	uint8_t c;
	int start = inp_fifo.start;
	c = inp_fifo.keys[start].code;
	if (c) {
		if (sym)
			*sym = inp_fifo.keys[start].sym;
		inp_fifo.keys[start].code = 0;
		start = (start + 1) % fifo_len;
		inp_fifo.start = start;
	}
	return c;
}

static const uint8_t X_map[7][3] = {{1, 0, 1},   {2, 2, 3},  {4, 4, 5},
	{8, 6, 7},   {16, 8, 9}, {32, 10, 11},
	{64, 12, 13}};

uint32_t
Keyboard::poll_keys(uint8_t state[4][14])
{
	int x, y;
	memset(state, 0, 4*14);
	if (!gpio_get_level((gpio_num_t)0))
		state[0][0] = 1;

	uint32_t sum = 0;
	for (int i = 0; i < 8; i++) {
		set_output(i);
		uint8_t inp = get_input();
		sum += inp;
		if (!inp)
			continue;
		for (int j = 0; j < 7; j ++) {
			if (!(inp & (1<<j)))
				continue;
			x = (i > 3)?X_map[j][1] : X_map[j][2];
			y = (i > 3)?(i-4):i;
			y = 3-y;
			state[y][x] = 1;
		}
	}
	return sum;
}

void
Keyboard::process_event(uint8_t state[4][14], uint8_t key, bool press)
{
	if (!press) {
		uint8_t code = keymap[key/14][key%14].code;
		repeat_code = 0;
		if (release_code == code && millis() - release_ms < 400) {
			if (code == KEY_FN)
				fn_mode ^= 1;
			else if (code == KEY_SHIFT)
				shift_mode ^= 1;
		}
		release_code = code;
		release_ms = millis();

		if (code == KEY_OPT)
			layout_toggle();
		return;
	}
	const char *s;
	const struct keysym_t *sym;

	uint8_t shift = get_shift();
	uint8_t fn = get_fn();
	uint8_t ctrl = get_ctrl();

	sym = &keymap[key/14][key%14];
	uint8_t code = sym->code;
//	Serial.println("Pressed: "+String(code));
	if(ctrl && sym->ctrl_code) {
		code = sym->ctrl_code;
		s = NULL;
	} else if (fn && sym->sym[layout].fn_code) {
		code = sym->sym[layout].fn_code;
		s = NULL;
	} else {
		if (!key && !gpio_get_level((gpio_num_t)0)) {
			/* home key? */
			code = KEY_ESC;
			s = NULL;
		} else if (shift)
			s = sym->sym[layout].shift_sym;
		else
			s = sym->sym[layout].sym;
	}
	inp_add(code, s);
	repeat_code = key;
	repeat_ms = millis();
}

void
Keyboard::layout_toggle()
{
	layout = layout ^ 1;
}

void
Keyboard::poll(void)
{
	static uint8_t new_state[4][14];
	uint8_t events[fifo_len];
	memset(events, 0, fifo_len);
	uint32_t sum = poll_keys(new_state);
	if (sum != old_poll) {
		old_poll = sum;
		poll_nr = poll_filter;
		return;
	}
	if (poll_nr --)
		return;
	poll_nr = poll_filter;
	int k = 0;
	for (int y = 0; y < 4 && k < fifo_len; y++) {
		for (int x = 0; x < 14 && k < fifo_len; x ++) {
			uint8_t old = state[y][x];
			uint8_t n = new_state[y][x];
			uint8_t code = y*14 + x;
			if (old != n) {
				process_event(new_state, code, n);
				state[y][x] = n;
			} else if (n &&
				repeat_code == code &&
				millis() - repeat_ms > 350) {
				process_event(new_state, code, true);
				repeat_ms -= 250; /* 50ms repeat */
			}
		}
	}
}
