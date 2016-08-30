#include "phasor.h"


static int pFreq(tPhasor *p, float freq) {
	
<<<<<<< HEAD
	p->inc = (1.0f / ((float)AUDIO_FREQ / freq))/2.0f;
=======
	p->inc = (freq/(float)AUDIO_FREQ)/2.0f;
>>>>>>> 207f92201c03fee0f83c32fe41dc5f462fa75f0f
	
	return 0;
}

static float pStep(tPhasor *p) {
	
	p->phase += p->inc;
	
<<<<<<< HEAD
	if (p->phase >= 1.0f) p->phase -= 0.0f; 
=======
	if (p->phase >= 1.0f) p->phase -= 1.0f; 
>>>>>>> 207f92201c03fee0f83c32fe41dc5f462fa75f0f
	
	return p->phase;
	
}


/*
static uint16_t pSamp(tPhasor *p) {
	
	return (uint16_t)(p->phase*0xFFFF);

}
*/

int tPInit(tPhasor *p) {
<<<<<<< HEAD
	p->phase = (float)0.0f;
	p->inc = (float)0.0f;
=======
	p->phase = 0.0f;
	p->inc = 0.0f;
>>>>>>> 207f92201c03fee0f83c32fe41dc5f462fa75f0f
	p->freq = &pFreq;
	p->step = &pStep;
	//p->samp = &pSamp;
	
	return 0; 
}
