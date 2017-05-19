#ifndef HAL_H_
#define HAL_H_

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/nvic.h>

#define PORT_LED		GPIOB
#define PIN_LED			GPIO0 //TIM2_CH2/3

#define PORT_TOUCH		GPIOA
#define PIN_TOUCH0		GPIO10 //TIM21_CH1
#define PIN_TOUCH1		GPIO3 //TIM21_CH2

extern volatile uint8_t main_tick;
extern volatile uint8_t tick;
static const uint16_t gamma_lookup[1024];

void systick_setup(int xms);
void clock_setup(void);
void gpio_setup(void);
void tim_setup(void);

#endif
