
#include "neuron.h"
#include "comm.h"

uint16_t input_pins[11] = {
    PIN_AXON1_IN
};

void neuronInit(neuron_t *n)
{
	uint8_t i;

	n->potential = 0;
	n->state = INTEGRATE;

	n->fire_time = 0;
	n->fire_potential = 0;

	n->hebb_time = 0;
	n->learning_state = NONE;

	n->leaky_current = 0;

	for (i=0;i<DENDRITE_COUNT;i++){
		n->dendrites[i].state = OFF;
		n->dendrites[i].current_value = 0;
		n->dendrites[i].type = EXCITATORY;
		n->dendrites[i].timestamp = 0;
		n->dendrites[i].pulse_time = 0;
		n->dendrites[i].alive_time = 0;
	}

	n->dendrites[0].magnitude = 12000;
	n->dendrites[1].magnitude = 8000;
	n->dendrites[2].magnitude = 5000;
	n->dendrites[3].magnitude = 2000;
	
	n->dendrite_ping_time[0] = 0;
	n->dendrite_ping_time[1] = 0;
	n->dendrite_ping_time[2] = 0;
	n->dendrite_ping_time[3] = 0;
	n->dendrite_ping_time[4] = 0;
	n->dendrite_ping_time[5] = 0;
	n->dendrite_ping_time[6] = 0;
	n->dendrite_ping_time[7] = 0;
	n->dendrite_ping_time[8] = 0;
	n->dendrite_ping_time[9] = 0;
}

void checkDendrites(neuron_t * n)
{
	uint8_t i;
	
}

void incrementHebbTime(neuron_t * n)
{
	uint8_t i;

	n->ms_count++;
	if (n->ms_count == n->time_multiple){
		n->hebb_time++;
		n->ms_count = 0;
	}

	if (n->hebb_time == UINT16_MAX - 1){
		n->hebb_time /= 2;
		for (i=0; i<DENDRITE_COUNT; i++){
			n->dendrites[i].timestamp /= 2;
		}
		n->time_multiple *= 2;
	}
}

void dendriteSwitchOff(dendrite_t *dendrite)
{
	dendrite->state = OFF;
	dendrite->pulse_time = 0;
	
	switch(dendrite->type){
		case EXCITATORY:
			dendrite->current_value += dendrite->magnitude;
			break;
		case INHIBITORY:
			dendrite->current_value -= dendrite->magnitude;
			break;
	}
}

void dendriteDecayStep(neuron_t * n)
{
	uint8_t i;

	for(i=0; i<DENDRITE_COUNT; i++){
		n->dendrites[i].current_value = (n->dendrites[i].current_value * 63 ) / 64;
	}
}

void membraneDecayStep(neuron_t * n)
{
	n->fire_potential = (n->fire_potential * 63) / 64;
}

int16_t calcNeuronPotential(neuron_t *n)
{
	uint8_t i;
	int16_t new_v = 0;
	for (i=0; i<DENDRITE_COUNT; i++){
		if (n->dendrites[i].state == ON){
			switch(n->dendrites[i].type){
				case EXCITATORY:
					new_v += n->dendrites[i].magnitude;
					break;
				case INHIBITORY:
					new_v -= n->dendrites[i].magnitude;
					break;
			}
		}
		new_v += n->dendrites[i].current_value; // each dendrite contributes its decay (*.current_value) and magnitude
	}
	return new_v;
}
