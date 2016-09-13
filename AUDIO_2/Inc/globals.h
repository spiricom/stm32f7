#include "main.h"

#define AUDIO_OUT_BUFFER_SIZE_IN              256
#define AUDIO_OUT_BUFFER_SIZE									256
#define AUDIO_OUT_BUFFER_TX_SIZE							AUDIO_OUT_BUFFER_SIZE/2
#define REV_BUFFER_INDEX											((AUDIO_OUT_BUFFER_TX_SIZE/2) - 1)
#define SAMPLE_RATE 													AUDIO_FREQUENCY_48K
#define VOLUME 																75

#define TWO_TO_15 32768.f

#define TP1_FREQ 375.f

extern int16_t buff[AUDIO_OUT_BUFFER_SIZE];
extern int16_t samp;

extern int INDEX;
extern int FLAG1;
extern int idx;
extern int gBufferSize; 

extern __IO uint32_t gVolume;
