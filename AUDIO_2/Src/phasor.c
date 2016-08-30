#include "phasor.h"


static int pFreq(tPhasor *p, float freq) {
	
	p->inc = (1.0 / ((float)AUDIO_FREQ / freq))/2.0;
	
	return 0;
}

static uint16_t pStep(tPhasor *p) {
	
	p->phase += p->inc;
	
	if (p->phase >= 1.0) p->phase -= 1.0; 
	
	return (uint16_t)(p->phase*0xFFFF);
	
}


/*
static uint16_t pSamp(tPhasor *p) {
	
	return (uint16_t)(p->phase*0xFFFF);

}
*/

int tPInit(tPhasor *p) {
	p->phase = (float)0.0;
	p->inc = (float)0.0;
	p->freq = &pFreq;
	p->step = &pStep;
	//p->samp = &pSamp;
	
	return 0; 
}
