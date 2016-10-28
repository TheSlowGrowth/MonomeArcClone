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
 

#include "hardware.h"
#include "ftdi_commands.h"
#include "ftdi.h"
#include "led_buffer.h"
#include "config.h"

uint8_t ftdi_in_buffer[FTDI_IN_BUFFER_SIZE];
int ftdi_in_buffer_pos;
// last byte for the next command
int ftdi_in_buffer_pos_max;

#define FTDI_BUFFER_OUT_LENGTH 	100

// ringbuffer for outgoing messages
volatile uint8_t FTDI_output[FTDI_BUFFER_OUT_LENGTH];
volatile uint8_t FTDI_output_front;
volatile uint8_t FTDI_output_back;


// functions for accessing the ringbuffers
static int8_t FTDI_hasOutgoingBytes( void ) 
{
	return FTDI_output_front != FTDI_output_back;
}

static int8_t FTDI_canTakeNewByte( void ) 
{
	if (FTDI_output_back == 0) 
	{
		return FTDI_output_front != FTDI_BUFFER_OUT_LENGTH-1;
	} 
	else 
	{
		return FTDI_output_front != FTDI_output_back-1;
	}
}

static volatile uint8_t FTDI_getOutputBufferBack( void ) 
{
	return FTDI_output[FTDI_output_back];
}
static void FTDI_removeOutputBufferBack( void ) 
{
	if (FTDI_output_back != FTDI_output_front)
		FTDI_output_back++;
		
	if (FTDI_output_back >= FTDI_BUFFER_OUT_LENGTH)
		FTDI_output_back -= FTDI_BUFFER_OUT_LENGTH;
}
static int8_t FTDI_addOutputBufferFront( volatile uint8_t m ) 
{
	
	if (!FTDI_canTakeNewByte()) 
	{
		return FALSE;
	}
		
	FTDI_output[FTDI_output_front] = m;
	
	FTDI_output_front++;
		
	if (FTDI_output_front >= FTDI_BUFFER_OUT_LENGTH)
		FTDI_output_front -= FTDI_BUFFER_OUT_LENGTH;
	
	return TRUE;
}

void FTDI_init( void ) 
{
	ftdi_in_buffer_pos = 0;
	FTDI_output_front = 0;
	FTDI_output_back = 0;
	// set outputs and inputs
	FTDI_STAT_DDR |= (1<<FTDI_WR) | (1<<FTDI_RD);
	FTDI_STAT_DDR &= ~(1<<FTDI_RXF) & ~(1<<FTDI_TXE);
	FTDI_STAT_PORT |= (1<<FTDI_WR) | (1<<FTDI_RD);
	// enable pin change interrupt
	PCICR |= (1<<PCIE1);
	PCMSK1 |= (1<<PCINT11);
}

