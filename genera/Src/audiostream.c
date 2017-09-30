/* Includes ------------------------------------------------------------------*/
#include "audiostream.h"

#include "codec.h"

#define AUDIO_FRAME_SIZE      128
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

#define BUTTERWORTH_ORDER 8

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
	
	//filter = tButterworthInit(BUTTERWORTH_ORDER, 2000.0f - cutoff_offset, 2000.0f + cutoff_offset);
  oldFilter = tSVFInit(SVFTypeBandpass, 2000.0f, 10.0f);
	lp = tSVFInit(SVFTypeBandpass, 40.0f, 200.0f);
	

	myCompressor->M = 24.0f;
	myCompressor->W = 24.0f;//24
	myCompressor->T = -24.0f;//24
	myCompressor->R = 3.f ; //3
	myCompressor->tauAttack =0.0f ;//1
	myCompressor->tauRelease =00.0f;//1
	
	adc[ADCJoyY] = tRampInit(18, 1);	
	adc[ADCPedal] = tRampInit(50, AUDIO_FRAME_SIZE);	
	adc[ADCBreath] = tRampInit(19, 1);	
	adc[ADCSlide] = tRampInit(50, AUDIO_FRAME_SIZE);	
	adc[ADCKnob] = tRampInit(500,1);	
	
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
int16_t position = 0;
uint16_t firstPositionValue = 0;

int counter = 0;
int flip = 1;
int envTimeout;


FTMode ftMode = FTFeedback;
float val; 

float amplitude = 0.0f;
float rampedAmp = 0.0f;

float fixed_knobs[6] = {.699f, .816f, .2107f, .3579f, .7641f, .7269f};
float squished_knobs[2] = {0.f,0.f};
float peak = 0.0f;
float harmonic = 0.0f;
float slide_tune = 1.0f;

void audioFrame(uint16_t buffer_offset)
{
	uint16_t i = 0;
	int16_t current_sample = 0;  
	
		// SET DEST KNOB
	tRampSetDest(adc[ADCKnob], adcVals[ADCKnob] * INV_TWO_TO_12);
	
	if (kMode == MasterTune)
	{
		fundamental_hz = tRampTick(adc[ADCKnob]) * 400.0f + 20.0f;
		fundamental_m = SOS_M / fundamental_hz * 0.5f;
	}
	else if (kMode == SlideTune)
	{
		slide_tune = tRampTick(adc[ADCKnob]) * 0.4f + 0.8f;
	}
	
	//cutoff_offset = fundamental_hz * 0.5f;
	
	// SET DEST SLIDE
	tRampSetDest(adc[ADCSlide], adcVals[ADCSlide]);
		// RAMP SLIDE 
	position = tRampTick(adc[ADCSlide]);
		
	slideLengthDiff = (position - firstPositionValue) * mPerVal * slide_tune;
	slideLength = fundamental_m + slideLengthDiff;
	
	float x = 12.0f * logf(slideLength / fundamental_m) * INV_LOG2;
	
	fundamental = fundamental_hz * powf(2.0f, (-x * INV_TWELVE));

	
	// SET DEST JOYY
	tRampSetDest(adc[ADCPedal], (adcVals[ADCPedal] * INV_TWO_TO_12));
	
	squished_knobs[1] =	((4096.0f - adcVals[ADCJoyY] - 2060) * 0.0018181818181818f);
	if (squished_knobs[1] < 0.0f)
	{
		squished_knobs[1] = 0.0f;
	}
	else if (squished_knobs[1] > 1.0f)
	{
		squished_knobs[1] = 1.0f;
	}
	
	

	// SET DEST JOYX
	tRampSetDest(adc[ADCJoyY], squished_knobs[1]);
	

	
	// SET DEST BREATH
	amplitude = adcVals[ADCBreath];
	amplitude = amplitude * INV_TWO_TO_12;
	amplitude = amplitude - breath_baseline;
	amplitude = amplitude * breath_mult;
	

	
	amplitude *= amp_mult;
	
	if (amplitude < 0.0f)					amplitude = 0.0f;
	else if (amplitude > 1.0f)		amplitude = 1.0f;
	
	tRampSetDest(adc[ADCBreath], amplitude);
	
	//val =  1.0f/(LN2 * bw0 ) -LN2 *bw0/24.0f + ( 0.5f/(LN2*LN2* bw0) + bw0/48.0f)*(n-1);
	
	if (myCompressor->isActive) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	else												HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);

	for (i = 0; i < (HALF_BUFFER_SIZE); i++)
	{
		if ((i & 1) == 0) {
			//current_sample = (int16_t)(audioTick((float) (audioInBuffer[buffer_offset + i] * INV_TWO_TO_15)) * TWO_TO_15);
			current_sample = (int16_t)(audioTick((float) (audioInBuffer[buffer_offset + i] * INV_TWO_TO_15)) * TWO_TO_15);
			//current_sample = (uint16_t) 0;
		}
		audioOutBuffer[buffer_offset + i] = current_sample;
	}
}

