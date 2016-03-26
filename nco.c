#include <stdint.h>
#include "tskcfg.h"
#include "nco.h"
#include "usbstk5515.h"
#include "usbstk5515_i2c.h"

#define NCO_FS 48000

typedef struct Note
{
	float freq;
	uint32_t delta;
	uint32_t pos;
} Note;

Note notes[10];

short ATT = 0;

const int16_t sinetable[] = { 0, 403, 806, 1208, 1611, 2013, 2415, 2817, 3218,
		3619, 4019, 4419, 4817, 5216, 5613, 6009, 6405, 6800, 7193, 7586, 7977,
		8367, 8756, 9144, 9530, 9915, 10298, 10680, 11060, 11438, 11815, 12190,
		12563, 12934, 13303, 13671, 14036, 14399, 14760, 15118, 15474, 15828,
		16180, 16529, 16876, 17220, 17561, 17900, 18236, 18570, 18900, 19228,
		19553, 19875, 20193, 20509, 20822, 21131, 21438, 21741, 22041, 22337,
		22630, 22920, 23206, 23489, 23768, 24043, 24315, 24584, 24848, 25109,
		25366, 25619, 25868, 26114, 26355, 26593, 26826, 27055, 27281, 27502,
		27719, 27931, 28140, 28344, 28544, 28740, 28931, 29118, 29301, 29479,
		29653, 29822, 29987, 30147, 30303, 30454, 30600, 30742, 30879, 31011,
		31139, 31262, 31381, 31494, 31603, 31707, 31807, 31901, 31991, 32075,
		32155, 32231, 32301, 32366, 32427, 32482, 32533, 32579, 32619, 32655,
		32686, 32712, 32733, 32749, 32760, 32767, 32767, 32764, 32755, 32742,
		32723, 32700, 32671, 32638, 32600, 32556, 32508, 32455, 32397, 32334,
		32266, 32194, 32116, 32034, 31946, 31854, 31757, 31656, 31549, 31438,
		31322, 31201, 31076, 30946, 30811, 30671, 30527, 30379, 30225, 30067,
		29905, 29738, 29567, 29391, 29210, 29025, 28836, 28643, 28445, 28243,
		28036, 27826, 27611, 27392, 27168, 26941, 26710, 26474, 26235, 25991,
		25744, 25493, 25238, 24979, 24716, 24450, 24180, 23906, 23629, 23348,
		23063, 22775, 22484, 22189, 21891, 21590, 21285, 20977, 20666, 20352,
		20034, 19714, 19391, 19064, 18735, 18403, 18069, 17731, 17391, 17048,
		16703, 16355, 16005, 15652, 15297, 14939, 14579, 14218, 13853, 13487,
		13119, 12749, 12377, 12003, 11627, 11249, 10870, 10489, 10107, 9723,
		9337, 8950, 8562, 8172, 7782, 7390, 6997, 6603, 6207, 5811, 5414, 5017,
		4618, 4219, 3819, 3419, 3018, 2616, 2214, 1812, 1410, 1007, 604, 201,
		-201, -604, -1007, -1410, -1812, -2214, -2616, -3018, -3419, -3819,
		-4219, -4618, -5017, -5414, -5811, -6207, -6603, -6997, -7390, -7782,
		-8172, -8562, -8950, -9337, -9723, -10107, -10489, -10870, -11249,
		-11627, -12003, -12377, -12749, -13119, -13487, -13853, -14218, -14579,
		-14939, -15297, -15652, -16005, -16355, -16703, -17048, -17391, -17731,
		-18069, -18403, -18735, -19064, -19391, -19714, -20034, -20352, -20666,
		-20977, -21285, -21590, -21891, -22189, -22484, -22775, -23063, -23348,
		-23629, -23906, -24180, -24450, -24716, -24979, -25238, -25493, -25744,
		-25991, -26235, -26474, -26710, -26941, -27168, -27392, -27611, -27826,
		-28036, -28243, -28445, -28643, -28836, -29025, -29210, -29391, -29567,
		-29738, -29905, -30067, -30225, -30379, -30527, -30671, -30811, -30946,
		-31076, -31201, -31322, -31438, -31549, -31656, -31757, -31854, -31946,
		-32034, -32116, -32194, -32266, -32334, -32397, -32455, -32508, -32556,
		-32600, -32638, -32671, -32700, -32723, -32742, -32755, -32764, -32768,
		-32767, -32760, -32749, -32733, -32712, -32686, -32655, -32619, -32579,
		-32533, -32482, -32427, -32366, -32301, -32231, -32155, -32075, -31991,
		-31901, -31807, -31707, -31603, -31494, -31381, -31262, -31139, -31011,
		-30879, -30742, -30600, -30454, -30303, -30147, -29987, -29822, -29653,
		-29479, -29301, -29118, -28931, -28740, -28544, -28344, -28140, -27931,
		-27719, -27502, -27281, -27055, -26826, -26593, -26355, -26114, -25868,
		-25619, -25366, -25109, -24848, -24584, -24315, -24043, -23768, -23489,
		-23206, -22920, -22630, -22337, -22041, -21741, -21438, -21131, -20822,
		-20509, -20193, -19875, -19553, -19228, -18900, -18570, -18236, -17900,
		-17561, -17220, -16876, -16529, -16180, -15828, -15474, -15118, -14760,
		-14399, -14036, -13671, -13303, -12934, -12563, -12190, -11815, -11438,
		-11060, -10680, -10298, -9915, -9530, -9144, -8756, -8367, -7977, -7586,
		-7193, -6800, -6405, -6009, -5613, -5216, -4817, -4419, -4019, -3619,
		-3218, -2817, -2415, -2013, -1611, -1208, -806, -403, 0 };

