
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WAVEPLAYER_H
#define __WAVEPLAYER_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
AUDIO_ErrorTypeDef AUDIO_PLAYER_Init(void);
AUDIO_ErrorTypeDef AUDIO_PLAYER_Start(void);
AUDIO_ErrorTypeDef AUDIO_PLAYER_Process(void);
AUDIO_ErrorTypeDef AUDIO_PLAYER_Stop(void);

#endif /* __WAVEPLAYER_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
