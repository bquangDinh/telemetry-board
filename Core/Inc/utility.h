/*
 * utility.h
 *
 *  Created on: Jun 1, 2025
 *      Author: buiqu
 */

#ifndef INC_UTILITY_H_
#define INC_UTILITY_H_

#include "stm32f0xx_hal.h"

#ifdef ENABLE_PRINTF_DEBUG // enable this in Build Properties of the IDE
#include <stdio.h>

#define DBG(fmt, ...) 	printf("[DBG] " fmt "\r\n", ##__VA_ARGS__)
#define ERR(fmt, ...) 	printf("[ERR] " fmt "\r\n", ##__VA_ARGS__)
#define WARN(fmt, ...) 	printf("[WARN] " fmt "\r\n", ##__VA_ARGS__)
#define printf(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define DBG(fmt, ...)
#define ERR(fmt, ...)
#define WARN(fmt, ...)
#define printf(fmt, ...)
#endif

#ifdef ENABLE_TIMER
// Specify the timer for Utility
extern TIM_HandleTypeDef htim2;

#define TIMER_INSTANCE &htim2

/**
 * @brief Delay for the given us using timer instance
 *
 * @param us		provided us for the timer to count up to
 */
void delay_us(int us);
#endif


// MACRO FUNCTIONS
#define PI 3.14f
#define CIRCLE_PERIMETER(RAD) 2*PI*RAD
#define ROUND_FLOAT(num) (int)(num + 0.5f)

#endif /* INC_UTILITY_H_ */
