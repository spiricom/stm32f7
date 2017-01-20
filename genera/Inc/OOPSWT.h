/*
  ==============================================================================

    OOPSWT.h
    Created: 4 Dec 2016 9:42:41pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#ifndef WAVETABLES_H_INCLUDED
#define WAVETABLES_H_INCLUDED

#define SINE_TABLE_SIZE 2048
#define SAW_TABLE_SIZE 2048
#define SQR_TABLE_SIZE 2048
#define TRI_TABLE_SIZE 2048
#define EXP_DECAY_TABLE_SIZE 65536
#define ATTACK_DECAY_INC_TABLE_SIZE 65536
#define SHAPER1_TABLE_SIZE 65536
#define ENVPOW_TABLE_SIZE 65536
#define TANH1_TABLE_SIZE 65536
#define ADC1_TABLE_SIZE 4096
#define MTOF1_TABLE_SIZE 4096
#define FB_TABLE_SIZE 4096
#define FILTERTAN_TABLE_SIZE 4096



extern const float FB1[FB_TABLE_SIZE];
extern const float FB2[FB_TABLE_SIZE];
extern const float FB3[FB_TABLE_SIZE];
extern const float FB4[FB_TABLE_SIZE];
extern const float FB5[FB_TABLE_SIZE];
extern const float FB6[FB_TABLE_SIZE];
extern const float FB7[FB_TABLE_SIZE];
extern const float FB8[FB_TABLE_SIZE];

// mtof lookup table based on input range [0.0,1.0) in 4096 increments - midi frequency values scaled between m25 and m134 (as done in previous code)
extern const float exp_decay[EXP_DECAY_TABLE_SIZE];
extern const float attack_decay_inc[ATTACK_DECAY_INC_TABLE_SIZE];
extern const float envelope_pow[ENVPOW_TABLE_SIZE];
extern const float filtertan[FILTERTAN_TABLE_SIZE];
extern const float mtof1[MTOF1_TABLE_SIZE];
extern const float adc1[ADC1_TABLE_SIZE];
extern const float shaper1[SHAPER1_TABLE_SIZE];
extern const float tanh1[TANH1_TABLE_SIZE];

/* sine wave table ripped from http://aquaticus.info/pwm-sine-wave */
extern const float sinewave[SINE_TABLE_SIZE];



extern const float sawtooth[11][SAW_TABLE_SIZE];
extern const float triangle[11][TRI_TABLE_SIZE];
extern const float squarewave[11][SAW_TABLE_SIZE];

#endif  // WAVETABLES_H_INCLUDED
