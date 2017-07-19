/*#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
*/
#include "HAL.h"
#include <string.h>
#include "mini-printf.h"


int get_touch(sensor_t sensor)
{
/*	Starts the touch sensor timer (TIM21) on the appropriate pin. Waits a bit for capture on pin. 
	Captures raw sensor value. Repeats for [samples] iterations. Blocking code. */

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
 	  /*	TIM21 prescaler (TIMx_PSC): */
	MMIO32((TIM21_BASE) + 0x28) = 0; //prescaler = clk
	/*	Enable capture */
	MMIO32((TIM21_BASE) + 0x20) |= (1<<0);
  	 	/*	Enable TIM21 counter: */
   	MMIO32((TIM21_BASE) + 0x00) |= (1<<0);
	/*	set pin pullup high */
	gpio_mode_setup(PORT_TOUCH, GPIO_MODE_AF, GPIO_PUPD_PULLUP, pin);
	/*	wait a few cycles to make sure we get the whole range */
	gpio_toggle(PORT_USART, PIN_USART_RX);
	for (i=0;i<10;i++)
	{
		__asm__("nop");
	}
	gpio_toggle(PORT_USART, PIN_USART_RX);
	return(MMIO32((TIM21_BASE) + 0x34)); //adds current input capture register value to accumulator
}

int get_slider_position(void)
{
	int val0, val1;
	val0 = get_touch(SENSOR0);
	val1 = get_touch(SENSOR1);
	if ((val1 - val0) < -4)
	{
		return 0;
	}
	return (4 - (val0 - val1));
}

int main(void)
{
	clock_setup();
	gpio_setup();
	tim_setup();
	setLED(0);
	systick_setup(100000);
	usart_setup();

	// debug
	gpio_mode_setup(PORT_USART, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, PIN_USART_RX);

	char strDisp[20];
	int i;
	for (i=0;i<20;i++)
	{
		strDisp[i] = 0;
	}
	int val;
	
	for(;;)
	{
		val = get_slider_position();
		gpio_mode_toggle(PORT_USART, PIN_USART_RX);
		setLED((val) * 100);

		mini_snprintf(strDisp, 20, "%d\r\n",val);
		usart_print(strDisp);
	
	}
}
