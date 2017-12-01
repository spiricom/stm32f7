/*
 * MIDI_application.c
 *
 *  Created on: 6 d�c. 2014
 *      Author: Xavier Halgand
 *
 *	Modified on: 9/12/16 by C.P. to handle the MIDI_IDLE state properly, and 
 *	added required code to be compatible with "NucleoSynth"
 *
 *	11/8/17 by C.P.: Version 0.7.7 - Use for Casio CTK-6200 Keyboard
 */

/* Includes ------------------------------------------------------------------*/
#include "audiostream.h"
#include "MIDI_application.h"
#include "usbh_core.h"
#include "usbh_MIDI.h"
#include "usb_host.h"

MIDI_ApplicationTypeDef MIDI_Appli_state = MIDI_APPLICATION_READY;
extern ApplicationTypeDef Appli_state;
extern USBH_HandleTypeDef hUsbHostFS;

uint8_t key, velocity, ctrl, data;

uint8_t paramvalue[256], firstkey, h;

/* Private define ------------------------------------------------------------*/

#define RX_BUFF_SIZE   64  /* Max Received data 64 bytes */

uint8_t MIDI_RX_Buffer[RX_BUFF_SIZE]; // MIDI reception buffer

/* Private function prototypes -----------------------------------------------*/
void ProcessReceivedMidiDatas(void);

/*-----------------------------------------------------------------------------*/
/**
 * @brief  Main routine for MIDI application, looped in main.c
 * @param  None
 * @retval none
 */
void MIDI_Application(void)
{
	if(Appli_state == APPLICATION_READY)
	{
		if(MIDI_Appli_state == MIDI_APPLICATION_READY)
		{
			USBH_MIDI_Receive(&hUsbHostFS, MIDI_RX_Buffer, RX_BUFF_SIZE); // just once at the beginning, start the first reception
			MIDI_Appli_state = MIDI_APPLICATION_RUNNING;
		}
	}
	if(Appli_state == APPLICATION_DISCONNECT)
	{
		MIDI_Appli_state = MIDI_APPLICATION_READY;
		USBH_MIDI_Stop(&hUsbHostFS);
	}
}

#define NUM_BUFFERS 10
int packer = 0;
uint8_t midiData[NUM_BUFFERS][64];
int b;

int16_t numberOfPackets;

/*-----------------------------------------------------------------------------*/
void ProcessReceivedMidiDatas(void)
{
	
	uint8_t *ptr = MIDI_RX_Buffer;
	midi_package_t pack;
	
	for (int i = 0; i < 64; i++)
	{
		midiData[b][i] = ptr[i];
	}
	
	b++; if (b >= NUM_BUFFERS) b= 0;
	
	

	numberOfPackets = USBH_MIDI_GetLastReceivedDataSize(&hUsbHostFS) >> 2; //each USB midi package is 4 bytes long
	
	if (numberOfPackets != 0) 
	{
		while(numberOfPackets-- > 0)
		{
			pack.cin_cable = *ptr ; ptr++ ;
			pack.evnt0 = *ptr ; ptr++ ;
			pack.evnt1 = *ptr ; ptr++ ;
			pack.evnt2 = *ptr ; ptr++ ;
			
			// Handle MIDI messages 
			switch(pack.evnt0)
			{
				case (0x80): // Note Off
					key = pack.evnt1;
					velocity = pack.evnt2;
				
					tMPoly_noteOff(poly, key);
			
					break;
				case (0x90): // Note On
					key = pack.evnt1;
					velocity = pack.evnt2;
				
					if (!velocity)
					{
						tMPoly_noteOff(poly, key);
					}
					else
					{
						tMPoly_noteOn(poly, key, velocity);
    
						for (int i = 0; i < NUM_VOICES; i++)
						{
								tSawtoothSetFreq(osc[i], OOPS_midiToFrequency(poly->voices[i][0]));
						}
					}
				
							
					break;
				case (0xA0):
					break;
				case (0xB0):
					ctrl = pack.evnt1;
					data = pack.evnt2;
					switch(ctrl)
					{
						case (0x01):
							break;
						case (0x0A):
							break;
						case (0x02):
							break;
						case (0x0C):
							break;
						case (0x0D):
							break;
						case (0x4B): 
							break;
						case (0x4C): 
							break;
						case (0x5C):
							break;
						case (0x5F):
							break;
						case (0x49):
							break;
						case (0x48):
							break;
						case (0x5B):
							break;
						case (0x5D): 
							break;
						case (0x4A): 
							break;
						case (0x47): 
							break;
						case (0x05): 
							break;
						case (0x54):
							break;
						case (0x10):
							break;
						case (0x11):
							break;
						case (0x12): 
							break;
						case (0x07): 
							break;
						case (0x13): 
							break;
						case (0x14): 
							break;
						case (64): // sustain
							break;
					}
          break;
				case (0xC0): // Program Change
					break;
				case (0xD0): // Mono Aftertouch
					break;
				case (0xE0): // Pitch Bend
					data = pack.evnt2;
					break;
				case (0xF0):
					break;
			}
		}
	}
}

void LocalMidiHandler(uint8_t param, uint8_t data)
{
	switch(param)
	{
		case (0): // pitch wheel
			break;
		case (1): // modulation wheel
			break;
		case (2): // Tuning
			
			break;
		case (3): // Wave Select
			
			break;
		case (4): // OSC Mix
			
			break;
		case (5): // De-Tune 
			
			break;
		case (6): // Scale
			
			break;
		case (7): // Resonance
			
			break;
		case (8): // Pulse Width Value
			
			break;
		case (9): // PWM Level
			
			break;
		case (10): // VCF Attack
			
			break;
		case (11): // VCF Decay
			
			break;
		case (12): // VCF Sustain
			
			break;
		case (13): // VCF Release
			
			break;
		case (14): // VCA Attack
			
			break;
		case (15): // VCA Decay
			
			break;
		case (16): // VCA Sustain
			
			break;
		case (17): // VCA Release
			
			break;
		case (18): // VCF Follow Level
			
			break;
		case (19): // ENV Follow Level
			
			break;
		case (20): // Velocity Select
			
			break;
		case (21): // VCF Envelope Level
			
			break;
		case (22): // Mod LFO rate
			
			break;
		case (23): // Pwm LFO rate
			
			break;
	}
	paramvalue[param] = data;
}

/*-----------------------------------------------------------------------------*/
/**
 * @brief  MIDI data receive callback.
 * @param  phost: Host handle
 * @retval None
 */
void USBH_MIDI_ReceiveCallback(USBH_HandleTypeDef *phost)
{
	ProcessReceivedMidiDatas();
	USBH_MIDI_Receive(&hUsbHostFS, MIDI_RX_Buffer, RX_BUFF_SIZE); // start a new reception
}

