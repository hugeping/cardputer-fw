#ifndef __SOUND_H_INCLUDED
#define __SOUND_H_INCLUDED
#include "../external.h"
#include "m5/Speaker_Class.hpp"

class Sound {
	m5::Speaker_Class *snd = NULL;
public:
	void set_vol(int v) {
		snd->setVolume(v);
	};
	void tone(int hz, int delay) {
		snd->tone(hz, delay);
	};
	void setup();
	~Sound();
};

#endif
