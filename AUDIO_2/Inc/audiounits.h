#include "stdint.h"

/* Phasor [0.0, 1.0) */
typedef struct _tPhasor {
	float phase; 
	float inc;
	float srate; 
	int(*freq)(struct _tPhasor *self, float freq);
	float(*step)(struct _tPhasor *self);
} tPhasor; 

int tPhasorInit(tPhasor *p, float sr);

/* Sawtooth */
typedef struct _tSawtooth{ 
	tPhasor tP;
	int(*freq)(struct _tSawtooth *self, float freq);
	float(*step)(struct _tSawtooth *self);
} tSawtooth;

int tSawtoothInit(tSawtooth *t, float sr);

/* Triangle */
typedef struct _tTriangle{ 
	tPhasor tP;
	int(*freq)(struct _tTriangle *self, float freq);
	float(*step)(struct _tTriangle *self);
} tTriangle;

int tTriangleInit(tTriangle *t, float sr);

/* Pulse */
typedef struct _tPulse{ 
	tPhasor tP;
	float pw; 
	int(*pwidth)(struct _tPulse *self, float pwidth);
	int(*freq)(struct _tPulse *self, float freq);
	float(*step)(struct _tPulse *self);
} tPulse;

int tPulseInit(tPulse *t, float sr, float pwidth);

/* Cycle */
typedef struct _tCycle { 
	tPhasor tP;
	const float *wt; //wavetable
	int wtlen; //wavetable length
	int(*freq)(struct _tCycle *self, float freq);
	float(*step)(struct _tCycle *self);
} tCycle;

int tCycleInit(tCycle *c, float sr, const float *table, int len);

