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

#include "i2c_s.h"
#include "hardware.h"
#include "../protocol.h"

// this uCs adress on the i2c bus
volatile uint8_t i2c_adress;

// 1 => reading as slave, 0 => writing as slave
volatile uint8_t i2c_rw;

volatile uint8_t i2c_busy;

// holds the number of the next byte of a message to transmit
volatile uint8_t i2c_transmit_position;

// temp_buffer for receiving
volatile struct i2c_message_in i2c_temp_message_in;

/*
 * A timeout is used for outgoing messages. If the delivering the
 * message failed  "I2C_TIMEOUT"-times, it will be removed from the
 * outgoing buffer.
 */
#define I2C_TIMEOUT 10
volatile uint8_t i2c_timeout_counter;

//#define I2C_A_NEWSTART 0


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
 
void i2c_init(uint8_t adress) 
{
	
	i2c_output_back = 0;
	i2c_output_front = 0;
	i2c_input_back = 0;
	i2c_input_front = 0;
	
	i2c_rw = 1;
	i2c_busy = 0;
	
	i2c_timeout_counter = 0;
	i2c_adress = adress & 0b01111111;
	
	
	TWCR = (I2C_ACK);
	TWAR = 0b00000001 | (i2c_adress<<1);
	TWSR = (0<<TWPS1) | (0<<TWPS0);            
	TWBR = ((F_CPU/I2C_clock)-16)/2;
}

uint8_t i2c_getAdress( void ) 
{
	return i2c_adress;
}

/***********************************************************************
 * 
 * These functions implement the slave behaviour
 * 
 **********************************************************************/
 
void i2c_trySendMessages( void ) 
{
	if (i2c_hasOutgoingMessages()) 
	{
		INT_LOW;
	}
	else 
	{
		INT_RELEASE;
	}
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
		// ======================================================
		// Receiver
		// ======================================================
		case I2C_GCALL_received_ack_returned:
		case I2C_SLA_W_received_ack_returned:
		case I2C_GCALL_received_upon_arbitration_lost: 
		case I2C_SLA_W_received_upon_arbitration_lost: 
		{
		
			if (i2c_canTakeNewIncomingMessage()) 
			{
				i2c_temp_message_in.length = 0;
				i2c_temp_message_in.error = 0;
				i2c_busy = 1;
				i2c_rw = 1;
				// acknolegde incoming transfer
				TWCR = I2C_ACK;
			}
			else 
			{
				i2c_busy = 0;
				// abort incoming transfer
				TWCR = I2C_NACK;
			}
		} break;
		
		// ==================================
		case I2C_SLA_W_data_received_ack_returned:
		case I2C_GCALL_data_received_ack_returned: 
		{
			// take new data
			i2c_temp_message_in.data[i2c_temp_message_in.length++] = TWDR;
			
			// if message is full and there is no storage left
			if (i2c_temp_message_in.length >= I2C_MAX_MESSAGE_IN_LENGTH) 
			{
				// abort transmission
				TWCR = I2C_NACK;
				break;
			}
			else 
			{
				// go on with transmission
				TWCR = I2C_ACK;
			}
		} break;
		
		// ==================================
		case I2C_SLA_W_data_received_nack_returned:
		case I2C_GCALL_data_received_nack_returned: 
		{
			// take new data
			i2c_temp_message_in.data[i2c_temp_message_in.length++] = TWDR;
			
			// set error flag in received message
			i2c_temp_message_in.error = I2C_IN_ERROR_OVERFLOW;
			i2c_addInputBufferFront(i2c_temp_message_in);
			i2c_busy = 0;
			
			// we want to receive more messages
			TWCR = I2C_ACK;
		} break;
		
		// ==================================
		case I2C_stop_or_start_while_adressed: 
		{
			// if data was received
			if (i2c_temp_message_in.length != 0) 
			{
				// append the received packet to input buffer
				i2c_addInputBufferFront(i2c_temp_message_in);
				// reset temp message
				i2c_temp_message_in.length = 0;
				i2c_temp_message_in.error = 0;
				i2c_busy = 0;
			}
				
			TWCR = I2C_ACK;
		} break;
		
		
		// ======================================================
		// Transmitter
		// ======================================================
		
		case I2C_SLA_R_received_ack_returned: 
		{
			if (i2c_hasOutgoingMessages()) 
			{
				i2c_busy = 1;
				i2c_rw = 0;
				i2c_transmit_position = 0;
				
				// load data
				TWDR = i2c_getOutputBufferBack()->data[i2c_transmit_position];
				i2c_transmit_position++;
				
				// does this message have more bytes?
				if (i2c_getOutputBufferBack()->length > 1)
					TWCR = I2C_ACK;
				else
					TWCR = I2C_NACK;
			}
			else 
			{
				TWDR = 0;
				TWCR = I2C_NACK;
			}
		} break;
		
		// ==================================
		case I2C_data_sent_ack_received: 
		{
			// load data
			TWDR = i2c_getOutputBufferBack()->data[i2c_transmit_position];
			i2c_transmit_position++;
			
			if (i2c_transmit_position < i2c_getOutputBufferBack()->length)
				TWCR = I2C_ACK;
			else
				// this byte is the last of this message
				TWCR = I2C_NACK;
		} break;
		
		// ==================================
		case I2C_data_sent_nack_received: 
		{
			if (i2c_transmit_position >= i2c_getOutputBufferBack()->length) 
			{
				// message transmitted successfully, 
				i2c_removeOutputBufferBack();
			}
							
			// more messages?
			if (i2c_hasOutgoingMessages()) 
			{
				INT_LOW;
			}
			else 
			{
				INT_RELEASE;
			}
			
			i2c_busy = 0;
			
			// stop the message, but listen to the bus
			TWCR = I2C_ACK;
		} break;
		
		// ==================================
		case I2C_last_data_sent_ack_received: 
		{ 
			// master did expect more data => message failed.
			i2c_busy = 0;
			
			// more messages?
			if (i2c_hasOutgoingMessages()) 
			{
				INT_LOW;
			}
			else 
			{
				INT_RELEASE;
			}
			
			// stop message but listen to the bus
			TWCR = I2C_ACK;
		} break;
		
		// ======================================================
        default: 
        {
			TWCR = I2C_NACK;
		}
	}
}






