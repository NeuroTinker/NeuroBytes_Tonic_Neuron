
#include "comm.h"
#include "HAL.h"

write_buffer_t write_buffer;

uint32_t message_buffer[2] = {0,0};
uint8_t message_buffer_count[2];

volatile uint16_t active_input_pins[2] = {0,0};

volatile uint16_t active_output_pins[2] = {PIN_AXON1_EX, PIN_AXON2_EX};

volatile uint32_t dendrite_pulses[4] = {0,0,0,0};
volatile uint8_t dendrite_pulse_count = 0;

volatile uint8_t blink_flag = 0;

volatile uint32_t nid_ping_time = 0;

volatile uint16_t nid_pin = 0;
uint32_t nid_port = 0;
volatile uint16_t nid_pin_out = 0;
uint32_t nid_port_out = 0;
uint8_t nid_i      =    4;
volatile uint32_t  nid_keep_alive = NID_PING_KEEP_ALIVE;


/* 
All available input pins are
    = {
        PIN_AXON1_IN,
        PIN_AXON2_IN,
        PIN_DEND1_EX,
        PIN_DEND1_IN, 
        PIN_DEND2_EX,
        PIN_DEND2_IN,
        PIN_DEND3_EX,
        PIN_DEND3_IN,
        PIN_DEND4_EX,
        PIN_DEND4_IN
    };
*/

uint32_t active_input_ports[2] = {
    PORT_DEND1_EX,
    PORT_DEND1_IN
};

uint32_t active_output_ports[2] = {
    PORT_AXON1_EX,
    PORT_AXON2_EX
};

uint16_t complimentary_pins[2] = {
    PIN_DEND1_IN,
    PIN_DEND1_EX
};

volatile uint8_t dendrite_pulse_flag[2] = {0,0};
volatile uint8_t dendrite_ping_flag[2] = {0,0};
uint8_t write_count = 0;
volatile uint16_t identify_time = IDENTIFY_TIME;
uint8_t identify_channel = 0;

void commInit(void)
{
    write_buffer.current_buffer = NONE_BUFF;
    write_buffer.write_count = 0;
}

