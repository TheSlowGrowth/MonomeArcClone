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
 
// defines hardware specific stuff

#define NUM_ENCS			4

#define POWER_PIN_INIT  	DDRB &= ~(1<< PB0)

#define POWER_ENABLED		bit_is_clear(PINB, PB0)
#define POWER_DISABLED		bit_is_set(PINB, PB0)

// I2C Interface
#define INT_IS_HIGH 		bit_is_set(PINB, PB1)
#define INT_PULLUP			{ DDRB &= ~(1<<PB1); PORTB |= (1<<PB1); }

// FTDI 245
#define FTDI_DATA_REG_PIN 	PIND
#define FTDI_DATA_REG_PORT	PORTD
#define FTDI_DATA_REG_DDR	DDRD

#define FTDI_RXF			PC3
#define FTDI_TXE			PC2
#define FTDI_RD				PC1
#define FTDI_WR				PC0

#define FTDI_STAT_PIN		PINC
#define FTDI_STAT_PORT		PORTC
#define FTDI_STAT_DDR		DDRC
