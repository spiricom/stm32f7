// Audio Units

#include "audiounits.h"

/* Phasor */
static int phasorFreq(tPhasor *p, float freq) {
	
	p->inc = (freq/p->srate);
	
	return 0;
}

static float phasorStep(tPhasor *p) {
	
	p->phase += p->inc;
	
	if (p->phase >= 1.0f) p->phase -= 1.0f; 

	return p->phase;
}

int tPhasorInit(tPhasor *p, float sr) {

	p->phase = 0.0f;
	p->inc = 0.0f;
	p->srate = sr;
	p->freq = &phasorFreq;
	p->step = &phasorStep;
	//p->samp = &pSamp;
	
	return 0; 
}

/* Sawtooth */
static int sawtoothFreq(tSawtooth *s, float freq) {	
	
	(s->tP).freq(&(s->tP),freq);
	
	return 0;
}

static int16_t sawtoothStep(tSawtooth *s) {
	float phase =  ((s->tP).step(&(s->tP)) * 2.0f) - 1.0f;
	return (int16_t)(phase * 0x8000); 
}


int tSawtoothInit(tSawtooth *s, float sr) {
	
	tPhasorInit(&(s->tP),sr);
	s->freq = &sawtoothFreq;
	s->step = &sawtoothStep;
	
	return 0; 
}

/* Triangle */
static int triangleFreq(tTriangle *t, float freq) {	
	
	(t->tP).freq(&(t->tP),freq);
	
	return 0;
}

static int16_t triangleStep(tTriangle *t) {
	float phase =  (t->tP).step(&(t->tP)) * 2.0f; 
	if (phase > 1.0f) phase = 2.0f - phase; 
	phase = (phase * 2.0f) - 1.0f;
	return (int16_t)(phase * 0x8000);
	
}


int tTriangleInit(tTriangle *t, float sr) {

	tPhasorInit(&(t->tP),sr);
	t->freq = &triangleFreq;
	t->step = &triangleStep;
	
	return 0; 
}

/* Square */
static int pulseWidth(tPulse *pl, float pwidth) {
	//pwidth [0.0, 1.0)
	float pw = pwidth;
	if (pw >= 1.0f) pw = 0.99f; 
	if (pw <= 0.0f) pw = 0.01f;
	pl->pw = (pw * 2.0f) - 1.0f; 
	return 0;
}

static int pulseFreq(tPulse *pl, float freq) {	
	
	(pl->tP).freq(&(pl->tP),freq);
	return 0;
}

static int16_t pulseStep(tPulse *pl) {
	float phase =  ((pl->tP).step(&(pl->tP)) * 2.0f)-1.0f; 
	if (phase < pl->pw) return (int16_t)INT16_MAX;
	else return (int16_t)INT16_MIN;
}


int tPulseInit(tPulse *pl, float sr, float pwidth) {
	
	tPhasorInit(&(pl->tP),sr);
	pl->pw = pwidth; 
	pl->pwidth = &pulseWidth;
	pl->freq = &pulseFreq;
	pl->step = &pulseStep;
	return 0; 
}


/* Cycle */
static int cFreq(tCycle *c, float freq) {	
	
	(c->tP).freq(&(c->tP),freq);
	
	return 0;
}

static int16_t cStep(tCycle *c) {
	
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

