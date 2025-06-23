/*
 * telemetry.c
 *
 *  Created on: Jun 1, 2025
 *      Author: buiqu
 */

#include "telemetry.h"

#define IGNORE_FIELD "x"
#define RESERVE_TWOBYTE_FIELD "-"

#define RELAY_STATE "rel"
#define MAX_CELL_NUMBER "Mcn"
#define POPULATED_CELLS "pcl"
#define PACK_SOC "soc"
#define PACK_CCL "ccl"
#define PACK_DCL "dcl"
#define PACK_CURRENT "pcr"
#define PACK_DOD "dod"
#define FAILSAFE_STATUS "fas"
#define PACK_CCL_KW "ckw"
#define PACK_DCL_KW "dkw"
#define PACK_INST_VOLTAGE "ivo"
#define PACK_HEALTH "hth"
#define PACK_OPEN_VOLTAGE "ovt"
#define PACK_AMPHOURS "amp"
#define PACK_RESISTANCE "res"
#define HIGH_TEMPERATURE "htp"
#define PACK_SUMMED_VOLTAGE "svt"
#define MAX_PACK_VOLTAGE "Mpv"
#define MIN_PACK_VOLTAGE "mpv"
#define FAN_SPEED "fsp"
#define TOTAL_PACK_CYCLES "tpc"
#define CURRENT_LIMITS_STATUS "cls"
#define MAX_PACK_DCL "Mpd"
#define HIGH_THERMISTOR_ID "hti"
#define MAX_PACK_CCL "Mpc"
#define AVERAGE_CURRENT "avc"
#define LOW_CELL_VOLTAGE "lcv"
#define LOW_CELL_VOLTAGE_ID "lvi"
#define HIGH_CELL_VOLTAGE "hcv"
#define HIGH_CELL_VOLTAGE_ID "hvi"
#define AVERAGE_CELL_VOLTAGE "acv"
#define LOW_OPENCELL_VOLTAGE "lov"
#define LOW_OPENCELL_ID "loi"
#define HIGH_OPENCELL_ID "hoi"
#define HIGH_OPENCELL_VOLTAGE "hov"
#define AVERAGE_OPENCELL_VOLTAGE "aov"
#define LOW_TEMPERATURE "ltp"
#define LOW_THERMISTOR_ID "lti"
#define AVERAGE_TEMPERATURE "atp"
#define INTERNAL_TEMPERATURE "itp"
#define REQUEST_FAN_SPEED "rfs"
#define ADAPTIVE_SOC "aso"
#define ADAPTIVE_AMPHOURS "aap"
#define ADAPTIVE_TOTAL_CAPACITY "atc"
#define MAX_CELL_VOLTAGE "Mcv"
#define DTC_FLAG_1 "df1"
#define DTC_FLAG_2 "df2"
#define MIN_CELL_VOLTAGE "mcv"
#define INPUT_SUPPLY_VOLTAGE "isv"
#define FAN_VOLTAGE "fvt"
#define CHECKSUM "cks"

