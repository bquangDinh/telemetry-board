/*
 * telemetry.c
 *
 *  Created on: Jun 1, 2025
 *      Author: buiqu
 */

#include "telemetry.h"

const char* const telemetry_labels[ROWS][COLS] = {
		{	"rel", "x", "Mcn", "x", "pcl", "fas", "x", "cks"	},
		{	"ccl", "x", "dcl", "x", "pcr", "piv", "pov", "cks"	},
		{	"soc", "pam", "prs", "dod", "phl", "psv", "pac", "cks"	},
		{	"Mpv", "mpv", "tpc", "crl", "pck", "pdk", "Mpd", "cks"	},
		{	"dt1", "dt2", "avc", "htp", "ltp", "atp", "fsp", "cks"	},
		{	"itp", "lcv", "hcv", "acv", "lov", "hov", "aov", "cks"	},
		{	"lcs", "hcs", "acs", "Mcv", "mcv", "hti", "lti", "cks"	},
		{	"hci", "lci", "hoi", "loi", "hii", "lii", "aam", "cks"	},
		{	"atc", "att", "icp", "ith", "int", "x", "x", "x"	}
};

// Flags to check whether telemetry has been initialized or not
// It prevents telemetry_init() from being called multiple times
// Or I forgot to call telemetry_init()
static int is_telemetry_inited = 0;

// Specify which USART to use to communicate with Note Card
extern UART_HandleTypeDef huart1;

#define NOTE_SERIAL &huart1

int telemetry_init() {
	if (is_telemetry_inited == 1) {
		// telemetry has been configured
		return 0;
	}
//
//	J *req = NoteNewRequest(NOTE_HUB_SET_CMD);
//
//	// Set product ID
//	JAddStringToObject(req, "product", NOTE_PRODUCT_ID);
//
//	// Set operation mode
//	JAddStringToObject(req, "mode", NOTE_HUB_OPERATION_MODE);
//
//	// Issue the request to Notecard
//	return NoteRequest(req);

	char *buffer = "{\"req\":\"hub.set\",\"product\":\"" NOTE_PRODUCT_ID "\",\"mode\":\"continuous\"}";

	DBG("[USART1] sends: %s", buffer);

	if (write_to_serial(buffer, sizeof(buffer)) != HAL_OK) {
		return -1;
	}

	is_telemetry_inited = 1;

	return 0;
}

int telemetry_send_bms_to_note(int row, const uint8_t* data) {
	assert_param(is_telemetry_inited == 1);
	assert_param(row >= 0 && row < ROWS);

	const char* const (*labels)[COLS] = &telemetry_labels[row];

	char buffer[256];
	int offset = 0;

	offset += snprintf(&buffer[offset], sizeof(buffer) - offset, "{\"req\":\"note.add\",\"file\":\"data.qo\",\"sync\":true,\"body\":{");

#ifdef IGNORE_CHECKSUM
	for (int i = 0; i < COLS - 1; ++i) {
#else
	for (int i = 0; i < COLS; ++i) {
#endif
		// Add comma if not the first field
		if (i != 0) {
			offset += sprintf(&buffer[offset], ",");
		}

		offset += snprintf(&buffer[offset], sizeof(buffer) - offset, "\"%s\":%d", (*labels)[i], data[i]);
	}

	// End JSON
	offset += snprintf(&buffer[offset], sizeof(buffer) - offset, "}}\n");

	// Check for overflow
	if (offset >= sizeof(buffer)) {
		return -1;
	}

	DBG("Message is: %s", buffer);

	// Transmit the final string over UART
	if (write_to_serial(buffer, offset) != HAL_OK) {
		return -1;
	}

	return 0;
}

int telemetry_send_data_to_note(const char* label, const uint32_t value) {
	assert_param(is_telemetry_inited == 1);
	assert_param(label != NULL);

	char buffer[256];
	int offset = 0;

	offset += snprintf(&buffer[offset], sizeof(buffer) - offset, "{\"req\":\"note.add\",\"file\":\"data.qo\",\"sync\":true,\"body\":{");

	offset += snprintf(&buffer[offset], sizeof(buffer) - offset, "\"%s\":%ld", label, value);

	// End JSON
	offset += snprintf(&buffer[offset], sizeof(buffer) - offset, "}}\n");

	// Check for overflow
	if (offset >= sizeof(buffer)) {
		return -1;
	}

	DBG("Message is: %s", buffer);

	// Transmit the final string over UART
	if (write_to_serial(buffer, offset) != HAL_OK) {
		return -1;
	}

	return 0;
}

static HAL_StatusTypeDef write_to_serial(const char* data, const int len) {
	return HAL_UART_Transmit(NOTE_SERIAL, (uint8_t*)data, len, HAL_MAX_DELAY);
}
