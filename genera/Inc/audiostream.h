/**
  ******************************************************************************
  * @file    Audio_playback_and_record/inc/waveplayer.h
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    26-June-2014
  * @brief   Header for waveplayer.c module.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */   
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AUDIOSTREAM_H
#define __AUDIOSTREAM_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

#include "OOPS.h"



//adc is
// 0 = joy Y
// 1 = knob
// 2 = pedal
// 3 = breath sensor
// 4 = slide

extern float testFreq;


typedef enum ADCInput
{
	ADCJoyY = 0,
	ADCKnob,
	ADCPedal,
	ADCBreath,
	ADCSlide,
	ADCInputNil,
	ADCInputCount = ADCInputNil
} ADCInput;

typedef enum FTMode
{
	FTFeedback = 0,
	FTSynthesisOne, 
	FTModeNil,
	FTModeCount = FTModeNil
} FTMode;

typedef enum KnobMode
{
	SlideTune = 0,
	MasterTune = 1, 
	OctaveTune = 2,
	DelayTune = 3,
	KnobModeNil = 4,
	KnobModeCount = 5
} KnobMode;

typedef enum HarmonicMode
{
	CaraMode = 0,
	RajeevMode,
	JennyMode,
	HarmonicModeNil,
	HarmonicModeCount = HarmonicModeNil
} HarmonicMode;

extern HarmonicMode hMode;

extern int octave;
extern int16_t position;
extern uint16_t firstPositionValue;
extern uint16_t knobValue;
extern float knobValueToUse;
extern uint16_t slideValue;

extern FTMode ftMode;
extern KnobMode kMode;
extern tRamp* adc[ADCInputCount];

extern float intHarmonic;
extern float floatHarmonic;
extern float fundamental;
extern float customFundamental;

extern tCompressor* myCompressor;
extern tDelayL* myDelay;
extern tDelayL* feedbackDelay;

extern tSVF* oldFilter;
extern tSVF* lp;
extern tButterworth* filter;
extern tRamp* myRamp;
extern tCycle* mySine;

extern tSawtooth* osc;
extern tRamp* freqRamp;


/* Exported types ------------------------------------------------------------*/
typedef enum
{
  BUFFER_OFFSET_NONE = 0,  
  BUFFER_OFFSET_HALF,  
  BUFFER_OFFSET_FULL,     
}BUFFER_StateTypeDef;


void slideValueChanged(uint16_t value);

void knobValueChanged(uint16_t value);

void buttonOneDown(void);

void buttonOneUp(void);

void buttonTwoDown(void);

void buttonTwoUp(void);

void presetButtonDown(void);

void presetButtonUp(void);

void setFundamental(float fund);


/* Exported constants --------------------------------------------------------*/

extern float fundamental_hz;
extern float fundamental_cm;
extern float fundamental_m;
extern float inv_fundamental_m;
extern float cutoff_offset;
extern float intPeak;
extern float floatPeak;
extern float slide_tune;

extern float valPerM;
extern float mPerVal;

#define SAMPLE_RATE 48000.f
#define INV_SAMPLE_RATE 1.f/SAMPLE_RATE 
#define SAMPLE_RATE_MS (SAMPLE_RATE / 1000.f)
#define INV_SR_MS 1.f/SAMPLE_RATE_MS
#define SAMPLE_RATE_DIV_PARAMS 48000.f / 3
#define SAMPLE_RATE_DIV_PARAMS_MS (SAMPLE_RATE_DIV_PARAMS / 1000.f)
#define INV_SR_DIV_PARAMS_MS 1.f/SAMPLE_RATE_DIV_PARAMS_MS

typedef enum LCDModeType
{
	LCDModeDisplayPitchClass = 0,
	LCDModeDisplayPitchMidi,
	LCDModeTypeNil,
	LCDModeCount = LCDModeTypeNil
} LCDModeType;

extern uint16_t knobValue;
extern int knobMoved;
extern int calibrated;
extern LCDModeType lcdMode;

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void audioInit(I2C_HandleTypeDef* hi2c, SAI_HandleTypeDef* hsaiIn, SAI_HandleTypeDef* hsaiOut, RNG_HandleTypeDef* hrandom, uint16_t* myADCarray);

void DMA1_TransferCpltCallback(DMA_HandleTypeDef *hdma);
void DMA1_HalfTransferCpltCallback(DMA_HandleTypeDef *hdma);
#endif /* __WAVEPLAYER_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

