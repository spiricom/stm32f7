
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WAVEPLAYER_H
#define __WAVEPLAYER_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
AUDIO_ErrorTypeDef AUDIO_Init(void);
AUDIO_ErrorTypeDef AUDIO_Start(void);
AUDIO_ErrorTypeDef AUDIO_Process(AUDIO_OUT_TransferStateTypeDef state);

#endif /* __WAVEPLAYER_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
