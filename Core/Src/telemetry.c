/*
 * telemetry.c
 *
 *  Created on: Jun 1, 2025
 *      Author: buiqu
 */

#include "telemetry.h"

// The number of items is from 0x100 -> 0x10B + 0x10C (Arduino) + 0x10D (telemetry) + 0x10E (Ras Pi 5) + 0x10F (Reserved) = 16 keys
#define NUM_ITEMS 16

static Telemetry_Data list[NUM_ITEMS]; // Array of telemetry data

#ifdef ENABLE_PRINTF_DEBUG
char log_buffer[64]; // Buffer for log messages
char has_log_buffer = 0; // Flag to indicate if log buffer has been used
#endif

// Specify which USART to use to communicate with Note Card
extern UART_HandleTypeDef huart1;

#define NOTE_SERIAL &huart1

int telemetry_init() {
	// Send the command to Note Card to set the product ID and mode
	char *buffer = "{\"req\":\"hub.set\",\"product\":\"" NOTE_PRODUCT_ID "\",\"mode\":\"continuous\",\"outbound\":1}";

	if (write_to_serial(buffer, sizeof(buffer)) != HAL_OK) {
		ERR("Failed to send telemetry init command to Note Card");

		return -1;
	}

	// Init the list
	for (int i = 0; i < NUM_ITEMS; ++i) {
		list[i].offset = 0x100 + i; // Set the offset to the CAN ID
		list[i].values[0] = '\0'; // Initialize values to empty
	}

	return 0;
}

void add_telemetry(uint32_t id, const char* value) {
	assert_param(value != NULL);

	// Find the index in the list based on the ID
	uint8_t index = (id - 0x100); // Convert ID to index (0x100 -> 0, 0x101 -> 1, ..., 0x111 -> 11)

	if (index < 0 || index >= NUM_ITEMS) {
#ifdef ENABLE_PRINTF_DEBUG
		sprintf(log_buffer, "Invalid telemetry ID: 0x%03X, index: %d", (unsigned int)id, index);
		has_log_buffer = 1;
#endif
		return;
	}

	// Copy 8 bytes into the values array
	memcpy(list[index].values, value, 8);

#ifdef ENABLE_PRINTF_DEBUG
	sprintf(log_buffer, "Added telemetry: ID=0x%03X | %02X %02X %02X %02X %02X %02X %02X %02X", (unsigned int)id,
		list[index].values[0], list[index].values[1], list[index].values[2],
		list[index].values[3], list[index].values[4], list[index].values[5],
		list[index].values[6], list[index].values[7]);
	has_log_buffer = 1;
#endif
}

void telemetry_sync_to_note() {
	char buffer[1024];
	int offset = 0;

	// Start JSON
	offset += snprintf(&buffer[offset], sizeof(buffer) - offset, "{\"req\":\"note.add\",\"file\":\"data.qo\",\"sync\":true,\"body\":{");

	// Build JSON Body
	for (uint8_t i = 0; i < NUM_ITEMS; ++i) {
		// Add comma if not the first item
		if (i > 0) {
			offset += snprintf(&buffer[offset], sizeof(buffer) - offset, ",");
		}
		// Add the telemetry data in the format "x<offset>":"<value>"
		offset += snprintf(&buffer[offset], sizeof(buffer) - offset, "\"x%d\":\"%02X %02X %02X %02X %02X %02X %02X %02X\"", list[i].offset, list[i].values[0], list[i].values[1], list[i].values[2], list[i].values[3], list[i].values[4], list[i].values[5], list[i].values[6], list[i].values[7]);
	}

	// End JSON
	offset += snprintf(&buffer[offset], sizeof(buffer) - offset, "}}\n");

	// Check for overflow
	if (offset >= sizeof(buffer)) {
		ERR("Failed to send data to note: Buffer overflow!");

		return;
	}

	DBG("Message is: %s", buffer);

	// Transmit the final string over UART
	if (write_to_serial(buffer, offset) != HAL_OK) {
		ERR("Failed to send telemetry data to Note Card");
	}
}

#ifdef ENABLE_PRINTF_DEBUG
void log_telemetry() {
	if (!has_log_buffer) {
		return; // No log to print
	}

	DBG("Telemetry Log: %s", log_buffer);

	// Clear the log buffer
	log_buffer[0] = '\0';

	has_log_buffer = 0; // Reset the flag
}
#endif

static HAL_StatusTypeDef write_to_serial(const char* data, const int len) {
	return HAL_UART_Transmit(NOTE_SERIAL, (uint8_t*)data, len, HAL_MAX_DELAY);
}
