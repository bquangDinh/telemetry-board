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

// More details: https://rutgers-solar-car.atlassian.net/wiki/spaces/RSC/pages/22511621/BMS+CAN+Message+Structures
#define COLS 8
#define ROWS 9
#define NOTE_PRODUCT_ID "com.gmail.rusolarcarclub:rsctest"

#define NOTE_HUB_SET_CMD "hub.set"
#define NOTE_CARD_ADD_REQUEST_CMD "note.add"
#define NOTE_HUB_OPERATION_MODE "continuous"
#define IGNORE_CHECKSUM

struct Telemetry_Data {
	char* label;
	int value;
	struct Telemetry_Data* next;
};

typedef struct Telemetry_Data Telemetry_Data;

extern const char* const telemetry_labels[ROWS][COLS];

/**
 * @brief Will reconfigure the BlueNoteCard to the right values before it starts sending data to Note Hub (just to make sure its configuration is correctly set)
 *
 * @retval 0 successfull
 * @retval -1 faild to init
 *
 * @warn Must init USART communication in main.c before calling any functions inside this file. Example: https://github.com/blues/note-stm32g0/blob/master/Src/main.c
 */
int telemetry_init();

/**
 * @brief Send BMS data to note. This will send all 8 fields of a BMS data to Note
 * Each CAN message has 8-byte data, each byte contains a field to a measurement
 * This function will send all 8 measurements (specified in 1-byte field) using pre-defined labels to Note
 *
 * @param[in] row		the row location of the labels defined in telemetry_labels
 * @param[in] data			data pointer to the array of data to send to note
 *
 * @retval 0 successful
 * @retval -1 failed to send
 */
int telemetry_send_bms_to_note(int row, const uint8_t* data);

int telemetry_send_data_to_note(const char* label, const uint32_t value);

int telemetry_sync_to_note();

/**
 * @brief Send data over USART
 */
static HAL_StatusTypeDef write_to_serial(const char* data, const int len);

/**
 * @brief Add Note data to a linked list, NoteCard will then pack all data inside the list to send all at once
 */
static int add_note_to_list(const char* label, int value);

static Telemetry_Data* make_note(const char* label, const int value);

static void delete_list();
#endif /* INC_TELEMETRY_H_ */
