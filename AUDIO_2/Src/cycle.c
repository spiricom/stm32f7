#include "cycle.h"


static int cFreq(tCycle *c, float freq) {	
	
	(c->tP).freq(&(c->tP),freq);
	
	return 0;
}

static float cStep(tCycle *c) {
	
	float phase =  (c->tP).step(&(c->tP));
	float temp = c->wtlen * phase;
	int intPart = (int)temp;
	float fracPart = temp - (float)intPart;
	int16_t samp0 = (int16_t)(c->wt[intPart]);
	if (++intPart >= c->wtlen) intPart = 0;
	int16_t samp1 = (int16_t)(c->wt[intPart]);
	return (samp0 + (samp1 - samp0) * fracPart);
}


int tCycleInit(tCycle *c, float sr, const int16_t *table, int len) {

	tPhasorInit(&(c->tP),sr);
	c->wt = table; 
	c->wtlen = len;
	c->freq = &cFreq;
	c->step = &cStep;
	
	return 0; 
}