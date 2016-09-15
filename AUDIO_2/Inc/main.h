  /* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stdio.h"
#include "stdint.h"

#include "stm32f7xx_hal.h"
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_audio.h"

#include "audiounits.h"

#include "globals.h"



/* Exported Defines ----------------------------------------------------------*/ 

/* Application State Machine Structure */
typedef enum {
  BUFFER_OFFSET_NONE = 0,  
  BUFFER_OFFSET_HALF,  
  BUFFER_OFFSET_FULL,     
}BUFFER_StateTypeDef;

typedef enum { 
	TRANSFER_NONE = 0,
	TRANSFER_HALF,
	TRANSFER_COMPLETE, 
}AUDIO_OUT_TransferStateTypeDef;
/* Audio buffer control struct */
typedef struct {
  uint8_t buff[AUDIO_OUT_BUFFER_SIZE];
}AUDIO_OUT_BufferTypeDef;

typedef enum {
  AUDIO_ERROR_NONE = 0,  
  AUDIO_ERROR_IO,
  AUDIO_ERROR_EOF,
  AUDIO_ERROR_INVALID_VALUE,     
}AUDIO_ErrorTypeDef;


/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
