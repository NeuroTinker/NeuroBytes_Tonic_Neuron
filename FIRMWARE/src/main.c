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
#define SEND_PING_TIME		50 // 80
#define BUTTON_PRESS_TIME	2

static uint32_t fingerprint[3] __attribute__((section (".fingerprint"))) __attribute__ ((__used__)) = {
	4, // device id
	1, // firmware version
	0  // unique id
};

int led;

int main(void)
{
	uint8_t		i;

	// counters
//	uint32_t	blink_time = 0; // counter for LED blink due to NID command
//	uint16_t	data_time = 0; // counter for sending data to NID
	uint16_t	send_ping_time = 0; // counter for sending a downstream ping
	uint16_t	last_fire_time = UINT16_MAX;
	uint16_t	current_fire_time = 0;
	uint16_t	fire_delay_time = 0;
	uint8_t		fire_flag = 0;

/*
	button debounce variables
	uint16_t	button_press_time = 0; 
	uint8_t		button_armed = 0;
	uint16_t	button_status = 0;
*/
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
	
	for(;;){
		if (main_tick == 1){ 
			main_tick = 0;
			// check to see if nid ping hasn't been received in last NID_PING_TIME ticks
			
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
			checkDendrites(&neuron);
			// current membrane potential comes from dendrites and any left over firing potential
			neuron.potential = calcNeuronPotential(&neuron);
			neuron.potential += neuron.fire_potential;
			neuron.fire_potential += neuron.leaky_current;
			membraneDecayStep(&neuron);
			dendriteDecayStep(&neuron);
			current_fire_time += 1;

			// if membrane potential is greater than threshold, fire
			if (neuron.potential > MEMBRANE_THRESHOLD){
				// fire for determined pulse width
				neuron.state = FIRE;
				neuron.fire_potential = HYPERPOLARIZATION;
				neuron.fire_time = PULSE_LENGTH;
				for (i=0; i<1; i++){
					neuron.dendrites[i].current_value = 0;
					neuron.dendrites[i].state = OFF;
				}
				// send downstream pulse
				fire_delay_time = FIRE_DELAY_TIME;
				fire_flag = 1;
			}

			if (fire_delay_time > 0){
				fire_delay_time -= 1;
			} else if (fire_flag == 1){
				fire_flag = 0;
				addWrite(DOWNSTREAM_BUFF, PULSE_MESSAGE);
			}

			led = get_slider_position() * 5;
			if (led > 0){ // >
				neuron.leaky_current = led * 5 / 12;
			}
			
			/*
			if (led > -1)
			{
				setLED(0, 0, led);
			}
			*/
			
			if (neuron.state == FIRE){
				neuron.fire_time -= 1;
				if (neuron.fire_time == 0){
					neuron.state = INTEGRATE;
				}
				setLED(9600,9600,9600);
			} else if (neuron.state == INTEGRATE){
				if (neuron.potential > 10000){
					setLED(200,0,0);
				} else if (neuron.potential > 0){
					setLED(neuron.potential / 50, 200 - (neuron.potential / 50), 0);
				} else if (neuron.potential < -10000){
					setLED(0,0, 200);
				} else if (neuron.potential < 0){
					setLED(0, 200 + (neuron.potential / 50), -1 * neuron.potential / 50);
				} else{
					setLED(0,200,0);
				}
			}
		
			if (neuron.leaky_current < 1000){
				setLED(0,100,neuron.leaky_current / 5);
			}
		}
	}
}
