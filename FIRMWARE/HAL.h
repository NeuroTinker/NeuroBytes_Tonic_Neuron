#ifndef HAL_H_
#define HAL_H_

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/nvic.h>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#define PORT_LED		GPIOB
#define PIN_LED			GPIO0 //TIM2_CH2/3

#define PORT_TOUCH		GPIOA
#define PIN_TOUCH0		GPIO10 //TIM21_CH1
#define PIN_TOUCH1		GPIO3 //TIM21_CH2

#define PORT_USART		GPIOB
#define PIN_USART_TX	GPIO6
#define PIN_USART_RX	GPIO7

static const uint16_t gamma_lookup[1024];

void usart_setup(void);
void usart_send(uint8_t word);
void systick_setup(int xms);
void clock_setup(void);
void gpio_setup(void);
void tim_setup(void);
void setLED(uint16_t val);

#endif
