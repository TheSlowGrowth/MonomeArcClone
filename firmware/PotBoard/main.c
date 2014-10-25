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

#include "leds.h"
#include "hardware.h"
#include "encoder.h"


// defines for the I2C interface
#define I2C_MAX_MESSAGE_OUT_LENGTH 4
#define I2C_MAX_MESSAGE_IN_LENGTH 34

#define I2C_BUFFER_OUT_LENGTH 6
#define I2C_BUFFER_IN_LENGTH 4

#define I2C_PORT PINC
#define I2C_SDA_PIN PC4
#define I2C_SCL_PIN PC5

#include "i2c_s.c"
#include "..\protocol.h"
#include "power.h"

int is_in_bounds(int16_t a, int16_t b, int16_t test) 
{
	if (a <= b) 
	{
		if ((test >= a) && (test <= b)) 
		{
			return 1;
		}
		else 
		{
			return 0; 
		}
	} else if (b < a) 
	{
		if ((test >= b) && (test <= a)) 
		{
			return 1;
		}
		else 
		{
			return 0; 
		}
	}
	return 0;		
}

void processNewMessages( void ) 
{
		
	while (i2c_hasIncomingMessages()) 
	{
		
		volatile struct i2c_message_in* m;
		m = i2c_getInputBufferBack();
		
		// clear command
		if (((m->data[0] & 0xF0) == PROT_CLEAR) 
			&& (m->length == PROT_CLEAR_LENGTH)) 
		{
			power_on();	
			uint8_t i;
			for (i = 0; i < 64; i++) 
			{
				setLED(i, 0);
			}
		} 
		// set single led command
		else if (((m->data[0] & 0xF0) == PROT_SET_SIGNLE_LED) 
			&& (m->length == PROT_SET_SIGNLE_LED_LENGTH)) 
		{
			power_on();	
			uint8_t led = ((m->data[0] & 0b00000011)<<4) | ((m->data[1]>>4) & 0x0f);
			uint8_t intensity = m->data[1] & 0x0f;
			setLED(led, intensity);
		}
		// set all leds command
		else if (((m->data[0] & 0xF0) == PROT_SET_ALL_LEDS) 
			&& (m->length == PROT_SET_ALL_LEDS_LENGTH)) 
		{
			power_on();
			uint8_t i;	
			for (i = 0; i < 32; i++) 
			{
				uint8_t intensitya = (m->data[i+1] & 0xf0) >> 4;
				uint8_t intensityb = m->data[i+1] & 0x0f;
				setLED(2*i, intensitya);
				setLED(2*i+1, intensityb);
			}
		}
		// power mode command
		else if (((m->data[0] & 0xF0) == PROT_POWER_MODE) 
			&& (m->length == PROT_POWER_MODE_LENGTH)) 
		{
			if (m->data[0] & 0b00000001) 
			{
				power_on();
			}
			else 
			{
				power_off();
			}
		}
		
		i2c_removeInputBufferBack();
	}
}

int main (void) 
{	
	ADDR_SW_PULLUPS;	
	_delay_ms(200);
	i2c_init(ADRR);
	
	powerInit();
	EncoderInit();
	LEDs_Init();
	
	sei();

	// main loop to do not-so-time-critical things
	while(1)
	{
		// check for changes on the encoder
		checkEncoder();	
		
		// process new messages from the USB board
		processNewMessages();
		
		// prepare new messages if necessary
		
		// first message: button changed?
		// check, if buffer can take new message
		if (i2c_canTakeNewOutgoingMessage()) 
		{
			// New button event
			if (newButtonEvent()) 
			{
				struct i2c_message_out m;
				m.length = PROT_BUTTON_LENGTH;
				m.data[0] = PROT_BUTTON;
				m.data[1] = i2c_getAdress()&0b01111111;
				
				if (getButtonState() == 1) 
					m.data[1] |= 0b10000000;
					
				// if message is stored in output buffer ...
				if (i2c_addOutputBufferFront(m)) {
					// ... mark this event as "sent".
					buttonEventSent();
				}
			}
		}
		
		// second message: encoder turned?
		// check once more, in case the buffer is full now
		if (i2c_canTakeNewOutgoingMessage()) 
		{
			// New encoder event
			int16_t turns = getTurns();
			
			if (turns != 0) 
			{
				struct i2c_message_out m;
				m.length = PROT_ENCODER_TURNED_LENGTH;
				m.data[0] = PROT_ENCODER_TURNED;
				m.data[1] = i2c_getAdress()&0b01111111;
				m.data[2] = (int8_t) turns;
					
				// if message is stored in output buffer ...
				if (i2c_addOutputBufferFront(m)) 
				{
					// ... mark this event as "sent".
					resetTurns();
				}
			}
		}
		
		// Try to start sending the previously prepared messages
		// messages a resent asynchronously, so this will only initiate 
		// sending something if necessary
		i2c_trySendMessages();
		
		
		// if the USB board told us to go to sleep, we should try to
		// sleep now
		//try_go_to_sleep();
	}
	
	return 0;
}

