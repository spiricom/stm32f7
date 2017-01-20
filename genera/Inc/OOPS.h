/*
  ==============================================================================

    OOPS.h
    Created: 18 Jan 2017 12:40:15pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#ifndef OOPSBASE_H_INCLUDED
#define OOPSBASE_H_INCLUDED

#include "OOPSBase.h"

#include "OOPSWT.h"

#pragma OOPS
int         OOPSInit            (float sr, float(*random)(void));
int         OOPSSetSampleRate   (float sampleRate);
int         OOPSGetSampleRate   (void);
int         OOPSRegister        (void (*updater)(void));

#pragma Basic utilities
float       OOPS_clip    (float min, float val, float max);

int         OOPS_isPrime (uint64_t number );

double      OOPS_midiToFrequency    ( double f );


#pragma Basic oscillators/waveforms
/* tPhasor: Aliasing phasor [0.0, 1.0) */
int      tPhasorInit(tPhasor *p);
int      tPhasorFreq(tPhasor *p, float freq);
float    tPhasorTick(tPhasor *p);
void     tPhasorSampleRateChanged (tPhasor *c);

/* tCycle: Cycle/Sine waveform */
int      tCycleInit(tCycle *c);
int      tCycleSetFreq(tCycle *c, float freq);
float    tCycleTick(tCycle *c);
void     tCycleSampleRateChanged (tCycle *c);

/* tSawtooth: Anti-aliased Sawtooth waveform*/
int      tSawtoothInit(tSawtooth *t);
int      tSawtoothSetFreq(tSawtooth *s, float freq);
float    tSawtoothTick(tSawtooth *s);
void     tSawtoothSampleRateChanged (tSawtooth *c);

/* tTriangle: Anti-aliased Triangle waveform */
int      tTriangleInit(tTriangle *t);
int      tTriangleSetFreq(tTriangle *t, float freq);
float    tTriangleTick(tTriangle *t);
void     tTriangleSampleRateChanged (tTriangle *c);

/* tSquare: Anti-aliased Square waveform */
int      tSquareInit  (tSquare *p);
int      tSquareSetFreq  (tSquare *p, float freq);
float    tSquareTick  (tSquare *p);
void     tSquareSampleRateChanged (tSquare *c);

/* tNoise: Noise generator */
int      tNoiseInit     (tNoise *n, NoiseType type);
float    tNoiseTick     (tNoise *n);
void     tNoiseSampleRateChanged (tNoise *c);

#pragma Filters

/* tOnePole: OnePole filter */
int      tOnePoleInit           (tOnePole *f, float thePole);
void     tOnePoleSetB0          (tOnePole *f, float b0);
void     tOnePoleSetA1          (tOnePole *f, float a1);
void     tOnePoleSetPole        (tOnePole *f, float thePole);
void     tOnePoleSetCoefficients(tOnePole *f, float b0, float a1);
void     tOnePoleSetGain        (tOnePole *f, float gain);
float    tOnePoleTick           (tOnePole *f, float input);
void     tOnePoleSampleRateChanged (tOnePole *c);

/* TwoPole filter */
int      tTwoPoleInit           (tTwoPole *f);
void     tTwoPoleSetB0          (tTwoPole *f, float b0);
void     tTwoPoleSetA1          (tTwoPole *f, float a1);
void     tTwoPoleSetA2          (tTwoPole *f, float a2);
void     tTwoPoleSetResonance   (tTwoPole *f, float freq, float radius, int normalize);
void     tTwoPoleSetCoefficients(tTwoPole *f, float b0, float a1, float a2);
void     tTwoPoleSetGain        (tTwoPole *f, float gain);
float    tTwoPoleTick           (tTwoPole *f, float input);
void     tTwoPoleSampleRateChanged (tTwoPole *c);

/* OneZero filter */
int      tOneZeroInit(tOneZero *f, float theZero);
void     tOneZeroSetB0(tOneZero *f, float b0);
void     tOneZeroSetB1(tOneZero *f, float b1);
void     tOneZeroSetZero(tOneZero *f, float theZero);
void     tOneZeroSetCoefficients(tOneZero *f, float b0, float b1);
void     tOneZeroSetGain(tOneZero *f, float gain);
float    tOneZeroTick(tOneZero *f, float input);

