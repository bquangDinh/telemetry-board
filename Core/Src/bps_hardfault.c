/*
 * bps_hardfault.c
 *
 *  Created on: Jun 17, 2025
 *      Author: RU Solar HighEnd 1
 */

#include "bps_hardfault.h"

void BPS_HardFault_Handler(uint8_t hardfault_occurred) {
	// Handle the hard fault
	send_hardfault_to_telemetry(hardfault_occurred);
	send_hardfault_to_can(hardfault_occurred);
}

static inline void send_hardfault_to_telemetry(uint8_t hardfault_occurred) {
	// Send hard fault information to telemetry
    // First byte indicates hard fault occurred
    // The rest of the bytes can be used for additional information if needed
    char msg[8] = {hardfault_occurred, 0, 0, 0, 0, 0, 0, 0};

    add_telemetry(RESERVED_CAN_ID, msg); // 0x10F is the reserved ID for hard faults
}

static inline void send_hardfault_to_can(uint8_t hardfault_occurred) {
	// Send hard fault information to CAN
    char msg[8] = {hardfault_occurred, 0, 0, 0, 0, 0, 0, 0};

    send_can_message(RESERVED_CAN_ID, msg);
}