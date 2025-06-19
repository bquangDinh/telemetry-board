/*
 * motor.c
 *
 *  Created on: Jun 17, 2025
 *      Author: RU Solar HighEnd 1
 */

#include "motor.h"

static int distance_counter = 0;

void motor_on_timer_interrupt() {
	// One sec has passed
	// Calculate distance traveled
	float dist = CIRCLE_PERIMETER(WHEEL_RADIUS) * distance_counter / (float)ONE_REV_TO_COUNTERS;

	// Calculate speed
	float speed = dist / TIME_PASSED;

	DBG("Distance traveled is %d meters", ROUND_FLOAT(dist));

	DBG("Speed is (rounded up to) %d m/s", ROUND_FLOAT(speed));

	send_speed_to_can(speed);

	send_speed_to_telemetry(speed);

	// Reset the counter
	distance_counter = 0;
}

void motor_on_gpio_interrupt() {
	// Whenever the motor wakes the GPIO interrupt, increase counter
	distance_counter++;
}

static inline void send_speed_to_can(const float speed) {
	assert_param(speed >= 0);

	FloatByte value = {
			.f = speed
	};

	DBG("Sending speed to CAN with ID: %d", BOARD_ID);

	char msg[8] = { value.bytes[0], value.bytes[1], value.bytes[2], value.bytes[3], 0, 0, 0, 0 };

	HAL_StatusTypeDef status = send_can_message(msg);

	if (status == HAL_OK) {
		DBG("Sent speed successfully to CAN!");
	} else {
		ERR("Failed to send speed to CAN. Status is %d", status);
	}
}

static inline void send_speed_to_telemetry(const float speed) {
	assert_param(speed >= 0);

	uint32_t scaled = (uint32_t)(speed * SCALED_INT_TWO_DECIMAL_PRECISION);

	DBG("Sending scaled speed of (%ld) to telemetry with board id [%d]", scaled, BOARD_ID);

	if (telemetry_send_data_to_note(SPEED_LABEL, scaled) == 0) {
		DBG("Sent successfully!");
	} else {
		DBG("Failed to send data to Note Card");
	}
}
