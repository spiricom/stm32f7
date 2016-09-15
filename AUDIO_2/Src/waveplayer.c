/* Includes ------------------------------------------------------------------*/
#include "waveplayer.h"
#include "wavetable.h"

int aBufferSize = 0.0f;
__IO uint32_t gVolume = (uint32_t)VOLUME;
int16_t buff[AUDIO_OUT_BUFFER_SIZE];

float fsamp = 0.0f;

tCycle sin2;
tCycle sin1;
tSawtooth saw1;
tTriangle tri1;
tPulse pulse1;

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
				float mod = sin2.step(&sin2);
				sin1.freq(&sin1,(((mod+1.0f)/2.0f) * 1000.) + 20.f);
				fsamp = sin1.step(&sin1);
		} 
		
		buff[idx+INDEX] =  (int16_t)(fsamp * INT16_MAX);
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
	tPulseInit(&pulse1, SAMPLE_RATE, 0.5f);
	tSawtoothInit(&saw1, SAMPLE_RATE);
	tTriangleInit(&tri1, SAMPLE_RATE);
	tCycleInit(&sin1, SAMPLE_RATE, sinewave, SINE_TABLE_SIZE);
	tCycleInit(&sin2, SAMPLE_RATE, sinewave, SINE_TABLE_SIZE);

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
