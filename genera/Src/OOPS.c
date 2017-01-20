/*
  ==============================================================================

    OOPS.c
    Created: 18 Jan 2017 12:40:15pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#include "OOPS.h"

// The C-embedded Audio Library.

#include "math.h"

#define TWO_TO_16 65536.f

OOPS *oops; //No touch.

#define EXPONENTIAL_TABLE_SIZE 65536

typedef enum TableName
{
    T20 = 0,
    T40,
    T80,
    T160,
    T320,
    T640,
    T1280,
    T2560,
    T5120,
    T10240,
    T20480,
    TableNameNil
} TableName;

float   OOPS_clip(float min, float val, float max) {
    
    if (val < min) {
        return min;
    } else if (val > max) {
        return max;
    } else {
        return val;
    }
}

int     OOPS_isPrime(uint64_t number )
{
    if ( number == 2 ) 
			return 1;
    if ( number & 1 ) 
		{
        for ( int i=3; i<(int)sqrt((double)number)+1; i+=2 )
            if ( (number % i) == 0 ) return 0;
        return 1; // prime
    }
    else return 0; // even
}

// Adapted from MusicDSP: http://www.musicdsp.org/showone.php?id=238
float OOPS_tanh(float x)
{
    
    if( x < -3 )
        return -1;
    else if( x > 3 )
        return 1;
    else
        return x * ( 27 + x * x ) / ( 27 + 9 * x * x );
}

//-----------------------------------------------------------------------------
// name: mtof()
// desc: midi to freq, from PD source
//-----------------------------------------------------------------------------
double OOPS_midiToFrequency( double f )
{
    if( f <= -1500 ) return (0);
    else if( f > 1499 ) return (OOPS_midiToFrequency(1499));
    else return ( pow(2, (f - 69) / 12.0) * 440.0 );
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //
void     tPhasorSampleRateChanged (tPhasor *p);
void     tCycleSampleRateChanged (tCycle *c);
void     tSawtoothSampleRateChanged (tSawtooth *c);
void     tTriangleSampleRateChanged (tTriangle *c);
void     tSquareSampleRateChanged (tSquare *c);
void     tRampSampleRateChanged(tRamp *r);
void     tNoiseSampleRateChanged (tNoise *c);
void     tOnePoleSampleRateChanged (tOnePole *c);
void     tTwoPoleSampleRateChanged (tTwoPole *c);
void     tOneZeroSampleRateChanged (tOneZero *c);
void     tTwoZeroSampleRateChanged (tTwoZero *c);
void     tPoleZeroSampleRateChanged (tPoleZero *c);
void     tBiQuadSampleRateChanged (tBiQuad *c);
void     tSVFSampleRateChanged (tSVF *c);
void     tSVFEfficientSampleRateChanged (tSVFEfficient *c);
void     tHighpassSampleRateChanged (tHighpass *c);
void     tDelaySampleRateChanged (tDelay *c);
void     tDelayLSampleRateChanged (tDelayL *c);
void     tDelayASampleRateChanged (tDelayA *c);
void     tEnvelopeSampleRateChanged (tEnvelope *c);
void     tADSRSampleRateChanged (tADSR *c);
void     tEnvelopeFollowerSampleRateChanged (tEnvelopeFollower *c);
void     tPRCRevSampleRateChanged (tPRCRev *c);
void     tNRevSampleRateChanged (tNRev *c);
void     tPluckSampleRateChanged (tPluck *c);
void    tStifKarpSampleRateChanged (tStifKarp *c);

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ PRCRev ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //

void    tPRCRevSetT60(tPRCRev *r, float t60)
{
    if ( t60 <= 0.0 ) t60 = 0.001f;
    
    r->combCoeff = pow(10.0, (-3.0 * tDelayGetDelay(&r->combDelay) * oops->invSampleRate / t60 ));
    
}

void    tPRCRevSetMix(tPRCRev *r, float mix)
{
    r->mix = mix;
}

float   tPRCRevTick(tPRCRev *r, float input)
{
    float temp, temp0, temp1, temp2;
    float out;
    
    r->lastIn = input;
    
    temp = tDelayGetLastOut(&r->allpassDelays[0]);
    temp0 = r->allpassCoeff * temp;
    temp0 += input;
    tDelayTick(&r->allpassDelays[0], temp0);
    temp0 = -(r->allpassCoeff * temp0) + temp;
    
    temp = tDelayGetLastOut(&r->allpassDelays[1]);
    temp1 = r->allpassCoeff * temp;
    temp1 += temp0;
    tDelayTick(&r->allpassDelays[1], temp1);
    temp1 = -(r->allpassCoeff * temp1) + temp;
    
    temp2 = temp1 + ( r->combCoeff * tDelayGetLastOut(&r->combDelay));
    
    out = r->mix * tDelayTick(&r->combDelay,temp2);
    
    temp = (1.0 - r->mix) * input;
    
    out += temp;
    
    r->lastOut = out;
    
    return out;
}


int     tPRCRevInit(tPRCRev *r, float t60, float delayBuffers[3][REV_DELAY_LENGTH])
{
    if (t60 <= 0.0) t60 = 0.001f;
    
    r->inv_441 = 1.0f/44100.0f;
    
    int lengths[4] = { 341, 613, 1557, 2137 }; // Delay lengths for 44100 Hz sample rate.
    double scaler = oops->sampleRate * r->inv_441;
    
    int delay, i;
    if (scaler != 1.0)
    {
        for (i=0; i<4; i++)
        {
            delay = (int) scaler * lengths[i];
            if ( (delay & 1) == 0)
                delay++;
            while ( !OOPS_isPrime(delay) )
                delay += 2;
            lengths[i] = delay;
        }
    }
    
    tDelayInit(&r->allpassDelays[0],    lengths[0], REV_DELAY_LENGTH, delayBuffers[0]);
    tDelayInit(&r->allpassDelays[1],    lengths[1], REV_DELAY_LENGTH, delayBuffers[1]);
    tDelayInit(&r->combDelay,           lengths[2], REV_DELAY_LENGTH, delayBuffers[2]);
    
    tPRCRevSetT60(r, t60);
    
    r->allpassCoeff = 0.7f;
    r->mix = 0.5f;
    
    OOPS_tPRCRevRegister(r);
    
    r->sampleRateChanged = &tPRCRevSampleRateChanged;
    
    return 1;
}

void    tNRevSetT60(tNRev *r, float t60)
{
    if (t60 <= 0.0)           t60 = 0.001f;
    
    for (int i=0; i<6; i++)   r->combCoeffs[i] = pow(10.0, (-3.0 * tDelayGetDelay(&r->combDelays[i]) * oops->invSampleRate / t60 ));
    
}

void    tNRevSetMix(tNRev *r, float mix)
{
    r->mix = mix;
}

float   tNRevTick(tNRev *r, float input)
{
    r->lastIn = input;
    
    float temp, temp0, temp1, temp2, out;
    int i;
    
    temp0 = 0.0;
    for ( i=0; i<6; i++ )
    {
        temp = input + (r->combCoeffs[i] * tDelayGetLastOut(&r->combDelays[i]));
        temp0 += tDelayTick(&r->combDelays[i],temp);
    }
    
    for ( i=0; i<3; i++ )
    {
        temp = tDelayGetLastOut(&r->allpassDelays[i]);
        temp1 = r->allpassCoeff * temp;
        temp1 += temp0;
        tDelayTick(&r->allpassDelays[i], temp1);
        temp0 = -(r->allpassCoeff * temp1) + temp;
    }
    
    // One-pole lowpass filter.
    r->lowpassState = 0.7f * r->lowpassState + 0.3f * temp0;
    temp = tDelayGetLastOut(&r->allpassDelays[3]);
    temp1 = r->allpassCoeff * temp;
    temp1 += r->lowpassState;
    tDelayTick(&r->allpassDelays[3], temp1 );
    temp1 = -(r->allpassCoeff * temp1) + temp;
    
    temp = tDelayGetLastOut(&r->allpassDelays[4]);
    temp2 = r->allpassCoeff * temp;
    temp2 += temp1;
    tDelayTick(&r->allpassDelays[4], temp2 );
    out = r->mix * ( -( r->allpassCoeff * temp2 ) + temp );
    
    /*
     temp = tDelayLGetLastOut(&r->allpassDelays[5]);
     temp3 = r->allpassCoeff * temp;
     temp3 += temp1;
     tDelayLTick(&r->allpassDelays[5], temp3 );
     lastFrame_[1] = effectMix_*( - ( r->allpassCoeff * temp3 ) + temp );
     */
    
    temp = ( 1.0f - r->mix ) * input;
    
    out += temp;
    
    r->lastOut = out;
    
    return out;
}


int     tNRevInit(tNRev *r, float t60, float delayBuffers[14][REV_DELAY_LENGTH])
{
    if (t60 <= 0.0) t60 = 0.001f;
    
    r->inv_441 = 1.0f/44100.0f;
    
    int lengths[15] = {1433, 1601, 1867, 2053, 2251, 2399, 347, 113, 37, 59, 53, 43, 37, 29, 19}; // Delay lengths for 44100 Hz sample rate.
    double scaler = oops->sampleRate / 25641.0f;
    
    int delay, i;
    
    for (i=0; i < 15; i++)
    {
        delay = (int) scaler * lengths[i];
        if ( (delay & 1) == 0)
            delay++;
        while ( !OOPS_isPrime(delay) )
            delay += 2;
        lengths[i] = delay;
    }
    
    for ( i=0; i<6; i++ )
    {
        tDelayInit(&r->combDelays[i], lengths[i], REV_DELAY_LENGTH, delayBuffers[i]);
        r->combCoeffs[i] = pow(10.0, (-3 * lengths[i] * oops->invSampleRate / t60));
    }
    
    for ( i=0; i<8; i++ )
        tDelayInit(&r->allpassDelays[i], lengths[i+6], REV_DELAY_LENGTH, delayBuffers[i+6]);
    
    for ( i=0; i<2; i++ )
    {
        tDelaySetDelay(&r->allpassDelays[i], lengths[i]);
        tDelaySetDelay(&r->combDelays[i], lengths[i+2]);
    }
    
    tNRevSetT60(r, t60);
    r->allpassCoeff = 0.7f;
    r->mix = 0.3f;
    
    OOPS_tNRevRegister(r);
    r->sampleRateChanged = &tNRevSampleRateChanged;
    
    
    return 0;
}

#pragma mark - Filters
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ OneZero Filter ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //
int     tOneZeroInit(tOneZero *f, float theZero)
{
    f->gain = 1.0f;
    f->lastIn = 0.0f;
    f->lastOut = 0.0f;
    tOneZeroSetZero(f, theZero);
    
    OOPS_tOneZeroRegister(f);
    f->sampleRateChanged = &tOneZeroSampleRateChanged;
    
    return 0;
}

