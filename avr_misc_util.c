/**
 * \file avr_misc_util.c
 * \date 12.5.2016 17:42:11
 * \brief Miscellaneous utility functions for the Atmel AVR8 platform (C source file) 
 * \author Mario Malenica\n 
 * e-mail: mario.malenica@inet.hr */
/* 
 Copyright (c) 2016, Mario Malenica. All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <avr/io.h>
#include <stdlib.h>
#include "avr_misc_util.h"

#if defined (UART_CALC_USE_ERROR_CHECK)
/** 
 * \def UART_ERR_CALC
 * Calculate UART baudrate error margin.\n
 * Max. allowed baudrate error is multiplied by 10 and casted to 8-bit ()and later divided by 1000). Without that cast GCC (at least on 4.9.2) generates 800 byte larger code for AVR
 */
#define UART_ERR_CALC(x)((uint32_t)(x*(uint32_t)(UART_MAX_ERR_RATE*10)/1000))
#endif

/* Calculate baudrate and prescaler bits */
int8_t calc_presc_CSbits(uint32_t freq, AVR_Timers timer_id, uint16_t *timer_ticks)
{
	const uint16_t prescalers[]={1,8,64,256,1024,1,8,32,64,128,256,1024};
	uint8_t prescaler_start=0;
	uint8_t prescaler_end=5;
	uint16_t timer_max=255;
	uint8_t counter;
	uint32_t temp;
	uint32_t val;


	/* Sanity check */
	if(freq==0)
	{
		return -1;
	}

	temp=F_CPU/freq;
	val=temp;
		
	switch(timer_id)
	{
		case AVR_TIMER0:
		break;
		
		case AVR_TIMER1:
		case AVR_TIMER3:
		case AVR_TIMER4:
		case AVR_TIMER5:
			timer_max=65535;
		break;
		
		case AVR_TIMER2:
			prescaler_start=5;
			prescaler_end=12;
		break;
		
		default:
			return -1;
		break;
	}
	
	
	counter=prescaler_start;

	do
	{
		val=(temp/prescalers[counter])-1;
		counter++;
	}while(val>timer_max && counter<prescaler_end);

	if(val>timer_max)
	{
		return -1;
	}

	if(counter==0)
	{
		counter=1;
	}

	*timer_ticks=val;
	return (counter-prescaler_start);
}

int16_t calc_uart_UBRRn(uint32_t baudrate, bool is_U2X_set)
{
	uint16_t temp;
	uint32_t check_baudrate;
	uint32_t first_diff;
	uint16_t first_val;
	uint32_t second_diff;
	uint8_t prescaler=16;
	
	/* Sanity check */
	if(baudrate==0)
	{
		return -1;
	}
	
	if(is_U2X_set)
	{
		prescaler=8;
	}

	temp=(F_CPU/(baudrate*prescaler))-1;

	if(temp>4095)
	{
		return -1;
	}
	

	check_baudrate=F_CPU/(prescaler*(temp+1));
		
	/* Exact baudrate match, get out of here... */
	if(check_baudrate==baudrate)
	{
		return (int16_t)temp;
	}
	
	/* Check if there's a result that's closer to the requested baudrate */
	
	first_diff=abs((int32_t)(check_baudrate-baudrate));
	first_val=temp;
	
	if(check_baudrate>baudrate)
	{
		temp++;
	}
	else
	{
		temp--;
	}

	check_baudrate=F_CPU/(prescaler*(temp+1));
	
	second_diff=abs((int32_t)(check_baudrate-baudrate));
	
	if(first_diff<second_diff)
	{
		temp=first_val;
	}
#if defined(UART_CALC_USE_ERROR_CHECK)
	else
	{
		first_diff=second_diff;
	}

	/* Check if the baudrate is inside of the error margin */
	if(first_diff>UART_ERR_CALC(baudrate))
	{
		return -1;
	}
#endif
	
	return (int16_t)temp;
}

/* For x=255, y=10, it will wrap result to 0 */
uint8_t round_near_mul_wovf(uint8_t x, uint8_t y)
{
	uint16_t temp=x;
	
	if(y==0)
	{
		return 0;
	}
	
	temp+=(y/2);
	temp-=temp%y;
	
	if(temp>UINT8_MAX) /* Overflow detected! */
	{
		temp/=y;
		temp*=y;
	}
	
	return (uint8_t)(temp);
}

/* For x=255, y=10, it will not overflow, but round to nearest smaller value (it will "saturate" the result) */
uint8_t round_near_mul_wsat(uint8_t x, uint8_t y)
{
	uint16_t temp=x;
	
	if(y==0)
	{
		return 0;
	}
		
	temp+=(y/2);
	temp-=temp%y;
	
	if(temp>UINT8_MAX) /* Overflow detected! */
	{
		temp=x/y;
		temp*=y;
	}	
	
	return (uint8_t)(temp);
}