#include "LCD.h"
#include "math.h"

uint16_t lcd_i2cDataSize = 3;
uint8_t lcd_I2cData[32];
uint32_t lcd_I2Ctimeout = 2000;

uint8_t i2cCount = 0;

void LCD_init(I2C_HandleTypeDef* hi2c) {
	
		//reset LCD
	lcd_I2cData[0] = 0xFE;
  lcd_I2cData[1] = 0x41;
	HAL_I2C_Master_Transmit(hi2c, LCD_I2C_ADDRESS, lcd_I2cData, 2, lcd_I2Ctimeout);
	
	//reset LCD
	lcd_I2cData[0] = 0xFE;
  lcd_I2cData[1] = 0x51;
	HAL_I2C_Master_Transmit(hi2c, LCD_I2C_ADDRESS, lcd_I2cData, 2, lcd_I2Ctimeout);
	
			//reset LCD
	lcd_I2cData[0] = 0xFE;
  lcd_I2cData[1] = 0x52;
	lcd_I2cData[2] = 40;
	HAL_I2C_Master_Transmit(hi2c, LCD_I2C_ADDRESS, lcd_I2cData, 3, lcd_I2Ctimeout);
	
		lcd_I2cData[0] = 0xFE;
  lcd_I2cData[1] = 0x53;
	lcd_I2cData[2] = 8;
	HAL_I2C_Master_Transmit(hi2c, LCD_I2C_ADDRESS, lcd_I2cData, 3, lcd_I2Ctimeout);
	
	
}

void LCD_clear(I2C_HandleTypeDef* hi2c) {
	lcd_I2cData[0] = 0xFE;
  lcd_I2cData[1] = CLEAR;
	HAL_I2C_Master_Transmit(hi2c, LCD_I2C_ADDRESS, lcd_I2cData, 2, lcd_I2Ctimeout);
	
}

void LCD_home(I2C_HandleTypeDef* hi2c) {
	lcd_I2cData[0] = 0xFE;
  lcd_I2cData[1] = HOME;
	HAL_I2C_Master_Transmit(hi2c, LCD_I2C_ADDRESS, lcd_I2cData, 2, lcd_I2Ctimeout);
	lcd_I2cData[0] = 0xFE;
  lcd_I2cData[1] = BLINK_OFF;
	HAL_I2C_Master_Transmit(hi2c, LCD_I2C_ADDRESS, lcd_I2cData, 2, lcd_I2Ctimeout);
	
}

void LCD_setCursor(I2C_HandleTypeDef* hi2c, uint8_t position) {
	lcd_I2cData[0] = 0xFE;
  lcd_I2cData[1] = SETCURSOR;
	lcd_I2cData[2] = position;
	HAL_I2C_Master_Transmit(hi2c, LCD_I2C_ADDRESS, lcd_I2cData, 3, lcd_I2Ctimeout);
	
}

void LCD_sendChar(I2C_HandleTypeDef* hi2c, uint8_t myChar) {
  lcd_I2cData[0] = myChar;
	HAL_I2C_Master_Transmit(hi2c, LCD_I2C_ADDRESS, lcd_I2cData, 1, lcd_I2Ctimeout);
	
}

void LCD_sendCharArray(I2C_HandleTypeDef* hi2c, uint8_t* myCharArray, uint8_t arrayLength) {
	
	HAL_I2C_Master_Transmit(hi2c, LCD_I2C_ADDRESS, myCharArray, arrayLength, lcd_I2Ctimeout);
}

void LCD_sendInteger(I2C_HandleTypeDef* hi2c, uint32_t myNumber, uint8_t numDigits)
{
	
	for (int i = 0; i < numDigits; i++)
	{
		int whichPlace = (uint32_t)(powf(10.0f,(numDigits - 1) - i));
		int thisDigit = (myNumber / whichPlace);
		lcd_I2cData[i] = thisDigit + 48;
		myNumber -= thisDigit * whichPlace;
	}
	
	HAL_I2C_Master_Transmit(hi2c, LCD_I2C_ADDRESS, lcd_I2cData, numDigits, lcd_I2Ctimeout);
}
