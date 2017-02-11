/*
  ==============================================================================

    OOPSInstrument.h
    Created: 20 Jan 2017 12:01:54pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#ifndef OOPSINSTRUMENT_H_INCLUDED
#define OOPSINSTRUMENT_H_INCLUDED

#include "OOPSMath.h"
#include "OOPSCore.h"

/* tPluck */
tPluck*     tPluckInit          (float lowestFrequency, float delayBuff[DELAY_LENGTH]);
float       tPluckTick          (tPluck*  const);
void        tPluckPluck         (tPluck*  const, float amplitude);
void        tPluckNoteOn        (tPluck*  const, float frequency, float amplitude ); // Start a note with the given frequency and amplitude.;
void        tPluckNoteOff       (tPluck*  const, float amplitude ); // Stop a note with the given amplitude (speed of decay).
void        tPluckSetFrequency  (tPluck*  const, float frequency ); // Set instrument parameters for a particular frequency.
void        tPluckControlChange (tPluck*  const, int number, float value); // Perform the control change specified by \e number and \e value (0.0 - 128.0).
float       tPluckGetLastOut    (tPluck*  const);

/* tStifKarp */
typedef enum SKControlType
{
    SKPickPosition = 0,
    SKStringDamping,
    SKDetune,
    SKControlTypeNil
} SKControlType;

tStifKarp*  tStifKarpInit               (float lowestFrequency, float delayBuff[2][DELAY_LENGTH]);
float       tStifKarpTick               (tStifKarp*  const);
void        tStifKarpPluck              (tStifKarp*  const, float amplitude);
void        tStifKarpNoteOn             (tStifKarp*  const, float frequency, float amplitude ); // Start a note with the given frequency and amplitude.;
void        tStifKarpNoteOff            (tStifKarp*  const, float amplitude ); // Stop a note with the given amplitude (speed of decay).
void        tStifKarpSetFrequency       (tStifKarp*  const, float frequency ); // Set instrument parameters for a particular frequency.
void        tStifKarpControlChange      (tStifKarp*  const, SKControlType type, float value); // Perform the control change specified by \e number and \e value (0.0 - 128.0).
void        tStifKarpSetStretch         (tStifKarp*  const, float stretch );//! Set the stretch "factor" of the string (0.0 - 1.0).
void        tStifKarpSetPickupPosition  (tStifKarp*  const, float position ); //! Set the pluck or "excitation" position along the string (0.0 - 1.0).
void        tStifKarpSetBaseLoopGain    (tStifKarp*  const, float aGain ); //! Set the base loop gain.
float       tStifKarpGetLastOut         (tStifKarp*  const);


#endif  // OOPSINSTRUMENT_H_INCLUDED
