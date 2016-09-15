
typedef struct _tCycle { 
	tPhasor tP;
	const int16_t *wt;
	int wtlen;
	int(*freq)(struct _tCycle *self, float freq);
	float(*step)(struct _tCycle *self);
} tCycle;

int tCycleInit(tCycle *c, float sr, const int16_t *table, int len);
