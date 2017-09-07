/* Includes ------------------------------------------------------------------*/
#include "audiostream.h"

#include "OOPSTest.h"

#include "codec.h"

#define AUDIO_FRAME_SIZE      64
#define HALF_BUFFER_SIZE      AUDIO_FRAME_SIZE * 2 //number of samples per half of the "double-buffer" (twice the audio frame size because there are interleaved samples for both left and right channels)
#define AUDIO_BUFFER_SIZE     AUDIO_FRAME_SIZE * 4 //number of samples in the whole data structure (four times the audio frame size because of stereo and also double-buffering/ping-ponging)

/* Ping-Pong buffer used for audio play */
int16_t audioOutBuffer[AUDIO_BUFFER_SIZE];
int16_t audioInBuffer[AUDIO_BUFFER_SIZE];


uint16_t* adcVals;

// I think we need to grab these from main
/*
static HAL_DMA_StateTypeDef spi1tx;
static HAL_DMA_StateTypeDef spi1rx;
*/

float audioTick(float audioIn);
float randomNumber(void);
void audioFrame(uint16_t buffer_offset);
float LN2;



typedef enum BOOL {
	FALSE = 0,
	TRUE
} BOOL;

float breath_baseline = 0.0f;
float breath_mult = 0.0f;

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

	myCompressor = tCompressorInit();
	myDelay = tDelayInit(0);
	filter = tSVFInit( SVFTypeBandpass, 2000,  100.0f);
	
	
	myCompressor->M = 0.0f;
	myCompressor->W = 24.0f;
	
	LN2 = log(2.0f);
	
	myRamp = tRampInit(100.0f, (48000.0f / (float)AUDIO_FRAME_SIZE));
	breath_baseline = ((adcVals[3] * INV_TWO_TO_12) + 0.1f);
	breath_mult = 1.0f / (1.0f-breath_baseline);
	
	
	mySine = tCycleInit();
	
	tCycleSetFreq(mySine, 220.0f);
}



int counter = 0;
int flip = 1;
int envTimeout;



float val; 

float knobs[6];

// 1: delay
// 2: Ratio
// 3: Attack 
// 4: Release
// 5: harmonic focus
// 6: size of normal bandwidth 

float amplitude = 0.0f;
float rampedAmp = 0.0f;

float fixed_knobs[6] = {.699f, .816f, .2107f, .3579f, .7641f, .7269f};

void audioFrame(uint16_t buffer_offset)
{
	uint16_t i = 0;
	int16_t current_sample = 0;  
	
	//int tauAttack, tauRelease;
  //float T, R, W, M; // Threshold, compression Ratio, decibel Width of knee transition, decibel Make-up gain
	for (int i = 0; i < 6; i++)	knobs[i] = (4096.0f - adcVals[i]) / 4096.0f;
	
	float bw0 = (float) pow(10,-7.0f*fixed_knobs[5]+2.0f); // sets the size of a normal bandwidth
	
	float n = 15*((knobs[2] - 0.35f) * 3.0f); // sets which harmonic to focus on 

	tDelaySetDelay(myDelay,128);
	val =  1.0f/(LN2 * bw0 ) -LN2 *bw0/24.0f + ( 0.5f/(LN2*LN2* bw0) + bw0/48.0f)*(n-1);
	myCompressor->T = -6.0f;
	myCompressor->R = 2.0f ; 
	myCompressor->tauAttack = 200.0f;
	myCompressor->tauRelease = 400.0f;
	
	
	tSVFSetFreq(filter, 58.27f*n);
	tSVFSetQ(filter,val );
	
	//tSVFSetFreq(filter, (adcVals[2] - 900.0f));
	
	
	if (myCompressor->isActive) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
	else												HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
	
	
	amplitude = (float)adcVals[3];
	tCycleSetFreq(mySine, 1000);
	
	amplitude = amplitude * INV_TWO_TO_12;
	amplitude = amplitude - breath_baseline;
	amplitude = amplitude * breath_mult;
	
	//amplitude = 1.0f;
	
	if (amplitude < 0.0f)
	{
		amplitude = 0.0f;
	}
	else if (amplitude > 1.0f)
	{
		amplitude = 1.0f;
	}
	tRampSetDest(myRamp, amplitude);
	rampedAmp = tRampTick(myRamp);

	
	for (i = 0; i < (HALF_BUFFER_SIZE); i++)
	{
		if ((i & 1) == 0) {
			//current_sample = (int16_t)(audioTick((float) (audioInBuffer[buffer_offset + i] * INV_TWO_TO_15)) * TWO_TO_15);
			current_sample = (int16_t)(audioTick((float) (audioInBuffer[buffer_offset + i] * INV_TWO_TO_15)) * TWO_TO_15);
			//current_sample = (uint16_t) 0;
		} else {
			
			//FM_in = (float)(audioInBuffer[buffer_offset + i] * INV_TWO_TO_15);
		}
		audioOutBuffer[buffer_offset + i] = current_sample;
	}
	
}

float sample = 0.f;

float audioTick(float audioIn) {
	
	sample = audioIn;
  sample = tCycleTick(mySine);
	
	//sample = sample * rampedAmp;

	//sample = tCompressorTick(myCompressor, sample);
	
	//sample = tSVFTick(filter, sample);
	//sample = tDelayTick(myDelay, sample);
	
	
	sample *= 0.9f;
	
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

void HAL_I2S_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
	;
}

void HAL_I2S_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
	;
}

