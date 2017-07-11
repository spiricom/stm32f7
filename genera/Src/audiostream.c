/* Includes ------------------------------------------------------------------*/
#include "audiostream.h"

#include "OOPSTest.h"

#include "codec.h"

#define AUDIO_BUFFER_SIZE             256 //four is the lowest number that makes sense -- 2 samples for each computed sample (L/R), and then half buffer fills
#define HALF_BUFFER_SIZE      (AUDIO_BUFFER_SIZE/2)

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

	myCompressor = tCompressorInit();
	myDelay = tDelayInit(0);
	filter = tSVFInit( SVFTypeBandpass, 2000,  100.0f);
	
	
	myCompressor->M = 0.0f;
	myCompressor->W = 24.0f;
	
	LN2 = log(2.0f);
	
}



int counter = 0;
int flip = 1;
int envTimeout;



float val; 

float knobs[6];

void audioFrame(uint16_t buffer_offset)
{
	uint16_t i = 0;
	int16_t current_sample = 0;  
	
	//int tauAttack, tauRelease;
  //float T, R, W, M; // Threshold, compression Ratio, decibel Width of knee transition, decibel Make-up gain
	for (int i = 0; i < 6; i++)	knobs[i] = (4096.0f - adcVals[i]) / 4096.0f;
	
	float bw0 = (float) pow(10,-7.0f*knobs[5]+2.0f); // sets the size of a normal bandwidth
	float n = 15*knobs[4]; // sets which harmonic to focus on 

	tDelaySetDelay(myDelay,256*knobs[0]);
	val =  1.0f/(LN2 * bw0 ) -LN2 *bw0/24.0f + ( 0.5f/(LN2*LN2* bw0) + bw0/48.0f)*(n-1);
	myCompressor->T = 0.550048828f * -60.0f ;
	myCompressor->R = knobs[1] * 24.0f ; 
	myCompressor->tauAttack = knobs[2] * 1024.0f;
	myCompressor->tauRelease = knobs[3] * 1024.0f;
	
	
	tSVFSetFreq(filter, 58.27f*n);
	tSVFSetQ(filter,val );
	
	if (myCompressor->isActive) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
	else												HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
	
	
	
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


float audioTick(float audioIn) {
	
	float sample = audioIn;
	
	
	sample = tCompressorTick(myCompressor, sample);
	
	
	sample = tSVFTick(filter, sample);
	
	sample = tDelayTick(myDelay, sample);
	
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

