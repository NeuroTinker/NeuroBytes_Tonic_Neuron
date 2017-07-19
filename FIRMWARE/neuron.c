
#include "neuron.h"
#include "comm.h"

uint16_t input_pins[2] = {
	PIN_DEND1_EX,
	PIN_DEND1_IN};

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

	for (i = 0; i < DENDRITE_COUNT; i++)
	{
		n->dendrites[i].state = OFF;
		n->dendrites[i].current_value = 0;
		n->dendrites[i].type = EXCITATORY;
		n->dendrites[i].timestamp = 0;
		n->dendrites[i].pulse_time = 0;
		n->dendrites[i].alive_time = 0;
	}

	n->dendrites[0].magnitude = 18000;
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

void checkDendrites(neuron_t *n)
{
	uint8_t i;

	for (i = 0; i < 2; i++)
	{

		// check if dendrite has received a new ping
		if (dendrite_ping_flag[i] != 0)
		{

			dendrite_ping_flag[i] = 0;

			n->dendrite_ping_time[i] = DEND_PING_TIME;
		}
		else if (n->dendrite_ping_time[i] == 1)
		{
			// dendrite ping has expired; reset the dendrite to inputs
			if (i % 2 == 0)
			{
				// i=1,3,5,7,9
				setAsInput(active_input_ports[1], complimentary_pins[0]);
				active_output_pins[1] = 0;
			}
			else
			{
				// i=0,4,6,8,10
				setAsInput(active_input_ports[0], complimentary_pins[1]);
				active_output_pins[0] = 0;
			}
		}

		if (n->dendrite_ping_time[i] > 0)
		{
			n->dendrite_ping_time[i] -= 1;
		}

		// check if dendrite has received a pulse
		if (dendrite_pulse_flag[i] != 0)
		{
			dendrite_pulse_flag[i] = 0;
			switch (input_pins[i])
			{
			case PIN_DEND1_EX:
				n->dendrites[0].type = EXCITATORY;
				n->dendrites[0].state = ON;
				n->dendrites[0].pulse_time = 0;
				break;
			case PIN_DEND1_IN:
				n->dendrites[0].type = INHIBITORY;
				n->dendrites[0].state = ON;
				n->dendrites[0].pulse_time = 0;
				break;

			default:
				break;
			}
		}
	}

	for (i = 0; i < 1; i++)
	{
		// switch dendrite off when pulse has expired
		if (n->dendrites[i].state == ON)
		{
			if (n->dendrites[i].pulse_time == 0)
				n->dendrites[i].timestamp = 0;
			n->dendrites[i].pulse_time += 1;
			if (n->dendrites[i].pulse_time >= PULSE_LENGTH)
			{
				dendriteSwitchOff(&(n->dendrites[i]));
			}
		}
	}
}

void incrementHebbTime(neuron_t *n)
{
	uint8_t i;

	n->ms_count++;
	if (n->ms_count == n->time_multiple)
	{
		n->hebb_time++;
		n->ms_count = 0;
	}

	if (n->hebb_time == UINT16_MAX - 1)
	{
		n->hebb_time /= 2;
		for (i = 0; i < DENDRITE_COUNT; i++)
		{
			n->dendrites[i].timestamp /= 2;
		}
		n->time_multiple *= 2;
	}
}

void dendriteSwitchOff(dendrite_t *dendrite)
{
	dendrite->state = OFF;
	dendrite->pulse_time = 0;

	switch (dendrite->type)
	{
	case EXCITATORY:
		dendrite->current_value += dendrite->magnitude;
		break;
	case INHIBITORY:
		dendrite->current_value -= dendrite->magnitude;
		break;
	}
}

void dendriteDecayStep(neuron_t *n)
{
	uint8_t i;

	for (i = 0; i < 1; i++)
	{
		n->dendrites[i].current_value = (n->dendrites[i].current_value * 63) / 64;
	}
}

void membraneDecayStep(neuron_t *n)
{
	n->fire_potential = (n->fire_potential * 63) / 64;
}

int16_t calcNeuronPotential(neuron_t *n)
{
	uint8_t i;
	int16_t new_v = 0;
	for (i = 0; i < 1; i++)
	{
		if (n->dendrites[i].state == ON)
		{
			switch (n->dendrites[i].type)
			{
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
