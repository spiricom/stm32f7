/*
  ==============================================================================

    OOPSOscillator.c
    Created: 20 Jan 2017 12:00:58pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#include "OOPSWavetables.h"
#include "OOPSOscillator.h"
#include "OOPS.h"

#if N_CYCLE
// Cycle
tCycle*    tCycleInit(void)
{
    tCycle* c = &oops.tCycleRegistry[oops.registryIndex[T_CYCLE]++];
    
    c->inc      =  0.0f;
    c->phase    =  0.0f;
    c->sampleRateChanged = &tCycleSampleRateChanged;
    
    
    
    return c;
}

int     tCycleSetFreq(tCycle* const c, float freq)
{
    if (freq < 0.0f) freq = 0.0f;
    
    c->freq = freq;
    c->inc = freq * oops.invSampleRate;
    
    return 0;
}

float   tCycleTick(tCycle* const c)
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

void     tCycleSampleRateChanged (tCycle* const c)
{
    c->inc = c->freq * oops.invSampleRate;
}
#endif //N_CYCLE

#if N_PHASOR
/* Phasor */
void     tPhasorSampleRateChanged (tPhasor* const p)
{
    p->inc = p->freq * oops.invSampleRate;
};

int     tPhasorSetFreq(tPhasor* const p, float freq)
{
    if (freq < 0.0f) freq = 0.0f;
    
    p->freq = freq;
    p->inc = freq * oops.invSampleRate;
    
    return 0;
}

float   tPhasorTick(tPhasor* const p)
{
    p->phase += p->inc;
    
    if (p->phase >= 1.0f) p->phase -= 1.0f;
    
    return p->phase;
}

tPhasor*    tPhasorInit(void)
{
    if (oops.registryIndex[T_PHASOR] >= N_PHASOR) return NULL;
    
    tPhasor* p = &oops.tPhasorRegistry[oops.registryIndex[T_PHASOR]++];
    
    p->phase = 0.0f;
    p->inc = 0.0f;
    
    p->sampleRateChanged = &tPhasorSampleRateChanged;
    
    return p;
    
}
#endif //N_PHASOR

#if N_SAWTOOTH
tSawtooth*    tSawtoothInit(void)
{
    if (oops.registryIndex[T_SAWTOOTH] >= N_SAWTOOTH) return NULL;
    
    tSawtooth* c = &oops.tSawtoothRegistry[oops.registryIndex[T_SAWTOOTH]++];
    
    c->inc      = 0.0f;
    c->phase    = 0.0f;
    
    c->sampleRateChanged = &tSawtoothSampleRateChanged;
    
    return c;
}

int     tSawtoothSetFreq(tSawtooth* const c, float freq)
{
    if (freq < 0.0f) freq = 0.0f;
    
    c->freq = freq;
    c->inc = freq * oops.invSampleRate;
    
    return 0;
}

float   tSawtoothTick(tSawtooth* const c)
{
    // Phasor increment
    c->phase += c->inc;
    if (c->phase >= 1.0f) c->phase -= 1.0f;
    
    float out = 0.0f;
    float w;
    
    int idx = (int)(c->phase * TRI_TABLE_SIZE);
    
    // Wavetable synthesis
    
    if (c->freq <= 20.0f)
    {
        out = sawtooth[T20][idx];
    }
    else if (c->freq <= 40.0f)
    {
        w = ((40.0f - c->freq) * INV_20);
        out = (sawtooth[T20][idx] * w) + (sawtooth[T40][idx] * (1.0f - w));
    }
    else if (c->freq <= 80.0f)
    {
        w = ((80.0f - c->freq) * INV_40);
        out = (sawtooth[T40][idx] * w) + (sawtooth[T80][idx] * (1.0f - w));
    }
    else if (c->freq <= 160.0f)
    {
        w = ((160.0f - c->freq) * INV_80);
        out = (sawtooth[T80][idx] * w) + (sawtooth[T160][idx] * (1.0f - w));
    }
    else if (c->freq <= 320.0f)
    {
        w = ((320.0f - c->freq) * INV_160);
        out = (sawtooth[T160][idx] * w) + (sawtooth[T320][idx] * (1.0f - w));
    }
    else if (c->freq <= 640.0f)
    {
        w = ((640.0f - c->freq) * INV_320);
        out = (sawtooth[T320][idx] * w) + (sawtooth[T640][idx] * (1.0f - w));
    }
    else if (c->freq <= 1280.0f)
    {
        w = ((1280.0f - c->freq) * INV_640);
        out = (sawtooth[T640][idx] * w) + (sawtooth[T1280][idx] * (1.0f - w));
    }
    else if (c->freq <= 2560.0f)
    {
        w = ((2560.0f - c->freq) * INV_1280);
        out = (sawtooth[T1280][idx] * w) + (sawtooth[T2560][idx] * (1.0f - w));
    }
    else if (c->freq <= 5120.0f)
    {
        w = ((5120.0f - c->freq) * INV_2560);
        out = (sawtooth[T2560][idx] * w) + (sawtooth[T5120][idx] * (1.0f - w));
    }
    else if (c->freq <= 10240.0f)
    {
        w = ((10240.0 - c->freq) * INV_5120);
        out = (sawtooth[T5120][idx] * w) + (sawtooth[T10240][idx] * (1.0f - w));
    }
    else if (c->freq <= 20480.0f)
    {
        w = ((20480.0f - c->freq) * INV_10240);
        out = (sawtooth[T10240][idx] * w) + (sawtooth[T20480][idx] * (1.0f - w));
    }
    else
    {
        out = sawtooth[T20480][idx];
    }
    
    return out;
}