float    tOneZeroGetPhaseDelay(tOneZero *f, float frequency ); // CAN / SHOULD MAKE GENERIC VERSION OF THIS FUNCTION FOR ALL FILTERS. ALL FILTERS SHOULD HAVE FIXED SIZE ARRAY OF As and Bs. Pass these arays to function. As/Bs not used are -1 in arrays.
void     tOneZeroSampleRateChanged (tOneZero *c);

/* TwoZero filter */
int      tTwoZeroInit           (tTwoZero *f);
void     tTwoZeroSetB0          (tTwoZero *f, float b0);
void     tTwoZeroSetB1          (tTwoZero *f, float b1);
void     tTwoZeroSetB2          (tTwoZero *f, float b2);
void     tTwoZeroSetNotch       (tTwoZero *f, float frequency, float radius);
void     tTwoZeroSetCoefficients(tTwoZero *f, float b0, float b1, float b2);
void     tTwoZeroSetGain        (tTwoZero *f, float gain);
float    tTwoZeroTick           (tTwoZero *f, float input);
void     tTwoZeroSampleRateChanged (tTwoZero *c);

/* PoleZero filter */
int      tPoleZeroInit              (tPoleZero *pzf);
void     tPoleZeroSetB0             (tPoleZero *pzf, float b0);
void     tPoleZeroSetB1             (tPoleZero *pzf, float b1);
void     tPoleZeroSetA1             (tPoleZero *pzf, float a1);
void     tPoleZeroSetCoefficients   (tPoleZero *pzf, float b0, float b1, float a1);
void     tPoleZeroSetAllpass        (tPoleZero *pzf, float coeff);
void     tPoleZeroSetBlockZero      (tPoleZero *pzf, float thePole);
void     tPoleZeroSetGain           (tPoleZero *pzf, float gain);
float    tPoleZeroTick              (tPoleZero *pzf, float input);
void     tPoleZeroSampleRateChanged (tPoleZero *c);

/* BiQuad filter */
int      tBiQuadInit           (tBiQuad *f);
void     tBiQuadSetB0          (tBiQuad *f, float b0);
void     tBiQuadSetA1          (tBiQuad *f, float a1);
void     tBiQuadSetA2          (tBiQuad *f, float a2);
void     tBiQuadSetNotch       (tBiQuad *f, float freq, float radius);
void     tBiQuadSetResonance   (tBiQuad *f, float freq, float radius, int normalize);
void     tBiQuadSetCoefficients(tBiQuad *f, float b0, float b1, float b2, float a1, float a2);
void     tBiQuadSetGain        (tBiQuad *f, float gain);
float    tBiQuadTick           (tBiQuad *f, float input);
void     tBiQuadSampleRateChanged (tBiQuad *c);

/* State Variable Filter, adapted from ???. */
int      tSVFInit(tSVF *svf, SVFType type, uint16_t cutoffKnob, float Q);
float    tSVFTick(tSVF *svf, float v0);
int      tSVFSetFreq(tSVF *svf, uint16_t cutoffKnob);
int      tSVFSetQ(tSVF *svf, float Q);
void     tSVFSampleRateChanged (tSVF *c);

/* Efficient State Variable Filter for 14-bit control input, [0, 4096). */
int      tSVFEfficientInit    (tSVFEfficient *svf, SVFType type, uint16_t cutoffKnob, float Q);
float    tSVFEfficientTick    (tSVFEfficient *svf, float v0);
int      tSVFEfficientSetFreq (tSVFEfficient *svf, uint16_t cutoffKnob);
int      tSVFEfficientSetQ    (tSVFEfficient *svf, float Q);
void     tSVFEfficientSampleRateChanged (tSVFEfficient *c);

/* Simple Highpass filter */
int      tHighpassInit   (tHighpass *hp, float freq);
int      tHighpassFreq   (tHighpass *hp, float freq);
float    tHighpassTick   (tHighpass *hp, float x);
void     tHighpassSampleRateChanged (tHighpass *c);

#pragma Time-based Utilities
/* Non-interpolating delay */
int         tDelayInit      (tDelay *d, uint32_t delay, uint32_t maxDelay, float *buff);
int         tDelaySetDelay  (tDelay *d, uint32_t delay);
uint32_t    tDelayGetDelay  (tDelay *d);
void        tDelayTapIn     (tDelay *d, float in, uint32_t tapDelay);
float       tDelayTapOut    (tDelay *d, uint32_t tapDelay);
float       tDelayAddTo     (tDelay *d, float value, uint32_t tapDelay);
float       tDelayTick      (tDelay *d, float sample);

