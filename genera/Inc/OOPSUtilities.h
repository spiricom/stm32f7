/*
  ==============================================================================

    OOPSUtilities.h
    Created: 20 Jan 2017 12:02:17pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#ifndef OOPSUTILITIES_H_INCLUDED
#define OOPSUTILITIES_H_INCLUDED

#include "OOPSCore.h"

/* Attack-Decay envelope */
tEnvelope*  tEnvelopeInit      (float attack, float decay, oBool loop);
int         tEnvelopeSetAttack (tEnvelope*  const, float attack);
int         tEnvelopeSetDecay  (tEnvelope*  const, float decay);
int         tEnvelopeLoop      (tEnvelope*  const, oBool loop);
int         tEnvelopeOn        (tEnvelope*  const, float velocity);
float       tEnvelopeTick      (tEnvelope*  const);

/* Attack-Decay-Sustain-Release envelope. */
tADSR*      tADSRInit(float attack, float decay, float sustain, float release);

/* Ramp */
tRamp*      tRampInit  (float time, int samples_per_tick);
int         tRampSetTime(tRamp*  const, float time);
int         tRampSetDest(tRamp*  const, float dest);
float       tRampTick   (tRamp*  const);

/* Envelope Follower */
tEnvelopeFollower*      tEnvelopeFollowerInit           (float attackThreshold, float decayCoeff);
int                     tEnvelopeFollowerDecayCoeff     (tEnvelopeFollower*  const, float decayCoeff);
int                     tEnvelopeFollowerAttackThresh   (tEnvelopeFollower*  const, float attackThresh);
float                   tEnvelopeFollowerTick           (tEnvelopeFollower*  const, float x);


#endif  // OOPSUTILITIES_H_INCLUDED
