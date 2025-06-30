/*
 * motor.h
 *
 *  Created on: Jun 17, 2025
 *      Author: RU Solar HighEnd 1
 */

#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_

#include "stm32f0xx_hal.h"
#include "utility.h"
#include "telemetry.h"
#include "can.h"

#define SPEED_LABEL "v" // use when about to send to NoteCard, it will be labeled as such
#define WHEEL_RADIUS 0.2794f // in meter
#define ONE_REV_TO_COUNTERS 100 // one revolution per 100 counters
#define TIME_PASSED 1 // in seconds
#define MOTOR_CAN_ID 0x10D

// Specify timer for motor
extern TIM_HandleTypeDef htim2;

#define MOTOR_TIMER_INSTANCE &htim2;

// Called when timer is triggered
void motor_on_timer_interrupt();

// Called when motor's gpio is triggered
void motor_on_gpio_interrupt();

static inline void send_speed_to_telemetry(const float speed);
static inline void send_speed_to_can(const float speed);
#endif /* INC_MOTOR_H_ */
