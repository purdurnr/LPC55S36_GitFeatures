/*
 * Copyright 2016-2022 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of NXP Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file    LPC55S36_Project.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "LPC55S36.h"
#include "fsl_debug_console.h"
/* TODO: insert other include files here. */
#include "fsl_power.h"

/* TODO: insert other definitions and declarations here. */
/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint32_t g_systickCounter;

/*******************************************************************************
 * Code
 ******************************************************************************/
void SysTick_Handler(void)
{
    if (g_systickCounter != 0U)
    {
        g_systickCounter--;
    }
}

void SysTick_DelayTicks(uint32_t n)
{
    g_systickCounter = n;
    while (g_systickCounter != 0U)
    {
    }
}

/*
 * @brief   Application entry point.
 */

uint8_t g_uart_ch;

/* FLEXCOMM0_IRQn interrupt handler */
void FLEXCOMM0_FLEXCOMM_IRQHANDLER(void) {
  /*  Place your code here */
    uint32_t StatusFlags;

    StatusFlags= USART_GetStatusFlags(BOARD_DEBUG_UART_BASEADDR);
        /* If new data arrived. */
    if ((kUSART_RxFifoNotEmptyFlag | kUSART_RxError) & StatusFlags )
    {
    	g_uart_ch = USART_ReadByte(BOARD_DEBUG_UART_BASEADDR);
    	g_uart_ch = g_uart_ch - '0';
    }
  /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F
     Store immediate overlapping exception return operation might vector to incorrect interrupt. */
  #if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
  #endif
}

int main(void) {

	uint8_t system_active;

    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
    USART_DisableInterrupts(BOARD_DEBUG_UART_BASEADDR, kStatus_USART_TxIdle);
    EnableIRQ(FLEXCOMM0_FLEXCOMM_IRQN);

#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    /* Init FSL debug console. */
    //BOARD_InitDebugConsole();
#endif

//    PRINTF("Hello World\n");

    /* Set systick reload value to generate 1ms interrupt */
    if (SysTick_Config(SystemCoreClock / 1000U))
    {
        while (1)
        {
        }
    }

    /* Enter an infinite loop, just incrementing a counter. */
    g_systickCounter = 5000U; // 5 seconds until low power mode
    system_active = 0;

    while(1) {

    	switch(g_uart_ch)
    	{
    	case 1:
        	system_active = 1;
        	GPIO_PortSet(BOARD_LED_RED_GPIO, BOARD_LED_RED_GPIO_PORT, 1u << BOARD_LED_RED_GPIO_PIN);
        	g_uart_ch = 0 ;
    		break ;

    	case 2:
        	system_active = 0;
        	g_systickCounter = 3000U; // 3 seconds until low power mode
        	GPIO_PortClear(BOARD_LED_RED_GPIO, BOARD_LED_RED_GPIO_PORT, 1u << BOARD_LED_RED_GPIO_PIN);
        	g_uart_ch = 0 ;
    		break ;

    	default:
    		if ((g_systickCounter==0) && (system_active == 0))
    		{
    			POWER_EnterSleep();
    		};

    	}
}




    return 0 ;
}
