/* Includes ------------------------------------------------------------------*/
#include "audiostream.h"

#include "codec.h"

#define AUDIO_FRAME_SIZE      128
#define HALF_BUFFER_SIZE      AUDIO_FRAME_SIZE * 2 //number of samples per half of the "double-buffer" (twice the audio frame size because there are interleaved samples for both left and right channels)
#define AUDIO_BUFFER_SIZE     AUDIO_FRAME_SIZE * 4 //number of samples in the whole data structure (four times the audio frame size because of stereo and also double-buffering/ping-ponging)

/* Ping-Pong buffer used for audio play */
int16_t audioOutBuffer[AUDIO_BUFFER_SIZE];
int16_t audioInBuffer[AUDIO_BUFFER_SIZE];

float fundamental_hz;
float fundamental_cm;
float fundamental_m;
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
tRamp* freqRamp;


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
	// set up the I2S driver to send audio data to the codec (and retrieve input as well)	
	HAL_SAI_Transmit_DMA(hsaiIn, (uint8_t *)&audioOutBuffer[0], AUDIO_BUFFER_SIZE);
	HAL_SAI_Receive_DMA(hsaiOut, (uint8_t *)&audioInBuffer[0], AUDIO_BUFFER_SIZE);

	myCompressor = tCompressorInit();
	myDelay = tDelayLInit(0);
	
	filter = tButterworthInit(BUTTERWORTH_ORDER, 2000.0f - cutoff_offset, 2000.0f + cutoff_offset);
  oldFilter = tSVFInit(SVFTypeBandpass, 2000.0f, 100.0f);
	lp = tSVFInit(SVFTypeBandpass, 40.0f, 100.0f);
	

	myCompressor->M = 0.0f;
	myCompressor->W = 24.0f;
	myCompressor->T = -12.0f;
	myCompressor->R = 3.0f ; 
	myCompressor->tauAttack = 1;
	myCompressor->tauRelease = 1;
	
	for (int i = 0; i < ADCInputCount; i++) adc[i] = tRampInit(20, 1);	
	
	tRampSetTime(adc[ADCJoyX], 200);
	
	tRampSetTime(adc[ADCSlide], 1);
	
	LN2 = log(2.0f);
	
	breath_baseline = ((adcVals[3] * INV_TWO_TO_12) + 0.1f);
	breath_mult = 1.0f / (1.0f-breath_baseline);
	
	osc = tSawtoothInit();
	
	tSawtoothSetFreq(osc, 220.0f);
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

float knobs[6];

// 1: delay
// 2: Ratio
// 3: Attack 
// 4: Release
// 5: harmonic focus
// 6: size of normal bandwidth 

/*
 
int posavg_idx = 0;
int16_t posavg[20];

uint16_t sum = 0;
int16_t lastPosition = 0;
		posavg_idx = (posavg_idx < 20) ? posavg_idx : 0;
		
		posavg[posavg_idx] = adcValues[ADC_SLIDE];
		
		++posavg_idx;
		
		sum = 0;
		for (int i = 0; i < 20; i++)
		{
			sum += posavg[i];  
		}
		
*/

float amplitude = 0.0f;
float rampedAmp = 0.0f;

float fixed_knobs[6] = {.699f, .816f, .2107f, .3579f, .7641f, .7269f};
float squished_knobs[2] = {0.f,0.f};
float peak = 0.0f;
float harmonic = 0.0f;

