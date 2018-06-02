/* Includes ------------------------------------------------------------------*/
#include "audiostream.h"

#include "codec.h"

#define AUDIO_FRAME_SIZE      128
#define HALF_BUFFER_SIZE      AUDIO_FRAME_SIZE * 2 //number of samples per half of the "double-buffer" (twice the audio frame size because there are interleaved samples for both left and right channels)
#define AUDIO_BUFFER_SIZE     AUDIO_FRAME_SIZE * 4 //number of samples in the whole data structure (four times the audio frame size because of stereo and also double-buffering/ping-ponging)
#define ACOUSTIC_DELAY				132



/* Ping-Pong buffer used for audio play */
int16_t audioOutBuffer[AUDIO_BUFFER_SIZE];
int16_t audioInBuffer[AUDIO_BUFFER_SIZE];

float fundamental_hz = 58.27;
float fundamental_cm;
float fundamental_m = 2.943195469366741f;
float inv_fundamental_m;
float cutoff_offset;
long long click_counter = 0;
const int D = 2*AUDIO_FRAME_SIZE + ACOUSTIC_DELAY;


uint16_t* adcVals;

// I think we need to grab these from main
/*
static HAL_DMA_StateTypeDef spi1tx;
static HAL_DMA_StateTypeDef spi1rx;
*/

float audioTick(float audioIn);
float audioTickL(float audioIn);
float audioTickR(float audioIn);
float fbtNIMETick(float audioIn);
float randomNumber(void);
void audioFrame(uint16_t buffer_offset);
float LN2;
float amp_mult = 5.0f;

tRamp* adc[ADCInputCount];

tCompressor* myCompressor;
tDelayL* myDelay;
tDelayL* feedbackDelay;

tSVF* oldFilter;
tSVF* lp;
tButterworth* filter;
tRamp* myRamp;
tSawtooth* osc;
tCycle* mySine;
tRamp* freqRamp;
tNoise* noise;
tRamp* slideRamp;

float testFreq = 0.0f;
float testDelay = 0.0f;

float valPerM;
float mPerVal;


typedef enum BOOL {
	FALSE = 0,
	TRUE
} BOOL;

float breath_baseline = 0.0f;
float breath_mult = 0.0f;
float harmonicHysteresis = 0.60f;
#define BUTTERWORTH_ORDER 4

void audioInit(I2C_HandleTypeDef* hi2c, SAI_HandleTypeDef* hsaiIn, SAI_HandleTypeDef* hsaiOut, RNG_HandleTypeDef* hrand, uint16_t* myADCarray)
{ 
	// Initialize the audio library. OOPS.
	OOPSInit(SAMPLE_RATE, &randomNumber);
	
	//now to send all the necessary messages to the codec
	AudioCodec_init(hi2c);

	HAL_Delay(100);
	
	adcVals = myADCarray;
	
	setFundamental(fundamental_hz);
	
	myCompressor = tCompressorInit();
	myDelay = tDelayLInit(0);
	tDelayLSetDelay(myDelay, testDelay); 
	feedbackDelay = tDelayLInit(0);
	
	filter = tButterworthInit(BUTTERWORTH_ORDER, 233.0f - cutoff_offset, 233.0f + cutoff_offset);
  oldFilter = tSVFInit(SVFTypeBandpass, 2000.0f, 20000.0f);
	lp = tSVFInit(SVFTypeBandpass, 40.0f, 20000.0f);
	

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
	myRamp = tRampInit(10, 1);
	slideRamp = tRampInit(10, 1);
	
	LN2 = log(2.0f);
	
	breath_baseline = ((adcVals[ADCBreath] * INV_TWO_TO_12) + 0.1f);
	breath_mult = 1.0f / (1.0f-breath_baseline);
	
	mySine = tCycleInit();
	
	osc = tSawtoothInit();
	
	noise = tNoiseInit(PinkNoise);
	
	// set up the I2S driver to send audio data to the codec (and retrieve input as well)	
	HAL_SAI_Transmit_DMA(hsaiIn, (uint8_t *)&audioOutBuffer[0], AUDIO_BUFFER_SIZE);
	HAL_SAI_Receive_DMA(hsaiOut, (uint8_t *)&audioInBuffer[0], AUDIO_BUFFER_SIZE);

	testFreq = 0;
	
	firstPositionValue = adcVals[ADCSlide];
	calibrated = 1;
}

