#include "stm32f7xx_hal.h"

#ifndef __LCD_H
#define __LCD_H

#define LCD_I2C_ADDRESS (0x1a << 1) // 7-bit address goes one bit over to the left to make room for R/W bit

void LCD_init(I2C_HandleTypeDef* hi2c);


#endif