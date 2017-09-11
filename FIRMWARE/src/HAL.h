#ifndef HAL_H_
#define HAL_H_

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/exti.h>

#include "comm.h"

#define PORT_R_LED			GPIOA
#define PORT_G_LED			GPIOA
#define PORT_B_LED			GPIOA
#define PIN_R_LED			GPIO1 //TIM2_CH2
#define PIN_G_LED			GPIO0 //TIM2_CH1
#define PIN_B_LED			GPIO2 //TIM2_CH3

#define PORT_IDENTIFY		GPIOB
#define PIN_IDENTIFY		GPIO7

#define PORT_AXON1_EX		GPIOA
#define PORT_AXON1_IN		GPIOA
#define PORT_AXON2_EX		GPIOA
#define PORT_AXON2_IN		GPIOA

#define PORT_DEND1_EX		GPIOB
#define PORT_DEND1_IN		GPIOB

#define PIN_AXON1_EX		GPIO4
#define PIN_AXON1_IN		GPIO3
#define PIN_AXON2_EX		GPIO6
#define PIN_AXON2_IN		GPIO5

#define PIN_DEND1_IN		GPIO4
#define PIN_DEND1_EX		GPIO3

#define PORT_TOUCH0_SENSE	GPIOA
#define PORT_TOUCH1_SENSE	GPIOA
#define PORT_TOUCH0_RES		GPIOA
#define PORT_TOUCH1_RES		GPIOB

#define PIN_TOUCH0_SENSE	GPIO10	//TIM21_CH1
#define PIN_TOUCH1_SENSE	GPIO9	//TIM21_CH2
#define PIN_TOUCH0_RES		GPIO8
#define PIN_TOUCH1_RES		GPIO1

typedef enum {
    SENSOR0,
    SENSOR1
} sensor_t;

extern volatile int sensor0_time;
extern volatile int sensor1_time;
extern volatile uint8_t touch_tick;
extern volatile uint8_t main_tick;
extern volatile uint8_t read_tick;
extern volatile uint32_t main_tick_count;
extern volatile uint8_t tick;
static const uint16_t gamma_lookup[1024];

void systick_setup(int xms);
void clock_setup(void);
void gpio_setup(void);
void tim_setup(void);
void LEDFullWhite(void);
void setLED(uint16_t r, uint16_t g, uint16_t b);
void setAsInput(uint32_t port, uint32_t pin);
void setAsOutput(uint32_t port, uint32_t pin);
void start_touch(sensor_t sensor);
int get_touch(sensor_t sensor);
int get_slider_position(void);

#endif