float       tDelayGetLastOut(tDelay *d);
float       tDelayGetLastIn (tDelay *d);
void     tDelaySampleRateChanged (tDelay *c);

/* Linearly-interpolating delay*/
int         tDelayLInit      (tDelayL *d, float delay, uint32_t maxDelay, float *buff);
int         tDelayLSetDelay  (tDelayL *d, float delay);
float       tDelayLGetDelay  (tDelayL *d);
void        tDelayLTapIn     (tDelayL *d, float in, uint32_t tapDelay);
float       tDelayLTapOut    (tDelayL *d, uint32_t tapDelay);
float       tDelayLAddTo     (tDelayL *d, float value, uint32_t tapDelay);
float       tDelayLTick      (tDelayL *d, float sample);

float       tDelayLGetLastOut(tDelayL *d);
float       tDelayLGetLastIn (tDelayL *d);
void     tDelayLSampleRateChanged (tDelayL *c);

/* Allpass-interpolating delay*/
int         tDelayAInit      (tDelayA *d, float delay, uint32_t maxDelay, float *buff);
int         tDelayASetDelay  (tDelayA *d, float delay);
float       tDelayAGetDelay  (tDelayA *d);
void        tDelayATapIn     (tDelayA *d, float in, uint32_t tapDelay);
float       tDelayATapOut    (tDelayA *d, uint32_t tapDelay);
float       tDelayAAddTo     (tDelayA *d, float value, uint32_t tapDelay);
float       tDelayATick      (tDelayA *d, float sample);

float       tDelayAGetLastOut(tDelayA *d);
float       tDelayAGetLastIn (tDelayA *d);
void     tDelayASampleRateChanged (tDelayA *c);


/* Attack-Decay envelope */
int      tEnvelopeInit(tEnvelope *env, float attack, float decay, int loop,
                       const float *exponentialTable, const float *attackDecayIncTable);
int      tEnvelopeSetAttack(tEnvelope *env, float attack);
int      tEnvelopeSetDecay(tEnvelope *env, float decay);
int      tEnvelopeLoop(tEnvelope *env, int loop);
int      tEnvelopeOn(tEnvelope *env, float velocity);
float    tEnvelopeTick(tEnvelope *env);
void     tEnvelopeSampleRateChanged (tEnvelope *c);

/* Attack-Decay-Sustain-Release envelope. */
int tADSRInit(tADSR *d, float attack, float decay, float sustain, float release);

/* Ramp */
int      tRampInit   (tRamp *r, float time, int samples_per_tick);
int      tRampSetTime(tRamp *r, float time);
int      tRampSetDest(tRamp *r, float dest);
float    tRampTick   (tRamp *r);
void     tRampSampleRateChanged (tRamp *c);

#pragma Complex/Miscellaneous
/* Envelope Follower */
int      tEnvelopeFollowerInit           (tEnvelopeFollower *ef, float attackThreshold, float decayCoeff);
int      tEnvelopeFollowerDecayCoeff     (tEnvelopeFollower *ef, float decayCoeff);
int      tEnvelopeFollowerAttackThresh   (tEnvelopeFollower *ef, float attackThresh);
float    tEnvelopeFollowerTick           (tEnvelopeFollower *ef, float x);
void     tEnvelopeFollowerSampleRateChanged (tEnvelopeFollower *c);

/* PRCRev: Reverb, adapted from STK, algorithm by Perry Cook. */
int     tPRCRevInit      (tPRCRev *r, float t60, float delayBuffers[3][REV_DELAY_LENGTH]);
void    tPRCRevSetT60    (tPRCRev *r, float t60);
void    tPRCRevSetMix    (tPRCRev *r, float mix);
float   tPRCRevTick      (tPRCRev *r, float input);
void     tPRCRevSampleRateChanged (tPRCRev *c);

/* NRev: Reverb, adpated from STK. */
int      tNRevInit   (tNRev *r, float t60, float delayBuffers[14][REV_DELAY_LENGTH]);
void     tNRevSetT60 (tNRev *r, float t60);
void     tNRevSetMix (tNRev *r, float mix);
float    tNRevTick   (tNRev *r, float input);
void     tNRevSampleRateChanged (tNRev *c);

