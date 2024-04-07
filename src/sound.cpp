#include "sound.h"

void
Sound::setup()
{
	snd = new m5::Speaker_Class;
	auto cfg = snd->config();
	cfg.pin_data_out = 42;
	cfg.pin_bck = 41;
	cfg.pin_ws = 43;
	cfg.i2s_port = i2s_port_t::I2S_NUM_1;
	snd->config(cfg);
	snd->begin();
}

Sound::~Sound()
{
	snd->end();
	delete snd;
}