void readInputs(void)
{
    uint8_t i;
    uint16_t value;
    uint32_t recipient_id;
    uint32_t header;
    uint32_t sender_id;
    uint32_t keep_alive;
    uint32_t data_frame;

    for (i=0; i<NUM_INPUTS; i++){
        // read each input that is currently receiving a message
        if (active_input_pins[i] != 0){

            // get new input value
            value = gpio_get(active_input_ports[i], active_input_pins[i]); // returns uint16 where bit position corresponds to pin number
            if (value != 0){
                value = 1;
            } else{
                value = 0;
            }

            // save new input value to buffer
            message_buffer[i] <<= 1;
            message_buffer[i] |= value;

            // when the message buffer has read 32-bits, the message is done being read and is processed
            if (++message_buffer_count[i] == 32){ // done reading message
                // Process message and set appropriate flags for main() or add messages to message buffer
                recipient_id = (message_buffer[i] & RECIPIENT_MASK) >> 28; // 3-bit recipient id 28
                keep_alive = (message_buffer[i] & KEEP_ALIVE_MASK) >> 22; // 6-bit keep alive 22
                sender_id = (message_buffer[i] & SENDER_MASK) >> 19; // 3-bit sender id 19
                header = (message_buffer[i] & HEADER_MASK) >> 16; // 3-bit message id 16
                data_frame = message_buffer[i] & DATA_MASK; // 16-bit data frame

                // decrement keep alive
                message_buffer[i] = (((keep_alive - 1) << 22) & KEEP_ALIVE_MASK) | (message_buffer[i] & ~KEEP_ALIVE_MASK);
                
                // analyze message header and determine what to do with it

                if (recipient_id == SELECTED4){
                    dendrite_pulse_flag[i] = 1;
                }

                if (header == BLINK && recipient_id == ALL){
                    /*
                        This is a NID->network blink message.
                        Set the blink_flag and the main routine will handle it.
                        Forward this message through the network.
                    */
                    if (blink_flag == 0){
                        // set blink_flag => main() will blink led
                        blink_flag = 1;
                        // forward message through network
                        addWrite(ALL_BUFF, message_buffer[i]);
                        write_buffer.source_pin = i;
                    }
                    
                } else if (recipient_id == NID){
                    /*
                        This is a neuron->NID message. 
                        
                        There is no need to process it so just forward to NID.
                    */
                    addWrite(NID_BUFF, message_buffer[i]);
                } else if (header == PING){
                    if (recipient_id == DOWNSTREAM){
                        /*
                            This is a upstream neuron -> downstream neuron ping message.

                            This message is used to figure out if the neuron->neuron connection is inhib or excit.
                            The pin that received the ping should be an input and its complimentary pin an output.

                            checkDendrites() resets dendrites to inputs if ping expires. Set dendrite_ping_flag so keep the current input/output configuration
                        */

                        dendrite_ping_flag[i] = 1;
                        /*
                        if (i == 0){
                            // excitatory
                            setAsOutput(active_input_ports[1], complimentary_pins[0]);
                            active_output_pins[1] = complimentary_pins[0];
                        } else{
                            // inhibitory
                            setAsOutput(active_input_ports[0], complimentary_pins[1]);
                            active_output_pins[0] = complimentary_pins[1];
                        }
                        */
                    } else if (recipient_id == ALL){
                        /*
                            This is a NID -> network ping message.

                            It is used for neurons to figure which input/output is closest to the NID.
                            The NID message with the largest keep_alive left is closest to NID.
                        */
                        if (active_input_pins[i] == nid_pin){
                            // NID ping was received on the existing nid_pin
                            nid_ping_time = 0; // main() will reset nid pin when this reaches NID_PING_TIME
                            if (keep_alive > 0){
                                addWrite(ALL_BUFF, message_buffer[i]); // forward message to the rest of the network
                                write_buffer.source_pin = i;
                            }
                        } else if ((NID_PING_KEEP_ALIVE - keep_alive) < nid_keep_alive){
                            // the received NID ping is closer to the NID so set new NID pin
                            nid_pin = active_input_pins[i]; // nid input
                            nid_pin_out = complimentary_pins[i]; // nid output
                            nid_port = active_input_ports[i];
                            nid_port_out = active_input_ports[i];
                            nid_ping_time = 0;
                            nid_keep_alive = NID_PING_KEEP_ALIVE - keep_alive; // new keep_alive for a  NID messages
                            setAsOutput(nid_port_out, nid_pin_out);
                        }
                    }
                } else if (header == IDENTIFY){
                    /*
                        This is a NID -> network identify message.

                        This message is used to assign neurons to a channel.
                        If the user presses the 'identify' button when this message is received then the neuron is assigned to a channel.
                    */
                    if (identify_time >= IDENTIFY_TIME){
                        // set the identify_time window to 0. main() will handle the rest
                        identify_time = 0;
                        identify_channel = data_frame; // the channel to be assigned is in the data frame
                    }
                    if (keep_alive > 0){
                        // pass the message
                        addWrite(ALL_BUFF, message_buffer[i]);
                        write_buffer.source_pin = i;
                    }
                }
                
                
                
                // deactivate input so that it doesn't keep getting read
                EXTI_PR |= active_input_pins[i];
                active_input_pins[i] = 0;
                // reset message buffer
                message_buffer[i] = 0;
                message_buffer_count[i] = 0;
            }
        }
    }
}


