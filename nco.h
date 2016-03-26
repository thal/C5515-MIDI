#ifndef NCO_H_
#define NCO_H_

#include <stdint.h>

#define AIC3204_I2C_ADDR 0x18
#define Xmit 0x20


void NCO_setAtt(unsigned int att);

void NCO_play(unsigned int tone, unsigned int seconds);

void NCO_sweep(unsigned int start, unsigned int end);

void NCO_startNote(float freq);

void NCO_stopNote(float freq);

#endif /* NCO_H_ */
