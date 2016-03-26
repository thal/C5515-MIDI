/* ============================================================================
 * Copyright (c) 2008-2012 Texas Instruments Incorporated.
 * Except for those rights granted to you in your license from TI, all rights
 * reserved.
 *
 * Software License Agreement
 * Texas Instruments (TI) is supplying this software for use solely and
 * exclusively on TI devices. The software is owned by TI and/or its suppliers,
 * and is protected under applicable patent and copyright laws.  You may not
 * combine this software with any open-source software if such combination would
 * cause this software to become subject to any of the license terms applicable
 * to such open source software.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
 * NO WARRANTIES APPLY TO THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY.
 * EXAMPLES OF EXCLUDED WARRANTIES ARE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE AND WARRANTIES OF NON-INFRINGEMENT,
 * BUT ALL OTHER WARRANTY EXCLUSIONS ALSO APPLY. FURTHERMORE, TI SHALL NOT,
 * UNDER ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, CONSEQUENTIAL
 * OR PUNITIVE DAMAGES, FOR ANY REASON WHATSOEVER.
 * ============================================================================
 */

#include <std.h>
#include <c55.h>
#include <string.h>
#include <que.h>
#include <tskcfg.h>
#include "csl_uart.h"
#include "csl_uartAux.h"
#include "csl_intc.h"
#include "csl_general.h"
#include "csl_sysctrl.h"

#include "uart.h"


#define CSL_TEST_FAILED         (1U)
#define CSL_TEST_PASSED         (0)
#define	CSL_UART_BUF_LEN    	(4U)

/* Global constants */
/* String length to be received and transmitted */
#define WR_STR_LEN        80
#define RD_STR_LEN        10

#define CSL_PLL_DIV_000    (0)
#define CSL_PLL_DIV_001    (1u)
#define CSL_PLL_DIV_002    (2u)
#define CSL_PLL_DIV_003    (3u)
#define CSL_PLL_DIV_004    (4u)
#define CSL_PLL_DIV_005    (5u)
#define CSL_PLL_DIV_006    (6u)
#define CSL_PLL_DIV_007    (7u)

#define CSL_PLL_CLOCKIN    (32768u)

#define PLL_CNTL1        *(ioport volatile unsigned *)0x1C20
#define PLL_CNTL2        *(ioport volatile unsigned *)0x1C21
#define PLL_CNTL3        *(ioport volatile unsigned *)0x1C22
#define PLL_CNTL4        *(ioport volatile unsigned *)0x1C23

/* Global data definition */
/* UART setup structure */
CSL_UartSetup mySetup =
{
	/* Input clock freq in MHz */
    100000000,
	/* Baud rate */
    31250,
	/* Word length of 8 */
    CSL_UART_WORD8,
	/* To generate 1 stop bit */
    0,
	/* Disable the parity */
    CSL_UART_DISABLE_PARITY,
	/* Enable trigger 14 fifo */
	CSL_UART_FIFO_DMA1_DISABLE_TRIG14,
	/* Loop Back enable */
	CSL_UART_NO_LOOPBACK,
	/* No auto flow control*/
	CSL_UART_NO_AFE ,
	/* No RTS */
	CSL_UART_NO_RTS ,
};

/* CSL UART data structures */
CSL_UartObj uartObj;
CSL_UartHandle hUart;


/* Interrupt vector start address */
extern void VECSTART(void);

/**
 *  \brief  Function to calculate the system clock
 *
 *  \param    none
 *
 *  \return   System clock value in Hz
 */
Uint32 getSysClk(void);

/**
 *  \brief  Interrupt Service Routine to handle UART Character Timeout Interrupt
 *
 *  \param  none
 *
 *  \return none
 */

QueMsg cmdBuf[100];
unsigned cmdBufIdx = 0;
void uart_ctoIsr(void)
{
	UART_read(hUart, &(cmdBuf[cmdBufIdx].msg), 0, 0);
	QUE_put(&QUE_cmd, &(cmdBuf[cmdBufIdx]));
	cmdBufIdx++;
	if(cmdBufIdx == 100) cmdBufIdx = 0;
}

/**
 *  \brief  Interrupt Service Routine to handle UART Receive Interrupt
 *
 *  \param  none
 *
 *  \return none
 */
void uart_rxIsr(void)
{
	UART_read(hUart, &(cmdBuf[cmdBufIdx].msg), 0, 0);
	cmdBufIdx++;
	QUE_put(&QUE_cmd, &(cmdBuf[cmdBufIdx]));
	if(cmdBufIdx == 100) cmdBufIdx = 0;
}


