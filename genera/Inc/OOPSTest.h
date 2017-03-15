#include "OOPS.h"

#define NUM_OSC 16

//tStifKarp* sk;
//float skBuff[2][DELAY_LENGTH];

tCycle* mySine;

tRamp* inputRamp[8];

// Feedback trombone signal chain
tSVF* svf;
tCompressor* myComp;
tDelayA* del;


//tCycle* osc;



