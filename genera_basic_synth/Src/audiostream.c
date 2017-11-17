/* Includes ------------------------------------------------------------------*/
#include "audiostream.h"

#include "codec.h"

#include "main.h"
#include "usb_host.h"
#include "usbh_core.h"
#include "usbh_MIDI.h"
#include "MIDI_application.h"

float gainPerVoice = 1.0f / NUM_VOICES;

tCycle* sine;
tSVF* filter;
tSawtooth* osc[NUM_VOICES];
tMPoly* poly;
tRamp* inputRamp[VocoboxInputNil];
uint16_t input[VocoboxInputNil];

#define AUDIO_FRAME_SIZE      1024
#define HALF_BUFFER_SIZE      AUDIO_FRAME_SIZE * 2 //number of samples per half of the "double-buffer" (twice the audio frame size because there are interleaved samples for both left and right channels)
#define AUDIO_BUFFER_SIZE     AUDIO_FRAME_SIZE * 4 //number of samples in the whole data structure (four times the audio frame size because of stereo and also double-buffering/ping-ponging)

/* Ping-Pong buffer used for audio play */
int16_t audioOutBuffer[AUDIO_BUFFER_SIZE];
int16_t audioInBuffer[AUDIO_BUFFER_SIZE];

uint64_t ADCfilterMemory[4] = {0,0,0,0};
uint16_t ADCfilteredValue = 0;

uint16_t* adcVals;

float audioTickL(float audioIn);
float audioTickR(float audioIn);
float randomNumber(void);
void audioFrame(uint16_t buffer_offset); // the function that gets called every audio frame

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
	
	//set the ADC input array to point to where the ADC stores its data
	adcVals = myADCarray;
	
	// set up the I2S driver to send audio data to the codec (and retrieve input as well)	
	HAL_SAI_Transmit_DMA(hsaiIn, (uint8_t *)&audioOutBuffer[0], AUDIO_BUFFER_SIZE);
	HAL_SAI_Receive_DMA(hsaiOut, (uint8_t *)&audioInBuffer[0], AUDIO_BUFFER_SIZE);
	
	
	poly = tMPoly_init();
	
	for (int i = 0; i < NUM_VOICES; i++)
	{
		osc[i] = tSawtoothInit();
	}

	for (int i = 0; i < (int)VocoboxInputNil; i++)
	{
		inputRamp[i] = tRampInit(10, 1);
	}
	
	filter = tSVFInit(SVFTypeLowpass, 500.0f, 5.0f);
	
	sine = tCycleInit();
	tCycleSetFreq(sine,220.0f);
}

static int ij;

static int frameCount = 0;
static int audioBusy = 0;
static int gateIn = 0;
static int onsetFlag = 0;

float rightIn = 0.0f;

void audioFrame(uint16_t buffer_offset)
{
	int16_t current_sample = 0;  
	
	if (audioBusy == 1)
	{
		audioError(); // this means it didn't finish the last round before it got interrupted again
	}
	audioBusy = 1;

	/*
	tRampSetDest(inputRamp[KnobOne],  HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0));
	tRampSetDest(inputRamp[KnobTwo],  HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1));
	tRampSetDest(inputRamp[KnobThree],  HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2));
	tRampSetDest(inputRamp[KnobFour],  HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4));
	tRampSetDest(inputRamp[Pedal],  HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3));
	*/
	
	//tSVFSetFreq(filter, input[KnobOne] << 1);
	//tSVFSetQ(filter, (float) input[KnobTwo] * INV_TWO_TO_12);
	
	for (ij = 0; ij < (HALF_BUFFER_SIZE); ij++)
	{
		if ((ij & 1) == 0) 
		{
			//Left channel input and output
			//current_sample = (int16_t)(audioTickL((float) (audioInBuffer[buffer_offset + ij] * INV_TWO_TO_15)) * TWO_TO_15);
		} 
		else 
		{
			//Right channel input and output
			current_sample = (int16_t)(audioTickR((float) (audioInBuffer[buffer_offset + ij] * INV_TWO_TO_15)) * TWO_TO_15);
		}
		//fill the buffer with the new sample that has just been calculated
		audioOutBuffer[buffer_offset + ij] = current_sample;
	}
	
	audioBusy = 0;
}

float audioTickL(float audioIn)
{

}

float audioTickR(float audioIn)
{
	/*
	for (int i = 0; i < VocoboxInputNil; i++)
	{
		input[i] = tRampTick(inputRamp[i]);
	}
	*/
	
	float sample = 0.0f;

	/*
	for (int8_t i = 0; i < NUM_VOICES; i++)
	{
			sample += tSawtoothTick(osc[i]) * poly->voices[i][1] * INV_TWO_TO_7 * gainPerVoice; 
		  //sample += tSawtoothTick(osc[i]) * gainPerVoice;	
	}
	*/
	
	//sample = tSVFTick(filter, sample) * 0.05f;
		
	sample = tCycleTick(sine);
	sample *= 0.05f;
	
	return sample;
}

void audioError(void)
{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);//green
}

void audioClipped(void)
{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);//red
}

void audioClippedMain(void)
{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET); //blue
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
