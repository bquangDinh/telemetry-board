/*
 * can.h
 *
 *  Created on: Jun 1, 2025
 *      Author: buiqu
 */

#ifndef INC_CAN_H_
#define INC_CAN_H_

#include "stm32f0xx_hal.h"
#include "utility.h"
#include "telemetry.h"

#define TEMP_SENSOR_ARDUINO_BOARD_ID 0x110

#define CABIN_TEMP_SENSOR_ID 0
#define TRUNK_TEMP_SENSOR_ID 1

#define CABIN_TEMP_LABEL "cabin_temp"
#define TRUNK_TEMP_LABEL "trunk_temp"

#define SCALED_INT_TWO_DECIMAL_PRECISION 100

typedef union {
	float f;
	uint8_t bytes[4];
} FloatByte;

/**
 * @brief Init and start CAN communication
 *
 * @retval 0 Success
 * @retval -1 CAN failed to apply filter
 * @retval -2 CAN failed to start
 * @retval -3 CAN failed to enable interrupt
 */
int init_can();

/**
 * @brief Handle received message from the CAN bus
 *
 * @param[in] hcan		Pointer to CAN handle
 */
void can_on_received_message_handler(CAN_HandleTypeDef* hcan);

static void handle_temp_arduino_message(const uint8_t* data);
static void handle_bms_message(const uint32_t id, const uint8_t* data);
#endif /* INC_CAN_H_ */