float   tOneZeroTick(tOneZero *f, float input)
{
    float in = input * f->gain;
    float out = f->b1 * f->lastIn + f->b0 * in;
    
    f->lastIn = in;
    
    return out;
}

void    tOneZeroSetZero(tOneZero *f, float theZero)
{
    if (theZero > 0.0f) f->b0 = 1.0f / (1.0f + theZero);
    else                f->b0 = 1.0f / (1.0f - theZero);
    
    f->b1 = -theZero * f->b0;
    
}

void    tOneZeroSetB0(tOneZero *f, float b0)
{
    f->b0 = b0;
}

void    tOneZeroSetB1(tOneZero *f, float b1)
{
    f->b1 = b1;
}

void    tOneZeroSetCoefficients(tOneZero *f, float b0, float b1)
{
    f->b0 = b0;
    f->b1 = b1;
}

void    tOneZeroSetGain(tOneZero *f, float gain)
{
    f->gain = gain;
}

float   tOneZeroGetPhaseDelay(tOneZero *f, float frequency )
{
    if ( frequency <= 0.0) frequency = 0.05f;
    
    float omegaT = 2 * PI * frequency * oops->invSampleRate;
    float real = 0.0, imag = 0.0;
    
    real += f->b0;
    
    real += f->b1 * cosf(omegaT);
    imag -= f->b1 * sinf(omegaT);
    
    real *= f->gain;
    imag *= f->gain;
    
    float phase = atan2f( imag, real );
    
    real = 0.0, imag = 0.0;
    
    phase -= atan2f( imag, real );
    
    phase = fmodf( -phase, 2 * PI );
    
    return phase / omegaT;
}


// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ OneZero Filter ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //
int     tTwoZeroInit(tTwoZero *f)
{
    f->gain = 1.0f;
    f->lastIn[0] = 0.0f;
    f->lastIn[1] = 0.0f;
    
    OOPS_tTwoZeroRegister(f);
    f->sampleRateChanged = &tTwoZeroSampleRateChanged;
    
    return 0;
}

float   tTwoZeroTick(tTwoZero *f, float input)
{
    float in = input * f->gain;
    float out = f->b2 * f->lastIn[1] + f->b1 * f->lastIn[0] + f->b0 * in;
    
    f->lastIn[1] = f->lastIn[0];
    f->lastIn[0] = in;
    
    return out;
}

void    tTwoZeroSetNotch(tTwoZero *f, float freq, float radius)
{
    // Should also deal with frequency being > half sample rate / nyquist. See STK
    if (freq < 0.0f)    freq = 0.0f;
    if (radius < 0.0f)  radius = 0.0f;
    
    f->b2 = radius * radius;
    f->b1 = -2.0f * radius * cosf(TWO_PI * freq * oops->invSampleRate); // OPTIMIZE with LOOKUP or APPROXIMATION
    
    // Normalize the filter gain. From STK.
    if ( f->b1 > 0.0f ) // Maximum at z = 0.
        f->b0 = 1.0f / ( 1.0f + f->b1 + f->b2 );
    else            // Maximum at z = -1.
        f->b0 = 1.0f / ( 1.0f - f->b1 + f->b2 );
    f->b1 *= f->b0;
    f->b2 *= f->b0;
    
}

void    tTwoZeroSetB0(tTwoZero *f, float b0)
{
    f->b0 = b0;
}

void    tTwoZeroSetB1(tTwoZero *f, float b1)
{
    f->b1 = b1;
}

void    tTwoZeroSetCoefficients(tTwoZero *f, float b0, float b1, float b2)
{
    f->b0 = b0;
    f->b1 = b1;
    f->b2 = b2;
}

void    tTwoZeroSetGain(tTwoZero *f, float gain)
{
    f->gain = gain;
}


// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ OnePole Filter ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //

void    tOnePoleSetB0(tOnePole *f, float b0)
{
    f->b0 = b0;
}

void    tOnePoleSetA1(tOnePole *f, float a1)
{
    if (a1 >= 1.0f)     a1 = 0.999999f;
    
    f->a1 = a1;
}

void    tOnePoleSetPole(tOnePole *f, float thePole)
{
    if (thePole >= 1.0f)    thePole = 0.999999f;
    
    // Normalize coefficients for peak unity gain.
    if (thePole > 0.0f)     f->b0 = (1.0f - thePole);
    else                    f->b0 = (1.0f + thePole);
    
    f->a1 = -thePole;
}

void    tOnePoleSetCoefficients(tOnePole *f, float b0, float a1)
{
    if (a1 >= 1.0f)     a1 = 0.999999f;
    
    f->b0 = b0;
    f->a1 = a1;
}

void    tOnePoleSetGain(tOnePole *f, float gain)
{
    f->gain = gain;
}

float   tOnePoleTick(tOnePole *f, float input)
{
    float in = input * f->gain;
    float out = (f->b0 * in) - (f->a1 * f->lastOut);
    
    f->lastIn = in;
    f->lastOut = out;
    
    return out;
}

