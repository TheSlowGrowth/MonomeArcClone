/*	 This file contains source code for the firmware of a monome arc 
 *   clone.
 * 
 *   Copyright (C) 2013  Johannes Neumann; mail@johannesneumann.net
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */  
 


#define TRUE (1==1)
#define FALSE (1!=1)

#include <inttypes.h>
#include <avr/io.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <math.h>

#include "i2c_m.h"
#include "hardware.h"
#include "../protocol.h"
// 1 => reading as master, 0 => writing as master
volatile uint8_t i2c_rw;
// 1 => busy
volatile uint8_t i2c_busy;

// holds the number of the next byte of a message to transmit
volatile uint8_t i2c_transmit_position;

// temp_buffer for receiving
volatile struct i2c_message_in i2c_temp_message_in;

/*
 * A timeout is used for outgoing messages. If the delivering the
 * message failed  "I2C_TIMEOUT"-times, it will be removed from the
 * outgoing buffer, assuming that the slave is stuck or not present.
 */
#define I2C_TIMEOUT 2
volatile uint8_t i2c_timeout_counter;


/***********************************************************************
 * 
 * These functions implement two ring buffers for incoming and
 * outgoing messages.
 * 
 **********************************************************************/


// ringbuffer for incoming messages
volatile struct i2c_message_in i2c_input[I2C_BUFFER_IN_LENGTH];
volatile uint8_t i2c_input_front;
volatile uint8_t i2c_input_back;

// ringbuffer for outgoing messages
volatile struct i2c_message_out i2c_output[I2C_BUFFER_OUT_LENGTH];
volatile uint8_t i2c_output_front;
volatile uint8_t i2c_output_back;


// functions for accessing the ringbuffers
int8_t i2c_hasOutgoingMessages( void ) 
{
	return i2c_output_front != i2c_output_back;
}

int8_t i2c_hasIncomingMessages( void ) 
{
	return i2c_input_front != i2c_input_back;
}

int8_t i2c_canTakeNewOutgoingMessage( void ) 
{
	if (i2c_output_back == 0) 
	{
		return i2c_output_front != I2C_BUFFER_OUT_LENGTH-1;
	} 
	else 
	{
		return i2c_output_front != i2c_output_back-1;
	}
}
int8_t i2c_canTakeNewIncomingMessage( void ) 
{
	if (i2c_input_back == 0) 
	{
		return i2c_input_front != I2C_BUFFER_IN_LENGTH-1;
	} 
	else 
	{
		return i2c_input_front != i2c_input_back-1;
	}
}

volatile struct i2c_message_in* i2c_getInputBufferBack( void ) 
{
	return &i2c_input[i2c_input_back];
}
volatile struct i2c_message_in* i2c_getInputBufferFront( void ) 
{
	return &i2c_input[i2c_input_front];
}
void i2c_removeInputBufferBack( void ) 
{
	if (i2c_input_back != i2c_input_front)
		i2c_input_back++;
		
	if (i2c_input_back >= I2C_BUFFER_IN_LENGTH)
		i2c_input_back -= I2C_BUFFER_IN_LENGTH;
}
int8_t i2c_addInputBufferFront( volatile struct i2c_message_in m ) 
{
	
	if (!i2c_canTakeNewIncomingMessage()) 
	{
		return FALSE;
	}
		
	i2c_input[i2c_input_front] = m;
		
	i2c_input_front++;
		
	if (i2c_input_front >= I2C_BUFFER_IN_LENGTH)
		i2c_input_front -= I2C_BUFFER_IN_LENGTH;
	
	return TRUE;
}

volatile struct i2c_message_out* i2c_getOutputBufferBack( void ) 
{
	return &i2c_output[i2c_output_back];
}

volatile struct i2c_message_out* i2c_getOutputBufferFront( void ) 
{
	return &i2c_output[i2c_output_front];
}

void i2c_removeOutputBufferBack( void ) 
{
	if (i2c_output_back != i2c_output_front)
		i2c_output_back++;
		
	if (i2c_output_back >= I2C_BUFFER_OUT_LENGTH)
		i2c_output_back -= I2C_BUFFER_OUT_LENGTH;
}

int8_t i2c_addOutputBufferFront( volatile struct i2c_message_out m ) 
{
	
	if (!i2c_canTakeNewOutgoingMessage()) 
	{
		return FALSE;
	}
		
	i2c_output[i2c_output_front] = m;
		
	i2c_output_front++;
		
	if (i2c_output_front >= I2C_BUFFER_OUT_LENGTH)
		i2c_output_front -= I2C_BUFFER_OUT_LENGTH;
	
	return TRUE;
}

/***********************************************************************
 * 
 * These functions implement general tasks
 * 
 **********************************************************************/
 
void i2c_init( void ) 
{
	
	i2c_output_back = 0;
	i2c_output_front = 0;
	i2c_input_back = 0;
	i2c_input_front = 0;
	
	i2c_rw = 1;
	i2c_busy = 0;
	
	i2c_timeout_counter = 0;	
	
	TWCR = (I2C_ACK);
	TWSR = (0<<TWPS1) | (0<<TWPS0);            
	TWBR = ((F_CPU/I2C_clock)-16)/2;
}

void waitWhileBusIsBusy( void ) 
{
	static int i2c_wait_counter;
	
	i2c_wait_counter = 10;
	
	while (i2c_wait_counter > 0) 
	{
		if ((!bit_is_set(I2C_PORT, I2C_SDA_PIN)) || (!bit_is_set(I2C_PORT, I2C_SCL_PIN))) 
		{
			i2c_wait_counter = 10;
		} else 
		{
			i2c_wait_counter--;
		}			
	}
}

