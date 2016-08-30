/* Includes ------------------------------------------------------------------*/
#include "waveplayer.h"
#include "wavetable.h"
#include "phasor.h"

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static AUDIO_OUT_BufferTypeDef  BufferCtl;
static __IO uint32_t uwVolume = 10;

tPhasor tP1;
float pA,pB;
uint16_t buff[AUDIO_OUT_BUFFER_SIZE];
uint16_t samp;



/* Private function prototypes -----------------------------------------------*/
static uint8_t PlayerInit(uint32_t AudioFreq);


/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes Audio Interface.
  * @param  None
  * @retval Audio error
  */
AUDIO_ErrorTypeDef AUDIO_PLAYER_Init(void)
{
	tPInit(&tP1);
	(tP1).freq(&tP1,55.);
	
	
  if(BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_AUTO, uwVolume, I2S_AUDIOFREQ_48K) == 0)
  {
    return AUDIO_ERROR_NONE;
  }
  else
  {
    return AUDIO_ERROR_IO;
  }
}

/**
  * @brief  Starts Audio streaming.    
  * @param  idx: 
  * @retval Audio error
  */ 
AUDIO_ErrorTypeDef AUDIO_PLAYER_Start()
{
		int i;
    /*Adjust the Audio frequency */
    PlayerInit(AUDIO_FREQ); 

		for (i = 0; i < AUDIO_OUT_BUFFER_SIZE; i++) {
			buff[i] = 0x00; 
		}

    BSP_AUDIO_OUT_Play((uint16_t*)&buff[0], AUDIO_OUT_BUFFER_SIZE);
          
		return AUDIO_ERROR_NONE;
}

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
		idx = AUDIO_OUT_BUFFER_SIZE/2;
	} else if (state == TRANSFER_COMPLETE) {
		idx = 0;
	}

	//uint8_t samp = 0x00;
	//for debugging
	pA = tP1.phase;
	pB = tP1.inc;
	
	for (i = 0; i < AUDIO_OUT_BUFFER_TX_SIZE/2; i++) {
		
		samp = (tP1).step(&tP1);
		//samp = (tP1).samp(&tP1);
		//samp = (uint8_t)(((float)i/(AUDIO_OUT_TRANSFER_SIZE/2)) * 0xFF); 
		
		buff[idx+2*i] = samp;
		buff[idx+2*i+1] = samp;	
	}
  
  return audio_error;
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
/*******************************************************************************
                            Static Functions
*******************************************************************************/

/**
  * @brief  Initializes the Wave player.
  * @param  AudioFreq: Audio sampling frequency
  * @retval None
  */
static uint8_t PlayerInit(uint32_t AudioFreq)
{ 
  /* Initialize the Audio codec and all related peripherals (I2S, I2C, IOExpander, IOs...) */  
  if(BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_BOTH, uwVolume, AudioFreq) != 0)
  {
    return 1;
  }
  else
  {
    BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
    return 0;
  } 
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