void audioFrame(uint16_t buffer_offset)
{
	uint16_t i = 0;
	int16_t current_sample = 0;  
	
	fundamental_hz = tRampTick(adc[ADCKnob]) * 200.0f + 30.0f;
	fundamental_m = SOS_M / fundamental_hz * 0.5f;
	
	cutoff_offset = fundamental_hz * 0.5f;
	
	//int tauAttack, tauRelease;
  //float T, R, W, M; // Threshold, compression Ratio, decibel Width of knee transition, decibel Make-up gain
	for (int i = 0; i < 6; i++)	knobs[i] = (4096.0f - adcVals[i]) / 4096.0f;
	
	// SET DEST SLIDE
	tRampSetDest(adc[ADCSlide], adcVals[ADC_SLIDE]);
	
	// RAMP SLIDE 
	position = tRampTick(adc[ADCSlide]);
		
	slideLengthDiff = (position - firstPositionValue) * M_PER_VAL;
	slideLength = fundamental_m + slideLengthDiff;
	
	float x = 12.0f * logf(slideLength / fundamental_m) * INV_LOG2;
	
	fundamental = fundamental_hz * powf(2.0f, (-x * INV_TWELVE));

	squished_knobs[0] = (((float)(adcVals[ADCJoyX] - 2050)) / 550.0f);
	
	if (squished_knobs[0] < 0.0f)
	{
		squished_knobs[0] = 0.0f;
	}
	else if (squished_knobs[0] > 1.0f)
	{
		squished_knobs[0] = 1.0f;
	}
	
	// SET DEST JOYY
	tRampSetDest(adc[ADCJoyX], squished_knobs[0]);
	
	squished_knobs[1] =	((4096.0f - adcVals[ADCJoyY] - 2100) / 550.0f);
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
	
	// SET DEST KNOB
	tRampSetDest(adc[ADCKnob], adcVals[ADCKnob] * INV_TWO_TO_12);
	
	// SET DEST BREATH
	tRampSetDest(adc[ADCBreath], adcVals[ADCBreath]);
	
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
#define NUM_HARMONICS 10.0f
#define INV_NUM_HARMONICS 0.1f
#define INPUT_BOOST 2000.0f

float lastHarmonic, tempHarmonic, mix;

float audioTick(float audioIn) 
{
	// RAMP BREATH
	amplitude = tRampTick(adc[ADCBreath]);

	amplitude = amplitude * INV_TWO_TO_12;
	amplitude = amplitude - breath_baseline;
	amplitude = amplitude * breath_mult;
	
	mix = tRampTick(adc[ADCKnob]);
	
	amplitude *= amp_mult;
	
	if (amplitude < 0.0f)					amplitude = 0.0f;
	else if (amplitude > 1.0f)		amplitude = 1.0f;
	
	// RAMP JOYY
	tempHarmonic = (NUM_HARMONICS * tRampTick(adc[ADCJoyY])) + 1.0f; // sets which harmonic to focus on
	
	if (((tempHarmonic - harmonic) > 0.2f) || 
			((tempHarmonic - harmonic) < -0.2f)) harmonic = tempHarmonic;
		
	peak =  fundamental * (uint8_t)harmonic;
	
	// RAMP JOYX
	tDelayLSetDelay(myDelay,tRampTick(adc[ADCJoyX]) * 64.0f);
		
	if (ftMode == FTSynthesisOne)
	{
		tSawtoothSetFreq(osc, peak);
		
		sample = tSawtoothTick(osc);
		
		sample = sample * amplitude;
	}
	else
	{
		sample = INPUT_BOOST * audioIn;
		
		sample = tCompressorTick(myCompressor, sample);
		
		//sample = mix * OOPS_clip(-1.0f, sample, 1.0f) + (1.0f - mix) * tCompressorTick(myCompressor, sample);
		//sample = OOPS_clip(-1.0f, sample, 1.0f);
		
		//tButterworthSetFreqs(filter, peak - cutoff_offset, peak + cutoff_offset);
		tSVFSetFreq(oldFilter, peak);
		tSVFSetFreq(lp, peak);
		
		//sample = tButterworthTick(filter, sample);
		sample = tSVFTick(oldFilter, sample);
		sample = tSVFTick(lp, sample);
		
		sample = tDelayLTick(myDelay, sample);
		
		//sample = OOPS_softClip(sample, 0.99f);
		
		sample = OOPS_clip(-1.0f, sample, 1.0f);
		
		sample *= amplitude;
		
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

