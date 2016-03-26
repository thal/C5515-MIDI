#ifndef MIDIUART_H_
#define MIDIUART_H_

typedef struct QueMsg
{
	QUE_Elem elem;
	char msg;
} QueMsg;


void ConfigureUart();
void TSK_Uart();

#endif /* MIDIUART_H_ */