const char* const telemetry_labels[ROWS][COLS] = {
		{	RELAY_STATE, RESERVE_TWOBYTE_FIELD, MAX_CELL_NUMBER, POPULATED_CELLS, FAILSAFE_STATUS, RESERVE_TWOBYTE_FIELD, PACK_SOC, CHECKSUM	}, // 0x100

		{	PACK_CCL, RESERVE_TWOBYTE_FIELD, PACK_DCL, RESERVE_TWOBYTE_FIELD, PACK_CURRENT, RESERVE_TWOBYTE_FIELD, PACK_DOD, CHECKSUM	}, // 0x101

		{	PACK_CCL_KW, RESERVE_TWOBYTE_FIELD, PACK_DCL_KW, RESERVE_TWOBYTE_FIELD, PACK_INST_VOLTAGE, RESERVE_TWOBYTE_FIELD, PACK_HEALTH, CHECKSUM	}, // 0x102

		{	PACK_OPEN_VOLTAGE, RESERVE_TWOBYTE_FIELD, PACK_AMPHOURS, RESERVE_TWOBYTE_FIELD, PACK_RESISTANCE, RESERVE_TWOBYTE_FIELD, HIGH_TEMPERATURE, CHECKSUM	}, // 0x103

		{	PACK_SUMMED_VOLTAGE, RESERVE_TWOBYTE_FIELD, MAX_PACK_VOLTAGE, RESERVE_TWOBYTE_FIELD, MIN_PACK_VOLTAGE, RESERVE_TWOBYTE_FIELD, FAN_SPEED, CHECKSUM	}, // 0x104

		{	TOTAL_PACK_CYCLES, RESERVE_TWOBYTE_FIELD, CURRENT_LIMITS_STATUS, RESERVE_TWOBYTE_FIELD, MAX_PACK_DCL, RESERVE_TWOBYTE_FIELD, HIGH_THERMISTOR_ID, CHECKSUM	}, // 0x105

		{	MAX_PACK_CCL, RESERVE_TWOBYTE_FIELD, AVERAGE_CURRENT, RESERVE_TWOBYTE_FIELD, LOW_CELL_VOLTAGE, RESERVE_TWOBYTE_FIELD, LOW_CELL_VOLTAGE_ID, CHECKSUM	}, // 0x106

		{	HIGH_CELL_VOLTAGE, RESERVE_TWOBYTE_FIELD, HIGH_CELL_VOLTAGE_ID, AVERAGE_CELL_VOLTAGE, RESERVE_TWOBYTE_FIELD, LOW_OPENCELL_VOLTAGE, RESERVE_TWOBYTE_FIELD, CHECKSUM	}, // 0x107

		{	LOW_OPENCELL_ID, HIGH_OPENCELL_VOLTAGE, RESERVE_TWOBYTE_FIELD, HIGH_OPENCELL_ID, AVERAGE_OPENCELL_VOLTAGE, RESERVE_TWOBYTE_FIELD, LOW_TEMPERATURE, CHECKSUM	}, // 0x108

		{	LOW_THERMISTOR_ID, AVERAGE_TEMPERATURE, INTERNAL_TEMPERATURE, REQUEST_FAN_SPEED, IGNORE_FIELD, IGNORE_FIELD, ADAPTIVE_SOC, CHECKSUM	}, // 0x109

		{	ADAPTIVE_AMPHOURS, RESERVE_TWOBYTE_FIELD, ADAPTIVE_TOTAL_CAPACITY, RESERVE_TWOBYTE_FIELD, MAX_CELL_VOLTAGE, RESERVE_TWOBYTE_FIELD, DTC_FLAG_1, CHECKSUM	}, // 0x110

		{	MIN_CELL_VOLTAGE, RESERVE_TWOBYTE_FIELD, INPUT_SUPPLY_VOLTAGE, RESERVE_TWOBYTE_FIELD, FAN_VOLTAGE, RESERVE_TWOBYTE_FIELD, DTC_FLAG_2, CHECKSUM	}, // 0x111
};

#define MAX_ITEM_IN_LIST 11

static Telemetry_Data *head = NULL;

static char list_num_items = 0;

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

	char *buffer = "{\"req\":\"hub.set\",\"product\":\"" NOTE_PRODUCT_ID "\",\"mode\":\"continuous\",\"outbound\":1}";

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

#ifdef IGNORE_CHECKSUM
	for (int i = 0; i < COLS - 1; ++i) {
#else
	for (int i = 0; i < COLS; ++i) {
#endif
		// Ignore field that is "x"
		if (strcmp((*labels)[i], IGNORE_FIELD) == 0) continue;

		// Check if this field requires 2 bytes to represent data
		if (i != COLS - 1) {
			// If we're not at the last column
			// Then we have to check if the next column relative to the current one is marked as "-"
			// Which means we have to take two bytes, not one byte
			if (strcmp((*labels)[i + 1], RESERVE_TWOBYTE_FIELD) == 0) {
				// (data[i + 1] << 8) | data[i] is for Little Endian format
				// If BMS is set to Big Endian format, reserve the order
				add_note_to_list((*labels)[i], (data[i + 1] << 8) | data[i]);

				i++;
			} else {
				add_note_to_list((*labels)[i], (int)data[i]);
			}
		} else {
			add_note_to_list((*labels)[i], (int)data[i]);
		}
	}

	return 0;
}