void     tSawtoothSampleRateChanged (tSawtooth* const c)
{
    c->inc = c->freq * oops.invSampleRate;
}
#endif //N_SAWTOOTH

#if N_TRIANGLE
/* Triangle */
void    tTriangleStart(tTriangle* const c)
{
    
}

tTriangle*    tTriangleInit(void)
{
    if (oops.registryIndex[T_TRIANGLE] >= N_TRIANGLE) return NULL;
    
    tTriangle* c = &oops.tTriangleRegistry[oops.registryIndex[T_TRIANGLE]++];
    
    c->inc      =  0.0f;
    c->phase    =  0.0f;
    
    c->sampleRateChanged = &tTriangleSampleRateChanged;
    
    return c;
}

int tTriangleSetFreq(tTriangle* const c, float freq)
{
    if (freq < 0.0f) freq = 0.0f;
    
    c->freq = freq;
    c->inc = freq * oops.invSampleRate;
    
    return 0;
}


float   tTriangleTick(tTriangle* const c)
{
    // Phasor increment
    c->phase += c->inc;
    if (c->phase >= 1.0f) c->phase -= 1.0f;
    
    float out = 0.0f;
    float w;
    
    int idx = (int)(c->phase * TRI_TABLE_SIZE);
    
    // Wavetable synthesis
    
    if (c->freq <= 20.0f)
    {
        out = triangle[T20][idx];
    }
    else if (c->freq <= 40.0f)
    {
        w = ((40.0f - c->freq) * INV_20);
        out = (triangle[T20][idx] * w) + (triangle[T40][idx] * (1.0f - w));
    }
    else if (c->freq <= 80.0f)
    {
        w = ((80.0f - c->freq) * INV_40);
        out = (triangle[T40][idx] * w) + (triangle[T80][idx] * (1.0f - w));
    }
    else if (c->freq <= 160.0f)
    {
        w = ((160.0f - c->freq) * INV_80);
        out = (triangle[T80][idx] * w) + (triangle[T160][idx] * (1.0f - w));
    }
    else if (c->freq <= 320.0f)
    {
        w = ((320.0f - c->freq) * INV_160);
        out = (triangle[T160][idx] * w) + (triangle[T320][idx] * (1.0f - w));
    }
    else if (c->freq <= 640.0f)
    {
        w = ((640.0f - c->freq) * INV_320);
        out = (triangle[T320][idx] * w) + (triangle[T640][idx] * (1.0f - w));
    }
    else if (c->freq <= 1280.0f)
    {
        w = ((1280.0f - c->freq) * INV_640);
        out = (triangle[T640][idx] * w) + (triangle[T1280][idx] * (1.0f - w));
    }
    else if (c->freq <= 2560.0f)
    {
        w = ((2560.0f - c->freq) * INV_1280);
        out = (triangle[T1280][idx] * w) + (triangle[T2560][idx] * (1.0f - w));
    }
    else if (c->freq <= 5120.0f)
    {
        w = ((5120.0f - c->freq) * INV_2560);
        out = (triangle[T2560][idx] * w) + (triangle[T5120][idx] * (1.0f - w));
    }
    else if (c->freq <= 10240.0f)
    {
        w = ((10240.0 - c->freq) * INV_5120);
        out = (triangle[T5120][idx] * w) + (triangle[T10240][idx] * (1.0f - w));
    }
    else if (c->freq <= 20480.0f)
    {
        w = ((20480.0f - c->freq) * INV_10240);
        out = (triangle[T10240][idx] * w) + (triangle[T20480][idx] * (1.0f - w));
    }
    else
    {
        out = triangle[T20480][idx];
    }
    
    return out;
}

void     tTriangleSampleRateChanged (tTriangle*  const c)
{
    c->inc = c->freq * oops.invSampleRate;
}
#endif //N_TRIANGLE

