/*
 * ds18b20.h
 *
 *  Created on: Jun 2, 2025
 *      Author: buiqu
 */

#ifndef SRC_DS18B20_H_
#define SRC_DS18B20_H_

#include "stm32f0xx_hal.h"
#include "utility.h"

#define DS18B20_DATA_LEN 9

#define DS18B20_STEP_12BIT		0.0625
#define DS18B20_STEP_11BIT		0.125
#define DS18B20_STEP_10BIT		0.25
#define DS18B20_STEP_9BIT		0.5

typedef enum {
	DS18B20_Resolution_9bits = 9,
	DS18B20_Resolution_10bits = 10,
	DS18B20_Resolution_11bits = 11,
	DS18B20_Resolution_12bits = 12
} DS18B20_Resolution_t;

#define SKIP_ROM_CMD 0xCC
#define CONVERT_T_CMD 0x44
#define READ_SCRATCHPAD_CMD 0xBE

static inline void set_gpio_output(GPIO_TypeDef  *port);
static inline void set_gpio_input(GPIO_TypeDef  *port);

static uint8_t ds18b20_reset(GPIO_TypeDef  *port, const uint32_t pin);
static void ds18b20_writebit(GPIO_TypeDef  *port, const uint32_t pin, uint8_t bit);
static uint8_t ds18b20_readbit(GPIO_TypeDef  *port, const uint32_t pin);
static void ds18b20_writebyte(GPIO_TypeDef  *port, const uint32_t pin, uint8_t byte);
static uint8_t ds18b20_readbyte(GPIO_TypeDef  *port, const uint32_t pin);
static inline void ds18b20_output_low(GPIO_TypeDef *port, const uint32_t pin);
static inline void ds18b20_output_high(GPIO_TypeDef *port, const uint32_t pin);
static inline uint8_t ds18b20_direct_gpio_read(GPIO_TypeDef *port, const uint32_t pin);

void ds18b20_start_sensor(GPIO_TypeDef *port, const uint32_t pin);

float ds18b20_readtemp(GPIO_TypeDef *port, const uint32_t pin);

void test_gpio(GPIO_TypeDef *port, const uint32_t pin);
#endif /* SRC_DS18B20_H_ */
