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
 
#include "power.h"
#include "hardware.h"
#include "leds.h"
#include "main.h"

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

uint8_t power_state;

void powerInit( void )
{
	ENC_POWER_OUTPUT;
	ENC_POWER_ON;
	power_state = ON;
	
}

void power_on( void ) 
{
	ENC_POWER_ON;
	power_state = ON;
}

void power_off( void )
{
	ENC_POWER_OFF;
	power_state = OFF;
}
/*
void try_go_to_sleep( void ) 
{
	
	// whenever the cpu wakes up, it will execute some interrupt and 
	// then land at the sleep_disable() call below. If the interrupt
	// was a TWI interrupt and a message was received, then this message
	// will be executed. If it was a POWER_MODE message
	// that told the cpu to stay awaik permanently, then this condition
	// will be false and this function is left.
	// Otherwise the cpu will go to sleep again.
	while (power_state == OFF)
	{
		cli();
		// enter sleep mode
		sleep_enable();
		sei();
		
		sleep_cpu();
		
		// when this is executed, cpu is running again.
		sleep_disable();
		
		// This seems to be neccessary for the TWI to work
		// after a wake-up
		TWCR &= ~((1<<TWSTO) | (1<<TWEN ));
		TWCR |= (1<<TWEN) | (1<<TWINT) | (1<<TWIE) | (1<<TWEA); 
		
		// Check if a message has arrived and process it if so.
		processNewMessages();
		
		//if (power_state != OFF)
			// just for testing!!!
		//	PORTC &= ~(1<< PC3);
	}
}*/

uint8_t getPowerState( void )
{
	return power_state;
}
