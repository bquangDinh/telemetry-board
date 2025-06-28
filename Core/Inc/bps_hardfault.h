/*
 * bps_hardfault.h
 *
 *  Created on: Jun 17, 2025
 *      Author: RU Solar HighEnd 1
 */

#ifndef INC_BPS_HARDFAULT_H_
#define INC_BPS_HARDFAULT_H_

#include "stm32f0xx_hal.h"
#include "telemetry.h"
#include "can.h"

// Call when a hard fault occurs
void BPS_HardFault_Handler(uint8_t hardfault_occurred);

static inline void send_hardfault_to_telemetry(uint8_t hardfault_occurred);
static inline void send_hardfault_to_can(uint8_t hardfault_occurred);

#endif /* INC_BPS_HARDFAULT_H_ */