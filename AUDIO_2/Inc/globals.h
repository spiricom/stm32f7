#include "main.h"

#define AUDIO_OUT_BUFFER_SIZE_IN                  256
#define AUDIO_OUT_BUFFER_SIZE									(AUDIO_OUT_BUFFER_SIZE_IN + (4 - (AUDIO_OUT_BUFFER_SIZE_IN % 4)))
#define AUDIO_OUT_BUFFER_TX_SIZE									AUDIO_OUT_BUFFER_SIZE/2
#define SAMPLE_RATE 															AUDIO_FREQUENCY_48K
#define VOLUME 																		75

#define TWO_TO_15 32768.f

#define TP1_FREQ 200.0f

extern int gBufferSize; 
extern __IO uint32_t gVolume;
