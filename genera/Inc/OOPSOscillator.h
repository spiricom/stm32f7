/*
  ==============================================================================

    OOPSOscillator.h
    Created: 20 Jan 2017 12:00:58pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#ifndef OOPSOSCILLATOR_H_INCLUDED
#define OOPSOSCILLATOR_H_INCLUDED

#include "OOPSCore.h"

/* tCycle: Cycle/Sine waveform */
tCycle*  tCycleInit         (void);
int      tCycleSetFreq      (tCycle*  const, float freq);
float    tCycleTick         (tCycle*  const);

/* tPhasor: Aliasing phasor [0.0, 1.0) */
tPhasor* tPhasorInit        (void);
int      tPhasorSetFreq     (tPhasor*  const, float freq);
float    tPhasorTick        (tPhasor*  const);

/* tSawtooth: Anti-aliased Sawtooth waveform*/
tSawtooth*  tSawtoothInit      (void);
int         tSawtoothSetFreq   (tSawtooth*  const, float freq);
float       tSawtoothTick      (tSawtooth*  const);

/* tTriangle: Anti-aliased Triangle waveform */
tTriangle*  tTriangleInit      (void);
int         tTriangleSetFreq   (tTriangle*  const, float freq);
float       tTriangleTick      (tTriangle*  const);

/* tSquare: Anti-aliased Square waveform */
tSquare*    tSquareInit        (void);
int         tSquareSetFreq     (tSquare*  const, float freq);
float       tSquareTick        (tSquare*  const);

/* tNoise. WhiteNoise, PinkNoise. */
tNoise*  tNoiseInit         (NoiseType type);
float    tNoiseTick         (tNoise*  const);


#endif  // OOPSOSCILLATOR_H_INCLUDED
