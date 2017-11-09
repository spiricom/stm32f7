/*
  ==============================================================================

    OPPSMemConfig.h
    Created: 23 Jan 2017 10:34:10pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#ifndef OPPSMEMCONFIG_H_INCLUDED
#define OPPSMEMCONFIG_H_INCLUDED

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                       *
 * If your application requires use of many instances of one component or is facing memory limitations,  *
 * use this set of defines to increase or limit the number of instances of each component. The library   *
 * will pre-allocate only the number of instances defined here.                                          *
 *                                                                                                       *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define SHAPER1_TABLE_SIZE 65536
extern const float shaper1[SHAPER1_TABLE_SIZE];

// Arbitrary number of instances.
#define NI 0

//#define     N_MYOBJECT            0   // Zero instances is fine.
#define     N_COMPRESSOR        1
#define     N_PRCREV            0
#define     N_NREV              0
#define     N_PLUCK             0
#define     N_STIFKARP          0
#define     N_NEURON            0
#define     N_PHASOR            1
#define     N_CYCLE             1
#define     N_SAWTOOTH          1
#define     N_TRIANGLE          1
#define     N_SQUARE            1
#define     N_NOISE             1 + (1 * N_STIFKARP) + (1 * N_PLUCK) // StifKarp and Pluck each contain 1 Noise component.
#define     N_ONEPOLE           NI + (1 * N_PLUCK)
#define     N_TWOPOLE           NI
#define     N_ONEZERO           NI + (1 * N_STIFKARP) + (1 * N_PLUCK) + (1 * N_NEURON)
#define 		N_BUTTERWORTH       1
#define     N_TWOZERO           NI
#define     N_POLEZERO          NI + (1 * N_NEURON)
#define     N_BIQUAD            NI + (4 * N_STIFKARP)
#define     N_SVF               1 + 32*N_BUTTERWORTH
#define     N_SVFE              NI
#define     N_HIGHPASS          NI
#define     N_DELAY             NI + (14 * N_NREV) + (3 * N_PRCREV)
#define     N_DELAYL            2 + (1 * N_STIFKARP) + (1 * N_PLUCK)
#define     N_DELAYA            NI + (1 * N_PRCREV) + (1 * N_STIFKARP)
#define     N_ENVELOPE          NI
#define     N_ADSR              NI
#define     N_RAMP              20

#define     N_ENVELOPEFOLLOW    NI

#define     DELAY_LENGTH        16000   // The maximum delay length of all Delay/DelayL/DelayA components.
                                            // Feel free to change to suit memory constraints or desired delay max length / functionality.

#define     INC_MISC_WT         0     // Set this to 1 if you are interested in the mtof1, adc1, tanh1, and shaper1 wavetables
                                        // and have spare memory.

// Preprocessor defines to determine whether to include component files in build.
#define INC_UTILITIES       (N_ENVELOPE || N_ENVELOPEFOLLOW || N_RAMP || N_ADSR || N_COMPRESSOR)
#define INC_DELAY           (N_DELAY || N_DELAYL || N_DELAYA)
#define INC_FILTER          (N_BUTTERWORTH || N_ONEPOLE || N_TWOPOLE || N_ONEZERO || N_TWOZERO || N_POLEZERO || N_BIQUAD || N_SVF || N_SVFE || N_HIGHPASS)
#define INC_OSCILLATOR      (N_PHASOR || N_SAWTOOTH || N_CYCLE || N_TRIANGLE || N_SQUARE || N_NOISE)
#define INC_REVERB          (N_NREV || N_PRCREV)
#define INC_INSTRUMENT      (N_STIFKARP || N_PLUCK)

#endif  // OPPSMEMCONFIG_H_INCLUDED
