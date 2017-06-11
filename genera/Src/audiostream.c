/* Includes ------------------------------------------------------------------*/
#include "audiostream.h"

#include "OOPSTest.h"

#include "codec.h"

#define AUDIO_FRAME_SIZE      8
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
	
	sine[0] = tCycleInit(); //these are declared in OOPSTest.h, and memory is allocated for them in OOPSMemConfig.h
	sine[1] = tCycleInit();
	hat = t808HihatInit();
	comp = tCompressorInit(15.f, 15.f);
}

static int ij;

static int frameCount = 0;
static int audioBusy = 0;
static int gateIn = 0;
static int onsetFlag = 0;

void audioFrame(uint16_t buffer_offset)
{
	int16_t current_sample = 0;  
	
	if (audioBusy == 1)
	{
		audioError(); // this means it didn't finish the last round before it got interrupted again
	}
	audioBusy = 1;
	
	t808HihatSetHighpassFreq(hat, (adcVals[1] * 2));
	t808HihatSetOscBandpassFreq(hat, adcVals[2]);
	t808HihatSetOscFreq(hat, (80.0f * ((adcVals[3] * INV_TWO_TO_12) + 1.0f)) + 0.0f);
	t808HihatSetStickBandpassFreq(hat, adcVals[0]);
	t808HihatSetOscNoiseMix(hat, (adcVals[4] * INV_TWO_TO_12));
	t808HihatSetDecay(hat, ((adcVals[5] * 0.125f) + (adcVals[8] * 1.0f)));
	
	if ((!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13)) && (gateIn == 0))
	{
		onsetFlag = 1;
		gateIn = 1;
	}
	else if ((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13)) && (gateIn == 1))
	{
		gateIn = 0;
	}
	if (onsetFlag)
	{
		t808HihatOn(hat, 1.0f);
		onsetFlag = 0;
	}

	
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
			current_sample = (int16_t)(audioTickR((float) (audioInBuffer[buffer_offset + ij] * INV_TWO_TO_15)) * TWO_TO_15);
		}
		//fill the buffer with the new sample that has just been calculated
		audioOutBuffer[buffer_offset + ij] = current_sample;
	}
	
	audioBusy = 0;
}

float audioTickL(float audioIn)
{
	//use audioIn if you want the input sample	
	
	float sample = 0.0f;
	sample = t808HihatTick(hat);
	sample = sample * 1.5f;
	
	//sample = tCompressorTick(comp, sample);
	if (sample > 1.0f)
	{
		sample = 1.0f;
		audioClippedMain();
	}
	else if (sample < -1.0f)
	{
		sample = -1.0f;
		audioClippedMain();
	}
	
	//tCycleSetFreq(sine[0], ((4095-adcVals[0]) + (audioIn * (4095-adcVals[4])))); //add together knob value and FM from audio input multiplied by another knob value
	//sample = .95f * tCycleTick(sine[0]);
	return sample;
}

float audioTickR(float audioIn)
{
	
	float sample = 0;
	//tCycleSetFreq(sine[1], ((4095-adcVals[1]) + (audioIn * (4095-adcVals[5])))); //add together knob value and FM from audio input multiplied by another knob value
	//sample = .95f * tCycleTick(sine[1]);
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
