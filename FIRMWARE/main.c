/*#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
*/
#include "HAL.h"
#include <string.h>
#include "mini-printf.h"


int get_touch(int sensor)
{
	int i;
	if (sensor == 0)
	{
		gpio_mode_setup(PORT_TOUCH, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, PIN_TOUCH0);

		for (i=0;i<10000;i++)
		{
			__asm__("nop");
		}

		gpio_mode_setup(PORT_TOUCH, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, PIN_TOUCH0);
		if (gpio_get(PORT_TOUCH, PIN_TOUCH0) > 0) 
		{
			return 1;
		}
		for (i=0;i<1000;i++)
		{	
			if(gpio_get(PORT_TOUCH, PIN_TOUCH0) > 0) 
			{
				return i;
			}
		}
		return -1;
	}
}

int main(void)
{
	clock_setup();
	gpio_setup();
	tim_setup();
	setLED(0);
	systick_setup(100000);
	usart_setup();

	char strDisp[20];
	int i;
	for (i=0;i<20;i++)
	{
		strDisp[i] = 0;
	}
	int val = 10;	
	for(;;)
	{
		val = get_touch(0);
		setLED(val);
		mini_snprintf(strDisp, 20, "%d\r\n", val);
		usart_print(strDisp);
	
		for (i=0;i<800000;i++)
	{
			__asm__("nop");
		}
	}
}
