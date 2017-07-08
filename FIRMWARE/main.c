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

int led;

int main(void)
{
	uint8_t		i;

	// counters
//	uint32_t	blink_time = 0; // counter for LED blink due to NID command
//	uint16_t	data_time = 0; // counter for sending data to NID
	uint16_t	send_ping_time = 0; // counter for sending a downstream ping

	// button debounce variables
//	uint16_t	button_press_time = 0; 
//	uint8_t		button_armed = 0;
//	uint16_t	button_status = 0;

	// current channel used to communicate to NID (e.g. CH. 1). 0 if neuron has not been selected by NID
//	uint32_t	nid_channel = 0b000;

//	uint32_t	message = 0; // staging variable for constructing messages to send to the communications routine

	// initialize neuron
	neuron_t 	neuron;
	neuronInit(&neuron);

	// initialize communication buffers
	commInit();

	clock_setup();
	gpio_setup();
	tim_setup();
	setLED(0,0,0);
	systick_setup(10);
	
	for(;;)
	{
//		if (main_tick == 1){ 
{	// ZF -- I commented main_tick as it seems to be causing >1s delays
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

			led = get_slider_position() * 5;
			
			if (led > -1)
			{
				setLED(0, 0, led);
			}

			//neuron.leaky_current = 5 * val;
		
		}
	}
}
