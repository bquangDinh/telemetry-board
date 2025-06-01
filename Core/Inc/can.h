/*
 * can.h
 *
 *  Created on: Jun 1, 2025
 *      Author: buiqu
 */

#ifndef INC_CAN_H_
#define INC_CAN_H_

#include "stm32f0xx_hal.h"

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

#endif /* INC_CAN_H_ */
