/*
 * MIDI_application.h
 *  Created on: 6 déc. 2014
 *      Author: Xavier Halgand
 *
 *	Modified on: 9/12/16 by C.P. to handle the MIDI_IDLE state properly, and 
 *	added required code to be compatible with "NucleoSynth"
 *
 */

#ifndef MIDI_APPLICATION_H_
#define MIDI_APPLICATION_H_

/* Includes ------------------------------------------------------------------*/

#include "stdio.h"
#include "usbh_core.h"
#include "usbh_MIDI.h"

#include <math.h>
#include <stdint.h>
#include <stdbool.h>

/*------------------------------------------------------------------------------*/
typedef enum
{
	MIDI_APPLICATION_IDLE = 0,
	MIDI_APPLICATION_START,
	MIDI_APPLICATION_READY,
	MIDI_APPLICATION_RUNNING,
	MIDI_APPLICATION_DISCONNECT
}
MIDI_ApplicationTypeDef;


/* Exported functions ------------------------------------------------------- */
void MIDI_Application(void);
void LocalMidiHandler(uint8_t param, uint8_t data);

/*------------------------------------------------------------------------------*/
#endif /* MIDI_APPLICATION_H_ */
