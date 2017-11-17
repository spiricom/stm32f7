/*
  ==============================================================================

    MyTest.h
    Created: 23 Jan 2017 9:39:38am
    Author:  Michael R Mulshine

  ==============================================================================
*/

#include "OOPS.h"

#define INV_TWO_TO_7 0.00787401574f

typedef enum VocoboxInput
{
	KnobOne = 0,
	KnobTwo,
	KnobThree,
	KnobFour,
	Pedal,
	VocoboxInputNil
} VocoboxInput;

extern tRamp* inputRamp[VocoboxInputNil];
extern uint16_t input[VocoboxInputNil];

extern tSVF* filter;

extern tSawtooth* osc[NUM_VOICES];

extern tCycle* sine;

extern tMPoly* poly;