int telemetry_send_data_to_note(const char* label, const uint32_t value) {
	assert_param(is_telemetry_inited == 1);
	assert_param(label != NULL);

	add_note_to_list(label, (int)value);

	return 0;
}

int telemetry_sync_to_note() {
	assert_param(is_telemetry_inited == 1);

	if (head == NULL) {
		// Nothing to sync
		return 0;
	}

	char buffer[512];
	int offset = 0;

	// Start JSON
	offset += snprintf(&buffer[offset], sizeof(buffer) - offset, "{\"req\":\"note.add\",\"file\":\"data.qo\",\"sync\":true,\"body\":{");

	// Build JSON Body
	Telemetry_Data *ptr = head;

	while (ptr != NULL) {
		// Add comma if not the first field
		if (ptr != head) {
			offset += sprintf(&buffer[offset], ",");
		}

		offset += snprintf(&buffer[offset], sizeof(buffer) - offset, "\"%s\":%d", ptr->label, ptr->value);

		ptr = ptr->next;
	}

	// End JSON
	offset += snprintf(&buffer[offset], sizeof(buffer) - offset, "}}\n");

	// Check for overflow
	if (offset >= sizeof(buffer)) {
		ERR("Failed to send data to note: Buffer overflow!");

		return -1;
	}

	DBG("Message is: %s", buffer);

	// Transmit the final string over UART
	if (write_to_serial(buffer, offset) != HAL_OK) {
		return -1;
	}

	// Clean up the list
	delete_list();

	return 0;
}

static HAL_StatusTypeDef write_to_serial(const char* data, const int len) {
	return HAL_UART_Transmit(NOTE_SERIAL, (uint8_t*)data, len, HAL_MAX_DELAY);
}

static int add_note_to_list(const char* label, int value) {
	if (list_num_items >= MAX_ITEM_IN_LIST) return -1; // skip

	if (head == NULL) {
		Telemetry_Data *node = make_note(label, value);

		if (node == NULL) return -1;

		head = node;

		list_num_items++;

		return 0;
	}

	// Check to see if the hash table already has the key
	Telemetry_Data* ptr = head;
	Telemetry_Data* prev = NULL;

	while (ptr != NULL) {
		if (strcmp(ptr->label, label) == 0) {
			// Just update the existing data
			ptr->value = value;

			return 0;
		}

		prev = ptr;

		ptr = ptr->next;
	}

	// Add to the end of the linked list
	Telemetry_Data *node = make_note(label, value);

	DBG("Made node with label [%s] and value [%d]", label, value);

	if (node == NULL) return -1;

	prev->next = node;

	list_num_items++;

	return 0;
}

static Telemetry_Data* make_note(const char* label, const int value) {
	Telemetry_Data *node = (Telemetry_Data*)malloc(sizeof(Telemetry_Data));

	if (node == NULL) {
		ERR("Failed to allocate memory for node");

		return NULL;
	}

	node->label = (char*)malloc(strlen(label) + 1);

	if (node->label == NULL) {
		ERR("Failed to allocate memory for node");

		return NULL;
	}

	strcpy(node->label, label);

	node->value = value;

	node->next = NULL;

	return node;
}

static void delete_list() {
	if (head == NULL) return;

	Telemetry_Data* ptr = NULL;

	while (head != NULL) {
		ptr = head;

		head = head->next;

		free(ptr->label);

		free(ptr);
	}

	head = NULL;
}
