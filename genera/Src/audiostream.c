/* Includes ------------------------------------------------------------------*/
#include "audiostream.h"

#include "codec.h"

#define AUDIO_FRAME_SIZE      64
#define HALF_BUFFER_SIZE      AUDIO_FRAME_SIZE * 2 //number of samples per half of the "double-buffer" (twice the audio frame size because there are interleaved samples for both left and right channels)
#define AUDIO_BUFFER_SIZE     AUDIO_FRAME_SIZE * 4 //number of samples in the whole data structure (four times the audio frame size because of stereo and also double-buffering/ping-ponging)

/* Ping-Pong buffer used for audio play */
int16_t audioOutBuffer[AUDIO_BUFFER_SIZE];
int16_t audioInBuffer[AUDIO_BUFFER_SIZE];

float fundamental_hz = 58.27;
float fundamental_cm;
float fundamental_m = 2.943195469366741f;
float inv_fundamental_m;
float cutoff_offset;


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
float amp_mult = 5.0f;

tRamp* adc[ADCInputCount];

tCompressor* myCompressor;
tDelayL* myDelay;

tSVF* oldFilter;
tSVF* lp;
tButterworth* filter;
tRamp* myRamp;
tSawtooth* osc;
tCycle* mySine;
tRamp* freqRamp;
tNoise* noise;


float valPerM;
float mPerVal;


typedef enum BOOL {
	FALSE = 0,
	TRUE
} BOOL;

float breath_baseline = 0.0f;
float breath_mult = 0.0f;
float harmonicHysteresis = 0.60f;
#define BUTTERWORTH_ORDER 2

void audioInit(I2C_HandleTypeDef* hi2c, SAI_HandleTypeDef* hsaiIn, SAI_HandleTypeDef* hsaiOut, RNG_HandleTypeDef* hrand, uint16_t* myADCarray)
{ 
	// Initialize the audio library. OOPS.
	OOPSInit(SAMPLE_RATE, &randomNumber);
	
	//now to send all the necessary messages to the codec
	AudioCodec_init(hi2c);

	HAL_Delay(100);
	
	adcVals = myADCarray;
	
	
	myCompressor = tCompressorInit();
	myDelay = tDelayLInit(0);
	
	filter = tButterworthInit(BUTTERWORTH_ORDER, 233.0f - cutoff_offset, 233.0f + cutoff_offset);
  oldFilter = tSVFInit(SVFTypeBandpass, 2000.0f, 10.0f);
	lp = tSVFInit(SVFTypeBandpass, 40.0f, 200.0f);
	

	myCompressor->M = 24.0f;
	myCompressor->W = 24.0f;//24
	myCompressor->T = -24.0f;//24
	myCompressor->R = 3.f ; //3
	myCompressor->tauAttack =0.0f ;//1
	myCompressor->tauRelease =00.0f;//1
	
	adc[ADCJoyY] = tRampInit(18, 1);	//18
	adc[ADCPedal] = tRampInit(18, 1);	
	adc[ADCBreath] = tRampInit(19, 1);	
	adc[ADCSlide] = tRampInit(20, AUDIO_FRAME_SIZE);	
	adc[ADCKnob] = tRampInit(30,1);	
	
	LN2 = log(2.0f);
	
	breath_baseline = ((adcVals[ADCBreath] * INV_TWO_TO_12) + 0.1f);
	breath_mult = 1.0f / (1.0f-breath_baseline);
	
	mySine = tCycleInit();
	
	osc = tSawtoothInit();
	
	noise = tNoiseInit(PinkNoise);
	
	// set up the I2S driver to send audio data to the codec (and retrieve input as well)	
	HAL_SAI_Transmit_DMA(hsaiIn, (uint8_t *)&audioOutBuffer[0], AUDIO_BUFFER_SIZE);
	HAL_SAI_Receive_DMA(hsaiOut, (uint8_t *)&audioInBuffer[0], AUDIO_BUFFER_SIZE);

}


uint16_t knobValue;

uint16_t slideValue;

