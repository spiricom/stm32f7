/* Includes ------------------------------------------------------------------*/
#include "waveplayer.h"
#include "wavetable.h"
#include "phasor.h"

int aBufferSize = 0.0f;
__IO uint32_t gVolume = (uint32_t)VOLUME;
tPhasor tP1;
uint16_t buff[AUDIO_OUT_BUFFER_SIZE];

//static AUDIO_OUT_BufferTypeDef  BufferCtl;

float incD;
uint16_t samp = 0x0000;

float freq = TP1_FREQ;
float phase = 0.0f;
float inc = 0.0f;


/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Manages Audio process. 
  * @param  None
  * @retval Audio error
  */
AUDIO_ErrorTypeDef AUDIO_Process(AUDIO_OUT_TransferStateTypeDef state)
{
	int idx, i;
	
  AUDIO_ErrorTypeDef audio_error = AUDIO_ERROR_NONE;
	if (state == TRANSFER_NONE) {
		return AUDIO_ERROR_IO;
	} else if (state == TRANSFER_HALF) {
		idx = 0;
	} else if (state == TRANSFER_COMPLETE) {
		idx = AUDIO_OUT_BUFFER_SIZE/2;
	}


	
	for (i = 0; i < (AUDIO_OUT_BUFFER_TX_SIZE/2); i++) {
		
		
		buff[idx+2*i] =  samp;
		buff[idx+2*i+1] =  samp;	
		
		samp =(uint16_t)(0xFFFF * (tP1).step(&tP1));
		
		/*
		phase += inc;
		if (phase >= 1.0f) phase -= 1.0f;
		
		samp = phase * 0xffff; 
		*/
		
		
	}
  
  return audio_error;
}


/**
  * @brief  Initializes Audio Interface.
  * @param  None
  * @retval Audio error
  */
AUDIO_ErrorTypeDef AUDIO_Init(void)
{
	tPInit(&tP1);
	(tP1).freq(&tP1,TP1_FREQ);
	
	inc = (freq/(float)SAMPLE_RATE)/2.0f;
	incD = inc;
	
	BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_AUTO, VOLUME, I2S_AUDIOFREQ_48K);
	BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);  // PCM 2-channel
	
	return AUDIO_ERROR_NONE;
}



/**
  * @brief  Starts Audio streaming.    
  * @param  idx: 
  * @retval Audio error
  */ 
AUDIO_ErrorTypeDef AUDIO_Start()
{
		int i;

		for (i = 0; i < AUDIO_OUT_BUFFER_SIZE; i++) {
			buff[i] = 0x00; 
		}
		
		BSP_AUDIO_OUT_Play((uint16_t*)&buff[0], AUDIO_OUT_BUFFER_SIZE);
          
		return AUDIO_ERROR_NONE;
}



/**
  * @brief  Stops Audio streaming.
  * @param  None
  * @retval Audio error
  */
AUDIO_ErrorTypeDef AUDIO_PLAYER_Stop(void)
{
  BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
  return AUDIO_ERROR_NONE;
}

/**
  * @brief  Calculates the remaining file size and new position of the pointer.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
  AUDIO_Process(TRANSFER_COMPLETE); 
}

/**
  * @brief  Manages the DMA Half Transfer complete interrupt.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
{ 
	AUDIO_Process(TRANSFER_HALF); 
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
