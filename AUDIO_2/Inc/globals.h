#include "main.h"

#define AUDIO_OUT_BUFFER_SIZE									256
#define AUDIO_OUT_BUFFER_TX_SIZE							AUDIO_OUT_BUFFER_SIZE/2
#define REV_BUFFER_INDEX											((AUDIO_OUT_BUFFER_TX_SIZE/2) - 1)
#define SAMPLE_RATE 													AUDIO_FREQUENCY_48K
#define VOLUME 																50

#define TWO_TO_15 32768.f

#define TP1_FREQ 220.0f

extern tPulse pulse1; 
extern tCycle sin1;
extern tCycle sin2;
extern tSawtooth saw1;
extern tTriangle tri1;

extern float sweep;

extern int16_t buff[AUDIO_OUT_BUFFER_SIZE];
extern float fsamp;

extern int INDEX;
extern int idx;
extern int gBufferSize; 

extern int16_t testWave;

extern __IO uint32_t gVolume;
