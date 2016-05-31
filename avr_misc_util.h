/**
 * \file avr_misc_util.h
 * \date 12.5.2016 17:41:38
 * \brief Miscellaneous utility functions for the Atmel AVR8 platform  (C header file)
 * \details Baud rate and frequency calculation functions are relatively large, I'm using them with ATmega chips when there's a need to dynamically\n
 *          calculate the values. Sure, I could rewrite most of the code in asm, but I wanted the code to be portable.\n
 *          You can always use those functions with any other C or C++ compiler and then only add a table of precalculated frequency/baud rate values to your code.
 * \author Mario Malenica\n
 *	e-mail: mario.malenica@inet.hr */
 
/* Copyright (c) 2016, Mario Malenica. All rights reserved.

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


#ifndef AVR_MISC_UTIL_H_
#define AVR_MISC_UTIL_H_

#include <inttypes.h>
#include <stdbool.h>

#if defined (__cplusplus)
extern "C"{
#endif

#ifndef F_CPU
#error Some of the functions will not compile without F_CPU defined and set to CPU clock value (in Hz)
#endif

/** \enum _AVR_Timers Enum containing a list of timers available on AVR8 microcontrollers */
typedef enum _AVR_Timers
{
	AVR_TIMER0=(uint8_t)0,
	AVR_TIMER1,
	AVR_TIMER2,
	AVR_TIMER3,
	AVR_TIMER4,
	AVR_TIMER5
}AVR_Timers; /**<A list of timers available on AVR8 microcontrollers */

/** \def UART_CALC_USE_ERROR_CHECK 
 *	If defined, activates baud rate error checking in function calc_uart_UBRRn().
 */
#define UART_CALC_USE_ERROR_CHECK (1)

#if defined (UART_CALC_USE_ERROR_CHECK)
/** 
 * \def UART_MAX_ERR_RATE
 * Max. allowed baudrate error, value in %
 */
#define UART_MAX_ERR_RATE (2.5)
#endif

/**
 * \fn int16_t calc_uart_UBRRn(uint32_t baudrate, bool is_U2X_set)
 * \brief Calculate value for the baudrate register (UBRRn) for a given baudrate
 * \param baudrate Selected baudrate
 * \param is_U2X_set Set it to true if X2 mode is used (double the UART speed, defined by bit 1 UCSRnA register), false otherwise
 * \return Value for UBRRn register or -1 in case of an error (baudrate=0, result larger than 12 bits, baud rate offset error too big)
 * \details If #UART_CALC_USE_ERROR_CHECK is defined, then the result will be compared to #UART_MAX_ERR_RATE.\n
 *          If error rate is greater than #UART_MAX_ERR_RATE, the function will return -1.\n
 * Compiled size (GCC 4.9.1): 858 bytes on -O0, 618 on -O1\n
 * Execution time (GCC 4.9.1): Between 1370-2900 CPU cycles, depending on argument values
 */
int16_t calc_uart_UBRRn(uint32_t baudrate, bool is_U2X_set);

/**
 * \fn int8_t calc_presc_CSbits(uint32_t freq, AVR_Timers timer_id, uint16_t *timer_ticks)
 * \brief Calculate timer and prescaler (CS bits) values for a given timer and frequency. Timer mode is CTC.
 * \param freq Target frequency
 * \param timer_id Chosen timer
 * \param [out] timer_ticks Calculated value to be used in OCR register
 * \return Prescaler (CS bits) value, or -1 in case of an error (freq=0, unknown timer_id, frequency too small for the selected timer)
 * \details The code only calculates the values, it does not check error offset.\n
 *          Timer2 has more prescalers available than timer0, so use it if you need 8-bit timer and accuracy is important.\n
 * Compiled size (GCC 4.9.1): 488 bytes on -O0, 418 on -O1\n
 * Execution time (GCC 4.9.1): Between 1000-4000 CPU cycles, depending on argument values
 */
int8_t calc_presc_CSbits(uint32_t freq, AVR_Timers timer_id, uint16_t *timer_ticks);

/**
 * \fn uint8_t round_near_mul_wovf(uint8_t x, uint8_t y)
 * \brief Rounds number "x" to nearest multiple of "y". Works similar to "half up" rounding, but in this case with integers.\n
 *        In case of an overflow, the result will be rounded to the next multiple of y (usually 0).
 * \param x Number to round
 * \param y Number whose multiple we're looking for
 * \returns Rounded value
 * \details Size (GCC 4.9.2): 228 bytes when compiled on -O0, 116 bytes on -O1\n
 *          Speed (GCC 4.9.2), non-hardcoded values for x and y, worst case (longest execution path): 533 CPU cycles on -O0,  438 cycles on -O1
 */
uint8_t round_near_mul_wovf(uint8_t x, uint8_t y);

/**
 * \fn uint8_t round_near_mul_wsat(uint8_t x, uint8_t y)
 * \brief Rounds number "x" to nearest multiple of "y". Works similar to round_near_mul_wovf(), but with one key defference.\n
 *        In case of an overflow, the result will be rounded to the nearest smaller multiple of y (it will "saturate" the result).
 * \param x Number to round
 * \param y Number whose multiple we're looking for
 * \returns Rounded value
 * \details Size (GCC 4.9.2): 248 bytes when compiled on -O0, 148 bytes on -O1\n
 *          Speed (GCC 4.9.2), non-hardcoded values for x and y, worst case (longest execution path): 412 CPU cycles on -O0,  329 cycles on -O1
 */
uint8_t round_near_mul_wsat(uint8_t x, uint8_t y);

#if defined (__cplusplus)
}
#endif

#endif /* AVR_MISC_UTIL_H_ */
