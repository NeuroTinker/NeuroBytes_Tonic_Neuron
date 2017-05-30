/*#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
*/
#include "HAL.h"
#include <string.h>
#include "mini-printf.h"


void start_touch(sensor_t sensor)
{
	int i, pin, tim_af;

	if (sensor == SENSOR0) 
	{
		pin = PIN_TOUCH0;
	}
	else if (sensor == SENSOR1)
	{
		pin = PIN_TOUCH1;
	}

	/*	Set pin as AF/TIM21 channel with pulldown on and wait for it to go low */
	gpio_mode_setup(PORT_TOUCH, GPIO_MODE_AF, GPIO_PUPD_PULLDOWN, pin);
	gpio_set_af(PORT_TOUCH, GPIO_AF0, pin);
	
   	 /*	Enable and reset TIM21: */
   	 MMIO32((RCC_BASE) + 0x34) |= (1<<2); //Enable TIM21
   	 MMIO32((RCC_BASE) + 0x24) |= (1<<2); //Set reset bit, TIM21
   	 MMIO32((RCC_BASE) + 0x24) &= ~(1<<2); //Clear reset bit, TIM21
	
  	/*	TIM21 control register 1 (TIMx_CR1): */
	//	Edge-aligned (default setting)
  	//	No clock division (default setting)
  	//	Up direction (default setting)

	/*	TIM21 capture/compare mode register (TIMx_CCMR1) */
	MMIO32((TIM21_BASE) + 0x18) |= (1<<sensor); //Compare/Capture 1 selection (bit 0 for PIN_TOUCH0, bit1 for PIN_TOUCH1)
	//	IC1F left at 0000, so sampling is done at clock speed with no digital filtering
	//	CC1P/CCNP left at 00, so trigger happens on rising edge
	//	IC1PS bits left at 00, no input prescaling	

	/*	Enable capture */
	MMIO32((TIM21_BASE) + 0x20) |= (1<<0);
  	 	/*	Enable TIM21 counter: */
   	MMIO32((TIM21_BASE) + 0x00) |= (1<<0);
	/*	set pin pullup high */
	gpio_mode_setup(PORT_TOUCH, GPIO_MODE_AF, GPIO_PUPD_PULLUP, pin);
}

int get_touch(sensor_t sensor)
{
	return(MMIO32((TIM21_BASE) + 0x34)); //adds current input capture register value to accumulator
}

int get_slider_position(void)
{

	if ((sensor1_time - sensor0_time) < -4)
	{
		return 0;
	}
	return (4 - (sensor0_time - sensor1_time));
}

int main(void)
{
	clock_setup();
	gpio_setup();
	tim_setup();
	setLED(0);
	systick_setup(100);
	
	int val = 0;
	int slider_position;
	int prev_slider_position = 0;
	int current_slider_position = 0;
	int prev_slider_movement = 0;
	int prev_slider_sum = 0;
	uint8_t prev_slider_time;
	uint8_t current_slider_time;
	
	for(;;)
	{
		if (main_tick == 1){

			main_tick = 0;

			slider_position = get_slider_position();
			
			current_slider_position = slider_position;
			if (current_slider_position + prev_slider_position != prev_slider_sum){
				if (current_slider_position - prev_slider_position == 1){
					val += 5;
					gpio_toggle(PORT_AXON1_EX, PIN_AXON1_EX);
					prev_slider_sum = current_slider_position + prev_slider_position;
				} else if (current_slider_position - prev_slider_position == -1){
					val -= 5;
					prev_slider_sum = current_slider_position + prev_slider_position;
				}
			}
			prev_slider_position = current_slider_position;

			if (val < 0){
				val = 0;
			} else if (val > 200){
				val = 200;
			}

			
			/*
			if (slider_position == current_slider_position){
				current_slider_time++;
			} else if (slider_position == prev_slider_position){
				current_slider_time = 0;
			} else{
				prev_slider_position = current_slider_position;
				prev_slider_time = current_slider_time;
				current_slider_position = slider_position;
			}
			*/

			/*
				Change val if slider position:
				1. had previous and current values for at least 1 ms
				2. previous and current values are 1 apart
			*/
		
			setLED(val);
		}
	}
}
