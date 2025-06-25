/*
 * telemetry.h
 *
 *  Created on: Jun 1, 2025
 *      Author: buiqu
 */

#ifndef INC_TELEMETRY_H_
#define INC_TELEMETRY_H_

#include <stdio.h>
#include <stdlib.h>
#include "stm32f0xx_hal.h"
#include "utility.h"
#include <string.h>  /* strcpy, strcmp */

#define NOTE_PRODUCT_ID "com.gmail.rusolarcarclub:rsctest"

#define NOTE_HUB_SET_CMD "hub.set"
#define NOTE_CARD_ADD_REQUEST_CMD "note.add"
#define NOTE_HUB_OPERATION_MODE "continuous"

struct Telemetry_Data {
	// hardcoded here -- CAN IDs are from 0x100 -> 0x111 (BMS) | 0x112 (Arduino) | 0x114 (telemetry) => so I just store offset value to save space
	uint8_t offset; // ex: 0x111 is 11

	char values[8]; // store 8 bytes value of CAN message
};

typedef struct Telemetry_Data Telemetry_Data;

int telemetry_init();

void telemetry_sync_to_note();

void add_telemetry(uint32_t id, const char* value);

void log_telemetry();

/**
 * @brief Send data over USART
 */
static HAL_StatusTypeDef write_to_serial(const char* data, const int len);
#endif /* INC_TELEMETRY_H_ */
