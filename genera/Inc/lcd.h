#include "stm32f7xx_hal.h"

#ifndef __LCD_H
#define __LCD_H

#define LCD_I2C_ADDRESS (0x50) //(0x1a << 1) // 7-bit address goes one bit over to the left to make room for R/W bit
//what the hell is this supposed to be? Datasheet says 50 (0x32), but library uses 0x28.  Also may need to be left shifted one bit for stm32

// Min and Max values
#define MIN_BRIGHTNESS		1
#define MAX_BRIGHTNESS		8
#define VALUE_OUT_OF_RANGE	1
#define MIN_CONTRAST		1
#define MAX_CONTRAST		50
#define MAX_STRLEN			40


// Serial LCD module commands
#define ON					0x41
#define OFF					0x42
#define SETCURSOR			0x45
#define HOME				0x46
#define CURSOR_ON			0x47
#define CURSOR_OFF			0x48
#define LEFT				0x49
#define RIGHT				0x4A
#define BLINK_ON			0x4B
#define BLINK_OFF			0x4C
#define BACKSPACE			0x4E
#define CLEAR				0x51
#define SETCONTRAST			0x52
#define SETBACKLIGHTBRIGHTNESS	0x53
#define LOADCUSTOMCHARACTER	0x54
#define SHIFTLEFT			0x55
#define SHIFTRIGHT			0x56
#define CHANGEBAUDRATE		0x61
#define CHANGEI2CADDRESS	0x62
#define DISPLAYVERSION		0x70
#define DISPLAYBAUDRATE		0x71
#define DISPLAYI2CADDRESS	0x72
#define COMMAND				0xFE



void LCD_init(I2C_HandleTypeDef* hi2c);
void LCD_clear(I2C_HandleTypeDef* hi2c);
void LCD_home(I2C_HandleTypeDef* hi2c);
void LCD_setCursor(I2C_HandleTypeDef* hi2c, uint8_t position);
void LCD_sendChar(I2C_HandleTypeDef* hi2c, uint8_t myChar);
void LCD_sendCharArray(I2C_HandleTypeDef* hi2c, uint8_t* myCharArray, uint8_t arrayLength);
void LCD_sendInteger(I2C_HandleTypeDef* hi2c, uint32_t myNumber, uint8_t numDigits);

#endif