int d(float Tfreq){
	int T = SAMPLE_RATE / Tfreq;
	return (T - D % T);
}

uint16_t knobValue;

uint16_t slideValue;

float slideLengthDiff = 0;
float slideLength = 0;

float fundamental = 0.0f;
float customFundamental = 48.9994294977f;
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

int octave = 3;
float octaveTransp[7] = { 0.125f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f};

void setFundamental(float fund)
{
	fundamental_hz = fund;
	cutoff_offset = fundamental_hz * 0.25f;
}

float temp = 0.0f, diff = 0.0f;
void thingToDoWhenKnobMoves(int input)
{
	hysteresisKnob = input;
	tRampSetDest(adc[ADCKnob], hysteresisKnob * INV_TWO_TO_12);
	
	if (kMode == MasterTune)
	{
		customFundamental = knobValueToUse * 400.0f + 20.0f;
		setFundamental(customFundamental);
	}
	else if (kMode == SlideTune)
	{
		slide_tune = knobValueToUse * 0.4f + 0.8f;
	}
	else if (kMode == OctaveTune)
	{
		octave = (int)(knobValueToUse * 7);
	}
}

//correct slidelengths for positions in m (determined by printing slideLengthM
float slidePositions[8] = {0.0f, 0.09f, 0.17f, 0.27f, 0.34f, 0.39f, 0.47f, 0.7f};
float slideLengthM = 0.0f;
float slideLengthChange = 0.0f;
float oldSlideLengthM = 0.0f;
float newTestDelay = 0.0f;
float oldIntHarmonic = 0.0f;

float delayCor[8][16] = {{0.0f, 0.0f, 43.0f, 56.0f, 48.0f, 32.0f, 18.0f, 26.0f, 23.0f, 14.0f, 11.0f, 5.0f, 1.0f, 0.0f, 0.0f, 7.0f}, 
													{0.0f, 0.0f, 24.0f, 39.0f, 41.0f, 31.0f, 17.0f, 1.0f, 104.0f, 100.0f, 93.0f, 74.0f, 4.0f, 66.0f, 3.0f, 63.0f},
													{0.0f, 0.0f, 56.0f, 82.0f, 69.0f, 39.0f, 29.0f, 23.0f, 121.0f, 97.0f, 179.0f, 80.0f, -5.0f, 76.0f, -4.0f, 0.0f},
													{0.0f, 0.0f, 1.0f, 18.0f, 38.0f, 20.0f, 9.0f, 123.0f, 109.0f, -1.0f, 93.0f, 69.0f, 65.0f, 140.0f, 1.0f, 67.0f},
													{0.0f, 0.0f, 6.0f, 37.0f, 43.0f, 30.0f, 10.0f, 4.0f, 117.0f, 99.0f, 93.0f, 56.0f, 74.0f, 54.0f, 1.0f, 132.0f},
													{0.0f, 0.0f, 159.0f, 133.0f, 117.0f, 95.0f, 56.0f, 151.0f, 120.0f, 108.0f, 104.0f, 103.0f, 103.0f, 22.0f, 87.0f, 78.0f},
													{0.0f, 0.0f, 141.0f, 119.0f, 104.0f, 92.0f, 215.0f, 162.0f, 125.0f, 111.0f, 108.0f, 104.0f, 104.0f, 98.0f, 95.0f, 80.0f},
													{0.0f, 0.0f, 141.0f, 119.0f, 104.0f, 92.0f, 215.0f, 162.0f, 125.0f, 111.0f, 108.0f, 104.0f, 104.0f, 98.0f, 95.0f, 80.0f}};

//takes index from delayValueCorrection to calculate weight for weighted average and returns the correct delay value (don't call directly)
float delayValCor(int slidePosInd, float slidePos){
    float fraction = ((slidePos - slidePositions[slidePosInd-1]) / (slidePositions[slidePosInd] - slidePositions[slidePosInd-1]));
    //determines weighted average of the appropriate two delayCor[][] values based on slide position and harmonic
    return ((delayCor[slidePosInd-1][((uint8_t) intHarmonic) - 1] * (1 - fraction)) + (delayCor[slidePosInd][((uint8_t) intHarmonic) - 1] * fraction));
}

