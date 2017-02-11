/*
  ==============================================================================

    OOPSFilter.h
    Created: 20 Jan 2017 12:01:10pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#ifndef OOPSFILTER_H_INCLUDED
#define OOPSFILTER_H_INCLUDED

#include "OOPSCore.h"

/* tOnePole: OnePole filter */
tOnePole*     tOnePoleInit           (float thePole);
void     tOnePoleSetB0          (tOnePole*  const, float b0);
void     tOnePoleSetA1          (tOnePole*  const, float a1);
void     tOnePoleSetPole        (tOnePole*  const, float thePole);
void     tOnePoleSetCoefficients(tOnePole*  const, float b0, float a1);
void     tOnePoleSetGain        (tOnePole*  const, float gain);
float    tOnePoleTick           (tOnePole*  const, float input);

/* TwoPole filter */
tTwoPole*     tTwoPoleInit           (void);
void     tTwoPoleSetB0          (tTwoPole*  const, float b0);
void     tTwoPoleSetA1          (tTwoPole*  const, float a1);
void     tTwoPoleSetA2          (tTwoPole*  const, float a2);
void     tTwoPoleSetResonance   (tTwoPole*  const, float freq, float radius, oBool normalize);
void     tTwoPoleSetCoefficients(tTwoPole*  const, float b0, float a1, float a2);
void     tTwoPoleSetGain        (tTwoPole*  const, float gain);
float    tTwoPoleTick           (tTwoPole*  const, float input);

/* OneZero filter */
tOneZero*     tOneZeroInit           (float theZero);
void     tOneZeroSetB0          (tOneZero*  const, float b0);
void     tOneZeroSetB1          (tOneZero*  const, float b1);
void     tOneZeroSetZero        (tOneZero*  const, float theZero);
void     tOneZeroSetCoefficients(tOneZero*  const, float b0, float b1);
void     tOneZeroSetGain        (tOneZero*  const, float gain);
float    tOneZeroTick           (tOneZero*  const, float input);
float    tOneZeroGetPhaseDelay(tOneZero *f, float frequency ); // CAN / SHOULD MAKE GENERIC VERSION OF THIS FUNCTION FOR ALL FILTERS. ALL FILTERS SHOULD HAVE FIXED SIZE ARRAY OF As and Bs. Pass these arays to function. As/Bs not used are -1 in arrays.

/* TwoZero filter */
tTwoZero*     tTwoZeroInit           (void);
void     tTwoZeroSetB0          (tTwoZero*  const, float b0);
void     tTwoZeroSetB1          (tTwoZero*  const, float b1);
void     tTwoZeroSetB2          (tTwoZero*  const, float b2);
void     tTwoZeroSetNotch       (tTwoZero*  const, float frequency, float radius);
void     tTwoZeroSetCoefficients(tTwoZero*  const, float b0, float b1, float b2);
void     tTwoZeroSetGain        (tTwoZero*  const, float gain);
float    tTwoZeroTick           (tTwoZero*  const, float input);

/* PoleZero filter */
tPoleZero*     tPoleZeroInit              (void);
void     tPoleZeroSetB0             (tPoleZero*  const, float b0);
void     tPoleZeroSetB1             (tPoleZero*  const, float b1);
void     tPoleZeroSetA1             (tPoleZero*  const, float a1);
void     tPoleZeroSetCoefficients   (tPoleZero*  const, float b0, float b1, float a1);
void     tPoleZeroSetAllpass        (tPoleZero*  const, float coeff);
void     tPoleZeroSetBlockZero      (tPoleZero*  const, float thePole);
void     tPoleZeroSetGain           (tPoleZero*  const, float gain);
float    tPoleZeroTick              (tPoleZero*  const, float input);

/* BiQuad filter */
tBiQuad*     tBiQuadInit           (void);
void     tBiQuadSetB0          (tBiQuad*  const, float b0);
void     tBiQuadSetB1          (tBiQuad*  const, float b1);
void     tBiQuadSetB2          (tBiQuad*  const, float b2);
void     tBiQuadSetA1          (tBiQuad*  const, float a1);
void     tBiQuadSetA2          (tBiQuad*  const, float a2);
void     tBiQuadSetNotch       (tBiQuad*  const, float freq, float radius);
void     tBiQuadSetResonance   (tBiQuad*  const, float freq, float radius, oBool normalize);
void     tBiQuadSetCoefficients(tBiQuad* const f, float b0, float b1, float b2, float a1, float a2);
void     tBiQuadSetGain        (tBiQuad*  const, float gain);
float    tBiQuadTick           (tBiQuad*  const, float input);

/* State Variable Filter, adapted from ???. */
tSVF*     tSVFInit       (SVFType type, float freq, float Q);
float    tSVFTick       (tSVF*  const, float v0);
int      tSVFSetFreq    (tSVF*  const, float freq);
int      tSVFSetQ       (tSVF*  const, float Q);

/* Efficient State Variable Filter for 14-bit control input, [0, 4096). */
tSVFE*     tSVFEInit    (SVFType type, uint16_t controlFreq, float Q);
float    tSVFETick    (tSVFE*  const, float v0);
int      tSVFESetFreq (tSVFE*  const, uint16_t controlFreq);
int      tSVFESetQ    (tSVFE*  const, float Q);

/* Simple Highpass filter */
tHighpass*     tHighpassInit      (float freq);
void     tHighpassSetFreq   (tHighpass*  const, float freq);
float    tHighpassGetFreq   (tHighpass*  const);
float    tHighpassTick      (tHighpass*  const, float x);



#endif  // OOPSFILTER_H_INCLUDED