#if N_SQUARE
/* Square */
tSquare*    tSquareInit(void)
{
    if (oops.registryIndex[T_SQUARE] >= N_SQUARE) return NULL;
    
    tSquare* c = &oops.tSquareRegistry[oops.registryIndex[T_SQUARE]++];
    
    c->inc      =  0.0f;
    c->phase    =  0.0f;
    c->sampleRateChanged = &tSquareSampleRateChanged;
    
    return c;
}

int     tSquareSetFreq(tSquare*  const c, float freq)
{
    if (freq < 0.0f) freq = 0.0f;
    
    c->freq = freq;
    c->inc = freq * oops.invSampleRate;
    
    return 0;
}

float   tSquareTick(tSquare* const c)
{
    // Phasor increment
    c->phase += c->inc;
    if (c->phase >= 1.0f) c->phase -= 1.0f;
    
    float out = 0.0f;
    float w = 0.0f;
    int idx = (int)(c->phase * TRI_TABLE_SIZE);
    
    // Wavetable synthesis
    
    if (c->freq <= 20.0f)
    {
        out = squarewave[T20][idx];
    }
    else if (c->freq <= 40.0f)
    {
        w = ((40.0f - c->freq) * INV_20);
        out = (squarewave[T20][idx] * w) + (squarewave[T40][idx] * (1.0f - w));
    }
    else if (c->freq <= 80.0f)
    {
        w = ((80.0f - c->freq) * INV_40);
        out = (squarewave[T40][idx] * w) + (squarewave[T80][idx] * (1.0f - w));
    }
    else if (c->freq <= 160.0f)
    {
        w = ((160.0f - c->freq) * INV_80);
        out = (squarewave[T80][idx] * w) + (squarewave[T160][idx] * (1.0f - w));
    }
    else if (c->freq <= 320.0f)
    {
        w = ((320.0f - c->freq) * INV_160);
        out = (squarewave[T160][idx] * w) + (squarewave[T320][idx] * (1.0f - w));
    }
    else if (c->freq <= 640.0f)
    {
        w = ((640.0f - c->freq) * INV_320);
        out = (squarewave[T320][idx] * w) + (squarewave[T640][idx] * (1.0f - w));
    }
    else if (c->freq <= 1280.0f)
    {
        w = ((1280.0f - c->freq) * INV_640);
        out = (squarewave[T640][idx] * w) + (squarewave[T1280][idx] * (1.0f - w));
    }
    else if (c->freq <= 2560.0f)
    {
        w = ((2560.0f - c->freq) * INV_1280);
        out = (squarewave[T1280][idx] * w) + (squarewave[T2560][idx] * (1.0f - w));
    }
    else if (c->freq <= 5120.0f)
    {
        w = ((5120.0f - c->freq) * INV_2560);
        out = (squarewave[T2560][idx] * w) + (squarewave[T5120][idx] * (1.0f - w));
    }
    else if (c->freq <= 10240.0f)
    {
        w = ((10240.0 - c->freq) * INV_5120);
        out = (squarewave[T5120][idx] * w) + (squarewave[T10240][idx] * (1.0f - w));
    }
    else if (c->freq <= 20480.0f)
    {
        w = ((20480.0f - c->freq) * INV_10240);
        out = (squarewave[T10240][idx] * w) + (squarewave[T20480][idx] * (1.0f - w));
    }
    else
    {
        out = squarewave[T20480][idx];
    }
    
    return out;
}

void     tSquareSampleRateChanged (tSquare*  const c)
{
    c->inc = c->freq * oops.invSampleRate;
}
#endif //N_SQUARE

#if N_NOISE
/* Noise */
tNoise*    tNoiseInit(NoiseType type)
{
    // If this returns null, you don't have memory allocated for enough tNoise objects. Check out OOPSMemConfig.h. 
    if (oops.registryIndex[T_NOISE] >= N_NOISE) return NULL;
    
    tNoise* n = &oops.tNoiseRegistry[oops.registryIndex[T_NOISE]++];
    
    n->type = type;
    n->rand = oops.random;
    
    return n;
}

float   tNoiseTick(tNoise* const n)
{
    float rand = n->rand();
    
    if (n->type == PinkNoise)
    {
        float tmp;
        n->pinkb0 = 0.99765f * n->pinkb0 + rand * 0.0990460f;
        n->pinkb1 = 0.96300f * n->pinkb1 + rand * 0.2965164f;
        n->pinkb2 = 0.57000f * n->pinkb2 + rand * 1.0526913f;
        tmp = n->pinkb0 + n->pinkb1 + n->pinkb2 + rand * 0.1848f;
        return (tmp * 0.05f);
    }
    else // WhiteNoise
    {
        return rand;
    }
}
#endif //N_NOISE