//calculates first index in slidePositions[] that is past the current slide position and passes into delayValCor to return the correct delay value
float delayValueCorrection(float slidePos){
    uint8_t i = 0;
    for(i; i < 8; i++){
        if(slidePos < slidePositions[i]){
            return delayValCor(i, slidePos);
        }
    }
    return NULL;
}		

float slideLengthPreRamp;

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
	tRampSetDest(adc[ADCSlide], adcVals[ADCSlide]>>SLIDE_BITS);
	
	
	//slideLength = fundamental_m + slideLengthDiff;
	
	//slideLength = tRampTick(slideRamp);
	position = tRampTick(adc[ADCSlide]);
	//position = adc[ADCSlide];
	slideLengthDiff = (position - firstPositionValue) * mPerVal * slide_tune;
	slideLengthM = (position - firstPositionValue) * mPerVal;
	slideLengthPreRamp = fundamental_m + slideLengthDiff;
	tRampSetDest(slideRamp, slideLengthPreRamp);
	//if(oldIntHarmonic != intHarmonic){
	
	//}
	//tRampSetDest(myRamp, newTestDelay);
	//oldSlideLengthM = slideLengthM;
	//oldIntHarmonic = intHarmonic;
	
	
	
	if (myCompressor->isActive) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	else												HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
	
	//tButterworthSetFreqs(filter, floatPeak  - cutoff_offset, floatPeak + cutoff_offset);
	//intPeak = 10 * fundamental_hz;
	tButterworthSetFreqs(filter, intPeak  - cutoff_offset, intPeak + cutoff_offset);
	tSVFSetFreq(oldFilter, intPeak);
	lp->g = oldFilter->g;
	lp->a1 = oldFilter->a1;
	lp->a2 = oldFilter->a2;
	lp->a3 = oldFilter->a3;
	
	for (i = 0; i < (HALF_BUFFER_SIZE); i++)
	{
		if ((i & 1) == 0) {
			current_sample = (int16_t)(audioTickL((float) (audioInBuffer[buffer_offset + i] * INV_TWO_TO_15)) * TWO_TO_15);
		}
		else
		{
			//current_sample = (int16_t)(audioTickR((float) (audioInBuffer[buffer_offset + i] * INV_TWO_TO_15)) * TWO_TO_15);
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

float thisDelayOut, lastDelayOut;
float feedbackAmt = 1.0f;

HarmonicMode hMode = RajeevMode;
float knobValueToUse = 0.0f;
//float newTestDelay = 0.0f;
float testDelayDiff = 0.0f;
//float oldIntHarmonic = 0.0f;

int initDelayCounter = 0;
#define INIT_DELAY SAMPLE_RATE

float prevInput = 0.0f;
float delayVals[16] = {0.0f, 0.0f, 25.0f, 30.0f, 58.0f, 75.0f, 58.0f, 90.0f, 107.0f, 24.0f, 26.0f, 170.0f, 172.0f, 170.0f, 7.0f, 14.0f};
							

float fbtNIMETick(float audioIn) 
{
	//choose harmonic
	prevInput = audioIn;
	
	knobValueToUse = tRampTick(adc[ADCKnob]);
	
	if (hMode == CaraMode)
	{
		floatHarmonic = (NUM_HARMONICS * tRampTick(adc[ADCJoyY])) + 1.0f; // sets which harmonic to focus on
		
	}
	else if (hMode == RajeevMode)
	{
		floatHarmonic = NUM_HARMONICS * (1.0f - tRampTick(adc[ADCJoyY])) + 1.0f;
		
	}
	else if (hMode == JennyMode)
	{
		floatHarmonic = tRampTick(adc[ADCJoyY]) * 2.0f - 1.0f;
		floatHarmonic = (floatHarmonic < 0.0f) ? 1.0f : (floatHarmonic * NUM_HARMONICS + 1.0f);
	}
	
	if (((floatHarmonic - intHarmonic) > (harmonicHysteresis)) || ((floatHarmonic - intHarmonic) < ( -1.0f * harmonicHysteresis)))
	{
		intHarmonic = (uint16_t) (floatHarmonic + 0.5f);
	}
	
	floatPeak = fundamental * floatHarmonic * octaveTransp[octave];
	intPeak = fundamental * intHarmonic * octaveTransp[octave];
	
	//some code to jump octaves if desired
	//floatPeak = peak * powf(2.0f, (float)((uint8_t)(tRampSample(adc[ADCPedal]) * 2.9f)));

	float pedal = tRampTick(adc[ADCPedal]);
	
	breath = adcVals[ADCBreath];
	breath = breath * INV_TWO_TO_12;
	breath = breath - breath_baseline;
	breath = breath * breath_mult;
	breath *= amp_mult;
	
	if (breath < 0.0f)					breath = 0.0f;
	else if (breath > 1.0f)		  breath = 1.0f;
	
	tRampSetDest(adc[ADCBreath], breath);
	
	rampedBreath = tRampTick(adc[ADCBreath]);
	
	//sample = INPUT_BOOST * amplitude  * audioIn;
	//sample = (sample * (1.0f - mix)) + (tNoiseTick(noise) * mix);
	//sample = INPUT_BOOST * audioIn;

	sample = audioIn * 0.05f;
	
	//sample = tButterworthTick(filter, sample);
	//tSVFSetFreq(oldFilter, testFreq);
	sample = tSVFTick(oldFilter, sample);
	//sample = tSVFTick(lp, sample);
	//sample = tCompressorTick(myCompressor, sample);
	
	if (ftMode == FTSynthesisOne) 
	{
		sample *= rampedBreath;;
	}
	else	
	{		
		sample *= pedal;
	}
	
	//sample = mix * OOPS_clip(-1.0f, sample, 1.0f) + (1.0f - mix) * tCompressorTick(myCompressor, sample);
	sample = OOPS_clip(-1.0f, sample, 1.0f);

	//tSVFSetQ(lp, tRampTick(adc[ADCPedal]));
	/*
	intPeak = 48.9994294977f * (intHarmonic - 1);

	testFreq -= 0.001f;
	if (testFreq <= 0.0f) testFreq = 2000.0f;
	tSVFSetFreq(oldFilter, testFreq);
	tSVFSetQ(oldFilter, 200.0f);
	lp->g = oldFilter->g;
	lp->a1 = oldFilter->a1;
	lp->a2 = oldFilter->a2;
	lp->a3 = oldFilter->a3;
	*/
	//tSVFSetFreq(lp, floatPeak);
	//tSVFSetQ(lp, 200.0f);

	//sample = tSVFTick(oldFilter, sample);

	//sample *= .8f;
	
	
	//testDelay += 0.0002f;
	//if (testDelay >= 128.0f) testDelay = 1.0f;
	//testDelay = knobValueToUse * 511.0f + 1.0f;
	//uncomment the second half of this line to add the delay corrections for 1st position
	//testDelay = d(intPeak) + knobValueToUse * 255.0f + 1.0f;
	
	/**
	if(oldIntHarmonic != intHarmonic){
		newTestDelay = (uint8_t) d(intPeak) + (uint8_t) delayValueCorrection(slideLengthM);
	}
	**/
	
	slideLength = tRampTick(slideRamp);
	float x = 12.0f * logf(slideLength / fundamental_m) * INV_LOG2;
	fundamental = fundamental_hz * powf(2.0f, (-x * INV_TWELVE));
	
	//testDelay = (uint8_t) d(intPeak) + (uint8_t) delayValueCorrection(slideLengthM);
	//newTestDelay = (uint8_t) d(intPeak) + (uint8_t) delayValueCorrection(slideLengthM);
	newTestDelay = (uint8_t) d(intPeak) + (uint8_t) delayValueCorrection(slideLengthM);
	tRampSetDest(myRamp, newTestDelay);
	//
	testDelay = (uint8_t) tRampTick(myRamp);
	
	
	//testDelay = d(intPeak);
	//testDelay = delayVals[((uint8_t)intHarmonic)-1];
	tDelayLSetDelay(feedbackDelay, testDelay);
	
	//tDelayLSetDelay(feedbackDelay,212.0f);
	sample = tDelayLTick(feedbackDelay, sample);
	
	// comment back in
	//sample *= rampedBreath;
	
	//sample *= pedal;
	
	sample = OOPS_clip(-1.0f, sample, 1.0f);
	//sample *= OUTPUT_GAIN; 
	
	return sample;
}
							
float audioTickL(float audioIn) 
{
	if (++initDelayCounter < INIT_DELAY){sample = 0.0f; return sample;};
	//choose harmonic
	prevInput = audioIn;
	
	knobValueToUse = tRampTick(adc[ADCKnob]);
	
	if (hMode == CaraMode)
	{
		floatHarmonic = (NUM_HARMONICS * tRampTick(adc[ADCJoyY])) + 1.0f; // sets which harmonic to focus on
		
	}
	else if (hMode == RajeevMode)
	{
		floatHarmonic = NUM_HARMONICS * (1.0f - tRampTick(adc[ADCJoyY])) + 1.0f;
		
	}
	else if (hMode == JennyMode)
	{
		floatHarmonic = tRampTick(adc[ADCJoyY]) * 2.0f - 1.0f;
		floatHarmonic = (floatHarmonic < 0.0f) ? 1.0f : (floatHarmonic * NUM_HARMONICS + 1.0f);
	}
	
	if (((floatHarmonic - intHarmonic) > (harmonicHysteresis)) || ((floatHarmonic - intHarmonic) < ( -1.0f * harmonicHysteresis)))
	{
		intHarmonic = (uint16_t) (floatHarmonic + 0.5f);
	}
	
	floatPeak = fundamental * floatHarmonic * octaveTransp[octave];
	intPeak = fundamental * intHarmonic * octaveTransp[octave];
	
	//some code to jump octaves if desired
	//floatPeak = peak * powf(2.0f, (float)((uint8_t)(tRampSample(adc[ADCPedal]) * 2.9f)));

	float pedal = tRampTick(adc[ADCPedal]);
	
	breath = adcVals[ADCBreath];
	breath = breath * INV_TWO_TO_12;
	breath = breath - breath_baseline;
	breath = breath * breath_mult;
	breath *= amp_mult;
	
	if (breath < 0.0f)					breath = 0.0f;
	else if (breath > 1.0f)		  breath = 1.0f;
	
	tRampSetDest(adc[ADCBreath], breath);
	
	rampedBreath = tRampTick(adc[ADCBreath]);
	
	
		
		//sample = INPUT_BOOST * amplitude  * audioIn;
		//sample = (sample * (1.0f - mix)) + (tNoiseTick(noise) * mix);
		//sample = INPUT_BOOST * audioIn;
	
		//sample = audioIn * 0.05f;
		sample = 0.07f * audioIn;
		
		//sample = tButterworthTick(filter, sample);
		//tSVFSetFreq(oldFilter, testFreq);
		sample = tSVFTick(oldFilter, sample);
		//sample = tSVFTick(lp, sample);
		//sample = tCompressorTick(myCompressor, sample);
		if (ftMode == FTSynthesisOne){
			sample *= pedal;
		}
		else{
			sample *= rampedBreath;
		}
	
		//sample = mix * OOPS_clip(-1.0f, sample, 1.0f) + (1.0f - mix) * tCompressorTick(myCompressor, sample);
		sample = OOPS_clip(-1.0f, sample, 1.0f);

		//tSVFSetQ(lp, tRampTick(adc[ADCPedal]));
		/*
		intPeak = 48.9994294977f * (intHarmonic - 1);

		testFreq -= 0.001f;
		if (testFreq <= 0.0f) testFreq = 2000.0f;
		tSVFSetFreq(oldFilter, testFreq);
		tSVFSetQ(oldFilter, 200.0f);
		lp->g = oldFilter->g;
		lp->a1 = oldFilter->a1;
		lp->a2 = oldFilter->a2;
		lp->a3 = oldFilter->a3;
		*/
		//tSVFSetFreq(lp, floatPeak);
		//tSVFSetQ(lp, 200.0f);

		//sample = tSVFTick(oldFilter, sample);

		//sample *= .8f;
		
		
		//testDelay += 0.0002f;
	  //if (testDelay >= 128.0f) testDelay = 1.0f;
		//testDelay = knobValueToUse * 511.0f + 1.0f;
		//uncomment the second half of this line to add the delay corrections for 1st position
		//testDelay = d(intPeak) + knobValueToUse * 255.0f + 1.0f;
		
		/**
		if(oldIntHarmonic != intHarmonic){
			newTestDelay = (uint8_t) d(intPeak) + (uint8_t) delayValueCorrection(slideLengthM);
		}
		**/
		
		slideLength = tRampTick(slideRamp);
		float x = 12.0f * logf(slideLength / fundamental_m) * INV_LOG2;
		fundamental = fundamental_hz * powf(2.0f, (-x * INV_TWELVE));
		
		//testDelay = (uint8_t) d(intPeak) + (uint8_t) delayValueCorrection(slideLengthM);
		//newTestDelay = (uint8_t) d(intPeak) + (uint8_t) delayValueCorrection(slideLengthM);
		newTestDelay = (uint8_t) d(intPeak) + (uint8_t) delayValueCorrection(slideLengthM);
		tRampSetDest(myRamp, newTestDelay);
		//
		testDelay = (uint8_t) tRampTick(myRamp);
		
		
		//testDelay = d(intPeak);
		//testDelay = delayVals[((uint8_t)intHarmonic)-1];
		tDelayLSetDelay(feedbackDelay, testDelay);
		
		//tDelayLSetDelay(feedbackDelay,212.0f);
		sample = tDelayLTick(feedbackDelay, sample);
		
		// comment back in
		//sample *= rampedBreath;
		
		//sample *= pedal;
		
		sample = OOPS_clip(-1.0f, sample, 1.0f);
		//sample *= OUTPUT_GAIN; 
		
		
	
	
	return sample;
}

float audioTickR(float audioIn) 
{
			//sample = INPUT_BOOST * amplitude  * audioIn;
		//sample = (sample * (1.0f - mix)) + (tNoiseTick(noise) * mix);
		//sample = INPUT_BOOST * audioIn;
		if (ftMode == FTSynthesisOne)
		{
			sample = prevInput;
			
			//sample = tButterworthTick(filter, sample);
			//tSVFSetFreq(oldFilter, testFreq);
			sample = tSVFTick(oldFilter, sample);
			//sample = tCompressorTick(myCompressor, sample);
			
			//sample = mix * OOPS_clip(-1.0f, sample, 1.0f) + (1.0f - mix) * tCompressorTick(myCompressor, sample);
			//sample = OOPS_clip(-1.0f, sample, 1.0f);

			//tSVFSetQ(lp, tRampTick(adc[ADCPedal]));
			/*
			intPeak = 48.9994294977f * (intHarmonic - 1);

			testFreq -= 0.001f;
			if (testFreq <= 0.0f) testFreq = 2000.0f;
			tSVFSetFreq(oldFilter, testFreq);
			tSVFSetQ(oldFilter, 200.0f);
			lp->g = oldFilter->g;
			lp->a1 = oldFilter->a1;
			lp->a2 = oldFilter->a2;
			lp->a3 = oldFilter->a3;
			*/
			//tSVFSetFreq(lp, floatPeak);
			//tSVFSetQ(lp, 200.0f);

			//sample = tSVFTick(oldFilter, sample);

			//sample *= .8f;
			
			
			testDelay = 100.0f;
			//if (testDelay >= 128.0f) testDelay = 1.0f;
			
			//testDelay = knobValueToUse * 511.0f + 1.0f;
			tDelayLSetDelay(feedbackDelay, testDelay);
			
			//tDelayLSetDelay(feedbackDelay,212.0f);
			sample = tDelayLTick(feedbackDelay, sample);
			
			// comment back in
			//sample *= rampedBreath;
			
			//sample *= pedal;
			
			sample = OOPS_clip(-1.0f, sample, 1.0f);
			//sample *= OUTPUT_GAIN; 
			

	
	}
	return sample;
}

float audioTickOld(float audioIn) 
{
	//choose harmonic
	knobValueToUse = tRampTick(adc[ADCKnob]);
	
	if (hMode == CaraMode)
	{
		floatHarmonic = (NUM_HARMONICS * tRampTick(adc[ADCJoyY])) + 1.0f; // sets which harmonic to focus on
	}
	else if (hMode == RajeevMode)
	{
		floatHarmonic = NUM_HARMONICS * (1.0f - tRampTick(adc[ADCJoyY])) + 1.0f;
	}
	else if (hMode == JennyMode)
	{
		floatHarmonic = tRampTick(adc[ADCJoyY]) * 2.0f - 1.0f;
		floatHarmonic = (floatHarmonic < 0.0f) ? 1.0f : (floatHarmonic * NUM_HARMONICS + 1.0f);
	}
	
	if (((floatHarmonic - intHarmonic) > (harmonicHysteresis)) || ((floatHarmonic - intHarmonic) < ( -1.0f * harmonicHysteresis)))
	{
		intHarmonic = (uint16_t) (floatHarmonic + 0.5f);
	}
	
	floatPeak = fundamental * floatHarmonic * octaveTransp[octave];
	intPeak = fundamental * intHarmonic * octaveTransp[octave];
	
	//some code to jump octaves if desired
	//floatPeak = peak * powf(2.0f, (float)((uint8_t)(tRampSample(adc[ADCPedal]) * 2.9f)));

	float pedal = tRampTick(adc[ADCPedal]);
	
	breath = adcVals[ADCBreath];
	breath = breath * INV_TWO_TO_12;
	breath = breath - breath_baseline;
	breath = breath * breath_mult;
	breath *= amp_mult;
	
	if (breath < 0.0f)					breath = 0.0f;
	else if (breath > 1.0f)		  breath = 1.0f;
	
	tRampSetDest(adc[ADCBreath], breath);
	
	rampedBreath = tRampTick(adc[ADCBreath]);
	if (ftMode == FTSynthesisOne)
	{
	
		
		
		
		tCycleSetFreq(mySine, floatPeak);
		
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
		
		//sample = (sample * (1.0f - pedal)) + (OOPS_CompoundChebyshevT(sample, 7, amps) * pedal);
		//sample *= 1.0f + pedal;
		
		//sample *= rampedBreath;
	
		if (kMode == DelayTune)
		{
			feedbackAmt = knobValueToUse;
			
			thisDelayOut = tDelayLTick(myDelay, sample + (feedbackAmt * lastDelayOut));
			
			sample = (0.5f * sample) + (0.5f * thisDelayOut);
			
			lastDelayOut = thisDelayOut;
		}
		
		//sample = OOPS_softClip(sample, .8f);
		//sample *= pedal;
		
		//sample *= amplitude;
		//sample *= tRampTick(adc[ADCPedal]) * amplitude;
	}
	else
	{
		//sample = INPUT_BOOST * amplitude  * audioIn;
		//sample = (sample * (1.0f - mix)) + (tNoiseTick(noise) * mix);
		//sample = INPUT_BOOST * audioIn;
	
		sample = audioIn;
		//tButterworthSetFreqs(filter, floatPeak  - cutoff_offset, floatPeak + cutoff_offset);
		//sample = tButterworthTick(filter, sample);
		
		//sample = tCompressorTick(myCompressor, sample);
		
		//sample = mix * OOPS_clip(-1.0f, sample, 1.0f) + (1.0f - mix) * tCompressorTick(myCompressor, sample);
		//sample = OOPS_clip(-1.0f, sample, 1.0f);

		//tSVFSetQ(lp, tRampTick(adc[ADCPedal]));
		/*
		intPeak = 48.9994294977f * (intHarmonic - 1);
		tSVFSetFreq(oldFilter, floatPeak);
		tSVFSetQ(oldFilter, 200.0f);
		lp->g = oldFilter->g;
		lp->a1 = oldFilter->a1;
		lp->a2 = oldFilter->a2;
		lp->a3 = oldFilter->a3;
		*/
		//tSVFSetFreq(lp, floatPeak);

		//sample = tSVFTick(oldFilter, sample);
		//sample = tSVFTick(lp, sample);
		//sample = sample * 12.0f;
		//sample = tNoiseTick(noise);

		//sample *= .8f;

		//tDelayLSetDelay(feedbackDelay,knobValueToUse * 256.0f);
		//tDelayLSetDelay(feedbackDelay,212.0f);
		//sample = tDelayLTick(feedbackDelay, sample);
		
		//sample *= 10.0f;
		//sample = OOPS_softClip(sample, 1.0f - pedal);

		//sample = tNoiseTick(noise);
		
		//sample *= rampedBreath;
		//sample *= pedal;
		
		sample = OOPS_clip(-1.0f, sample, 1.0f);
		//sample *= OUTPUT_GAIN; 
		/*
		if (counter > 24000)
		{
			sample = 1.0f;
			counter = 0; 
		}
		else
		{
			sample = 0.0f;
		}
		counter++;
		*/
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