void addWrite(message_buffers_t buffer, uint32_t message)
{
    /*
        This function adds a new message to the write buffer.
    */
    switch (buffer){
        case DOWNSTREAM_BUFF:
            write_buffer.downstream[write_buffer.downstream_ready_count] = message;
            write_buffer.downstream_ready_count += 1;
            break;
        case NID_BUFF:
            write_buffer.nid[write_buffer.nid_ready_count] = message;
            write_buffer.nid_ready_count += 1;
            break;
        case ALL_BUFF:
            write_buffer.all[write_buffer.all_ready_count] = message;
            write_buffer.all_ready_count += 1;
            break;
        default:
            break;
    }
}

void write()
{
    /*
        Pop 1-bit off the write_buffer and write it to corresponding output pins
    */
    uint8_t i;
    if (write_buffer.write_count == 33){
        // Message is done being written. Move to next message in the buffer.
        switch (write_buffer.current_buffer){
            case DOWNSTREAM_BUFF:
                for (i=0; i<2; i++){
                    write_buffer.downstream[i] = write_buffer.downstream[i+1];
                }
                write_buffer.downstream_ready_count -= 1;
                break;
            case NID_BUFF:
                for (i=0; i<4; i++){
                    write_buffer.nid[i] = write_buffer.nid[i+1];
                }
                write_buffer.nid_ready_count -= 1;
                break;
            case ALL_BUFF:
                for (i=0; i<2; i++){
                    write_buffer.all[i] = write_buffer.all[i+1];
                }
                write_buffer.all_ready_count -= 1;
                break;
            default:
                break;
        }
        write_buffer.current_buffer = NONE_BUFF;
        write_buffer.write_count = 0;
    }

    if (write_buffer.current_buffer == NONE_BUFF){
        if (write_buffer.downstream_ready_count != 0){
            write_buffer.current_buffer = DOWNSTREAM_BUFF;
        } else if (write_buffer.nid_ready_count != 0){
            write_buffer.current_buffer = NID_BUFF;
        } else if (write_buffer.all_ready_count != 0){
            write_buffer.current_buffer = ALL_BUFF;
        }
    } else{
        write_buffer.write_count += 1;
        switch (write_buffer.current_buffer){
            case DOWNSTREAM_BUFF:
                writeDownstream();
                break;
            case NID_BUFF:
                writeNID();
                break;
            case ALL_BUFF:
                writeAll();
                break;
            default:
                break;
        }
    }


}

void writeDownstream(void)
{
    uint32_t value;
    value = write_buffer.downstream[0] & 0x80000000;
    write_buffer.downstream[0] <<= 1;

    // we should have both axon out pins be on the same port that way they can be written together
    if (value != 0){
        gpio_set(PORT_AXON1_EX, PIN_AXON1_EX);
        for (i=0; i<100; i++){
            __asm__("NOP");
        }
        gpio_set(PORT_AXON2_EX, PIN_AXON2_EX);
    }else{
        gpio_clear(PORT_AXON1_EX, PIN_AXON1_EX);
        for (i=0; i<100; i++){
            __asm__("NOP");
        }
        gpio_clear(PORT_AXON2_EX, PIN_AXON2_EX);
    }
}

void writeAll(void)
{
    /*
        Write to all pins on the NeuroByte except for the pin that received the message
    */

    uint8_t i;

    uint32_t value;
    // pop next bit off of buffer
    value = write_buffer.all[0] & 0x80000000;
    write_buffer.all[0] <<= 1;

    // write to all output pins except for the pin the message was received on
    for (i=0;i<2;i++){
        if (active_output_pins[i] != 0 && active_output_pins[i] != complimentary_pins[write_buffer.source_pin]){
            if (value != 0){
                gpio_set(active_output_ports[i], active_output_pins[i]);
            } else {
                gpio_clear(active_output_ports[i], active_output_pins[i]);
            }
        } 
    }
}

void writeNID(void)
{
    uint32_t value;
    value = write_buffer.nid[0] & 0x80000000;
    write_buffer.nid[0] <<= 1;

    if (value != 0){
        gpio_set(nid_port_out, nid_pin_out);
    } else{
        gpio_clear(nid_port_out, nid_pin_out);
    }
}