float sample = 0.0f;

#define OUTPUT_GAIN 0.9f
#define NUM_HARMONICS 14.0f
#define INV_NUM_HARMONICS 0.0769230769230769f
//#define INPUT_BOOST 2000.0f
#define INPUT_BOOST 30.0f

float lastHarmonic, tempHarmonic, mix;
float sawSample = 0.0f;
		float amps[7] = {1.0f, 0.5f, .33f, .25f, .2f, .125f, .125f};
float audioTick(float audioIn) 
{
	// RAMP BREATH
	amplitude = tRampTick(adc[ADCBreath]);
	
	mix = tRampTick(adc[ADCPedal]);
	
	// RAMP JOYY
	tempHarmonic = (NUM_HARMONICS * tRampTick(adc[ADCJoyY])) + 1.0f; // sets which harmonic to focus on
	/*
	if (((tempHarmonic - harmonic) > 0.25f) || 
			((tempHarmonic - harmonic) < -0.25f)) harmonic = tempHarmonic;
		*/
	harmonic = tempHarmonic;
	//peak =  fundamental * (uint8_t)harmonic;
	peak =  fundamental * harmonic;
	//peak = peak * powf(2.0f, (float)((uint8_t)(tRampSample(adc[ADCPedal]) * 2.9f)));
	if (peak > 40000)
	{
		peak = 40000;
	}
	// RAMP JOYX
	//tDelayLSetDelay(myDelay,tRampTick(adc[ADCJoyX]) * 64.0f);
		
	if (ftMode == FTSynthesisOne)
	{
		
		tCycleSetFreq(mySine, peak);
		sample = tCycleTick(mySine);
		
		
		
		tSawtoothSetFreq(osc, peak);
		sawSample = tSawtoothTick(osc);
		//sawSample = sawSample *amplitude * 2.0f;
		//sawSample = OOPS_reedTable(sample, mix, mix);
		
		
		//sample = sample * amplitude * 2.0f;
		//sample = OOPS_shaper(sample, (tRampTick(adc[ADCJoyX]) * 0.0024390243902439f));
		
		//sawSample = OOPS_softClip(sawSample, .8f);
		//sample = ((sample * (1.0f - mix)) + (sawSample * sawSample * sawSample * mix)) * amplitude;
		//sample = sample * amplitude;
		//sample = OOPS_softClip(sample, .8f);
		
		sample = (sample * (1.0f - mix)) + (OOPS_CompoundChebyshevT(sample, 7, amps) * mix);
		sample *= 2.0f;
		sample = OOPS_softClip(sample, .8f);
		sample *= amplitude;
		//sample *= tRampTick(adc[ADCPedal]) * amplitude;
	}
	else
	{
		//sample = INPUT_BOOST * amplitude  * audioIn;
		//sample = (sample * (1.0f - mix)) + (tNoiseTick(noise) * mix);
		//sample = INPUT_BOOST * audioIn;
		sample = audioIn;
		sample = tCompressorTick(myCompressor, sample);
		
		//sample = mix * OOPS_clip(-1.0f, sample, 1.0f) + (1.0f - mix) * tCompressorTick(myCompressor, sample);
		//sample = OOPS_clip(-1.0f, sample, 1.0f);
		//tSVFSetQ(oldFilter, tRampTick(adc[ADCPedal]));
		//tSVFSetQ(lp, tRampTick(adc[ADCPedal]));
		
		tSVFSetFreq(oldFilter, peak);
		//lp->g = oldFilter->g;
		//lp->a1 = oldFilter->a1;
		//lp->a2 = oldFilter->a2;
		//lp->a3 = oldFilter->a3;
		
		//tSVFSetFreq(lp, peak);

		sample = tSVFTick(oldFilter, sample);
		//sample = tSVFTick(lp, sample);
		//sample = sample * 12.0f;
		sample *= .8f;
		float delayTime = (tRampTick(adc[ADCKnob]) * 128.0f);
		tDelayLSetDelay(myDelay, delayTime);
		sample = tDelayLTick(myDelay, sample);
		
		//sample = OOPS_softClip(sample, 0.8f);
		//sample = tNoiseTick(noise);
		/*
		if (counter > 48000)
		{
			sample = 1;
			counter = 0;
		}
		else
		{
			sample = 0;
		}
		counter++;
		*/
		sample *= tRampTick(adc[ADCPedal]);
		
		sample = OOPS_clip(-1.0f, sample, 1.0f);
		sample *= OUTPUT_GAIN; 
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

