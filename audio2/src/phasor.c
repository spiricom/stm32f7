#include "phasor.h"


static int pFreq(tPhasor *p, float freq) {

	p->inc = (freq/(float)AUDIO_FREQ)/2.0f;

	return 0;
}

static float pStep(tPhasor *p) {

	p->phase += p->inc;

	if (p->phase >= 1.0f) p->phase -= 1.0f;

	return p->phase;

}


/*
static uint16_t pSamp(tPhasor *p) {

	return (uint16_t)(p->phase*0xFFFF);

}
*/

int tPInit(tPhasor *p) {
	p->phase = 0.0f;
	p->inc = 0.0f;
	p->freq = &pFreq;
	p->step = &pStep;
	//p->samp = &pSamp;

	return 0;
}