#pragma Physical Models

/* tPluck */
int     tPluckInit          (tPluck *p, float lowestFrequency, float delayBuff[REV_DELAY_LENGTH]);
float   tPluckTick          (tPluck *p);
void    tPluckPluck         (tPluck *p, float amplitude);
void    tPluckNoteOn        (tPluck *p, float frequency, float amplitude ); // Start a note with the given frequency and amplitude.;
void    tPluckNoteOff       (tPluck *p, float amplitude ); // Stop a note with the given amplitude (speed of decay).
void    tPluckSetFrequency  (tPluck *p, float frequency ); // Set instrument parameters for a particular frequency.
void    tPluckControlChange (tPluck *p, int number, float value); // Perform the control change specified by \e number and \e value (0.0 - 128.0).
float   tPluckGetLastOut    (tPluck *p);
void     tPluckSampleRateChanged (tPluck *c);

/* tStifKarp */
typedef enum SKControlType
{
    SKPickPosition = 0,
    SKStringDamping,
    SKDetune,
    SKControlTypeNil
} SKControlType;

int     tStifKarpInit          		(tStifKarp *p, float lowestFrequency, float delayBuff[2][REV_DELAY_LENGTH]);
float   tStifKarpTick               (tStifKarp *p);
void    tStifKarpPluck              (tStifKarp *p, float amplitude);
void    tStifKarpNoteOn             (tStifKarp *p, float frequency, float amplitude ); // Start a note with the given frequency and amplitude.;
void    tStifKarpNoteOff            (tStifKarp *p, float amplitude ); // Stop a note with the given amplitude (speed of decay).
void    tStifKarpSetFrequency       (tStifKarp *p, float frequency ); // Set instrument parameters for a particular frequency.
void    tStifKarpControlChange      (tStifKarp *p, SKControlType type, float value); // Perform the control change specified by \e number and \e value (0.0 - 128.0).
void    tStifKarpSetStretch         (tStifKarp *p, float stretch );//! Set the stretch "factor" of the string (0.0 - 1.0).
void    tStifKarpSetPickupPosition  (tStifKarp *p, float position ); //! Set the pluck or "excitation" position along the string (0.0 - 1.0).
void    tStifKarpSetBaseLoopGain    (tStifKarp *p, float aGain ); //! Set the base loop gain.
float   tStifKarpGetLastOut         (tStifKarp *p);
void    tStifKarpSampleRateChanged (tStifKarp *c);

int OOPS_tPhasorRegister(tPhasor *o);
int OOPS_tCycleRegister(tCycle *o);
int OOPS_tSawtoothRegister(tSawtooth *o);
int OOPS_tTriangleRegister(tTriangle *o);
int OOPS_tSquareRegister(tSquare *o);
int OOPS_tNoiseRegister(tNoise *o);
int OOPS_tOnePoleRegister(tOnePole *o);
int OOPS_tTwoPoleRegister(tTwoPole *o);
int OOPS_tOneZeroRegister(tOneZero *o);
int OOPS_tTwoZeroRegister(tTwoZero *o);
int OOPS_tPoleZeroRegister(tPoleZero *o);
int OOPS_tBiQuadRegister(tBiQuad *o);
int OOPS_tSVFRegister(tSVF *o);
int OOPS_tSVFEfficientRegister(tSVFEfficient *o);
int OOPS_tHighpassRegister(tHighpass *o);
int OOPS_tDelayRegister(tDelay *o);
int OOPS_tDelayLRegister(tDelayL *o);
int OOPS_tDelayARegister(tDelayA *o);
int OOPS_tEnvelopeRegister(tEnvelope *o);
int OOPS_tADSRRegister(tADSR *o);
int OOPS_tRampRegister(tRamp *o);
int OOPS_tEnvelopeFollowerRegister(tEnvelopeFollower *o);
int OOPS_tPRCRevRegister(tPRCRev *o);
int OOPS_tNRevRegister(tNRev *o);
int OOPS_tPluckRegister(tPluck *o);
int OOPS_tStifKarpRegister(tStifKarp *o);




#endif  // OOPSBASE_H_INCLUDED