/***********************************************************************
 * 
 * 
 * 
 **********************************************************************/
 
void i2c_trySendMessages( void ) 
{
	if (i2c_busy != 0)
		return;
		
	if (i2c_hasOutgoingMessages()) 
	{
		i2c_rw = 0;		
		i2c_timeout_counter = 0;
		TWCR=I2C_START;
	}
}

int8_t i2c_tryReceiveMessageFrom( uint8_t slave_adress ) 
{
	if (i2c_busy != 0)
		return -1;
		
	if (i2c_canTakeNewIncomingMessage()) 
	{
		i2c_rw = 1;	
		i2c_temp_message_in.sender = slave_adress;
		i2c_busy = 1;	
		TWCR=I2C_START;
		return 1;
	}
	else return -1;
}
 
 /***********************************************************************
 * 
 * This is the main ISR that handles all bus events
 * 
 **********************************************************************/

ISR(TWI_vect) 
{
	uint8_t i2c_status = TWSR & 0xF8; 
	
	switch (i2c_status) 
	{
		// ==================================
		case I2C_repeated_start_sent:
		case I2C_start_sent: 
		{	
			i2c_busy = 1;	
			i2c_transmit_position = 0;
			
			// is reading
			if (i2c_rw > 0) 
			{
				i2c_temp_message_in.length = 0;
				
				// send adress and read mode (SLA+R)
				TWDR = ((i2c_temp_message_in.sender<<1) | 0b00000001);
				TWCR = I2C_ENABLE;
			}
			// else: is writing
			else 
			{
				// send adress and write mode (SLA+W)
				TWDR = ((i2c_getOutputBufferBack()->receiver<<1) & 0b11111110);
				TWCR = I2C_ENABLE;
			}
		} break; 
		
		// ==================================
		case I2C_arbitration_lost_or_nack_received: 
		{
			// try sending it again
			if (i2c_timeout_counter < I2C_TIMEOUT) 
			{				
				// try again
				TWCR=I2C_START; 
			} else 
			{
				// message can not be sent, remove from buffer
				i2c_removeOutputBufferBack();
				i2c_busy = 0;
			}	
			
			TWCR = I2C_ACK;
		}
		
		// ==================================
		case I2C_SLA_R_sent_nack_received: 
		{
			// send stop condition
			TWCR = I2C_STOP;
			// wait until it's sent
            while(TWCR & (1<<TWSTO));
                        			
			TWCR = I2C_ACK;
			i2c_busy = 0;
		} break;
		
		// ==================================
		case I2C_SLA_R_sent_ack_received: 
		{
			TWCR = I2C_ACK;
		} break;
		
		// ==================================
		case I2C_data_received_ack_sent: 
		{
			// try to find out, what the length of this message is			
			if (i2c_transmit_position == 0) 
			{
				switch (TWDR) 
				{
					case PROT_ENCODER_TURNED: 
					{
						i2c_temp_message_in.length = PROT_ENCODER_TURNED_LENGTH;
					} break;
					case PROT_BUTTON: 
					{
						i2c_temp_message_in.length = PROT_BUTTON_LENGTH;
					} break;
				}
			}
			
			// store data
			i2c_temp_message_in.data[i2c_transmit_position] = TWDR;
			i2c_transmit_position++;
			
			// want more data?
			if (i2c_transmit_position < i2c_temp_message_in.length-1)
				TWCR = I2C_ACK;
			else
				TWCR = I2C_NACK;
		} break;
		
		// ==================================
		case I2C_data_received_nack_sent: 
		{
			// store data
			i2c_temp_message_in.data[i2c_transmit_position] = TWDR;
			
			// send stop condition
			TWCR = I2C_STOP;
			// wait until it's sent
            while(TWCR & (1<<TWSTO));
                        
			// end message
			i2c_addInputBufferFront(i2c_temp_message_in);
			
			i2c_busy = 0;
			TWCR = I2C_ACK;
		} break;
		
		// ======================================================
		// Transmitter
		// ======================================================
		case I2C_SLA_W_sent_nack_received:
		case I2C_data_sent_nack_received: 
		{
			// send stop condition
			TWCR = I2C_STOP;
			// wait until it's sent
            while(TWCR & (1<<TWSTO));
			// increase the timeout for this message
			i2c_timeout_counter++;
			// try sending it again
			if (i2c_timeout_counter < I2C_TIMEOUT) 
			{				
				// try again
				TWCR=I2C_START; 
			} 
			else 
			{
				// message can not be sent, remove from buffer
				i2c_removeOutputBufferBack();
				i2c_busy = 0;
			}		
		} break;
		
		// ==================================
		case I2C_SLA_W_sent_ack_received:
		case I2C_data_sent_ack_received: 
		{
			volatile struct i2c_message_out* m = i2c_getOutputBufferBack();
			if (i2c_transmit_position < m->length) 
			{
				// send next byte
				TWDR = i2c_getOutputBufferBack()->data[i2c_transmit_position++];
				TWCR = I2C_ENABLE;
			} else 
			{
				// end message by sending a STOP condition
				TWCR = I2C_STOP;
				// wait until it's sent
				while(TWCR & (1<<TWSTO));
				// remove this message from the buffer
				i2c_removeOutputBufferBack();
				
				i2c_busy = 0;
				
				// get ready for the next messages
				TWCR = (I2C_ACK);
			}
		} break;
		
		
		// ======================================================
        default: 
        {
			TWCR = I2C_NACK;
		}
	}
}






