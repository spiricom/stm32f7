/* Includes ------------------------------------------------------------------*/
#include "audiostream.h"

#include "OOPSTest.h"

#include "codec.h"

#define AUDIO_BUFFER_SIZE             64 //four is the lowest number that makes sense -- 2 samples for each computed sample (L/R), and then half buffer fills
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
	//now to send all the necessary messages to the codec
	adcVals = myADCarray;
	
	AudioCodec_init(hi2c);
	HAL_Delay(100);
	
	
	// set up the I2S driver to send audio data to the codec (and retrieve input as well)	
	HAL_SAI_Transmit_DMA(hsaiIn, (uint8_t *)&audioOutBuffer[0], AUDIO_BUFFER_SIZE);
	HAL_SAI_Receive_DMA(hsaiOut, (uint8_t *)&audioInBuffer[0], AUDIO_BUFFER_SIZE);
	
	hrandom = hrand;
	
	OOPSInit(SAMPLE_RATE*4, &randomNumber);
	//tCycleInit(&mySine1);
	
	for (int i = 0; i < NUM_SINES; i++)
	{
		tCycleInit(&mySine[i]);
		tCycleSetFreq(&mySine[i], 80.0f * (i + 1.0f));
	}
	
	tPluckInit(&pluck1, 40.0f, pluckBuff1);
}



int counter = 0;
int flip = 1;
void audioFrame(uint16_t buffer_offset)
{
	uint16_t i = 0;
	int16_t current_sample = 0;  
	
	float base = ((4095 - adcVals[0]) >> 2) + 40.0f;
	
	if (++counter == 4000)
	{
		counter = 0;
		if (flip == 1)
		{
			flip = 0;
			tPluckSetFrequency(&pluck1, base);
			tPluckPluck(&pluck1, 1.0f);
		}
		else if (flip == 0)
		{
			flip = 1;
			tPluckSetFrequency(&pluck1, base + 0.5f * base);
			tPluckPluck(&pluck1, 1.0f);
		}
	}
	
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
	
	float sample = 0.0f;
	
	//sample = ((randomNumber() * 2.0f) - 1.0f);  // random between -1 and 1
	
	for (int i = 0; i < NUM_SINES; i++)
	{
		//sample += tCycleTick(&mySine[i]) * 0.05f;
	}
	
	//sample = tCycleTick(&mySine1);
	//sample = tNoiseTick(&myNoise);
	//sample *= tEnvelopeTick(&env);
	
	sample = audioIn;
	
	sample = 0.5f * tPluckTick(&pluck1);
	
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