/*
 * NCO_play( unsigned int tone )
 *
 * Play one period of a sinusoid with frequency specified
 *
 * Arg: tone = the desired ouput frequency
 * Arg: seconds = duration of frequency output
 *
 */
void NCO_play(unsigned int tone, unsigned int seconds) {
	uint16_t i,j;

	uint32_t pa = 0;
	uint32_t delta = ((float)0xffffffff / NCO_FS) * tone;

	uint16_t index;
	/* Play Tone */
	for(i = 0; i < seconds; i++) {
		for (j = 0; j < 48000; j++) {
			while((Xmit & I2S0_IR) == 0); // Wait for transmit interrupt to be pending
			index = pa >> (32 - 9);
			I2S0_W0_MSW_W = (sinetable[index]) >> ATT; // 16 bit left channel transmit audio data
			I2S0_W1_MSW_W = (sinetable[index]) >> ATT;// 16 bit right channel transmit audio data

			pa += delta;
		}
	}

}

/*
 * NCO_sweep( unsigned int start, unsigned int end )
 *
 * Play a sweep signal across a frequency range
 *
 * Arg: start = the beginning of the frequency range
 * Arg: end = the end of the frequency range
 *
 */
void NCO_sweep(unsigned int start, unsigned int end)
{

	uint32_t pa = 0;
	uint32_t delta = ((float)0xffffffff / NCO_FS) * start;

	uint16_t index;
	// Sweep by changing the delta every few samples
	int16_t i;

	while(start < end) {
		// Note: change the limit of i here to change the duration of the sweep
		// Even better would be to have a 'seconds' argument for the function
		for (i = 0; i < 18; i++) {
			while((Xmit & I2S0_IR) == 0); // Wait for transmit interrupt to be pending
			index = pa >> (32 - 9);
			I2S0_W0_MSW_W = (sinetable[index]) >> ATT; // 16 bit left channel transmit audio data
			I2S0_W1_MSW_W = (sinetable[index]) >> ATT;// 16 bit right channel transmit audio data

			pa += delta;
		}
		start++;
		delta = ((float)0xffffffff / NCO_FS) * start;
	}
}

/*
 * NCO_setAtt()
 *
 * Set the global attenuation value
 */

void NCO_setAtt(unsigned int att)
{
	ATT = att;
}



void NCO_startNote(float freq)
{
	int i;
	for(i = 0; i < 10; i++)
	{
		if(notes[i].freq == 0)
		{
			Uint32 delta = ((float)0xffffffff / NCO_FS) * freq;
			notes[i].freq = freq;
			notes[i].delta = delta;
			notes[i].pos = 0;
			break;
		}
	}
}

void NCO_stopNote(float freq)
{
	int i;
	for(i = 0; i < 10; i++)
	{
		if(notes[i].freq == freq)
		{
			notes[i].freq = 0;
			notes[i].delta = 0;
			notes[i].pos = 0;
		}
	}
}

void TSK_Osc()
{
	int i;
	for(i = 0; i < 10; i++)
	{
		Note * note = &(notes[i]);
		note->freq = 0.0;
		note->delta = 0.0;
		note->pos = 0;

	}
	while(1)
	{
		int16_t sample = 0;
		int j;
		for(j = 0; j < 10; j++)
		{
			if(notes[j].freq > 0)
			{
				Note * thisNote = &(notes[j]);
				unsigned index = (thisNote->pos) >> (32 - 9);
				sample += sinetable[index] >> ATT;
				thisNote->pos += thisNote->delta;
			}
		}
		while((Xmit & I2S0_IR) == 0)
		{
			TSK_yield();// Wait for transmit interrupt to be pending
		}
		TSK_disable();
		I2S0_W0_MSW_W = sample; // 16 bit left channel transmit audio data
		I2S0_W1_MSW_W = sample;	// 16 bit right channel transmit audio data
		TSK_enable();
	}

}
