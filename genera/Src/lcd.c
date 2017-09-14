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

void LCD_writeInteger(uint8_t* buffer, uint32_t myNumber, uint8_t numDigits)
{
	for (int i = 0; i < numDigits; i++)
	{
		int whichPlace = (uint32_t)(powf(10.0f,(numDigits - 1) - i));
		int thisDigit = (myNumber / whichPlace);
		buffer[i] = thisDigit + 48;
		myNumber -= thisDigit * whichPlace;
	}
}

uint8_t pitches[24] = 
{	
	'C', ' ',
	'C', '#',
	'D', ' ',
	'D', '#',
	'E', ' ',
	'F', ' ',
	'F', '#',
	'G', ' ',
	'G', '#',
	'A', ' ',
	'A', '#',
	'B', ' '
};
	

void LCD_sendPitch(I2C_HandleTypeDef* hi2c, float midi)
{
	int pclass, octave, note, neg = 0; float offset;
	
	note = (int)midi;
	offset = midi - note;

	if ((midi + 0.5f) > (note+1))
	{
		note += 1;
		offset = (1.0f - offset) + 0.01;
		neg = 1;
	}

	pclass = (note % 12);
	octave = (int)(note / 12) - 1;
	
	int idx = 0;

	lcd_I2cData[idx++] = pitches[pclass*2];
	lcd_I2cData[idx++] = pitches[pclass*2+1];
	
	LCD_writeInteger(&lcd_I2cData[idx++], octave, 1);
	
	lcd_I2cData[idx++] = ' ';
	
	if (neg == 1) 	
		lcd_I2cData[idx++] = '-';
	else								
		lcd_I2cData[idx++] = '+';
	
	LCD_writeInteger(&lcd_I2cData[idx], (uint32_t) (offset * 100.0f), 2);

	HAL_I2C_Master_Transmit(hi2c, LCD_I2C_ADDRESS, lcd_I2cData, idx+2, lcd_I2Ctimeout);
}

void LCD_sendFixedFloat(I2C_HandleTypeDef* hi2c, float input, uint8_t numDigits, uint8_t numDecimal)
{
	int nonzeroHasHappened = 0, decimalHasHappened = 0;
	
	uint32_t myNumber = (uint32_t)(input * powf(10.0f, numDecimal));
	
	int idx = 0, i = 0;
	
	while (i < numDigits)
	{
		if ((decimalHasHappened == 0) && ((numDigits-i) == numDecimal))
		{
			if (nonzeroHasHappened == 0)
			{
				lcd_I2cData[idx-1] = '0';
				nonzeroHasHappened = 1;
			}
			
			lcd_I2cData[idx++] = '.';
			decimalHasHappened = 1;
		}
		else
		{

			int whichPlace = (uint32_t) powf(10.0f,(numDigits - 1 - i));
			int thisDigit = (myNumber / whichPlace);
			
			if (nonzeroHasHappened == 0)
			{
				if (thisDigit > 0) 
				{
					lcd_I2cData[idx++] = thisDigit + 48;
					nonzeroHasHappened = 1;
				}
				else
				{
					lcd_I2cData[idx++] = ' ';
				}
			}
			else
			{
				lcd_I2cData[idx++] = thisDigit + 48;
			}
			
			myNumber -= thisDigit * whichPlace;
			
			i++;
		}
	}
	
	HAL_I2C_Master_Transmit(hi2c, LCD_I2C_ADDRESS, lcd_I2cData, idx, lcd_I2Ctimeout);
}

void LCD_sendFloatyFloat(I2C_HandleTypeDef* hi2c, float myNumber, uint8_t numDigits)
{
	float temp = myNumber;
	int count = 0;
	
	while (temp > 1.0f)
	{
		temp /= 10.0f;
		count++;
	}
	
	float scale = powf(10.0f, (numDigits - count));
	
	int myNumberScaled = (int)(scale * myNumber);
	
	int i = 0;
	int idx = 0;
	while (i < numDigits)
	{
		if (i == count)
		{
			lcd_I2cData[idx++] = '.';
		}
		else
		{
			int whichPlace = (uint32_t)(powf(10.0f,(numDigits - 1) - i));
			int thisDigit = (myNumberScaled / whichPlace);
			lcd_I2cData[idx++] = thisDigit + 48;
			myNumberScaled -= thisDigit * whichPlace;
			i++;
		}
	}
	
	HAL_I2C_Master_Transmit(hi2c, LCD_I2C_ADDRESS, lcd_I2cData, numDigits, lcd_I2Ctimeout);
}
