#include "OOPS.h"

tStifKarp* sk;
float skBuff[2][DELAY_LENGTH];

tNoise* noise;

tCycle* sine;

#define NUM_OSC 5
tSawtooth* saw[NUM_OSC];
tEnvelope* env;
tSVFE* svf;
tSVFE* sawSvf;
tRamp* ramp[9];

tNRev* rev;
