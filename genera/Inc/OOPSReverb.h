/*
  ==============================================================================

    OOPSReverb.h
    Created: 20 Jan 2017 12:02:04pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#ifndef OOPSREVERB_H_INCLUDED
#define OOPSREVERB_H_INCLUDED

#include "OOPSMath.h"
#include "OOPSCore.h"

/* PRCRev: Reverb, adapted from STK, algorithm by Perry Cook. */
tPRCRev* tPRCRevInit      (float t60, float delayBuffers[3][DELAY_LENGTH]);
void     tPRCRevSetT60    (tPRCRev*  const, float t60);
void     tPRCRevSetMix    (tPRCRev*  const, float mix);
float    tPRCRevTick      (tPRCRev*  const, float input);

/* NRev: Reverb, adpated from STK. */
tNRev*   tNRevInit   (float t60, float delayBuffers[14][DELAY_LENGTH]);
void     tNRevSetT60 (tNRev*  const, float t60);
void     tNRevSetMix (tNRev*  const, float mix);
float    tNRevTick   (tNRev*  const, float input);


#endif  // OOPSREVERB_H_INCLUDED