// processes an incoming message
static void FTDI_processMessage( void ) 
{
	switch (ftdi_in_buffer[0]) 
	{
		case COMM_I_LED_SINGLE: 
		{
			uint8_t enc = ftdi_in_buffer[1];
			uint8_t led = ftdi_in_buffer[2];
			uint8_t val = ftdi_in_buffer[3];
			while (enc >= NUM_ENCS)
				enc -= NUM_ENCS;
			while (led >= 64)
				led -= NUM_ENCS;
			BUFFER_setLED(enc, led, val);
			break;
		}
		case COMM_I_LED_ALL: 
		{
			uint8_t enc = ftdi_in_buffer[1];
			uint8_t val = ftdi_in_buffer[2];
			while (enc >= NUM_ENCS)
				enc -= NUM_ENCS;
			BUFFER_setAllInEncoder(enc, val);
			break;
		}
		case COMM_I_LED_COMPLETE: 
		{
			uint8_t enc = ftdi_in_buffer[1];
			uint8_t* val_p = &ftdi_in_buffer[2];
			while (enc >= NUM_ENCS)
				enc -= NUM_ENCS;
			BUFFER_setCompleteEncoder(enc, val_p);
			break;
		}
		case COMM_I_LED_RANGE: 
		{
			uint8_t enc = ftdi_in_buffer[1];
			uint8_t start = ftdi_in_buffer[2];
			uint8_t end = ftdi_in_buffer[3];
			uint8_t val = ftdi_in_buffer[4];
			
			while (enc >= NUM_ENCS)
				enc -= NUM_ENCS;
			while (start >= 64)
				start -= 64;
			while (end >= 64)
				end -= 64;
			
			BUFFER_setRange(enc, start, end, val);
			
			break;
		}
		case COMM_I_QUERY: 
		{
            // answer with 0x00 A B (total message length: 3 bytes)
            // where A is the section (grids, encoders, analog inputs, etc.)
            // and B is the number that are available.
			while (!FTDI_canTakeNewByte())
				FTDI_trySendCommandsToHost();
			FTDI_addOutputBufferFront(COMM_O_QUERY_RESP);
			while (!FTDI_canTakeNewByte())
				FTDI_trySendCommandsToHost();
			FTDI_addOutputBufferFront(0x05); // 5 == encoders
			while (!FTDI_canTakeNewByte())
				FTDI_trySendCommandsToHost();
			FTDI_addOutputBufferFront((uint8_t) NUM_ENCS);
            // monome eurorack modules (like ansible) expect the response to be
            // 6 bytes long, although they don't seem to care about the additional
            // bytes. Seems dodgy, but the only way to deal with this is to send
            // another response for a section we don't actually have
            // lets simply report that we have 0 grids.
            while (!FTDI_canTakeNewByte())
				FTDI_trySendCommandsToHost();
			FTDI_addOutputBufferFront(COMM_O_QUERY_RESP);
			while (!FTDI_canTakeNewByte())
				FTDI_trySendCommandsToHost();
			FTDI_addOutputBufferFront(0x01); // 1 == grid
			while (!FTDI_canTakeNewByte())
				FTDI_trySendCommandsToHost();
			FTDI_addOutputBufferFront(0); // no grids.
			break;
		}
		case COMM_I_WRITE_ID: 
		{
			CONFIG_writeID(&ftdi_in_buffer[1]);
			// no break here, so query answer will be sent
		}
		case COMM_I_QUERY_ID: 
		{
			while (!FTDI_canTakeNewByte())
				FTDI_trySendCommandsToHost();
			FTDI_addOutputBufferFront(COMM_O_QUERY_ID_RESP);
			
			int i;
			uint8_t* ID_p = CONFIG_getID();
			for (i = 0; i < 32; i++) {
				while (!FTDI_canTakeNewByte())
					FTDI_trySendCommandsToHost();
				FTDI_addOutputBufferFront(*ID_p++);
			}
			break;
		}
		case COMM_I_REQ_GRID_OFFSET: 
		{
			break;
		}
		case COMM_I_SET_GRID_OFFSET: 
		{
			break;
		}
		case COMM_I_REQ_GRID_SIZE: 
		{
			while (!FTDI_canTakeNewByte())
				FTDI_trySendCommandsToHost();
			FTDI_addOutputBufferFront(COMM_O_REP_GRID_SIZE);
			while (!FTDI_canTakeNewByte())
				FTDI_trySendCommandsToHost();
			FTDI_addOutputBufferFront(0x00);
			while (!FTDI_canTakeNewByte())
				FTDI_trySendCommandsToHost();
			FTDI_addOutputBufferFront(0x00);
			break;
		}
		case COMM_I_SET_GRID_SIZE: 
		{
			break;
		}
		case COMM_I_GET_ADDR: 
		{
			break;
		}
		case COMM_I_SET_ADDR: 
		{
			break;
		}
		case COMM_I_GET_FIRMWARE: 
		{
			// answer with rubbish firmware number
			while (!FTDI_canTakeNewByte())
				FTDI_trySendCommandsToHost();
			FTDI_addOutputBufferFront(COMM_O_REP_FIRMWARE);
			
			int i;
			for (i = 0; i < 8; i++) {
				while (!FTDI_canTakeNewByte())
					FTDI_trySendCommandsToHost();
				FTDI_addOutputBufferFront('-');
			}
			break;
		}
		default: {
			break;
		}
	}
}

