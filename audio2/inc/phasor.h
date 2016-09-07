#include "main.h"

/* Phasor class */
typedef struct _tPhasor {
	float phase; // [0.0, 1.0)
	float inc;
	int(*freq)(struct _tPhasor *self, float freq);
	float(*step)(struct _tPhasor *self);
	//uint16_t(*samp)(struct _tPhasor *self);
} tPhasor;

int tPInit(tPhasor *p);