float slideLengthDiff = 0;
float slideLength = 0;

float fundamental = 0.0f;
float customFundamental = 58.27f;
int16_t position = 0;
uint16_t firstPositionValue = 0;

int counter = 0;
int flip = 1;
int envTimeout;


FTMode ftMode = FTSynthesisOne;
float val; 

float breath = 0.0f;
float rampedBreath = 0.0f;

float slide_tune = 1.0f;

int hysteresisKnob = 0;
int hysteresisAmount = 512;

void setFundamental(float fund)
{
	fundamental_hz = fund;
	cutoff_offset = fundamental_hz * 0.5f;
}

float temp = 0.0f, diff = 0.0f;
void thingToDoWhenKnobMoves(int input)
{
	hysteresisKnob = input;
	tRampSetDest(adc[ADCKnob], hysteresisKnob * INV_TWO_TO_12);
	
	if (kMode == MasterTune)
	{
		
		temp = tRampSample(adc[ADCKnob]) * 400.0f + 20.0f;
		diff = customFundamental - temp; 
		diff = (diff < 0.0f) ? -diff : diff;
		
		if (diff < 1.0f)
		{
			customFundamental = temp;
			setFundamental(customFundamental);
		}
	}
	else if (kMode == SlideTune)
	{
		slide_tune = tRampSample(adc[ADCKnob]) * 0.4f + 0.8f;
	}
}

void audioFrame(uint16_t buffer_offset)
{
	uint16_t i = 0;
	int16_t current_sample = 0;  
	
	tRampSetDest(adc[ADCPedal], (adcVals[ADCPedal] * INV_TWO_TO_12));
	tRampSetDest(adc[ADCKnob], adcVals[ADCKnob] * INV_TWO_TO_12);
	tRampSetDest(adc[ADCJoyY], 1.0f - ((adcVals[ADCJoyY] * INV_TWO_TO_12) - 0.366f) * 3.816f);
	
	if (knobMoved)
	{
		thingToDoWhenKnobMoves(adcVals[ADCKnob]);
	}
	else
	{
		if ((adcVals[ADCKnob] > (hysteresisKnob + hysteresisAmount)) || 
				(adcVals[ADCKnob] < (hysteresisKnob - hysteresisAmount)))
		{
			thingToDoWhenKnobMoves(adcVals[ADCKnob]);
			knobMoved = 1;
		}
	}
	
	// SET DEST SLIDE
	tRampSetDest(adc[ADCSlide], adcVals[ADCSlide]);
	position = tRampTick(adc[ADCSlide]);
	slideLengthDiff = (position - firstPositionValue) * mPerVal * slide_tune;
	slideLength = fundamental_m + slideLengthDiff;
	
	float x = 12.0f * logf(slideLength / fundamental_m) * INV_LOG2;
	
	fundamental = fundamental_hz * powf(2.0f, (-x * INV_TWELVE));
	
	if (myCompressor->isActive) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	else												HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
	
	
	for (i = 0; i < (HALF_BUFFER_SIZE); i++)
	{
		if ((i & 1) == 0) {
			current_sample = (int16_t)(audioTick((float) (audioInBuffer[buffer_offset + i] * INV_TWO_TO_15)) * TWO_TO_15);
		}
		audioOutBuffer[buffer_offset + i] = current_sample;
	}
}

float sample = 0.0f;

#define OUTPUT_GAIN 0.9f
#define NUM_HARMONICS 15.0f
#define INPUT_BOOST 30.0f

float floatHarmonic, floatPeak, intPeak, mix;
float intHarmonic;
float sawSample = 0.0f;
float amps[7] = {1.0f, 0.5f, .33f, .25f, .2f, .125f, .125f};


