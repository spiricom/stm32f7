#include "OOPS.h"

#define NUM_OSC 16

#define CUTOFF_DIFF (58.27f * 0.5f)


tCompressor* myCompressor;
tDelay* myDelay;

tSVF* oldFilter;
tButterworth* filter;
tRamp* myRamp;
tCycle* mySine;
tRamp* freqRamp;



