/* Includes ------------------------------------------------------------------*/
#include "waveplayer.h"
#include "wavetable.h"

int aBufferSize = 0.0f;
__IO uint32_t gVolume = (uint32_t)VOLUME;
int16_t buff[AUDIO_OUT_BUFFER_SIZE];

int16_t samp = 0x0000;

tCycle tC1;
tSawtooth tS1;
tTriangle tT1;
tPulse tP1;

int INDEX = 0;
int idx = 0;

int16_t testWave = 0;

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Manages Audio process. 
  * @param  None
  * @retval Audio error
  */
AUDIO_ErrorTypeDef AUDIO_Process(AUDIO_OUT_TransferStateTypeDef state)
{
  AUDIO_ErrorTypeDef audio_error = AUDIO_ERROR_NONE;
	if (state == TRANSFER_NONE) {
		return AUDIO_ERROR_IO;
	} else if (state == TRANSFER_HALF) {
		idx = 0;
	} else if (state == TRANSFER_COMPLETE) {
		idx = AUDIO_OUT_BUFFER_SIZE/2;	
	}

	for (INDEX = 0; INDEX < (AUDIO_OUT_BUFFER_SIZE/2); INDEX++) {
		if ((INDEX & 1) == 0) {
			//samp = (tP1).step(&tP1);
			//samp = (tT1).step(&tT1);
			//samp = (tS1).step(&tS1);
			samp = (tC1).step(&tC1);
		} 
		buff[idx+INDEX] =  samp;

		//samp = (uint16_t)(sinewave[(int)(WAVETABLE2_SIZE * phase)] + 32768.0);
		//samp = (uint16_t) (phase * 65535.0f);
			
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
	tPulseInit(&tP1, SAMPLE_RATE, 0.5f);
	tSawtoothInit(&tS1, SAMPLE_RATE);
	tTriangleInit(&tT1, SAMPLE_RATE);
	tCycleInit(&tC1, SAMPLE_RATE, sinewave, WAVETABLE2_SIZE);

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
		
		BSP_AUDIO_OUT_Play((uint16_t*)&buff[0], AUDIO_OUT_BUFFER_SIZE*2);
          
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

void BSP_AUDIO_OUT_Error_CallBack(void) {
	testWave++;
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
