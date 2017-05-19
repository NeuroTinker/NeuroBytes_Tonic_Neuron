/*#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
*/
#include "HAL.h"

int main(void)
{
	clock_setup();
	gpio_setup();
	tim_setup();
	setLED(0);
	systick_setup(100000);
	usart_setup();


	int val = 10;	
	for(;;)
	{
		setLED(val);
		usart_send(10);
	}
}