float audioTick(float audioIn) 
{
	//choose harmonic
	floatHarmonic = (NUM_HARMONICS * tRampTick(adc[ADCJoyY])) + 1.0f; // sets which harmonic to focus on
	if (((floatHarmonic - intHarmonic) > (harmonicHysteresis)) || ((floatHarmonic - intHarmonic) < ( -1.0f * harmonicHysteresis)))
	{
		intHarmonic = (uint16_t) (floatHarmonic + 0.5f);
	}
	floatPeak = fundamental * floatHarmonic;
	intPeak = fundamental * intHarmonic;
	
	//some code to jump octaves if desired
	//floatPeak = peak * powf(2.0f, (float)((uint8_t)(tRampSample(adc[ADCPedal]) * 2.9f)));
	
	
	float delayTime = (tRampTick(adc[ADCKnob]) * 256.0f);

	float pedal = tRampTick(adc[ADCPedal]);
	
	breath = adcVals[ADCBreath];
	breath = breath * INV_TWO_TO_12;
	breath = breath - breath_baseline;
	breath = breath * breath_mult;
	breath *= amp_mult;
	if (breath < 0.0f)					breath = 0.0f;
	else if (breath > 1.0f)		breath = 1.0f;
	tRampSetDest(adc[ADCBreath], breath);
	
	rampedBreath = tRampTick(adc[ADCBreath]);
	if (ftMode == FTSynthesisOne)
	{
		
		tCycleSetFreq(mySine, intPeak);
		sample = tCycleTick(mySine);
	
		//tSawtoothSetFreq(osc, peak);
		//sawSample = tSawtoothTick(osc);
		//sawSample = sawSample *amplitude * 2.0f;
		//sawSample = OOPS_reedTable(sample, -.6f, .2f);
		
		
		//sample = sample * amplitude * 2.0f;
		//sample = OOPS_shaper(sample, (tRampTick(adc[ADCJoyX]) * 0.0024390243902439f));
		
		//sawSample = OOPS_softClip(sawSample, .8f);
		//sample = ((sample * (1.0f - mix)) + (sawSample * sawSample * sawSample * mix)) * amplitude;
		//sample = sample * amplitude;
		//sample = OOPS_softClip(sample, .8f);
		
		sample = (sample * (1.0f - pedal)) + (OOPS_CompoundChebyshevT(sample, 7, amps) * pedal);
		sample *= 1.0f + pedal;
		sample = OOPS_softClip(sample, .8f);
		
		sample *= rampedBreath;
		//sample *= amplitude;
		//sample *= tRampTick(adc[ADCPedal]) * amplitude;
	}
	else
	{
		//sample = INPUT_BOOST * amplitude  * audioIn;
		//sample = (sample * (1.0f - mix)) + (tNoiseTick(noise) * mix);
		//sample = INPUT_BOOST * audioIn;
		sample = audioIn;
		tButterworthSetFreqs(filter, floatPeak - cutoff_offset, floatPeak + cutoff_offset);
		sample = tButterworthTick(filter, sample);
		
		//sample = tCompressorTick(myCompressor, sample);
		
		//sample = mix * OOPS_clip(-1.0f, sample, 1.0f) + (1.0f - mix) * tCompressorTick(myCompressor, sample);
		//sample = OOPS_clip(-1.0f, sample, 1.0f);

		//tSVFSetQ(lp, tRampTick(adc[ADCPedal]));
		
		//tSVFSetFreq(oldFilter, floatPeak);
		//lp->g = oldFilter->g;
		//lp->a1 = oldFilter->a1;
		//lp->a2 = oldFilter->a2;
		//lp->a3 = oldFilter->a3;
		
		//tSVFSetFreq(lp, peak);

		//sample = tSVFTick(oldFilter, sample);
		//sample = tSVFTick(lp, sample);
		//sample = sample * 12.0f;
		//sample = tNoiseTick(noise);
		//sample *= .8f;
		
		tDelayLSetDelay(myDelay, delayTime);
		sample = tDelayLTick(myDelay, sample);
		sample *= 10.0f;
		sample = OOPS_softClip(sample, 1.0f - pedal);
		//sample = tNoiseTick(noise);
		
		sample *= rampedBreath;
		
		sample = OOPS_clip(-1.0f, sample, 1.0f);
		//sample *= OUTPUT_GAIN; 
	}
	
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

