#ifndef HAL_H_
#define HAL_H_

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/usart.h>

#include "comm.h"

#define NUM_INPUTS 4
#define HAS_AXONS   true
#define HAS_DENDS   true
#define NUM_AXONS   2
#define NUM_DENDS   1
#define COMPLIMENTARY_I(i)  i + (i % 2) - ((i+1) % 2)
#define IS_EXCITATORY(i)        (i % 2)

#define LPUART1         LPUART1_BASE

#define PORT_LPUART1_TX GPIOA
#define PORT_LPUART1_RX GPIOA

#define PIN_LPUART1_RX  GPIO13
#define PIN_LPUART1_TX  GPIO14

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
#define PORT_AXON2_EX		GPIOB
#define PORT_AXON2_IN		GPIOA

#define PORT_DEND1_EX		GPIOB
#define PORT_DEND1_IN		GPIOB

#define PIN_AXON1_EX		GPIO6
#define PIN_AXON1_IN		GPIO5
#define PIN_AXON2_EX		GPIO0
#define PIN_AXON2_IN		GPIO7

#define PIN_DEND1_IN		GPIO3
#define PIN_DEND1_EX		GPIO4

#define PORT_TOUCH0_SENSE	GPIOA
#define PORT_TOUCH1_SENSE	GPIOA
#define PORT_TOUCH0_RES		GPIOA
#define PORT_TOUCH1_RES		GPIOB

#define PIN_TOUCH0_SENSE	GPIO10	//TIM21_CH1
#define PIN_TOUCH1_SENSE	GPIO9	//TIM21_CH2
#define PIN_TOUCH0_RES		GPIO8
#define PIN_TOUCH1_RES		GPIO1

#define ACTIVATE_INPUT(I, PIN)   active_input_pins[(I)] = PIN; active_input_tick[(I)] = (read_tick + 2) % 3

extern const uint16_t complimentary_pins[NUM_INPUTS];
extern const uint32_t complimentary_ports[NUM_INPUTS];
extern volatile uint16_t active_input_pins[NUM_INPUTS];
extern uint32_t active_input_ports[NUM_INPUTS];
extern volatile uint16_t active_output_pins[NUM_INPUTS];
extern uint32_t active_output_ports[NUM_INPUTS];
extern volatile uint8_t active_input_tick[NUM_INPUTS];

extern volatile uint8_t dendrite_pulse_flag[NUM_INPUTS];
extern volatile uint8_t dendrite_ping_flag[NUM_INPUTS];

typedef enum {
    SENSOR0,
    SENSOR1
} sensor_t;

typedef struct{
    uint8_t device_type;
    uint32_t unique_id;
    uint8_t firmware_version;
} fingerprint_t;

extern volatile int sensor0_time;
extern volatile int sensor1_time;
extern volatile uint8_t touch_tick;
extern volatile uint8_t main_tick;
extern volatile uint8_t read_tick;
extern volatile uint32_t main_tick_count;
extern volatile uint16_t tick;
static const uint16_t gamma_lookup[1024];

void systick_setup(void);
void clock_setup(void);
void gpio_setup(void);
void tim_setup(void);
void lpuart_setup(void);
void LEDFullWhite(void);
void setLED(uint16_t r, uint16_t g, uint16_t b);
void setAsInput(uint32_t port, uint32_t pin);
void setAsOutput(uint32_t port, uint32_t pin);
void start_touch(sensor_t sensor);
int get_touch(sensor_t sensor);
int get_slider_position(void);
bool checkVersion(uint32_t device_id, uint32_t version);

#endif
