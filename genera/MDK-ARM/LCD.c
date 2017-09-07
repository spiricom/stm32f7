#include "LCD.h"

uint16_t lcd_i2cDataSize = 2;
uint8_t lcd_I2cData[2] = {0,0};
uint32_t lcd_I2Ctimeout = 2000;

void LCD_init(I2C_HandleTypeDef* hi2c) {
	
		//reset LCD
	lcd_I2cData[0] = 0x1e;
  lcd_I2cData[1] = 0x00;
	HAL_I2C_Master_Transmit(hi2c, LCD_I2C_ADDRESS, lcd_I2cData, lcd_i2cDataSize, lcd_I2Ctimeout);
}

