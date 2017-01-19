/* Includes ------------------------------------------------------------------*/
#include "audiostream.h"

#include "codec.h"

#define AUDIO_BUFFER_SIZE             64 //four is the lowest number that makes sense -- 2 samples for each computed sample (L/R), and then half buffer fills
#define HALF_BUFFER_SIZE      (AUDIO_BUFFER_SIZE/2)

/* Ping-Pong buffer used for audio play */
int16_t audioOutBuffer[AUDIO_BUFFER_SIZE];
int16_t audioInBuffer[AUDIO_BUFFER_SIZE];


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

/*
I2C_HandleTypeDef hi2c2;
DMA_HandleTypeDef hdma_i2c2_rx;
DMA_HandleTypeDef hdma_i2c2_tx;

QSPI_HandleTypeDef hqspi;

RNG_HandleTypeDef hrng;

SAI_HandleTypeDef hsai_BlockA1;
SAI_HandleTypeDef hsai_BlockB1;
DMA_HandleTypeDef hdma_sai1_a;
DMA_HandleTypeDef hdma_sai1_b;
*/

RNG_HandleTypeDef* hrandom;

void audioInit(I2C_HandleTypeDef* hi2c, SAI_HandleTypeDef* hsaiIn, SAI_HandleTypeDef* hsaiOut, RNG_HandleTypeDef* hrand)
{ 
	//now to send all the necessary messages to the codec
	AudioCodec_init(hi2c);
	HAL_Delay(100);
	
	// set up the I2S driver to send audio data to the codec (and retrieve input as well)	
	HAL_SAI_Transmit_DMA(hsaiIn, (uint8_t *)&audioOutBuffer[0], AUDIO_BUFFER_SIZE);
	HAL_SAI_Receive_DMA(hsaiOut, (uint8_t *)&audioInBuffer[0], AUDIO_BUFFER_SIZE);
	
	hrandom = hrand;
 
}

// Returns random floating point value [0.0,1.0)
float randomNumber(void) {
	
	uint32_t rand;
	HAL_RNG_GenerateRandomNumber(hrandom, &rand);
	float num = (((float)(rand >> 16))- 32768.f) * INV_TWO_TO_15;
	return num;
}


void audioFrame(uint16_t buffer_offset)
{
	uint16_t i = 0;
	int16_t current_sample = 0;  
	
	for (i = 0; i < (HALF_BUFFER_SIZE); i++)
	{
		if ((i & 1) == 0) {
			  //current_sample = (int16_t)(audioTick((float) (audioInBuffer[buffer_offset + i] * INV_TWO_TO_15)) * TWO_TO_15);
			current_sample = (int16_t)(audioTick((float) (audioInBuffer[buffer_offset + i] * INV_TWO_TO_15)) * TWO_TO_15);
		} else {
			
			//FM_in = (float)(audioInBuffer[buffer_offset + i] * INV_TWO_TO_15);
		}
		audioOutBuffer[buffer_offset + i] = current_sample;
	}
	
}


float audioTick(float audioIn) {
	
	float sample;
	sample = ((randomNumber() * 2.0f) - 1.0f);  // random between -1 and 1
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

