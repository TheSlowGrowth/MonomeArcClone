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
 
// This file contains the main function

#include <inttypes.h>
#include <avr/io.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <math.h>

// defines for the I2C interface
#define I2C_MAX_MESSAGE_OUT_LENGTH 34
#define I2C_MAX_MESSAGE_IN_LENGTH 4

#define I2C_BUFFER_OUT_LENGTH 4
#define I2C_BUFFER_IN_LENGTH 4

#define I2C_PORT PINC
#define I2C_SDA_PIN PC4
#define I2C_SCL_PIN PC5

#include "../protocol.h"
#include "i2c_m.c"
#include "ftdi.c"
#include "led_buffer.c"
#include "config.c"
#include "startup_animation.h"

uint8_t button_down[NUM_ENCS];
int8_t encoder_turns[NUM_ENCS];

uint8_t current_slave_input;
uint8_t current_slave_output;

//flags
uint8_t must_activate_power;
uint8_t must_deactivate_power;


void processNewMessages( void ) 
{
		
	while (i2c_hasIncomingMessages()) {
		
		volatile struct i2c_message_in* m;
		m = i2c_getInputBufferBack();
		
		if (((m->data[0] & 0xF0) == PROT_BUTTON) 
			&& (m->length == PROT_BUTTON_LENGTH)) {
				
			uint8_t from = (m->data[1] & 0b01111111) - 1;
				
			if (m->data[1] & 0b10000000) 
			{
				while (!FTDI_canTakeNewByte())
					FTDI_trySendCommandsToHost();
				FTDI_addOutputBufferFront(COMM_O_ENCODER_DOWN);
				
				while (!FTDI_canTakeNewByte())
					FTDI_trySendCommandsToHost();
				FTDI_addOutputBufferFront(from);
			} 
			else 
			{
				while (!FTDI_canTakeNewByte())
					FTDI_trySendCommandsToHost();
				FTDI_addOutputBufferFront(COMM_O_ENCODER_UP);
				
				while (!FTDI_canTakeNewByte())
					FTDI_trySendCommandsToHost();
				FTDI_addOutputBufferFront(from);
			}
		} 
		else if (((m->data[0] & 0xF0) == PROT_ENCODER_TURNED) 
			&& (m->length == PROT_ENCODER_TURNED_LENGTH)) 
		{
				
			uint8_t from = (m->data[1] & 0b01111111) - 1;
				
			while (!FTDI_canTakeNewByte())
				FTDI_trySendCommandsToHost();
			FTDI_addOutputBufferFront(COMM_O_ENCODER);
			
			while (!FTDI_canTakeNewByte())
				FTDI_trySendCommandsToHost();
			FTDI_addOutputBufferFront(from);
			
			while (!FTDI_canTakeNewByte())
				FTDI_trySendCommandsToHost();
			FTDI_addOutputBufferFront(m->data[2]);
		} 
		
		
		i2c_removeInputBufferBack();
	}
}

int main (void) 
{			
	_delay_ms(500);

	// Init Hardware...
	POWER_PIN_INIT;
	// enable pin change interrupt for PWREN pin
	PCICR |= (1 << PCIE0);     // set PCIE0 to enable PCMSK0
    PCMSK0 |= (1 << PCINT0);   // set PCINT0 to trigger an interrupt
	
	// enable timer
	TCCR2B |= (1<<CS22) | (1<<CS21);
	TIMSK2 |= (1<<TOIE2);
	
	INT_PULLUP;
	
	// Init variables ...
	current_slave_input = 0;
	current_slave_output = 0;
	
	// init software modules
	i2c_init();
	BUFFER_init();
	FTDI_init();
	
	// init flags
	must_activate_power = 1;
	must_deactivate_power = 0;

	// enable interrupts
	sei();

	while(1)
	{	
		FTDI_trySendCommandsToHost();
		processNewMessages();
		i2c_trySendMessages();
		
		// if INT is low, then a slave wants to send data
		if (!INT_IS_HIGH) 
		{
			// Try to get message from a slave, if message is sent, then try next slave
			// until the INT line goes high
			if (i2c_tryReceiveMessageFrom(current_slave_input + 1) >= 0) 
			{
				current_slave_input++;
				if (current_slave_input >= NUM_ENCS)
					current_slave_input = 0;
			}	
		}
		
		// try send wake-up command if neccessary
		while ((must_activate_power > 0) && i2c_canTakeNewOutgoingMessage()) 
		{
			struct i2c_message_out m;
			m.receiver = must_activate_power;
			m.length = PROT_POWER_MODE_LENGTH;
			m.data[0] = PROT_POWER_MODE | 0b00000001;
				
			i2c_addOutputBufferFront(m);
			must_activate_power--;
		}
		// try send sleep command if neccessary
		while ((must_deactivate_power > 0) && i2c_canTakeNewOutgoingMessage()) 
		{
			struct i2c_message_out m;
			m.receiver = must_deactivate_power;
			m.length = PROT_POWER_MODE_LENGTH;
			m.data[0] = PROT_POWER_MODE | 0b00000000;
			
			i2c_addOutputBufferFront(m);
			must_deactivate_power--;
		}
	}
	
	return 0;
}

// called every three ms
ISR(TIMER2_OVF_vect) {
	if (POWER_DISABLED)
		return;
		
	// run startup animation (only done once)
	runStartupAnimation();
	
	// try to update a slave with new display data
	if (i2c_canTakeNewOutgoingMessage()) 
	{
		struct i2c_message_out m;
		m.receiver = current_slave_output+1;
		m.length = PROT_SET_ALL_LEDS_LENGTH;
		m.data[0] = PROT_SET_ALL_LEDS;
		
		uint8_t t;
		for (t = 0; t < 32; t++) 
		{	
			m.data[t+1] = ((BUFFER_getLED(current_slave_output,2*t)<<4)&0xf0) | ((BUFFER_getLED(current_slave_output,2*t+1)) & 0x0f);
		}
		
		i2c_addOutputBufferFront(m);
		
		current_slave_output++;
		if (current_slave_output >= NUM_ENCS)
			current_slave_output = 0;
	}
}

// pin change interrupt for power pin
ISR(PCINT0_vect) 
{
	if (POWER_ENABLED) 
	{
		// in case the output buffer is full and can't take more 
		// messages, we would have to wait in this interrupt until
		// there is space, because this message MUST be sent to the 
		// slaves. 
		// To avoid this we just set a flag and do it in the main loop.
		// The flag will be decreased by one for every slave that has 
		// received a power mode message, thus all slaves will get a
		// message.
		must_activate_power = NUM_ENCS;
	}
	else
	{
		// see above
		must_deactivate_power = NUM_ENCS;
	}
}
