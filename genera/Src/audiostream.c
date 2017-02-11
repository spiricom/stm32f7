/* Includes ------------------------------------------------------------------*/
#include "audiostream.h"

#include "OOPSTest.h"

#include "codec.h"

#define AUDIO_BUFFER_SIZE             128 //four is the lowest number that makes sense -- 2 samples for each computed sample (L/R), and then half buffer fills
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
	
	//sk = tStifKarpInit(220.0f, skBuff);
	for (int i = 0; i < 8; i++)
	{
		saw[i] = tSawtoothInit();
	}
	
	env = tEnvelopeInit(10.0f, 1000.0f, OFALSE);
	svf = tSVFEInit(SVFTypeBandpass, 2096, 1.0);
	
	//osc = tCycleInit();
	
	//tCycleSetFreq(osc, 215.0f);
	
	//tStifKarpSetFrequency(sk, 220.0f);
	
	//tStifKarpControlChange(sk, SKStringDamping, 60);
	//tStifKarpControlChange(sk, SKDetune, 128.0f);
	//tStifKarpControlChange(sk, SKPickPosition, 60);
}



int counter = 0;
int flip = 1;
int envTimeout;

void audioFrame(uint16_t buffer_offset)
{
	uint16_t i = 0;
	int16_t current_sample = 0;  
	
	//float base = ((4095 - adcVals[0]) >> 2) + 220.0f;
	
	//tCycleSetFreq(osc, ((4095 - adcVals[1]) >> 2) + 40.0f);
	
	for (i = 0; i < 7; i++)
		tSawtoothSetFreq(saw[i], ((4095 - adcVals[i])) + 40.0f);
	
	tSawtoothSetFreq(saw[7], 100.0f);
	//tPluckSetFrequency(sk, base);
	
	tSVFESetFreq(svf, 4095 - adcVals[7]);
	
	/*
	tStifKarpControlChange(sk, SKPickPosition, (4095-adcVals[1]) >> 5);
	tStifKarpControlChange(sk, SKDetune, (4095-adcVals[2]) >> 5);
	tStifKarpControlChange(sk, SKStringDamping, (4095-adcVals[3]) >> 5);
	*/
	
	if(!HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_6))
	{
		if (envTimeout == 0)
		{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
			tEnvelopeOn(env, 1.0);
		}
		envTimeout = 10;
	}
	
	
	if (envTimeout > 0)
	{
		envTimeout--;
	}
	else
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
	}
			
	if (++counter == 500) 
	{
		counter = 0;
		//tStifKarpPluck(sk, 0.75f);
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
	
	//sample = 0.5f * tStifKarpTick(sk);
	
	for (int i = 0; i < 2; i++)
	{
		//sample += 0.1f * tSawtoothTick(saw[i]);
	}
	//sample = sample * 0.0f;
	//+sample = sample * tEnvelopeTick(env);
	//sample = tSVFETick(svf, sample);
	//sample += 0.5f * tCycleTick(osc);
	
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