/**
 *  \brief  Interrupt Dispatcher to identify interrupt source
 *
 *  This function identify the type of UART interrupt generated and
 *  calls concerned ISR to handle the interrupt
 *
 *  \param  none
 *
 *  \return none
 */
interrupt void UART_intrDispatch(void)
{
	Uint16 eventId = 0;

	IRQ_disable(UART_EVENT);

	/* Get the event Id which caused interrupt */
	eventId = UART_getEventId(hUart);

	if (((void (*)(void))(hUart->UART_isrDispatchTable[eventId])))
	{
		((void (*)(void))(hUart->UART_isrDispatchTable[eventId]))();
	}

	IRQ_enable(UART_EVENT);

	return;
}

void ConfigureUart()
{

	CSL_UartIsrAddr    isrAddr;
	CSL_Status         status;
	Uint32            sysClk;

	sysClk = getSysClk();

	mySetup.clkInput = sysClk;

	status = SYS_setEBSR(CSL_EBSR_FIELD_PPMODE,
						 CSL_EBSR_PPMODE_1);
	if(CSL_SOK != status)
		return;

	/* Loop counter and error flag */
	status = UART_init(&uartObj,CSL_UART_INST_0,UART_INTERRUPT);
	if(CSL_SOK != status)
		return;

	status = SYS_setEBSR(CSL_EBSR_FIELD_PPMODE,
						 CSL_EBSR_PPMODE_1);
	if(CSL_SOK != status)
		return;

	/* Handle created */
	hUart = (CSL_UartHandle)(&uartObj);

	/* Configure UART registers using setup structure */
	status = UART_setup(hUart,&mySetup);
	if(CSL_SOK != status)
		return;

	/* Configure and Register the UART interrupts*/
	isrAddr.rbiAddr  = uart_rxIsr;
	isrAddr.ctoi     = uart_ctoIsr;

	/* Disable interrupt */
	IRQ_globalDisable();

	/* Clear any pending interrupts */
	IRQ_clearAll();

	/* Disable all the interrupts */
	IRQ_disableAll();

	IRQ_setVecs((Uint32)(&VECSTART));

	/* Configuring Interrupt */
	IRQ_plug (UART_EVENT, &UART_intrDispatch);

	/* Enabling Interrupt */
	IRQ_enable(UART_EVENT);
	IRQ_globalEnable();

	/* Set the UART callback function */
	status = UART_setCallback(hUart,&isrAddr);
	if(CSL_SOK != status)
		return;

	status = UART_eventEnable(hUart, CSL_UART_RECVOR_REG_DATA_INTERRUPT);
	if(CSL_SOK != status)
		return;
}

/*
 *	TSK thread responsible for setting up and receiving UART interrupts
 */
void TSK_Uart()
{
	ConfigureUart();

	while(1)
	{
		// Sleep forever, waiting for interrupts
		TSK_sleep(SYS_FOREVER);
	}
}

/**
 *  \brief  Function to calculate the clock at which system is running
 *
 *  \param    none
 *
 *  \return   System clock value in Hz
 */

Uint32 getSysClk(void)
{
	Bool      pllRDBypass;
	Bool      pllOutDiv;
	Uint32    sysClk;
	Uint16    pllM;
	Uint16    pllRD;
	Uint16    pllOD;

	pllM = CSL_FEXT(CSL_SYSCTRL_REGS->CGCR1, SYS_CGCR1_M);

	pllRD = CSL_FEXT(CSL_SYSCTRL_REGS->CGCR2, SYS_CGCR2_RDRATIO);
	pllOD = CSL_FEXT(CSL_SYSCTRL_REGS->CGCR4, SYS_CGCR4_ODRATIO);

	pllRDBypass = CSL_FEXT(CSL_SYSCTRL_REGS->CGCR2, SYS_CGCR2_RDBYPASS);
	pllOutDiv   = CSL_FEXT(CSL_SYSCTRL_REGS->CGCR4, SYS_CGCR4_OUTDIVEN);

	sysClk = CSL_PLL_CLOCKIN;

	if (0 == pllRDBypass)
	{
		sysClk = sysClk/(pllRD + 4);
	}

	sysClk = (sysClk * (pllM + 4));

	if (1 == pllOutDiv)
	{
		sysClk = sysClk/(pllOD + 1);
	}

	/* Return the value of system clock in KHz */
	return(sysClk);
}
