/*
 * utility.c
 *
 *  Created on: Jun 1, 2025
 *      Author: buiqu
 */

#include "utility.h"

#ifdef ENABLE_PRINTF_DEBUG

// Change to the name of huart variable
extern UART_HandleTypeDef huart2;

#define USART_INSTANCE &huart2

int _write(int file, char *ptr, int len) {
	HAL_UART_Transmit(USART_INSTANCE, (uint8_t*)ptr, len, HAL_MAX_DELAY);

	return len;
}
#endif

#ifdef ENABLE_TIMER
void delay_us(int us) {
	 __HAL_TIM_SET_COUNTER(TIMER_INSTANCE, 0);  // reset counter

	 while (__HAL_TIM_GET_COUNTER(TIMER_INSTANCE) < us);
}
#endif
