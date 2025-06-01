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
#endif /* INC_UTILITY_H_ */
