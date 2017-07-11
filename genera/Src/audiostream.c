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

float audioTickL(float audioIn);
float audioTickR(float audioIn);
float randomNumber(void);
void audioFrame(uint16_t buffer_offset); // the function that gets called every audio frame

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
	
	//set the ADC input array to point to where the ADC stores its data
	adcVals = myADCarray;
	
	// set up the I2S driver to send audio data to the codec (and retrieve input as well)	
	HAL_SAI_Transmit_DMA(hsaiIn, (uint8_t *)&audioOutBuffer[0], AUDIO_BUFFER_SIZE);
	HAL_SAI_Receive_DMA(hsaiOut, (uint8_t *)&audioInBuffer[0], AUDIO_BUFFER_SIZE);
	
	myRamp = tRampInit(100.0f, (48000.0f / (float)AUDIO_FRAME_SIZE));
	/*
	saw[0] = tSawtoothInit(); //these are declared in OOPSTest.h, and memory is allocated for them in OOPSMemConfig.h
	saw[1] = tSawtoothInit();
	
	tSawtoothSetFreq(saw[0], 440.0f);
	hat = t808HihatInit();
	comp = tCompressorInit(15.f, 15.f);
	myRamp = tRampInit(100.0f, (48000.0f / (float)AUDIO_FRAME_SIZE));
	*/
	breath_baseline = ((adcVals[4] * INV_TWO_TO_12) + 0.1f);
	breath_mult = 1.0f / (1.0f-breath_baseline);
	
}

static int ij;

static int frameCount = 0;
static int audioBusy = 0;
static int gateIn = 0;
static int onsetFlag = 0;
int16_t currentInput = 0;
int16_t currentOutput = 0;

float amplitude = 0.0f;
float rampedAmp = 0.0f;

void audioFrame(uint16_t buffer_offset)
{
	int16_t current_sample = 0;  
	
	amplitude = (float)adcVals[4];
	//amplitude = 1.0f;
	
	amplitude = amplitude * INV_TWO_TO_12;
	//amplitude = amplitude - breath_baseline;
	//amplitude = amplitude * breath_mult;
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

	for (ij = 0; ij < (HALF_BUFFER_SIZE); ij++)
	{
		
		if ((ij & 1) == 0) 
		{
			//Left channel input and output
			current_sample = (int16_t)(audioTickL((float) (audioInBuffer[buffer_offset + ij] * INV_TWO_TO_15)) * TWO_TO_15);
		} 
		else 
		{
			//Right channel input and output
			//current_sample = (int16_t)(audioTickR((float) (audioInBuffer[buffer_offset + ij] * INV_TWO_TO_15)) * TWO_TO_15);
		}
		
		//fill the buffer with the new sample that has just been calculated
		audioOutBuffer[buffer_offset + ij] = current_sample;
		//audioOutBuffer[buffer_offset + ij] = ((((float)audioInBuffer[buffer_offset + ij] * INV_TWO_TO_15) * 0.9f) * TWO_TO_15);
		//currentInput = audioInBuffer[buffer_offset + ij];
		//currentOutput = audioOutBuffer[buffer_offset + ij];
	}
}

float audioTickL(float audioIn)
{
	
	float sample = 0.0f;
	sample = audioIn * rampedAmp;
	
	//sample = audioIn;
	return sample;
}

float audioTickR(float audioIn)
{
	
	float sample = 0.0f;
	sample = audioIn * rampedAmp;
	return sample;
	
}

void audioError(void)
{
		//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);//green
}

void audioClipped(void)
{
		//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);//red
}

void audioClippedMain(void)
{
		//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET); //blue
}



void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
	audioFrame(HALF_BUFFER_SIZE);
}

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
  audioFrame(0);
}

//currently the error callback does nothing
void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai)
{
	if (hsai->ErrorCode == HAL_SAI_ERROR_OVR)
	{
		;
	}
	else if (hsai->ErrorCode == HAL_SAI_ERROR_UDR)
	{
		;
	}
}
