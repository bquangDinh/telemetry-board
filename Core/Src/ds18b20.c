/*
 * ds18b20.c
 *
 *  Created on: Jun 2, 2025
 *      Author: buiqu
 */

#include "ds18b20.h"

static inline void set_gpio_output(GPIO_TypeDef *port) {
//	GPIO_InitTypeDef GPIO_InitStruct = {0};
//
//	GPIO_InitStruct.Pin = pin;
//
//	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//
//	// Already been pulled by using external resistor
//	GPIO_InitStruct.Pull = GPIO_NOPULL;
//
//	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//
//	HAL_GPIO_Init(port, &GPIO_InitStruct);

	port->MODER &= ~(0x3 << (5 * 2));  // Clear MODER5 bits
	port->MODER |=  (0x1 << (5 * 2));  // Set MODER5 = 01 (output)
}

static inline void set_gpio_input(GPIO_TypeDef *port) {
//	GPIO_InitTypeDef GPIO_InitStruct = {0};
//
//	GPIO_InitStruct.Pin = pin;
//
//	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//
//	// Already been pulled by using external resistor
//	GPIO_InitStruct.Pull = GPIO_NOPULL;
//
//	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//
//	HAL_GPIO_Init(port, &GPIO_InitStruct);

	port->MODER &= ~(0x3 << (5 * 2));  // Set MODER5 = 00 (input)
}

static inline void ds18b20_output_low(GPIO_TypeDef *port, const uint32_t pin) {
	port->BSRR = pin << 16; // Reset the 1-Wire pin
}

static inline void ds18b20_output_high(GPIO_TypeDef *port, const uint32_t pin) {
	port->BSRR = pin; // Set the 1-Wire pin
}

static inline uint8_t ds18b20_direct_gpio_read(GPIO_TypeDef *port, const uint32_t pin) {
	return (port->IDR & pin) ? 1 : 0;
}

static uint8_t ds18b20_reset(GPIO_TypeDef *port, const uint32_t pin) {
//	uint8_t retries = 125;
	uint8_t presence;

//	set_gpio_input(port);

//	// wait until the wire is high... just in case
//	do {
//		if (--retries == 0) return 0;
//		delay_us(2);
//	} while (ds18b20_direct_gpio_read(port, pin) != GPIO_PIN_SET);

	set_gpio_output(port);

	ds18b20_output_low(port, pin);

	delay_us(480);

	set_gpio_input(port);

	// If the device is active on the bus, the pin should be low
	presence = ds18b20_direct_gpio_read(port, pin) == GPIO_PIN_RESET;

	delay_us(410);

	return presence;
}

static void ds18b20_writebit(GPIO_TypeDef *port, const uint32_t pin, uint8_t bit) {
	set_gpio_output(port);

	ds18b20_output_low(port, pin);

		if (bit) {
			// Write '1': Release early
			delay_us(10);                      // Pull low for 5us
			ds18b20_output_high(port, pin);
			delay_us(55);                     // Wait until slot end
		} else {
			// Write '0': Hold low entire slot
			delay_us(65);                     // Keep low
			ds18b20_output_high(port, pin);        // Release after full slot
			delay_us(5);                      // Recovery
		}
}

static uint8_t ds18b20_readbit(GPIO_TypeDef *port, const uint32_t pin) {
	uint8_t bit = 0;

	set_gpio_output(port); // set this pin to OUTPUT mode

	ds18b20_output_low(port, pin);

	delay_us(3);

	set_gpio_input(port); // release the bus

	delay_us(10);

	bit = ds18b20_direct_gpio_read(port, pin);

	delay_us(53);

	return bit;
}

static void ds18b20_writebyte(GPIO_TypeDef *port, const uint32_t pin, uint8_t byte) {
	uint8_t bitMask;

	for (bitMask = 0x01; bitMask; bitMask <<= 1) {
		ds18b20_writebit(port, pin, (bitMask & byte) ? 1 : 0);
	}
}

static uint8_t ds18b20_readbyte(GPIO_TypeDef *port, const uint32_t pin) {
	 uint8_t bitMask;
	 uint8_t r = 0;

	 for (bitMask = 0x01; bitMask; bitMask <<= 1) {
		 if ( ds18b20_readbit(port, pin)) r |= bitMask;
	 }

	 return r;
}

void ds18b20_start_sensor(GPIO_TypeDef *port, const uint32_t pin) {
	set_gpio_output(port);

		ds18b20_output_low(port, pin);

		delay_us(480);

		set_gpio_input(port);

	delay_us(80);

	ds18b20_direct_gpio_read(port, pin);

	delay_us(400);
}

float ds18b20_readtemp(GPIO_TypeDef *port, const uint32_t pin) {
	uint8_t data[DS18B20_DATA_LEN];
	uint32_t temperature;
	float result;

	// Reset to start a transaction
	if (ds18b20_reset(port, pin) == 0) {
		DBG("[DS18B20] Failed to reset");
		return -1000;
	}

	// Start conversion on the sensor(s)
	ds18b20_writebyte(port, pin, SKIP_ROM_CMD);

	ds18b20_writebyte(port, pin, CONVERT_T_CMD);

	// Wait until the conversion is done
	HAL_Delay(850);  // wait full time

//	// Check if the bus is released
//	if (!ds18b20_readbit(port, pin)) {
//		ERR("Bus is busy!");
//		return -1000;
//	}

	// Start another transaction
	if (ds18b20_reset(port, pin) == 0) {
			DBG("[DS18B20] Failed to reset");
			return -1000;
		}

	ds18b20_writebyte(port, pin, SKIP_ROM_CMD);

	ds18b20_writebyte(port, pin, READ_SCRATCHPAD_CMD);

	for (int i = 0; i < DS18B20_DATA_LEN; ++i) {
		data[i] = ds18b20_readbyte(port, pin);
	}

	if (ds18b20_reset(port, pin) == 0) {
			DBG("[DS18B20] Failed to reset");
			return -1000;
		}

	temperature = (data[1] << 8) | data[0];

	result = temperature * 0.0625f;

	for (int i = 0; i < DS18B20_DATA_LEN; ++i) {
	    printf("[%d] = 0x%02X | ", i, data[i]);
	}

	printf("\n");

	return result;
}

void test_gpio(GPIO_TypeDef *port, const uint32_t pin) {
	set_gpio_output(port);
	    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
	    HAL_Delay(500);
	    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
	    HAL_Delay(500);
}
