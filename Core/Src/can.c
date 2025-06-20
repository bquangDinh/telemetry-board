/*
 * can.c
 *
 *  Created on: Jun 1, 2025
 *      Author: buiqu
 */


#include "can.h"

/**
 * CAN Instance
 */
extern CAN_HandleTypeDef hcan;

#define CAN_Instance &hcan

/*
 * CAN RX Header - stores the header of the received message
 */
static CAN_RxHeaderTypeDef rx_header;
static CAN_TxHeaderTypeDef tx_header;
static uint32_t tx_mailbox;
static uint8_t rx_data[8];

int init_can() {
	// Set up tx header
	tx_header.StdId = BOARD_ID;
	tx_header.IDE = CAN_ID_STD; // use standard id, not extended 11-bit id
	tx_header.RTR = CAN_RTR_DATA;
	tx_header.DLC = 8;
	tx_header.TransmitGlobalTime = DISABLE;

	// Set up filters
	// Since the telemetry board should only accept CAN data from BMS
	// Each BMS message starts with the first 3 bit as 001
	// So we need to set up a ID-MASK filter that only accept messages if the first 3 bits
	// of the ID are 001
	CAN_FilterTypeDef can_filter;

	can_filter.FilterActivation = CAN_FILTER_ENABLE;
	can_filter.FilterFIFOAssignment = CAN_RX_FIFO0;
	can_filter.FilterMode = CAN_FILTERMODE_IDMASK;
	can_filter.FilterScale = CAN_FILTERSCALE_32BIT;
	can_filter.FilterBank = 0;

	// Match ID wheter the first 3 bits are 001
	can_filter.FilterIdHigh = 0x100 << 5;

	// Since we don't use Extended ID, we don't need to set up this
	can_filter.FilterIdLow = 0x0000;

	// Set up the mask that only cares about the first 3 bits of the incoming ID
	// a.k.a  0b11100000000
	can_filter.FilterMaskIdHigh = 0x700 << 5;

	// Since we don't use Extended ID, we don't need to set up this
	can_filter.FilterMaskIdLow = 0x0000;

	// Apply filter
	if (HAL_CAN_ConfigFilter(CAN_Instance, &can_filter) != HAL_OK) {
		//TODO: report CAN errors back to telemetry web server
		return -1;
	}

	// Start CAN
	if (HAL_CAN_Start(CAN_Instance) != HAL_OK) {
		//TODO: report CAN error back to telemetry web server
		return -2;
	}

	// Enable interupt
	if (HAL_CAN_ActivateNotification(
			CAN_Instance,
			CAN_IT_RX_FIFO0_MSG_PENDING |
			CAN_IT_RX_FIFO0_FULL |
			CAN_IT_RX_FIFO0_OVERRUN |
			CAN_IT_WAKEUP |
			CAN_IT_SLEEP_ACK |
			CAN_IT_ERROR_WARNING |
			CAN_IT_ERROR_PASSIVE |
			CAN_IT_BUSOFF |
			CAN_IT_LAST_ERROR_CODE |
			CAN_IT_ERROR
		) != HAL_OK) {
		//TODO: report CAN error back to telemetry web server
		return -3;
	}

	return 0;
}

void can_on_received_message_handler(CAN_HandleTypeDef* hcan) {
	assert_param(hcan != NULL);

	// Retrieve message from FIFO
	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK) {
		//TODO: reports CAN error back to telemetry web server
		return;
	}

	if (rx_header.StdId == TEMP_SENSOR_ARDUINO_BOARD_ID) {
		handle_temp_arduino_message(rx_data);
	} else {
		handle_bms_message(rx_header.StdId, rx_data);
	}
}

HAL_StatusTypeDef send_can_message(const char* data) {
	assert_param(data != NULL);

	return HAL_CAN_AddTxMessage(CAN_Instance, &tx_header, (uint8_t*)data, &tx_mailbox);
}

static void handle_temp_arduino_message(const uint8_t* data) {
	// Arduno send 4-byte float number in 4 1-byte field
	// The first byte indicates the ID of the sensor
	// The next 4 bytes indicate the sensor value
	// The last 3 bytes are ignored
	assert_param(data != NULL);

	uint8_t sensor_id = data[0];

	FloatByte value = {
		.bytes = { data[1], data[2], data[3], data[4] }
	};

	uint32_t scaled = (uint32_t)(value.f * SCALED_INT_TWO_DECIMAL_PRECISION);

	const char* label = NULL;

	if (sensor_id == CABIN_TEMP_SENSOR_ID)
		label = CABIN_TEMP_LABEL;
	else if (sensor_id == TRUNK_TEMP_SENSOR_ID)
		label = TRUNK_TEMP_LABEL;

	if (label) {
		if (telemetry_send_data_to_note(label, scaled) == 0)
			DBG("Sent Arduino data successfully!");
		else
			DBG("Failed to send data to Note Card");
	}
}

static void handle_bms_message(const uint32_t id, const uint8_t* data) {
	char row = id - 0x100;

//	DBG("Sending data to Note Card...");

	if (telemetry_send_bms_to_note(row, rx_data) == 0) {
		DBG("Sent BMS successfully!");
	} else {
		DBG("Failed to send data to Note Card");
	}
}