void FTDI_tryGetCommandsFromHost( void ) 
{
	// set Data port to input
	FTDI_DATA_REG_DDR = 0x00;
	
	// if data available
	while (bit_is_clear(FTDI_STAT_PIN, FTDI_RXF)) 
	{
		// set RD low 
		FTDI_STAT_PORT &= ~(1<<FTDI_RD);
		asm volatile ("nop");asm volatile ("nop");
		asm volatile ("nop");asm volatile ("nop");
		// read byte
		ftdi_in_buffer[ftdi_in_buffer_pos] = FTDI_DATA_REG_PIN;
		// set RD high again
		FTDI_STAT_PORT |= (1<<FTDI_RD);
		// increase read pointer
		ftdi_in_buffer_pos++;
		
		// check for command type on the first byte
		if (ftdi_in_buffer_pos == 1) 
		{
			 
			switch (ftdi_in_buffer[0]) 
			{
				case COMM_I_LED_SINGLE: 		ftdi_in_buffer_pos_max = COMM_I_LED_SINGLE_LENGTH; break;
				case COMM_I_LED_ALL: 			ftdi_in_buffer_pos_max = COMM_I_LED_ALL_LENGTH; break;
				case COMM_I_LED_COMPLETE: 		ftdi_in_buffer_pos_max = COMM_I_LED_COMPLETE_LENGTH; break;
				case COMM_I_LED_RANGE: 			ftdi_in_buffer_pos_max = COMM_I_LED_RANGE_LENGTH; break;
				case COMM_I_QUERY: 				ftdi_in_buffer_pos_max = COMM_I_QUERY_LENGTH; break;
				case COMM_I_QUERY_ID: 			ftdi_in_buffer_pos_max = COMM_I_QUERY_ID_LENGTH; break;
				case COMM_I_WRITE_ID: 			ftdi_in_buffer_pos_max = COMM_I_WRITE_ID_LENGTH; break;
				case COMM_I_REQ_GRID_OFFSET:	ftdi_in_buffer_pos_max = COMM_I_REQ_GRID_OFFSET_LENGTH; break;
				case COMM_I_SET_GRID_OFFSET: 	ftdi_in_buffer_pos_max = COMM_I_SET_GRID_OFFSET_LENGTH; break;
				case COMM_I_REQ_GRID_SIZE: 		ftdi_in_buffer_pos_max = COMM_I_REQ_GRID_SIZE_LENGTH; break;
				case COMM_I_SET_GRID_SIZE: 		ftdi_in_buffer_pos_max = COMM_I_SET_GRID_SIZE_LENGTH; break;
				case COMM_I_GET_ADDR: 			ftdi_in_buffer_pos_max = COMM_I_GET_ADDR_LENGTH; break;
				case COMM_I_SET_ADDR: 			ftdi_in_buffer_pos_max = COMM_I_SET_ADDR_LENGTH; break;
				case COMM_I_GET_FIRMWARE: 		ftdi_in_buffer_pos_max = COMM_I_GET_FIRMWARE_LENGTH; break;
				default: 
				{
					ftdi_in_buffer_pos_max = 1;
				}
			}
		}
		
		// check if message complete
		if (ftdi_in_buffer_pos == ftdi_in_buffer_pos_max) 
		{
			// process message
			FTDI_processMessage();
			// reset counter
			ftdi_in_buffer_pos = 0;
		}
	}
	
}

void FTDI_trySendCommandsToHost( void ) 
{
	// set RD high
	FTDI_STAT_PORT |= (1<<FTDI_RD);
	// set Data port to output
	FTDI_DATA_REG_DDR = 0xFF;
	
	while (FTDI_hasOutgoingBytes() && bit_is_clear(FTDI_STAT_PIN, FTDI_TXE)) 
	{
		// set WR high
		FTDI_STAT_PORT |= (1<<FTDI_WR);
		// Output data
		FTDI_DATA_REG_PORT = FTDI_getOutputBufferBack();
		// set WR low to write data
		FTDI_STAT_PORT &= ~(1<<FTDI_WR);
		// remove data from buffer
		FTDI_removeOutputBufferBack();
	}
}

ISR(PCINT1_vect) 
{
	if (bit_is_clear(FTDI_STAT_PIN, FTDI_RXF)) 
	{
		FTDI_tryGetCommandsFromHost();
	}
}
