#include "stdint.h"

/* Phasor class */
typedef struct _tPhasor {
	float phase; // [0.0, 1.0)
	float inc;
	float srate; 
	int(*freq)(struct _tPhasor *self, float freq);
	float(*step)(struct _tPhasor *self);
	//uint16_t(*samp)(struct _tPhasor *self);
} tPhasor; 

int tPhasorInit(tPhasor *p, float sr);

/* Cycle class */
typedef struct _tCycle { 
	tPhasor tP;
	const int16_t *wt;
	int wtlen;
	int(*freq)(struct _tCycle *self, float freq);
	float(*step)(struct _tCycle *self);
} tCycle;

int tCycleInit(tCycle *c, float sr, const int16_t *table, int len);
