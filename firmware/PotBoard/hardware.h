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
 
/*********************************************************
 * This file holds defines for hardware configuration
 * 
 * Using these in the code makes it much more readable.
 ********************************************************/

// Defines for the TLC5940
#define OUT_XLAT 	DDRD |= (1<<PD1)
#define XLAT_HIGH 	PORTD |= (1<<PD1)
#define XLAT_LOW 	PORTD &= ~(1<<PD1)

#define OUT_VPRG 	DDRB |= (1<<PB0)
#define VPRG_HIGH 	PORTB |= (1<<PB0)
#define VPRG_LOW 	PORTB &= ~(1<<PB0)

#define OUT_GSCLK	DDRB |= (1<<PB1)
#define GSCLK_HIGH 	PORTB |= (1<<PB1)
#define GSCLK_LOW 	PORTB &= ~(1<<PB1)

#define XERR_HIGH 	bit_is_set(PIND, PD0)

// Defines for the 74HC595
#define OUT_RCK 	DDRD |= (1<<PD4)
#define RCK_HIGH 	PORTD |= (1<<PD4)
#define RCK_LOW 	PORTD &= ~(1<<PD4)


// Defines for the communication port
#define INT_LOW 		PORTD &= ~(1<<PD2); DDRD |= (1<<PD2);
#define INT_RELEASE 	DDRD &= ~(1<<PD2);
#define INT_IS_HIGH 	bit_is_set(PIND, PD2)

// Defines for the adress switch
#define ADDR_SW_PULLUPS	PORTC |= (1<<PC0) | (1<<PC1) | (1<<PC2) | (1<<PC3)
#define ADRR			~(PINC | 0xF0) + 1


// Defines for the rotary encoder
#define ENC_SWITCH_PULLUP   PORTD |= (1<<PD3)
#define ENC_POWER_OUTPUT   	DDRD |= (1<<PD7)
#define ENC_POWER_OFF   	PORTD |= (1<<PD7)
#define ENC_POWER_ON	  	PORTD &= ~(1<<PD7)
#define ENC_SWITCH_IS_HIGH 	bit_is_set(PIND, PD3)
#define ENC_CHAN_A_IS_HIGH 	bit_is_set(PIND, PD5)
#define ENC_CHAN_B_IS_HIGH 	bit_is_set(PIND, PD6)



