#include <std.h>
#include <c55.h>
#include <string.h>

#include "tskcfg.h"

#include "midi.h"
#include "nco.h"
#include "uart.h"
#include "i2s.h"
#include "aic3204.h"

void main()
{
    // Entry point.  After exit DSP BIOS scheduler starts

    // Configure the AIC3204 CODEC chip
    ConfigureAic3204();

    // Configure I2S0 port for transmit
    EnableI2sPort();

    // Set attenuation of the NCO
    NCO_setAtt(5);

}

void TSK_Midi()
{
	// Position in message
	int messagePos = 0;
	// Active command
	char activeCmd = 0x00;

	while(1)
	{
		if(!QUE_empty(&QUE_cmd))
		{
			QueMsg * cmdPtr = (QueMsg *)QUE_get(&QUE_cmd);
			char cmd = cmdPtr->msg;

			if(cmd == 0x90)			// Note On
			{
				messagePos = 1;
				activeCmd = 0x90;
			}
			else if(cmd == 0x80)	// Note Off
			{
				// Turn off an active note
				messagePos = 1;
				activeCmd = 0x80;
			}
			else
			{
				// This is a data byte
				if(activeCmd == 0x90)
				{
					if(messagePos == 1)
					{
						// Note value
						NCO_startNote(MIDI_freq[cmd - MIDI_START]);
						messagePos = 2;
					}
					else if(messagePos == 2)
					{
						// Velocity
						// Velocity not supported at this point
						messagePos = 0;
						activeCmd = 0x00;
					}
				}
				else if(activeCmd == 0x80)
				{
					if(messagePos == 1)
					{
						// Note value
						NCO_stopNote(MIDI_freq[cmd - MIDI_START]);
						messagePos = 2;
					}
					else if(messagePos == 2)
					{
						// Velocity
						// Velocity not supported at this point
						messagePos = 0;
						activeCmd = 0x00;
					}
				}
			}
		}
		TSK_yield();
	}
}

