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

static Telemetry_Data *head = NULL;

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
		add_note_to_list((*labels)[i], (int)data[i]);
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
	if (head == NULL) {
		Telemetry_Data *node = make_note(label, value);

		if (node == NULL) return -1;

		head = node;

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

		free(ptr);
	}
}
