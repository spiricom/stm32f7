#include "codec.h"

uint16_t i2cDataSize = 2;
uint8_t myI2cData[2] = {0,0};
uint32_t I2Ctimeout = 2000;

void AudioCodec_init(I2C_HandleTypeDef* hi2c) {

	// compared to the datasheet for the WM8731 codec, all of the register numbers are left-shifted by one (to take into account the read/write bit in I2C)
	
	//now to send all the necessary messages to the codec (pack them in myI2cData)
	
	//reset the DAC
	myI2cData[0] = 0x1e;
  myI2cData[1] = 0x00;
	HAL_I2C_Master_Transmit(hi2c, CODEC_I2C_ADDRESS, myI2cData, i2cDataSize, I2Ctimeout);
	
	//power reduction register - turn power on but don't turn off OutputPD bit yet (also currently leaving microphone bias and oscillator/clkout disabled)
	//if you want to use the microphone input, this will have to be set to 0x70 instead of 0x72
	myI2cData[0] = 0x0c;
  myI2cData[1] = 0xE1;
	HAL_I2C_Master_Transmit(hi2c, CODEC_I2C_ADDRESS, myI2cData, i2cDataSize, I2Ctimeout);
 
	//digital data format - I2S 
	myI2cData[0] = 0x0e;
  myI2cData[1] = 0x02;
	HAL_I2C_Master_Transmit(hi2c, CODEC_I2C_ADDRESS, myI2cData, i2cDataSize, I2Ctimeout);

  //left in setup register
	myI2cData[0] = 0x00;
  myI2cData[1] = LINVOL;
	HAL_I2C_Master_Transmit(hi2c, CODEC_I2C_ADDRESS, myI2cData, i2cDataSize, I2Ctimeout);
	
  //right in setup register
	myI2cData[0] = 0x02;
  myI2cData[1] = RINVOL;
	HAL_I2C_Master_Transmit(hi2c, CODEC_I2C_ADDRESS, myI2cData, i2cDataSize, I2Ctimeout);
 
  //left headphone out register
	myI2cData[0] = 0x04;
  //myI2cData[1] = LHPVOL;
	myI2cData[1] = 0; // not using headphone outs on genera (line outs only)
	HAL_I2C_Master_Transmit(hi2c, CODEC_I2C_ADDRESS, myI2cData, i2cDataSize, I2Ctimeout);
 
  //right headphone out register
	myI2cData[0] = 0x06;
	myI2cData[1] = 0; // not using headphone outs on genera (line outs only)
  //myI2cData[1] = RHPVOL;
	HAL_I2C_Master_Transmit(hi2c, CODEC_I2C_ADDRESS, myI2cData, i2cDataSize, I2Ctimeout);
 
  //digital audio path configuration
	myI2cData[0] = 0x0a;
  myI2cData[1] = ADCHPD;
	HAL_I2C_Master_Transmit(hi2c, CODEC_I2C_ADDRESS, myI2cData, i2cDataSize, I2Ctimeout);
 
  //analog audio path configuration
	myI2cData[0] = 0x08;
  myI2cData[1] = ((uint8_t)((SIDEATT << 6)|(SIDETONE << 5)|(DACSEL << 4)|(BYPASS << 3)|(INSEL << 2)|(MUTEMIC << 1)|(MICBOOST << 0)));
	HAL_I2C_Master_Transmit(hi2c, CODEC_I2C_ADDRESS, myI2cData, i2cDataSize, I2Ctimeout);
	
	//clock configuration
	myI2cData[0] = 0x10;
  myI2cData[1] = 0x00; 
	HAL_I2C_Master_Transmit(hi2c, CODEC_I2C_ADDRESS, myI2cData, i2cDataSize, I2Ctimeout);
 
	//codec enable
	myI2cData[0] = 0x12;
  myI2cData[1] = 0x01; 
	HAL_I2C_Master_Transmit(hi2c, CODEC_I2C_ADDRESS, myI2cData, i2cDataSize, I2Ctimeout);


//power reduction register - now turn off OutputPD bit (also currently leaving microphone bias and oscillator/clkout disabled)
	//if you want to use the microphone input, this will have to be set to 0x60 instead of 0x62
	myI2cData[0] = 0x0c;
  myI2cData[1] = 0x61;
	
	HAL_I2C_Master_Transmit(hi2c, CODEC_I2C_ADDRESS, myI2cData, i2cDataSize, I2Ctimeout);
}
