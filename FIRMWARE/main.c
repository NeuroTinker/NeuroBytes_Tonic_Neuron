/*#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
*/
#include "HAL.h"
#include <string.h>
#include "mini-printf.h"

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
		setLED(val);
		mini_snprintf(strDisp, 20, "%d\r\n", val);
		usart_print(strDisp);
		if (val < 1023) 
		{
			val ++;
		}
		else 
		{
			val = 0;
		}
	}
}
