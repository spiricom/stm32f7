/* Includes ------------------------------------------------------------------*/
#include "audiostream.h"

#include "OOPSTest.h"

#include "codec.h"

#define AUDIO_FRAME_SIZE      512
#define HALF_BUFFER_SIZE      AUDIO_FRAME_SIZE * 2 //number of samples per half of the "double-buffer" (twice the audio frame size because there are interleaved samples for both left and right channels)
#define AUDIO_BUFFER_SIZE     AUDIO_FRAME_SIZE * 4 //number of samples in the whole data structure (four times the audio frame size because of stereo and also double-buffering/ping-ponging)

/* Ping-Pong buffer used for audio play */
int16_t audioOutBuffer[AUDIO_BUFFER_SIZE];
int16_t audioInBuffer[AUDIO_BUFFER_SIZE];


uint16_t* adcVals;

float audioTickR(float audioIn);
float audioTickL(float audioIn);
float randomNumber(void);
void audioFrame(uint16_t buffer_offset);



typedef enum BOOL {
	FALSE = 0,
	TRUE
} BOOL;


RNG_HandleTypeDef* hrandom;

// Returns random floating point value [0.0,1.0)
float randomNumber(void) {
	
	uint32_t rand;
	HAL_RNG_GenerateRandomNumber(hrandom, &rand);
	float num = (((float)(rand >> 16))- 32768.f) * INV_TWO_TO_15;
	return num;
	
}



void audioInit(I2C_HandleTypeDef* hi2c, SAI_HandleTypeDef* hsaiIn, SAI_HandleTypeDef* hsaiOut, RNG_HandleTypeDef* hrand, uint16_t* myADCarray)
{ 
	// Initialize the audio library. OOPS.
	OOPSInit(SAMPLE_RATE, &randomNumber);
	
	//now to send all the necessary messages to the codec
	AudioCodec_init(hi2c);
	
	HAL_Delay(100);
	
	adcVals = myADCarray;
	// set up the I2S driver to send audio data to the codec (and retrieve input as well)	
	HAL_SAI_Transmit_DMA(hsaiIn, (uint8_t *)&audioOutBuffer[0], AUDIO_BUFFER_SIZE);
	HAL_SAI_Receive_DMA(hsaiOut, (uint8_t *)&audioInBuffer[0], AUDIO_BUFFER_SIZE);
	
	hrandom = hrand;
	
	sk = tStifKarpInit(220.0f, skBuff);
	for (int i = 0; i < NUM_OSC; i++)
	{
		saw[i] = tSawtoothInit();
	}
	
	//these are for if you want to comment in the super-simple test noise and sine wave instead of the 
	// more complex example
	//comment these in if you want them
	// if you want to use other elements in the OOPS library, add them in the OOPSTest file, and then init them like this
	/*
	noise = tNoiseInit(WhiteNoise);
	sine = tCycleInit();
	tCycleSetFreq(sine, 220.0f);
	*/
	
	svf = tSVFEInit(SVFTypeBandpass, 2096, 1.0);
	sawSvf = tSVFEInit(SVFTypeLowpass, 2096, 1.3);
	
	tStifKarpSetFrequency(sk, 110.0f);
	tStifKarpControlChange(sk, SKStringDamping, 60);
	tStifKarpControlChange(sk, SKDetune, 128.0f);
	tStifKarpControlChange(sk, SKPickPosition, 60);
	
	//these are used to smooth input parameters (knob settings for now)
	for (int i = 0; i < 9; i++)
	{
		ramp[i] = tRampInit(20.0f, AUDIO_FRAME_SIZE);
		tRampSetTime(ramp[i], 30.0f);
	}
}


int counter = 0;
int flip = 1;
int envTimeout;
float myAmp;

static int ij;

void audioFrame(uint16_t buffer_offset)
{
	uint16_t i = 0;
	int16_t current_sample = 0;  
	

	for (i = 0; i < NUM_OSC; i++)
	{
		tRampSetDest(ramp[i+4],((float)(((4095 - adcVals[i]) >> 2) + 40)));
		tSawtoothSetFreq(saw[i], tRampTick(ramp[i+4]));
	}
	
	tSVFESetFreq(svf, 4095 - adcVals[0]);
	tSVFESetFreq(sawSvf, 4095 - adcVals[5]);
	
	// update parameters of the Karplus Strong model based on knob positions
	tRampSetDest(ramp[0],((float)((4095-adcVals[4]) >> 3)+30));
	float karpFreq = tRampTick(ramp[0]);
	tStifKarpSetFrequency(sk, karpFreq);
	
	tRampSetDest(ramp[1],((float)((4095-adcVals[1]) >> 5)));
	float karpPick = tRampTick(ramp[1]);
	tStifKarpControlChange(sk, SKPickPosition, karpPick);
	
	tRampSetDest(ramp[2],((float)((4095-adcVals[2]) >> 5)));
	float karpDetune = tRampTick(ramp[2]);
	tStifKarpControlChange(sk, SKDetune, karpDetune);
	
	tRampSetDest(ramp[3],((float)((4095-adcVals[3]) >> 5)));
	float karpDamp = tRampTick(ramp[3]);
	tStifKarpControlChange(sk, SKStringDamping, karpDamp);
	
	uint16_t counterMax = (adcVals[5] >> 4);
	if (counter++ >= counterMax) 
	{
		counter = 0;		
		tStifKarpPluck(sk, 0.99f);
	}	
 
	for (ij = 0; ij < (HALF_BUFFER_SIZE); ij++)
	{
		if ((ij & 1) == 0) 
		{
			//Left channel input and output
			//current_sample = (int16_t)(tNoiseTick(noise) * TWO_TO_15); //comment this in instead for simple white noise
			current_sample = (int16_t)(audioTickL((float) (audioInBuffer[buffer_offset + ij] * INV_TWO_TO_15)) * TWO_TO_15);
		} 
		else 
		{
			//Right channel input and output
			//current_sample = (int16_t)(tCycleTick(sine) * TWO_TO_15); // comment this in instead for simple sine wave
			current_sample = (int16_t)(audioTickR((float) (audioInBuffer[buffer_offset + ij] * INV_TWO_TO_15)) * TWO_TO_15);
		}
		
		audioOutBuffer[buffer_offset + ij] = current_sample;
	}
	
}

float audioTickL(float audioIn)
{
	//use audioIn if you want the input sample	
	
	float sample = 0.0f;

	for (int i = 0; i < NUM_OSC; i++)
	{
		sample += 0.13f * tSawtoothTick(saw[i]); // sawtooth waves
	}

	sample =  tSVFETick(sawSvf, sample); //through a filter
	return sample;
}

float audioTickR(float audioIn)
{
	
	//use audioIn if you want the input sample
	
	float sample = 0.9f * tStifKarpTick(sk); //karplus strong pluck

	sample =  tSVFETick(svf, sample); //through a filter

	return sample;
	
}


void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
	audioFrame(HALF_BUFFER_SIZE);
}

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
  audioFrame(0);
}

// the recieve callbacks are synchronous with the transmit ones, and are therefore not needed
void HAL_I2S_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
	;
}

void HAL_I2S_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
	;
}