int     tOnePoleInit(tOnePole *f, float thePole)
{
    f->gain = 1.0f;
    f->a0 = 1.0;
    
    tOnePoleSetPole(f, thePole);
    
    f->lastIn = 0.0f;
    f->lastOut = 0.0f;
    
    OOPS_tOnePoleRegister(f);
    f->sampleRateChanged = &tOnePoleSampleRateChanged;
    
    
    return 0;
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ TwoPole Filter ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //
int     tTwoPoleInit(tTwoPole *f)
{
    f->gain = 1.0f;
    f->a0 = 1.0;
    f->b0 = 1.0;
    
    f->lastOut[0] = 0.0f;
    f->lastOut[1] = 0.0f;
    
    OOPS_tTwoPoleRegister(f);
    f->sampleRateChanged = &tTwoPoleSampleRateChanged;
    
    return 0;
}

float   tTwoPoleTick(tTwoPole *f, float input)
{
    float in = input * f->gain;
    float out = (f->b0 * in) - (f->a1 * f->lastOut[0]) - (f->a2 * f->lastOut[1]);
    
    f->lastOut[1] = f->lastOut[0];
    f->lastOut[0] = out;
    
    return out;
}

void    tTwoPoleSetB0(tTwoPole *f, float b0)
{
    f->b0 = b0;
}

void    tTwoPoleSetA1(tTwoPole *f, float a1)
{
    f->a1 = a1;
}

void    tTwoPoleSetA2(tTwoPole *f, float a2)
{
    f->a2 = a2;
}


void    tTwoPoleSetResonance(tTwoPole *f, float frequency, float radius, int normalize)
{
    if (frequency < 0.0f)   frequency = 0.0f; // need to also handle when frequency > nyquist
    if (radius < 0.0f)      radius = 0.0f;
    if (radius >= 1.0f)     radius = 0.999999f;
    
    f->a2 = radius * radius;
    f->a1 =  -2.0f * radius * cos(TWO_PI * frequency * oops->invSampleRate);
    
    if ( normalize )
    {
        // Normalize the filter gain ... not terribly efficient.
        float real = 1 - radius + (f->a2 - radius) * cos(TWO_PI * 2 * frequency * oops->invSampleRate);
        float imag = (f->a2 - radius) * sin(TWO_PI * 2 * frequency * oops->invSampleRate);
        f->b0 = sqrt( pow(real, 2) + pow(imag, 2) );
        
        // NEED TO OPTIMIZE and make sure sqrt/pow/cos are compatible. How do we want to handle this for platforms that it won't be compatible on?
    }
}

void    tTwoPoleSetCoefficients(tTwoPole *f, float b0, float a1, float a2)
{
    f->b0 = b0;
    f->a1 = a1;
    f->a2 = a2;
}

void    tTwoPoleSetGain(tTwoPole *f, float gain)
{
    f->gain = gain;
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ PoleZero Filter ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //

void    tPoleZeroSetB0(tPoleZero *pzf, float b0)
{
    pzf->b0 = b0;
}

void    tPoleZeroSetB1(tPoleZero *pzf, float b1)
{
    pzf->b1 = b1;
}

void    tPoleZeroSetA1(tPoleZero *pzf, float a1)
{
    if (a1 >= 1.0f) // a1 should be less than 1.0
    {
        a1 = 0.999999f;
    }
    
    pzf->a1 = a1;
}

void    tPoleZeroSetCoefficients(tPoleZero *pzf, float b0, float b1, float a1)
{
    if (a1 >= 1.0f) // a1 should be less than 1.0
    {
        a1 = 0.999999f;
    }
    
    pzf->b0 = b0;
    pzf->b1 = b1;
    pzf->a1 = a1;
}

void    tPoleZeroSetAllpass(tPoleZero *pzf, float coeff)
{
    if (coeff >= 1.0f) // allpass coefficient >= 1.0 makes filter unstable
    {
        coeff = 0.999999f;
    }
    
    pzf->b0 = coeff;
    pzf->b1 = 1.0f;
    pzf->a0 = 1.0f;
    pzf->a1 = coeff;
}

void    tPoleZeroSetBlockZero(tPoleZero *pzf, float thePole)
{
    if (thePole >= 1.0f) // allpass coefficient >= 1.0 makes filter unstable
    {
        thePole = 0.999999f;
    }
    
    pzf->b0 = 1.0f;
    pzf->b1 = -1.0f;
    pzf->a0 = 1.0f;
    pzf->a1 = -thePole;
}

void    tPoleZeroSetGain(tPoleZero *pzf, float gain)
{
    pzf->gain = gain;
}

float   tPoleZeroTick(tPoleZero *pzf, float input)
{
    float in = input * pzf->gain;
    float out = (pzf->b0 * in) + (pzf->b1 * pzf->lastIn) - (pzf->a1 * pzf->lastOut);
    
    pzf->lastIn = in;
    pzf->lastOut = out;
    
    return out;
}

int     tPoleZeroInit(tPoleZero *pzf)
{
    pzf->gain = 1.0f;
    pzf->b0 = 1.0;
    pzf->a0 = 1.0;
    
    pzf->lastIn = 0.0f;
    pzf->lastOut = 0.0f;
    
    OOPS_tPoleZeroRegister(pzf);
    pzf->sampleRateChanged = &tPoleZeroSampleRateChanged;
    
    return 1;
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ BiQuad Filter ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //
int     tBiQuadInit(tBiQuad *f)
{
    f->gain = 1.0f;
    
    f->b0 = 0.0f;
    f->a0 = 0.0f;
    
    f->lastIn[0] = 0.0f;
    f->lastIn[1] = 0.0f;
    f->lastOut[0] = 0.0f;
    f->lastOut[1] = 0.0f;
    
    OOPS_tBiQuadRegister(f);
    f->sampleRateChanged = &tBiQuadSampleRateChanged;
    
    return 0;
}

float   tBiQuadTick(tBiQuad *f, float input)
{
    float in = input * f->gain;
    float out = f->b0 * in + f->b1 * f->lastIn[0] + f->b2 * f->lastIn[1];
    out -= f->a2 * f->lastOut[1] + f->a1 * f->lastOut[0];
    
    f->lastIn[1] = f->lastIn[0];
    f->lastIn[0] = in;
    
    f->lastOut[1] = f->lastOut[0];
    f->lastOut[0] = out;
    
    return out;
}

void    tBiQuadSetResonance(tBiQuad *f, float freq, float radius, int normalize)
{
    // Should also deal with frequency being > half sample rate / nyquist. See STK
    if (freq < 0.0f)    freq = 0.0f;
    if (radius < 0.0f)  radius = 0.0f;
    if (radius >= 1.0f)  radius = 1.0f;
    
    f->a2 = radius * radius;
    f->a1 = -2.0f * radius * cosf(TWO_PI * freq * oops->invSampleRate);
    
    if (normalize)
    {
        f->b0 = 0.5f - 0.5f * f->a2;
        f->b1 = 0.0f;
        f->b2 = -f->b0;
    }
}

void    tBiQuadSetNotch(tBiQuad *f, float freq, float radius)
{
    // Should also deal with frequency being > half sample rate / nyquist. See STK
    if (freq < 0.0f)    freq = 0.0f;
    if (radius < 0.0f)  radius = 0.0f;
    
    f->b2 = radius * radius;
    f->b1 = -2.0f * radius * cosf(TWO_PI * freq * oops->invSampleRate); // OPTIMIZE with LOOKUP or APPROXIMATION
    
    // Does not attempt to normalize filter gain.
}

void tBiQuadSetEqualGainZeros(tBiQuad *f)
{
    f->b0 = 1.0f;
    f->b1 = 0.0f;
    f->b2 = -1.0f;
}

void    tBiQuadSetB0(tBiQuad *f, float b0)
{
    f->b0 = b0;
}

void    tBiQuadSetB1(tBiQuad *f, float b1)
{
    f->b1 = b1;
}

void    tBiQuadSetB2(tBiQuad *f, float b2)
{
    f->b2 = b2;
}

void    tBiQuadSetA1(tBiQuad *f, float a1)
{
    f->a1 = a1;
}

void    tBiQuadSetA2(tBiQuad *f, float a2)
{
    f->a2 = a2;
}

void    tBiQuadSetCoefficients(tBiQuad *f, float b0, float b1, float b2, float a1, float a2)
{
    f->b0 = b0;
    f->b1 = b1;
    f->b2 = b2;
    f->a1 = a1;
    f->a2 = a2;
}

void    tBiQuadSetGain(tBiQuad *f, float gain)
{
    f->gain = gain;
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ Delay ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //
int     tDelayInit (tDelay *d, uint32_t delay, uint32_t maxDelay, float *buff)
{
    if (delay > maxDelay)   d->delay = maxDelay;
    else                    d->delay = delay;
    
    d->maxDelay = maxDelay;
    d->buff = buff;
    
    d->inPoint = 0;
    d->outPoint = 0;
    
    d->lastIn = 0.0f;
    d->lastOut = 0.0f;
    
    d->gain = 1.0f;
    
    tDelaySetDelay(d, d->delay);
    
    OOPS_tDelayRegister(d);
    d->sampleRateChanged = &tDelaySampleRateChanged;
    
    return 0;
}

float   tDelayTick (tDelay *d, float input)
{
    // Input
    d->lastIn = input;
    d->buff[d->inPoint++] = input * d->gain;
    if ( d->inPoint == d->maxDelay)     d->inPoint = 0;
    
    // Output
    d->lastOut = d->buff[d->outPoint++];
    if ( d->outPoint == d->maxDelay)    d->outPoint = 0;
    
    return d->lastOut;
}


int     tDelaySetDelay (tDelay *d, uint32_t delay)
{
    if (delay >= d->maxDelay)    d->delay = d->maxDelay;
    else                         d->delay = delay;
    
    // read chases write
    if ( d->inPoint >= delay )  d->outPoint = d->inPoint - d->delay;
    else                        d->outPoint = d->maxDelay + d->inPoint - d->delay;
    
    return 0;
}

float tDelayTapOut (tDelay *d, uint32_t tapDelay)
{
    int32_t tap = d->inPoint - tapDelay - 1;
    
    // Check for wraparound.
    while ( tap < 0 )   tap += d->maxDelay;
    
    return d->buff[tap];
    
}

void tDelayTapIn (tDelay *d, float value, uint32_t tapDelay)
{
    int32_t tap = d->inPoint - tapDelay - 1;
    
    // Check for wraparound.
    while ( tap < 0 )   tap += d->maxDelay;
    
    d->buff[tap] = value;
}

float tDelayAddTo (tDelay *d, float value, uint32_t tapDelay)
{
    int32_t tap = d->inPoint - tapDelay - 1;
    
    // Check for wraparound.
    while ( tap < 0 )   tap += d->maxDelay;
    
    return (d->buff[tap] += value);
}

uint32_t   tDelayGetDelay (tDelay *d)
{
    return d->delay;
}

float   tDelayGetLastOut (tDelay *d)
{
    return d->lastOut;
}

float   tDelayGetLastIn (tDelay *d)
{
    return d->lastIn;
}

void tDelaySetGain (tDelay *d, float gain)
{
    if (gain < 0.0f)    d->gain = 0.0f;
    else                d->gain = gain;
}

float tDelayGetGain (tDelay *d)
{
    return d->gain;
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ DelayL ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //
int     tDelayLInit (tDelayL *d, float delay, uint32_t maxDelay, float *buff)
{
    d->gain = 1.0f;
    
    d->lastIn = 0.0f;
    d->lastOut = 0.0f;
    
    if (delay < 0.0f)           d->delay = 0.0f;
    else if (delay <= maxDelay) d->delay = delay;
    else                        d->delay = maxDelay;
    
    d->maxDelay = maxDelay;
    d->buff = buff;
    
    d->inPoint = 0;
    d->outPoint = 0;
    
    tDelayLSetDelay(d, d->delay);
    
    OOPS_tDelayLRegister(d);
    d->sampleRateChanged = &tDelayLSampleRateChanged;
    
    return 0;
}

float   tDelayLTick (tDelayL *d, float input)
{
    d->buff[d->inPoint++] = input * d->gain;
    
    // Increment input pointer modulo length.
    if ( d->inPoint == d->maxDelay )    d->inPoint = 0;
    
    
    // First 1/2 of interpolation
    d->lastOut = d->buff[d->outPoint] * d->omAlpha;
    
    // Second 1/2 of interpolation
    if (d->outPoint + 1 < d->maxDelay)
        d->lastOut += d->buff[d->outPoint+1] * d->alpha;
    else
        d->lastOut += d->buff[0] * d->alpha;
    
    // Increment output pointer modulo length.
    if ( ++(d->outPoint) == d->maxDelay )   d->outPoint = 0;
    
    return d->lastOut;
}

int     tDelayLSetDelay (tDelayL *d, float delay)
{
    if (delay < 0.0f)               d->delay = 0.0f;
    else if (delay <= d->maxDelay)  d->delay = delay;
    else                            d->delay = d->maxDelay;
    
    float outPointer = d->inPoint - d->delay;
    
    while ( outPointer < 0 )
        outPointer += d->maxDelay; // modulo maximum length
    
    d->outPoint = (uint32_t) outPointer;   // integer part
    
    d->alpha = outPointer - d->outPoint; // fractional part
    d->omAlpha = 1.0f - d->alpha;
    
    if ( d->outPoint == d->maxDelay ) d->outPoint = 0;
    
    return 0;
}

float tDelayLTapOut (tDelayL *d, uint32_t tapDelay)
{
    int32_t tap = d->inPoint - tapDelay - 1;
    
    // Check for wraparound.
    while ( tap < 0 )   tap += d->maxDelay;
    
    return d->buff[tap];
    
}

void tDelayLTapIn (tDelayL *d, float value, uint32_t tapDelay)
{
    int32_t tap = d->inPoint - tapDelay - 1;
    
    // Check for wraparound.
    while ( tap < 0 )   tap += d->maxDelay;
    
    d->buff[tap] = value;
}

float tDelayLAddTo (tDelayL *d, float value, uint32_t tapDelay)
{
    int32_t tap = d->inPoint - tapDelay - 1;
    
    // Check for wraparound.
    while ( tap < 0 )   tap += d->maxDelay;
    
    return (d->buff[tap] += value);
}

float   tDelayLGetDelay (tDelayL *d)
{
    return d->delay;
}

float   tDelayLGetLastOut (tDelayL *d)
{
    return d->lastOut;
}

float   tDelayLGetLastIn (tDelayL *d)
{
    return d->lastIn;
}

void tDelayLSetGain (tDelayL *d, float gain)
{
    if (gain < 0.0f)    d->gain = 0.0f;
    else                d->gain = gain;
}

float tDelayLGetGain (tDelayL *d)
{
    return d->gain;
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ DelayA ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //
int     tDelayAInit (tDelayA *d, float delay, uint32_t maxDelay, float *buff)
{
    d->gain = 1.0f;
    
    d->lastIn = 0.0f;
    d->lastOut = 0.0f;
    
    if (delay < 0.5f)           d->delay = 0.5f;
    else if (delay <= maxDelay) d->delay = delay;
    else                        d->delay = maxDelay;
    
    d->maxDelay = maxDelay;
    d->buff = buff;
    
    d->inPoint = 0;
    d->outPoint = 0;
    
    tDelayASetDelay(d, d->delay);
    
    d->apInput = 0.0f;
    
    OOPS_tDelayARegister(d);
    d->sampleRateChanged = &tDelayASampleRateChanged;
    
    return 0;
}

float   tDelayATick (tDelayA *d, float input)
{
    d->buff[d->inPoint++] = input * d->gain;
    
    // Increment input pointer modulo length.
    if ( d->inPoint >= d->maxDelay )    d->inPoint = 0;
    
    // Do allpass interpolation delay.
    float out = d->lastOut * -d->coeff;
    out += d->apInput + ( d->coeff * d->buff[d->outPoint] );
    d->lastOut = out;
    
    // Save allpass input
    d->apInput = d->buff[d->outPoint++];
    
    // Increment output pointer modulo length.
    if (d->outPoint >= d->maxDelay )   d->outPoint = 0;
    
    return d->lastOut;
}

int     tDelayASetDelay (tDelayA *d, float delay)
{
    if (delay < 0.5f)               d->delay = 0.5f;
    else if (delay <= d->maxDelay)  d->delay = delay;
    else                            d->delay = d->maxDelay;
    
    // outPoint chases inPoint
    float outPointer = (float)d->inPoint - d->delay + 1.0f;
    
    while ( outPointer < 0 )    outPointer += d->maxDelay;  // mod max length
    
    d->outPoint = (uint32_t) outPointer;         // integer part
    
    if ( d->outPoint >= d->maxDelay )   d->outPoint = 0;
    
    d->alpha = 1.0f + (float)d->outPoint - outPointer; // fractional part
    
    if ( d->alpha < 0.5f )
    {
        // The optimal range for alpha is about 0.5 - 1.5 in order to
        // achieve the flattest phase delay response.
        
        d->outPoint += 1;
        
        if ( d->outPoint >= d->maxDelay ) d->outPoint -= d->maxDelay;
        
        d->alpha += 1.0f;
    }
    
    d->coeff = (1.0f - d->alpha) / (1.0f + d->alpha);  // coefficient for allpass
    
    return 0;
}

float tDelayATapOut (tDelayA *d, uint32_t tapDelay)
{
    int32_t tap = d->inPoint - tapDelay - 1;
    
    // Check for wraparound.
    while ( tap < 0 )   tap += d->maxDelay;
    
    return d->buff[tap];
    
}

void tDelayATapIn (tDelayA *d, float value, uint32_t tapDelay)
{
    int32_t tap = d->inPoint - tapDelay - 1;
    
    // Check for wraparound.
    while ( tap < 0 )   tap += d->maxDelay;
    
    d->buff[tap] = value;
}

float tDelayAAddTo (tDelayA *d, float value, uint32_t tapDelay)
{
    int32_t tap = d->inPoint - tapDelay - 1;
    
    // Check for wraparound.
    while ( tap < 0 )   tap += d->maxDelay;
    
    return (d->buff[tap] += value);
}

float   tDelayAGetDelay (tDelayA *d)
{
    return d->delay;
}

float   tDelayAGetLastOut (tDelayA *d)
{
    return d->lastOut;
}

float   tDelayAGetLastIn (tDelayA *d)
{
    return d->lastIn;
}

void tDelayASetGain (tDelayA *d, float gain)
{
    if (gain < 0.0f)    d->gain = 0.0f;
    else                d->gain = gain;
}

float tDelayAGetGain (tDelayA *d)
{
    return d->gain;
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ Envelope ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //
int     tEnvelopeSetAttack(tEnvelope *env, float attack)
{
    uint16_t attackIndex;
    
    if (attack < 0) {
        attackIndex = 0;
    } else if (attack < 8192) {
        attackIndex = ((uint16_t)(attack * 8.0f))-1;
    } else {
        attackIndex = UINT16_MAX;
    }
    
    env->attackInc = env->inc_buff[attackIndex];
    
    return 0;
}

int     tEnvelopeSetDecay(tEnvelope *env, float decay)
{
    uint16_t decayIndex;
    
    if (decay < 0) {
        decayIndex = 0;
    } else if (decay < 8192) {
        decayIndex = ((uint16_t)(decay * 8.0f))-1;
    } else {
        decayIndex = ((uint16_t)(8192 * 8.0f))-1;
    }
    
    env->decayInc = env->inc_buff[decayIndex];
    
    
    return 0;
}

int     tEnvelopeLoop(tEnvelope *env, int loop)
{
    env->loop = loop;
    
    return 0;
}


int     tEnvelopeOn(tEnvelope *env, float velocity)
{
    if (env->inAttack || env->inDecay) // In case envelope retriggered while it is still happening.
    {
        env->rampPhase = 0;
        env->inRamp = 1;
        env->rampPeak = env->next;
    }
    else // Normal start.
    {
        env->inAttack = 1;
    }
    
    
    env->attackPhase = 0;
    env->decayPhase = 0;
    env->inDecay = 0;
    env->gain = velocity;
    
    return 0;
}

float   tEnvelopeTick(tEnvelope *env)
{
    if (env->inRamp)
    {
        if (env->rampPhase > UINT16_MAX)
        {
            env->inRamp = 0;
            env->inAttack = 1;
            env->next = 0.0f;
        }
        else
        {
            env->next = env->rampPeak * env->exp_buff[(uint32_t)env->rampPhase];
        }
        
        env->rampPhase += env->rampInc;
    }
    
    if (env->inAttack)
    {
        
        // If attack done, time to turn around.
        if (env->attackPhase > UINT16_MAX)
        {
            env->inDecay = 1;
            env->inAttack = 0;
            env->next = env->gain * 1.0f;
        }
        else
        {
            // do interpolation !
            env->next = env->gain * env->exp_buff[UINT16_MAX - (uint32_t)env->attackPhase]; // inverted and backwards to get proper rising exponential shape/perception
        }
        
        // Increment envelope attack.
        env->attackPhase += env->attackInc;
        
    }
    
    if (env->inDecay)
    {
        
        // If decay done, finish.
        if (env->decayPhase >= UINT16_MAX)
        {
            env->inDecay = 0;
            
            if (env->loop)
            {
                env->attackPhase = 0;
                env->decayPhase = 0;
                env->inAttack = 1;
            }
            else
            {
                env->next = 0.0f;
            }
            
        } else {
            
            env->next = env->gain * (env->exp_buff[(uint32_t)env->decayPhase]); // do interpolation !
        }
        
        // Increment envelope decay;
        env->decayPhase += env->decayInc;
    }
    
    return env->next;
}

int     tEnvelopeInit(tEnvelope *env, float attack, float decay, int loop,
                      const float *exponentialTable, const float *attackDecayIncTable)
{
    env->exp_buff = exponentialTable;
    env->inc_buff = attackDecayIncTable;
    env->buff_size = sizeof(exponentialTable);
    
    env->loop = loop;
    
    if (attack > 8192)
        attack = 8192;
    if (attack < 0)
        attack = 0;
    
    if (decay > 8192)
        decay = 8192;
    if (decay < 0)
        decay = 0;
    
    uint16_t attackIndex = ((uint16_t)(attack * 8.0f))-1;
    uint16_t decayIndex = ((uint16_t)(decay * 8.0f))-1;
    uint16_t rampIndex = ((uint16_t)(2.0f * 8.0f))-1;
    
    if (attackIndex < 0)
        attackIndex = 0;
    if (decayIndex < 0)
        decayIndex = 0;
    if (rampIndex < 0)
        rampIndex = 0;
    
    env->inRamp = 0;
    env->inAttack = 0;
    env->inDecay = 0;
    
    env->attackInc = env->inc_buff[attackIndex];
    env->decayInc = env->inc_buff[decayIndex];
    env->rampInc = env->inc_buff[rampIndex];
    
    OOPS_tEnvelopeRegister(env);
    env->sampleRateChanged = &tEnvelopeSampleRateChanged;
    
    return 0;
}



float   tSVFTick(tSVF *svf, float v0)
{
    float v1,v2,v3;
    float low,high;
    v3 = v0 - svf->ic2eq;
    v1 = (svf->a1 * svf->ic1eq) + (svf->a2 * v3);
    v2 = svf->ic2eq + (svf->a2 * svf->ic1eq) + (svf->a3 * v3);
    svf->ic1eq = (2.0f * v1) - svf->ic1eq;
    svf->ic2eq = (2.0f * v2) - svf->ic2eq;
    
    if (svf->type == SVFTypeLowpass)        return v2;
    else if (svf->type == SVFTypeBandpass)  return v1;
    else if (svf->type == SVFTypeHighpass)  return v0 - (svf->k * v1) - v2;
    else if (svf->type == SVFTypeNotch)     return v0 - (svf->k * v1);
    else if (svf->type == SVFTypePeak)      return v0 - (svf->k * v1) - (2.0f * v2);
    else                                    return 0.0f;
    
}

int     tSVFSetFreq(tSVF *svf, uint16_t cutoffKnob)
{
    svf->g = filtertan[cutoffKnob];
    svf->a1 = 1.0f/(1.0f + svf->g * (svf->g + svf->k));
    svf->a2 = svf->g * svf->a1;
    svf->a3 = svf->g * svf->a2;
    
    return 0;
}

int     tSVFSetQ(tSVF *svf, float Q)
{
    svf->k = 1.0f/OOPS_clip(0.01f,Q,10.0f);
    svf->a1 = 1.0f/(1.0f + svf->g * (svf->g + svf->k));
    svf->a2 = svf->g * svf->a1;
    svf->a3 = svf->g * svf->a2;
    
    return 0;
}

int     tSVFInit(tSVF *svf, SVFType type, uint16_t cutoffKnob, float Q)
{
    svf->type = type;
    
    svf->ic1eq = 0;
    svf->ic2eq = 0;
    
    float a1,a2,a3,g,k;
    g = filtertan[cutoffKnob];
    k = 1.0f/OOPS_clip(0.01f,Q,10.0f);
    a1 = 1.0f/(1.0f+g*(g+k));
    a2 = g*a1;
    a3 = g*a2;
    
    svf->g = g;
    svf->k = k;
    svf->a1 = a1;
    svf->a2 = a2;
    svf->a3 = a3;
    
    OOPS_tSVFRegister(svf);
    svf->sampleRateChanged = &tSVFSampleRateChanged;
    
    return 0;
}



float   tSVFEfficientTick(tSVFEfficient *svf, float v0)
{
    float v1,v2,v3;
    float low,high;
    v3 = v0 - svf->ic2eq;
    v1 = (svf->a1 * svf->ic1eq) + (svf->a2 * v3);
    v2 = svf->ic2eq + (svf->a2 * svf->ic1eq) + (svf->a3 * v3);
    svf->ic1eq = (2.0f * v1) - svf->ic1eq;
    svf->ic2eq = (2.0f * v2) - svf->ic2eq;
    
    if (svf->type == SVFTypeLowpass)        return v2;
    else if (svf->type == SVFTypeBandpass)  return v1;
    else if (svf->type == SVFTypeHighpass)  return v0 - (svf->k * v1) - v2;
    else if (svf->type == SVFTypeNotch)     return v0 - (svf->k * v1);
    else if (svf->type == SVFTypePeak)      return v0 - (svf->k * v1) - (2.0f * v2);
    else                                    return 0.0f;
    
}

int     tSVFEfficientSetFreq(tSVFEfficient *svf, uint16_t cutoffKnob)
{
    svf->g = filtertan[cutoffKnob];
    svf->a1 = 1.0f/(1.0f + svf->g * (svf->g + svf->k));
    svf->a2 = svf->g * svf->a1;
    svf->a3 = svf->g * svf->a2;
    
    return 0;
}

int     tSVFEfficientSetQ(tSVFEfficient *svf, float Q)
{
    svf->k = 1.0f/Q;
    svf->a1 = 1.0f/(1.0f + svf->g * (svf->g + svf->k));
    svf->a2 = svf->g * svf->a1;
    svf->a3 = svf->g * svf->a2;
    
    return 0;
}

int     tSVFEfficientInit(tSVFEfficient *svf, SVFType type, uint16_t cutoffKnob, float Q)
{
    svf->type = type;
    
    svf->ic1eq = 0;
    svf->ic2eq = 0;
    
    float a1,a2,a3,g,k;
    g = filtertan[cutoffKnob];
    k = 1.0f/Q;
    a1 = 1.0f/(1.0f+g*(g+k));
    a2 = g*a1;
    a3 = g*a2;
    
    svf->g = g;
    svf->k = k;
    svf->a1 = a1;
    svf->a2 = a2;
    svf->a3 = a3;
    
    OOPS_tSVFEfficientRegister(svf);
    svf->sampleRateChanged = &tSVFEfficientSampleRateChanged;
    
    return 0;
}

float   tEnvelopeFollowerTick(tEnvelopeFollower *ef, float x)
{
    if (x < 0.0f ) x = -x;  /* Absolute value. */
    
    if ((x >= ef->y) && (x > ef->a_thresh)) ef->y = x;                      /* If we hit a peak, ride the peak to the top. */
    else                                    ef->y = ef->y * ef->d_coeff;    /* Else, exponential decay of output. */
    
    //ef->y = envelope_pow[(uint16_t)(ef->y * (float)UINT16_MAX)] * ef->d_coeff; //not quite the right behavior - too much loss of precision?
    //ef->y = powf(ef->y, 1.000009f) * ef->d_coeff;  // too expensive
    
    if( ef->y < VERY_SMALL_FLOAT)   ef->y = 0.0f;
    
    return ef->y;
}

int     tEnvelopeFollowerDecayCoeff(tEnvelopeFollower *ef, float decayCoeff)
{
    return ef->d_coeff = decayCoeff;
}

int     tEnvelopeFollowerAttackThresh(tEnvelopeFollower *ef, float attackThresh)
{
    return ef->a_thresh = attackThresh;
}

int     tEnvelopeFollowerInit(tEnvelopeFollower *ef, float attackThreshold, float decayCoeff)
{
    ef->y = 0.0f;
    ef->a_thresh = attackThreshold;
    ef->d_coeff = decayCoeff;
    
    OOPS_tEnvelopeFollowerRegister(ef);
    ef->sampleRateChanged = &tEnvelopeFollowerSampleRateChanged;
    
    return 0;
}

/* Highpass */
int     tHighpassFreq(tHighpass *hp, float freq)
{
    
    hp->R = (1.0f-((freq * 2.0f * 3.14f) * oops->invSampleRate));
    
    return 0;
}

// From JOS DC Blocker
float   tHighpassTick(tHighpass *hp, float x)
{
    hp->ys = x - hp->xs + hp->R * hp->ys;
    hp->xs = x;
    return hp->ys;
}

int     tHighpassInit(tHighpass *hp, float freq)
{
    hp->R = (1.0f-((freq * 2.0f * 3.14f)* oops->invSampleRate));
    hp->ys = 0.0f;
    hp->xs = 0.0f;
    
    OOPS_tHighpassRegister(hp);
    hp->sampleRateChanged = &tHighpassSampleRateChanged;
    
    return 0;
}

/* Phasor */
void     tPhasorSampleRateChanged (tPhasor *p)
{
    p->inc = p->freq * oops->invSampleRate;
};

int     tPhasorFreq(tPhasor *p, float freq)
{
    p->freq = freq;
    p->inc = freq * oops->invSampleRate;
    
    return 0;
}

float   tPhasorTick(tPhasor *p)
{
    p->phase += p->inc;
    
    if (p->phase >= 1.0f) p->phase -= 1.0f;
    
    return p->phase;
}

int     tPhasorInit(tPhasor *p)
{
    p->phase = 0.0f;
    p->inc = 0.0f;
    
    OOPS_tPhasorRegister(p);
    p->sampleRateChanged = &tPhasorSampleRateChanged;
    
    return 0;
}


/* Cycle */
void     tCycleSampleRateChanged (tCycle *c)
{
    c->inc = c->freq * oops->invSampleRate;
}


int     tCycleInit(tCycle *c)
{
    // Underlying phasor
    c->inc      =  (OOPSFloat) 0.0;
    c->phase    =  (OOPSFloat) 0.0;
    
    OOPS_tCycleRegister(c);
    c->sampleRateChanged = &tCycleSampleRateChanged;
    
    return 0;
}

int     tCycleSetFreq(tCycle *c, float freq)
{
    c->freq = freq;
    c->inc = freq * oops->invSampleRate;
    
    return 0;
}

float   tCycleTick(tCycle *c)
{
    // Phasor increment
    c->phase += c->inc;
    if (c->phase >= 1.0f) c->phase -= 1.0f;
    
    // Wavetable synthesis
    float temp = SINE_TABLE_SIZE * c->phase;
    int intPart = (int)temp;
    float fracPart = temp - (float)intPart;
    float samp0 = sinewave[intPart];
    if (++intPart >= SINE_TABLE_SIZE) intPart = 0;
    float samp1 = sinewave[intPart];
    return (samp0 + (samp1 - samp0) * fracPart);
}


/* Sawtooth */
void     tSawtoothSampleRateChanged (tSawtooth *c)
{
    c->inc = c->freq * oops->invSampleRate;
}

int     tSawtoothInit(tSawtooth *c)
{
    // Underlying phasor
    c->inc = 0.0f;
    c->phase = 0.0f;
    
    OOPS_tSawtoothRegister(c);
    c->sampleRateChanged = &tSawtoothSampleRateChanged;
    
    return 0;
}
int     tSawtoothSetFreq(tSawtooth *c, float freq)
{
    c->freq = freq;
    c->inc = freq * oops->invSampleRate;
    
    return 0;
}

float   tSawtoothTick(tSawtooth *c)
{
    // Phasor increment
    c->phase += c->inc;
    if (c->phase >= 1.0f) c->phase -= 1.0f;
    
    float out = 0.0f;
    float w1, w2;
    
    int idx = (int)(c->phase * TRI_TABLE_SIZE);
    
    // Wavetable synthesis
    
    if (c->freq <= 20.0f)
    {
        out = sawtooth[T20][idx];
    }
    else if (c->freq <= 40.0f)
    {
        w1 = (40.0f - c->freq) / 20.0f;
        w2 = 1.0f - w1;
        
        out = (sawtooth[T20][idx] * w1) + (sawtooth[T40][idx] * w2);
    }
    else if (c->freq <= 80.0f)
    {
        w1 = (80.0f - c->freq) / 40.0f;
        w2 = 1.0f - w1;
        
        out = (sawtooth[T40][idx] * w1) + (sawtooth[T80][idx] * w2);
    }
    else if (c->freq <= 160.0f)
    {
        w1 = (160.0f - c->freq) / 80.0f;
        w2 = 1.0f - w1;
        
        out = (sawtooth[T80][idx] * w1) + (sawtooth[T160][idx] * w2);
    }
    else if (c->freq <= 320.0f)
    {
        w1 = (320.0f - c->freq) / 160.0f;
        w2 = 1.0f - w1;
        
        out = (sawtooth[T160][idx] * w1) + (sawtooth[T320][idx] * w2);
    }
    else if (c->freq <= 640.0f)
    {
        w1 = (640.0f - c->freq) / 320.0f;
        w2 = 1.0f - w1;
        
        out = (sawtooth[T320][idx] * w1) + (sawtooth[T640][idx] * w2);
    }
    else if (c->freq <= 1280.0f)
    {
        w1 = (1280.0f - c->freq) / 640.0f;
        w2 = 1.0f - w1;
        
        out = (sawtooth[T640][idx] * w1) + (sawtooth[T1280][idx] * w2);
    }
    else if (c->freq <= 2560.0f)
    {
        w1 = (2560.0f - c->freq) / 1280.0f;
        w2 = 1.0f - w1;
        
        out = (sawtooth[T1280][idx] * w1) + (sawtooth[T2560][idx] * w2);
    }
    else if (c->freq <= 5120.0f)
    {
        w1 = (5120.0f - c->freq) / 2560.0f;
        w2 = 1.0f - w1;
        
        out = (sawtooth[T2560][idx] * w1) + (sawtooth[T5120][idx] * w2);
    }
    else if (c->freq <= 10240.0f)
    {
        w1 = (10240.0 - c->freq) / 5120.0f;
        w2 = 1.0f - w1;
        
        out = (sawtooth[T5120][idx] * w1) + (sawtooth[T10240][idx] * w2);
    }
    else if (c->freq <= 20480.0f)
    {
        w1 = (20480.0f - c->freq) / 10240.0f;
        w2 = 1.0f - w1;
        
        out = (sawtooth[T10240][idx] * w1) + (sawtooth[T20480][idx] * w2);
    }
    else
    {
        out = sawtooth[T20480][idx];
    }
    
    return out;
}

/* Triangle */

int     tTriangleInit(tTriangle *c)
{
    // Underlying phasor
    c->inc = 0.0f;
    c->phase = 0.0f;
    
    OOPS_tTriangleRegister(c);
    c->sampleRateChanged = &tTriangleSampleRateChanged;
    
    return 0;
}
int
tTriangleSetFreq(tTriangle *c, float freq)
{
    c->freq = freq;
    c->inc = freq * oops->invSampleRate;
    
    return 0;
}


float   tTriangleTick(tTriangle *c)
{
    // Phasor increment
    c->phase += c->inc;
    if (c->phase >= 1.0f) c->phase -= 1.0f;
    
    float out = 0.0f;
    float w1, w2;
    
    int idx = (int)(c->phase * TRI_TABLE_SIZE);
    
    // Wavetable synthesis
    
    if (c->freq <= 20.0f)
    {
        out = triangle[T20][idx];
    }
    else if (c->freq <= 40.0f)
    {
        w1 = (40.0f - c->freq) / 20.0f;
        w2 = 1.0f - w1;
        
        out = (triangle[T20][idx] * w1) + (triangle[T40][idx] * w2);
    }
    else if (c->freq <= 80.0f)
    {
        w1 = (80.0f - c->freq) / 40.0f;
        w2 = 1.0f - w1;
        
        out = (triangle[T40][idx] * w1) + (triangle[T80][idx] * w2);
    }
    else if (c->freq <= 160.0f)
    {
        w1 = (160.0f - c->freq) / 80.0f;
        w2 = 1.0f - w1;
        
        out = (triangle[T80][idx] * w1) + (triangle[T160][idx] * w2);
    }
    else if (c->freq <= 320.0f)
    {
        w1 = (320.0f - c->freq) / 160.0f;
        w2 = 1.0f - w1;
        
        out = (triangle[T160][idx] * w1) + (triangle[T320][idx] * w2);
    }
    else if (c->freq <= 640.0f)
    {
        w1 = (640.0f - c->freq) / 320.0f;
        w2 = 1.0f - w1;
        
        out = (triangle[T320][idx] * w1) + (triangle[T640][idx] * w2);
    }
    else if (c->freq <= 1280.0f)
    {
        w1 = (1280.0f - c->freq) / 640.0f;
        w2 = 1.0f - w1;
        
        out = (triangle[T640][idx] * w1) + (triangle[T1280][idx] * w2);
    }
    else if (c->freq <= 2560.0f)
    {
        w1 = (2560.0f - c->freq) / 1280.0f;
        w2 = 1.0f - w1;
        
        out = (triangle[T1280][idx] * w1) + (triangle[T2560][idx] * w2);
    }
    else if (c->freq <= 5120.0f)
    {
        w1 = (5120.0f - c->freq) / 2560.0f;
        w2 = 1.0f - w1;
        
        out = (triangle[T2560][idx] * w1) + (triangle[T5120][idx] * w2);
    }
    else if (c->freq <= 10240.0f)
    {
        w1 = (10240.0 - c->freq) / 5120.0f;
        w2 = 1.0f - w1;
        
        out = (triangle[T5120][idx] * w1) + (triangle[T10240][idx] * w2);
    }
    else if (c->freq <= 20480.0f)
    {
        w1 = (20480.0f - c->freq) / 10240.0f;
        w2 = 1.0f - w1;
        
        out = (triangle[T10240][idx] * w1) + (triangle[T20480][idx] * w2);
    }
    else
    {
        out = triangle[T20480][idx];
    }
    
    return out;
}

void     tTriangleSampleRateChanged (tTriangle *c)
{
    c->inc = c->freq * oops->invSampleRate;
}


/* Square */

int     tSquareInit(tSquare *c)
{
    // Underlying phasor
    c->inc = 0.0f;
    c->phase = 0.0f;
    
    OOPS_tSquareRegister(c);
    c->sampleRateChanged = &tSquareSampleRateChanged;
    
    return 0;
}

int     tSquareSetFreq(tSquare *c, float freq)
{
    c->freq = freq;
    c->inc = freq * oops->invSampleRate;
    
    return 0;
}



float   tSquareTick(tSquare *c)
{
    // Phasor increment
    c->phase += c->inc;
    if (c->phase >= 1.0f) c->phase -= 1.0f;
    
    float out = 0.0f;
    float w1, w2;
    
    int idx = (int)(c->phase * TRI_TABLE_SIZE);
    
    // Wavetable synthesis
    
    if (c->freq <= 20.0f)
    {
        out = squarewave[T20][idx];
    }
    else if (c->freq <= 40.0f)
    {
        w1 = (40.0f - c->freq) / 20.0f;
        w2 = 1.0f - w1;
        
        out = (squarewave[T20][idx] * w1) + (squarewave[T40][idx] * w2);
    }
    else if (c->freq <= 80.0f)
    {
        w1 = (80.0f - c->freq) / 40.0f;
        w2 = 1.0f - w1;
        
        out = (squarewave[T40][idx] * w1) + (squarewave[T80][idx] * w2);
    }
    else if (c->freq <= 160.0f)
    {
        w1 = (160.0f - c->freq) / 80.0f;
        w2 = 1.0f - w1;
        
        out = (squarewave[T80][idx] * w1) + (squarewave[T160][idx] * w2);
    }
    else if (c->freq <= 320.0f)
    {
        w1 = (320.0f - c->freq) / 160.0f;
        w2 = 1.0f - w1;
        
        out = (squarewave[T160][idx] * w1) + (squarewave[T320][idx] * w2);
    }
    else if (c->freq <= 640.0f)
    {
        w1 = (640.0f - c->freq) / 320.0f;
        w2 = 1.0f - w1;
        
        out = (squarewave[T320][idx] * w1) + (squarewave[T640][idx] * w2);
    }
    else if (c->freq <= 1280.0f)
    {
        w1 = (1280.0f - c->freq) / 640.0f;
        w2 = 1.0f - w1;
        
        out = (squarewave[T640][idx] * w1) + (squarewave[T1280][idx] * w2);
    }
    else if (c->freq <= 2560.0f)
    {
        w1 = (2560.0f - c->freq) / 1280.0f;
        w2 = 1.0f - w1;
        
        out = (squarewave[T1280][idx] * w1) + (squarewave[T2560][idx] * w2);
    }
    else if (c->freq <= 5120.0f)
    {
        w1 = (5120.0f - c->freq) / 2560.0f;
        w2 = 1.0f - w1;
        
        out = (squarewave[T2560][idx] * w1) + (squarewave[T5120][idx] * w2);
    }
    else if (c->freq <= 10240.0f)
    {
        w1 = (10240.0 - c->freq) / 5120.0f;
        w2 = 1.0f - w1;
        
        out = (squarewave[T5120][idx] * w1) + (squarewave[T10240][idx] * w2);
    }
    else if (c->freq <= 20480.0f)
    {
        w1 = (20480.0f - c->freq) / 10240.0f;
        w2 = 1.0f - w1;
        
        out = (squarewave[T10240][idx] * w1) + (squarewave[T20480][idx] * w2);
    }
    else
    {
        out = squarewave[T20480][idx];
    }
    
    return out;
}

void     tSquareSampleRateChanged (tSquare *c)
{
    c->inc = c->freq * oops->invSampleRate;
}


/* Noise */
float   tNoiseTick(tNoise *n)
{
    float rand = n->rand();
    
    if (n->type == NoiseTypePink)
    {
        float tmp;
        n->pinkb0 = 0.99765f * n->pinkb0 + rand * 0.0990460f;
        n->pinkb1 = 0.96300f * n->pinkb1 + rand * 0.2965164f;
        n->pinkb2 = 0.57000f * n->pinkb2 + rand * 1.0526913f;
        tmp = n->pinkb0 + n->pinkb1 + n->pinkb2 + rand * 0.1848f;
        return (tmp * 0.05f);
    }
    else // NoiseTypeWhite
    {
        return rand;
    }
}

int     tNoiseInit(tNoise *n, NoiseType type)
{
    n->type = type;
    n->rand = oops->random;
    
    OOPS_tNoiseRegister(n);
    n->sampleRateChanged = &tNoiseSampleRateChanged;
    
    return 0;
}

int     tRampSetTime(tRamp *r, float time)
{
    r->time = time;
    r->inc = ((r->dest-r->curr)/r->time * r->inv_sr_ms)*((float)r->samples_per_tick);
    return 0;
}

int     tRampSetDest(tRamp *r, float dest)
{
    r->dest = dest;
    r->inc = ((r->dest-r->curr)/r->time * r->inv_sr_ms)*((float)r->samples_per_tick);
    return 0;
}

float   tRampTick(tRamp *r) {
    
    r->curr += r->inc;
    
    if (((r->curr >= r->dest) && (r->inc > 0.0f)) || ((r->curr <= r->dest) && (r->inc < 0.0f))) r->inc = 0.0f;
    
    return r->curr;
}

int     tRampInit(tRamp *r, float time, int samples_per_tick)
{
    r->inv_sr_ms = 1.0f/(oops->sampleRate*0.001f);
    r->curr = 0.0f;
    r->dest = 0.0f;
    r->time = time;
    r->samples_per_tick = samples_per_tick;
    r->inc = ((r->dest-r->curr)/r->time * r->inv_sr_ms)*((float)r->samples_per_tick);
    
    OOPS_tRampRegister(r);
    r->sampleRateChanged = &tRampSampleRateChanged;
    
    return 0;
}

void    tRampSampleRateChanged(tRamp *r)
{
    r->inv_sr_ms = 1.0f / (oops->sampleRate * 0.001f);
    r->inc = ((r->dest-r->curr)/r->time * r->inv_sr_ms)*((float)r->samples_per_tick);
}

#pragma Physical Models
/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ tPluck ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */
int     tPluckInit          (tPluck *p, float lowestFrequency,  float delayBuff[REV_DELAY_LENGTH])
{
    if ( lowestFrequency <= 0.0f )  lowestFrequency = 10.0f;
    
    tNoiseInit(&p->noise, NoiseTypeWhite);
    tOnePoleInit(&p->pickFilter, 0.0f);
    tOneZeroInit(&p->loopFilter, 0.0f);
    
    
    tDelayAInit(&p->delayLine, 0.0f, REV_DELAY_LENGTH, delayBuff);
    
    tPluckSetFrequency(p, 220.0f);
    
    OOPS_tPluckRegister(p);
    p->sampleRateChanged = &tPluckSampleRateChanged;
    
    return 0;
}

float   tPluckGetLastOut    (tPluck *p)
{
    return p->lastOut;
}

float   tPluckTick          (tPluck *p)
{
    return (p->lastOut = 3.0f * tDelayATick(&p->delayLine, tOneZeroTick(&p->loopFilter, tDelayAGetLastOut(&p->delayLine) * p->loopGain ) ));
}

void    tPluckPluck         (tPluck *p, float amplitude)
{
    if ( amplitude < 0.0f)      amplitude = 0.0f;
    else if (amplitude > 1.0f)  amplitude = 1.0f;
    
    tOnePoleSetPole(&p->pickFilter, 0.999f - (amplitude * 0.15));
    tOnePoleSetGain(&p->pickFilter, amplitude * 0.5f );
    
    // Fill delay with noise additively with current contents.
    for ( uint32_t i = 0; i < (uint32_t)tDelayAGetDelay(&p->delayLine); i++ )
        tDelayATick(&p->delayLine, 0.6f * tDelayAGetLastOut(&p->delayLine) + tOnePoleTick(&p->pickFilter, tNoiseTick(&p->noise) ) );
}

// Start a note with the given frequency and amplitude.;
void    tPluckNoteOn        (tPluck *p, float frequency, float amplitude )
{
    tPluckSetFrequency( p, frequency );
    tPluckPluck( p, amplitude );
}

// Stop a note with the given amplitude (speed of decay).
void    tPluckNoteOff       (tPluck *p, float amplitude )
{
    if ( amplitude < 0.0f)      amplitude = 0.0f;
    else if (amplitude > 1.0f)  amplitude = 1.0f;
    
    p->loopGain = 1.0f - amplitude;
}

// Set instrument parameters for a particular frequency.
void    tPluckSetFrequency  (tPluck *p, float frequency )
{
    if ( frequency <= 0.0f )   frequency = 0.001f;
    
    // Delay = length - filter delay.
    float delay = ( oops->sampleRate / frequency ) - tOneZeroGetPhaseDelay(&p->loopFilter, frequency );
    
    tDelayASetDelay(&p->delayLine, delay );
    
    p->loopGain = 0.99f + (frequency * 0.000005f);
    
    if ( p->loopGain >= 0.999f ) p->loopGain = 0.999f;
    
}

// Perform the control change specified by \e number and \e value (0.0 - 128.0).
void    tPluckControlChange (tPluck *p, int number, float value)
{
    return;
}
/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ tStifKarp ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */
int     tStifKarpInit          (tStifKarp *p, float lowestFrequency, float delayBuff[2][REV_DELAY_LENGTH])
{
    if ( lowestFrequency <= 0.0f )  lowestFrequency = 8.0f;
    
    tDelayAInit(&p->delayLine, 0.0f, REV_DELAY_LENGTH, delayBuff[0]);
    tDelayLInit(&p->combDelay, 0.0f, REV_DELAY_LENGTH, delayBuff[1]);
    
    tOneZeroInit(&p->filter, 0.0f);
    
    tNoiseInit(&p->noise, NoiseTypeWhite);
    
    for (int i = 0; i < 4; i++)
    {
        tBiQuadInit(&p->biquad[i]);
    }
    
    p->pluckAmplitude = 0.3f;
    p->pickupPosition = 0.4f;
    
    p->stretching = 0.9999f;
    p->baseLoopGain = 0.995f;
    p->loopGain = 0.999f;
    
    tStifKarpSetFrequency( p, 220.0f );
    
    OOPS_tStifKarpRegister(p);
    p->sampleRateChanged = &tStifKarpSampleRateChanged;
    
    return 0;
}

float   tStifKarpGetLastOut    (tStifKarp *p)
{
    return p->lastOut;
}

int mycount;
float   tStifKarpTick          (tStifKarp *p)
{
    float temp = tDelayAGetLastOut(&p->delayLine) * p->loopGain;
    
    // Calculate allpass stretching.
    for (int i=0; i<4; i++)     temp = tBiQuadTick(&p->biquad[i],temp);
    
    // Moving average filter.
    temp = tOneZeroTick(&p->filter, temp);
    
    float out = tDelayATick(&p->delayLine, temp);
    out = out - tDelayLTick(&p->combDelay, out);
    p->lastOut = out;
    
    return p->lastOut;
}

void    tStifKarpPluck         (tStifKarp *p, float amplitude)
{
    if ( amplitude < 0.0f)      amplitude = 0.0f;
    else if (amplitude > 1.0f)  amplitude = 1.0f;
    
    p->pluckAmplitude = amplitude;
    
    for ( uint32_t i=0; i < (uint32_t)tDelayAGetDelay(&p->delayLine); i++ )
    {
        // Fill delay with noise additively with current contents.
        tDelayATick(&p->delayLine, (tDelayAGetLastOut(&p->delayLine) * 0.6f) + 0.4f * tNoiseTick(&p->noise) * p->pluckAmplitude );
        //delayLine_.tick( combDelay_.tick((delayLine_.lastOut() * 0.6) + 0.4 * noise->tick() * pluckAmplitude_) );
    }
}

// Start a note with the given frequency and amplitude.;
void    tStifKarpNoteOn        (tStifKarp *p, float frequency, float amplitude )
{
    tStifKarpSetFrequency( p, frequency );
    tStifKarpPluck( p, amplitude );
}

// Stop a note with the given amplitude (speed of decay).
void    tStifKarpNoteOff       (tStifKarp *p, float amplitude )
{
    if ( amplitude < 0.0f)      amplitude = 0.0f;
    else if (amplitude > 1.0f)  amplitude = 1.0f;
    
    p->loopGain = 1.0f - amplitude;
}

// Set instrument parameters for a particular frequency.
void    tStifKarpSetFrequency  (tStifKarp *p, float frequency )
{
    if ( frequency <= 0.0f )   frequency = 0.001f;
    
    p->lastFrequency = frequency;
    p->lastLength = oops->sampleRate / p->lastFrequency;
    float delay = p->lastLength - 0.5f;
    tDelayASetDelay(&p->delayLine, delay );
    
    // MAYBE MODIFY LOOP GAINS
    p->loopGain = p->baseLoopGain + (frequency * 0.000005f);
    if (p->loopGain >= 1.0f) p->loopGain = 0.99999f;
    
    tStifKarpSetStretch(p, p->stretching);
    
    tDelayLSetDelay(&p->combDelay, 0.5f * p->pickupPosition * p->lastLength );
    
}

//! Set the stretch "factor" of the string (0.0 - 1.0).
void    tStifKarpSetStretch         (tStifKarp *p, float stretch )
{
    p->stretching = stretch;
    float coefficient;
    float freq = p->lastFrequency * 2.0f;
    float dFreq = ( (0.5f * oops->sampleRate) - freq ) * 0.25f;
    float temp = 0.5f + (stretch * 0.5f);
    if ( temp > 0.9999f ) temp = 0.9999f;
    
    for ( int i=0; i<4; i++ )
    {
        coefficient = temp * temp;
        tBiQuadSetA2(&p->biquad[i], coefficient);
        tBiQuadSetB0(&p->biquad[i], coefficient);
        tBiQuadSetB2(&p->biquad[i], 1.0f);
        
        coefficient = -2.0f * temp * cos(TWO_PI * freq / oops->sampleRate);
        tBiQuadSetA1(&p->biquad[i], coefficient);
        tBiQuadSetB1(&p->biquad[i], coefficient);
        
        freq += dFreq;
    }
}

//! Set the pluck or "excitation" position along the string (0.0 - 1.0).
void    tStifKarpSetPickupPosition  (tStifKarp *p, float position )
{
    if (position < 0.0f)        p->pickupPosition = 0.0f;
    else if (position <= 1.0f)  p->pickupPosition = position;
    else                        p->pickupPosition = 1.0f;
    
    tDelayLSetDelay(&p->combDelay, 0.5f * p->pickupPosition * p->lastLength);
}

//! Set the base loop gain.
void    tStifKarpSetBaseLoopGain    (tStifKarp *p, float aGain )
{
    p->baseLoopGain = aGain;
    p->loopGain = p->baseLoopGain + (p->lastFrequency * 0.000005f);
    if ( p->loopGain > 0.99999f ) p->loopGain = 0.99999f;
}

// Perform the control change specified by \e number and \e value (0.0 - 128.0).
void    tStifKarpControlChange (tStifKarp *p, SKControlType type, float value)
{
    if ( value < 0.0f )         value = 0.0f;
    else if (value > 128.0f)   value = 128.0f;
    
    float normalizedValue = value * ONE_OVER_128;
    
    if (type == SKPickPosition) // 4
        tStifKarpSetPickupPosition( p, normalizedValue );
    else if (type == SKStringDamping) // 11
        tStifKarpSetBaseLoopGain( p, 0.97f + (normalizedValue * 0.03f) );
    else if (type == SKDetune) // 1
        tStifKarpSetStretch( p, 0.91f + (0.09f * (1.0f - normalizedValue)) );
    
}
/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */



void     tNoiseSampleRateChanged (tNoise *c) {};

void     tOnePoleSampleRateChanged (tOnePole *c) {};

void     tTwoPoleSampleRateChanged (tTwoPole *c) {};

void     tOneZeroSampleRateChanged (tOneZero *c) {};

void     tTwoZeroSampleRateChanged (tTwoZero *c) {};

void     tPoleZeroSampleRateChanged (tPoleZero *c) {};

void     tBiQuadSampleRateChanged (tBiQuad *c) {};

void     tSVFSampleRateChanged (tSVF *c) {};

void     tSVFEfficientSampleRateChanged (tSVFEfficient *c) {};

void     tHighpassSampleRateChanged (tHighpass *c) {};

void     tDelaySampleRateChanged (tDelay *c) {};

void     tDelayLSampleRateChanged (tDelayL *c) {};

void     tDelayASampleRateChanged (tDelayA *c) {};

void     tEnvelopeSampleRateChanged (tEnvelope *c) {};

void     tADSRSampleRateChanged (tADSR *c) {};

void     tEnvelopeFollowerSampleRateChanged (tEnvelopeFollower *c) {};

void     tPRCRevSampleRateChanged (tPRCRev *c) {};

void     tNRevSampleRateChanged (tNRev *c) {};

void     tPluckSampleRateChanged (tPluck *c) {};

void    tStifKarpSampleRateChanged (tStifKarp *c) {};


// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //

int OOPSInit(float sr, float(*random)(void))
{
    oops = (OOPS*) calloc(1, sizeof(OOPS));
    
    for (int i = 0; i < NI; i++)    oops->tPhasorRegistry[i]            = (tPhasor*) calloc(1, sizeof(tPhasor));
    for (int i = 0; i < NI; i++)    oops->tCycleRegistry[i]             = (tCycle*) calloc(1, sizeof(tCycle));
    for (int i = 0; i < NI; i++)    oops->tSawtoothRegistry[i]          = (tSawtooth*) calloc(1, sizeof(tSawtooth));
    for (int i = 0; i < NI; i++)    oops->tTriangleRegistry[i]          = (tTriangle*) calloc(1, sizeof(tTriangle));
    for (int i = 0; i < NI; i++)    oops->tSquareRegistry[i]            = (tSquare*) calloc(1, sizeof(tSquare));
    for (int i = 0; i < NI; i++)    oops->tNoiseRegistry[i]             = (tNoise*) calloc(1, sizeof(tNoise));
    for (int i = 0; i < NI; i++)    oops->tOnePoleRegistry[i]           = (tOnePole*) calloc(1, sizeof(tOnePole));
    for (int i = 0; i < NI; i++)    oops->tTwoPoleRegistry[i]           = (tTwoPole*) calloc(1, sizeof(tTwoPole));
    for (int i = 0; i < NI; i++)    oops->tOneZeroRegistry[i]           = (tOneZero*) calloc(1, sizeof(tOneZero));
    for (int i = 0; i < NI; i++)    oops->tTwoZeroRegistry[i]           = (tTwoZero*) calloc(1, sizeof(tTwoZero));
    for (int i = 0; i < NI; i++)    oops->tPoleZeroRegistry[i]          = (tPoleZero*) calloc(1, sizeof(tPoleZero));
    for (int i = 0; i < NI; i++)    oops->tBiQuadRegistry[i]            = (tBiQuad*) calloc(1, sizeof(tBiQuad));
    for (int i = 0; i < NI; i++)    oops->tSVFRegistry[i]               = (tSVF*) calloc(1, sizeof(tSVF));
    for (int i = 0; i < NI; i++)    oops->tSVFEfficientRegistry[i]      = (tSVFEfficient*) calloc(1, sizeof(tSVFEfficient));
    for (int i = 0; i < NI; i++)    oops->tHighpassRegistry[i]          = (tHighpass*) calloc(1, sizeof(tHighpass));
    for (int i = 0; i < NI; i++)    oops->tDelayRegistry[i]             = (tDelay*) calloc(1, sizeof(tDelay));
    for (int i = 0; i < NI; i++)    oops->tDelayLRegistry[i]            = (tDelayL*) calloc(1, sizeof(tDelayL));
    for (int i = 0; i < NI; i++)    oops->tDelayARegistry[i]            = (tDelayA*) calloc(1, sizeof(tDelayA));
    for (int i = 0; i < NI; i++)    oops->tEnvelopeRegistry[i]          = (tEnvelope*) calloc(1, sizeof(tEnvelope));
    for (int i = 0; i < NI; i++)    oops->tADSRRegistry[i]              = (tADSR*) calloc(1, sizeof(tADSR));
    for (int i = 0; i < NI; i++)    oops->tRampRegistry[i]              = (tRamp*) calloc(1, sizeof(tRamp));
    for (int i = 0; i < NI; i++)    oops->tEnvelopeFollowerRegistry[i]  = (tEnvelopeFollower*) calloc(1, sizeof(tEnvelopeFollower));
    for (int i = 0; i < NI; i++)    oops->tPRCRevRegistry[i]            = (tPRCRev*) calloc(1, sizeof(tPRCRev));
    for (int i = 0; i < NI; i++)    oops->tNRevRegistry[i]              = (tNRev*) calloc(1, sizeof(tNRev));
    for (int i = 0; i < NI; i++)    oops->tPluckRegistry[i]             = (tPluck*) calloc(1, sizeof(tPluck));
    for (int i = 0; i < NI; i++)    oops->tStifKarpRegistry[i]          = (tStifKarp*) calloc(1, sizeof(tStifKarp));
    
    oops->sampleRate = sr;
    
    oops->invSampleRate = 1.0f/sr;
    
    oops->random = random;
    
    for (int i = 0; i < T_INDEXCNT; i++)
        oops->registryIndex[i] = 0;
    
    return 0;
}
#define OOPSSampleRateChanged(THIS) oops->THIS->sampleRateChanged(oops->THIS)

int OOPSSetSampleRate(float sampleRate)
{
    oops->sampleRate = sampleRate;
    oops->invSampleRate = 1.0f/sampleRate;
    
    for (int i = 0; i < oops->registryIndex[T_PHASOR]; i++)         OOPSSampleRateChanged(tPhasorRegistry[i]);
    for (int i = 0; i < oops->registryIndex[T_CYCLE]; i++)          OOPSSampleRateChanged(tCycleRegistry[i]);
    for (int i = 0; i < oops->registryIndex[T_SAWTOOTH]; i++)       OOPSSampleRateChanged(tSawtoothRegistry[i]);
    for (int i = 0; i < oops->registryIndex[T_TRIANGLE]; i++)       OOPSSampleRateChanged(tTriangleRegistry[i]);
    for (int i = 0; i < oops->registryIndex[T_SQUARE]; i++)         OOPSSampleRateChanged(tSquareRegistry[i]);
    for (int i = 0; i < oops->registryIndex[T_NOISE]; i++)          OOPSSampleRateChanged(tNoiseRegistry[i]);
    for (int i = 0; i < oops->registryIndex[T_ONEPOLE]; i++)        OOPSSampleRateChanged(tOnePoleRegistry[i]);
    for (int i = 0; i < oops->registryIndex[T_TWOPOLE]; i++)        OOPSSampleRateChanged(tTwoPoleRegistry[i]);
    for (int i = 0; i < oops->registryIndex[T_ONEZERO]; i++)        OOPSSampleRateChanged(tOneZeroRegistry[i]);
    for (int i = 0; i < oops->registryIndex[T_TWOZERO]; i++)        OOPSSampleRateChanged(tTwoZeroRegistry[i]);
    for (int i = 0; i < oops->registryIndex[T_POLEZERO]; i++)       OOPSSampleRateChanged(tPoleZeroRegistry[i]);
    for (int i = 0; i < oops->registryIndex[T_BIQUAD]; i++)         OOPSSampleRateChanged(tBiQuadRegistry[i]);
    for (int i = 0; i < oops->registryIndex[T_SVF]; i++)            OOPSSampleRateChanged(tSVFRegistry[i]);
    for (int i = 0; i < oops->registryIndex[T_SVFE]; i++)           OOPSSampleRateChanged(tSVFEfficientRegistry[i]);
    for (int i = 0; i < oops->registryIndex[T_HIGHPASS]; i++)       OOPSSampleRateChanged(tHighpassRegistry[i]);
    for (int i = 0; i < oops->registryIndex[T_DELAY]; i++)          OOPSSampleRateChanged(tDelayRegistry[i]);
    for (int i = 0; i < oops->registryIndex[T_DELAYL]; i++)         OOPSSampleRateChanged(tDelayLRegistry[i]);
    for (int i = 0; i < oops->registryIndex[T_DELAYA]; i++)         OOPSSampleRateChanged(tDelayARegistry[i]);
    for (int i = 0; i < oops->registryIndex[T_ENVELOPE]; i++)       OOPSSampleRateChanged(tEnvelopeRegistry[i]);
    for (int i = 0; i < oops->registryIndex[T_ADSR]; i++)           OOPSSampleRateChanged(tADSRRegistry[i]);
    for (int i = 0; i < oops->registryIndex[T_RAMP]; i++)           OOPSSampleRateChanged(tRampRegistry[i]);
    for (int i = 0; i < oops->registryIndex[T_ENVELOPEFOLLOW]; i++) OOPSSampleRateChanged(tEnvelopeFollowerRegistry[i]);
    for (int i = 0; i < oops->registryIndex[T_PRCREV]; i++)         OOPSSampleRateChanged(tPRCRevRegistry[i]);
    for (int i = 0; i < oops->registryIndex[T_NREV]; i++)           OOPSSampleRateChanged(tNRevRegistry[i]);
    for (int i = 0; i < oops->registryIndex[T_PLUCK]; i++)          OOPSSampleRateChanged(tPluckRegistry[i]);
    for (int i = 0; i < oops->registryIndex[T_STIFKARP]; i++)       OOPSSampleRateChanged(tStifKarpRegistry[i]);
    
    return 0;
}

int OOPSGetSampleRate()
{
    return oops->sampleRate;
}


int OOPS_tPhasorRegister(tPhasor *o)
{
    oops->tPhasorRegistry[oops->registryIndex[T_PHASOR]] = o;
    
    return oops->registryIndex[T_PHASOR]++;
}

int OOPS_tCycleRegister(tCycle *o)
{
    oops->tCycleRegistry[oops->registryIndex[T_CYCLE]] = o;
    
    return oops->registryIndex[T_CYCLE]++;
}

int OOPS_tSawtoothRegister(tSawtooth *o)
{
    oops->tSawtoothRegistry[oops->registryIndex[T_SAWTOOTH]] = o;
    
    return oops->registryIndex[T_SAWTOOTH]++;
}

int OOPS_tTriangleRegister(tTriangle *o)
{
    oops->tTriangleRegistry[oops->registryIndex[T_TRIANGLE]] = o;
    
    return oops->registryIndex[T_TRIANGLE]++;
}

int OOPS_tSquareRegister(tSquare *o)
{
    oops->tSquareRegistry[oops->registryIndex[T_SQUARE]] = o;
    
    return oops->registryIndex[T_SQUARE]++;
}

int OOPS_tNoiseRegister(tNoise *o)
{
    oops->tNoiseRegistry[oops->registryIndex[T_NOISE]] = o;
    
    return oops->registryIndex[T_NOISE]++;
}

int OOPS_tOnePoleRegister(tOnePole *o)
{
    oops->tOnePoleRegistry[oops->registryIndex[T_ONEPOLE]] = o;
    
    return oops->registryIndex[T_ONEPOLE]++;
}

int OOPS_tTwoPoleRegister(tTwoPole *o)
{
    oops->tTwoPoleRegistry[oops->registryIndex[T_TWOPOLE]] = o;
    
    return oops->registryIndex[T_TWOPOLE]++;
}

int OOPS_tOneZeroRegister(tOneZero *o)
{
    oops->tOneZeroRegistry[oops->registryIndex[T_ONEZERO]] = o;
    
    return oops->registryIndex[T_ONEZERO]++;
}

int OOPS_tTwoZeroRegister(tTwoZero *o)
{
    oops->tTwoZeroRegistry[oops->registryIndex[T_TWOZERO]] = o;
    
    return oops->registryIndex[T_TWOZERO]++;
}

int OOPS_tPoleZeroRegister(tPoleZero *o)
{
    oops->tPoleZeroRegistry[oops->registryIndex[T_POLEZERO]] = o;
    
    return oops->registryIndex[T_POLEZERO]++;
}

int OOPS_tBiQuadRegister(tBiQuad *o)
{
    oops->tBiQuadRegistry[oops->registryIndex[T_BIQUAD]] = o;
    
    return oops->registryIndex[T_BIQUAD]++;
}

int OOPS_tSVFRegister(tSVF *o)
{
    oops->tSVFRegistry[oops->registryIndex[T_SVF]] = o;
    
    return oops->registryIndex[T_SVF]++;
}

int OOPS_tSVFEfficientRegister(tSVFEfficient *o)
{
    oops->tSVFEfficientRegistry[oops->registryIndex[T_SVFE]] = o;
    
    return oops->registryIndex[T_SVFE]++;
}

int OOPS_tHighpassRegister(tHighpass *o)
{
    oops->tHighpassRegistry[oops->registryIndex[T_HIGHPASS]] = o;
    
    return oops->registryIndex[T_HIGHPASS]++;
}

int OOPS_tDelayRegister(tDelay *o)
{
    oops->tDelayRegistry[oops->registryIndex[T_DELAY]] = o;
    
    return oops->registryIndex[T_DELAY]++;
}

int OOPS_tDelayLRegister(tDelayL *o)
{
    oops->tDelayLRegistry[oops->registryIndex[T_DELAYL]] = o;
    
    return oops->registryIndex[T_DELAYL]++;
}
int OOPS_tDelayARegister(tDelayA *o)
{
    oops->tDelayARegistry[oops->registryIndex[T_DELAYA]] = o;
    
    return oops->registryIndex[T_DELAYA]++;
}

int OOPS_tEnvelopeRegister(tEnvelope *o)
{
    oops->tEnvelopeRegistry[oops->registryIndex[T_ENVELOPE]] = o;
    
    return oops->registryIndex[T_ENVELOPE]++;
}

int OOPS_tADSRRegister(tADSR *o)
{
    oops->tADSRRegistry[oops->registryIndex[T_ADSR]] = o;
    
    return oops->registryIndex[T_ADSR]++;
}

int OOPS_tRampRegister(tRamp *o)
{
    oops->tRampRegistry[oops->registryIndex[T_RAMP]] = o;
    
    return oops->registryIndex[T_RAMP]++;
}

int OOPS_tEnvelopeFollowerRegister(tEnvelopeFollower *o)
{
    oops->tEnvelopeFollowerRegistry[oops->registryIndex[T_ENVELOPEFOLLOW]] = o;
    
    return oops->registryIndex[T_ENVELOPEFOLLOW]++;
}

int OOPS_tPRCRevRegister(tPRCRev *o)
{
    oops->tPRCRevRegistry[oops->registryIndex[T_PRCREV]] = o;
    
    return oops->registryIndex[T_PRCREV]++;
}

int OOPS_tNRevRegister(tNRev *o)
{
    oops->tNRevRegistry[oops->registryIndex[T_NREV]] = o;
    
    return oops->registryIndex[T_NREV]++;
}

int OOPS_tPluckRegister(tPluck *o)
{
    oops->tPluckRegistry[oops->registryIndex[T_PLUCK]] = o;
    
    return oops->registryIndex[T_PLUCK]++;
}

int OOPS_tStifKarpRegister(tStifKarp *o)
{
    oops->tStifKarpRegistry[oops->registryIndex[T_STIFKARP]] = o;
    
    return oops->registryIndex[T_STIFKARP]++;
}


