#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/exti.h>


#include "comm.h"
#include "HAL.h"
#include "neuron.h"

#define BLINK_TIME			100
#define DATA_TIME			10
#define DEND_PING_TIME		200 // 1000 ms
#define	NID_PING_TIME		200 // 1000 ms
#define SEND_PING_TIME		80 // 80
#define BUTTON_PRESS_TIME	2


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
	
	//	Set pin as AF/TIM21 channel with pulldown on and wait for it to go low 
	gpio_mode_setup(PORT_TOUCH, GPIO_MODE_AF, GPIO_PUPD_PULLDOWN, pin);
	gpio_set_af(PORT_TOUCH, GPIO_AF0, pin);
	
   	 
   	 MMIO32((RCC_BASE) + 0x34) |= (1<<2); //Enable TIM21
   	 MMIO32((RCC_BASE) + 0x24) |= (1<<2); //Set reset bit, TIM21
   	 MMIO32((RCC_BASE) + 0x24) &= ~(1<<2); //Clear reset bit, TIM21
	
  	//	TIM21 control register 1 (TIMx_CR1):
	//	Edge-aligned (default setting)
  	//	No clock division (default setting)
  	//	Up direction (default setting)

	//	TIM21 capture/compare mode register (TIMx_CCMR1)
	MMIO32((TIM21_BASE) + 0x18) |= (1<<sensor); //Compare/Capture 1 selection (bit 0 for PIN_TOUCH0, bit1 for PIN_TOUCH1)
	//	IC1F left at 0000, so sampling is done at clock speed with no digital filtering
	//	CC1P/CCNP left at 00, so trigger happens on rising edge
	//	IC1PS bits left at 00, no input prescaling	
	
	MMIO32((TIM21_BASE) + 0x20) |= (1<<0); // enable capture
   	MMIO32((TIM21_BASE) + 0x00) |= (1<<0); // enable TIM21 counter
	gpio_mode_setup(PORT_TOUCH, GPIO_MODE_AF, GPIO_PUPD_PULLUP, pin); // set pin pullup hight
	
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
	uint8_t		i;

	// counters
	uint32_t	blink_time = 0; // counter for LED blink due to NID command
	uint16_t	data_time = 0; // counter for sending data to NID
	uint16_t	send_ping_time = 0; // counter for sending a downstream ping

	// button debounce variables
	uint16_t	button_press_time = 0; 
	uint8_t		button_armed = 0;
	uint16_t	button_status = 0;

	// current channel used to communicate to NID (e.g. CH. 1). 0 if neuron has not been selected by NID
	uint32_t	nid_channel = 0b000;

	uint32_t	message = 0; // staging variable for constructing messages to send to the communications routine

	// initialize neuron
	neuron_t 	neuron;
	neuronInit(&neuron);

	// initialize communication buffers
	commInit();

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

			// check to see if nid ping hasn't been received in last NID_PING_TIME ticks
			if (nid_ping_time++ > NID_PING_TIME){
				// nid no longer connected
				nid_keep_alive = NID_PING_KEEP_ALIVE; // reset nid_keep_alive
				nid_pin = 0; // clear the nid pin
				nid_pin_out = 0;
			}
			
			// send a downstream ping every SEND_PING_TIME ticks
			if (send_ping_time++ > SEND_PING_TIME){
				// send downstream ping through axon
				addWrite(DOWNSTREAM_BUFF, DEND_PING);
				send_ping_time = 0;
			}

			/*
				Check dendrites for pings and adjut pins accordingly.
				Also check dendrites for pulses and calculate new membrane potential.
			*/

			// decay the firing potential
			membraneDecayStep(&neuron);

			// current membrane potential comes from dendrites and any left over firing potential
			//neuron.potential = calcNeuronPotential(&neuron);
			neuron.potential = neuron.fire_potential;
			neuron.fire_potential += neuron.leaky_current;

			// if membrane potential is greater than threshold, fire
			if (neuron.potential > MEMBRANE_THRESHOLD){
				// fire for determined pulse width
				neuron.state = FIRE;
				neuron.fire_potential = HYPERPOLARIZATION;
				neuron.fire_time = PULSE_LENGTH;
				// send downstream pulse
				addWrite(DOWNSTREAM_BUFF, PULSE_MESSAGE);
			}

			slider_position = get_slider_position();
			
			current_slider_position = slider_position;
			if (current_slider_position + prev_slider_position != prev_slider_sum){
				if (current_slider_position - prev_slider_position == 1){
					val += 5;
					//gpio_toggle(PORT_AXON1_EX, PIN_AXON1_EX);
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
 
			//neuron.leaky_current = 5 * val;
		
			setLED(val);
		}
	}
}
