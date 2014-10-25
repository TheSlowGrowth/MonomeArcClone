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
 

#include "led_buffer.h"
#include "hardware.h"

uint8_t buffer[NUM_ENCS][64];

void BUFFER_init( void ) 
{
	BUFFER_setAll(0);
}

void BUFFER_setLED(uint8_t n, uint8_t x, uint8_t val) 
{
	buffer[n][x] = val;
}

void BUFFER_setAll(uint8_t val) 
{
	int n;
	int x;
	for (n = 0; n < NUM_ENCS; n++) 
	{
		for (x = 0; x < 64; x++) 
		{
			buffer[n][x] = val;
		}
	}
}

void BUFFER_setAllInEncoder(uint8_t n, uint8_t val) 
{
	int x;
	for (x = 0; x < 64; x++) 
	{
		buffer[n][x] = val;
	}
}

void BUFFER_setCompleteEncoder(uint8_t n, uint8_t* vals) 
{
	int x;
	for (x = 0; x < 32; x++) 
	{
		buffer[n][2*x + 1] = vals[x]&0x0F;
		buffer[n][2*x    ] = (vals[x]>>4)&0x0F;
	}
}


void BUFFER_setRange(uint8_t n, uint8_t start, uint8_t end, uint8_t val) 
{
	if (start <= end) 
	{
		int x;
		for (x = start; x <= end ; x++) 
		{
			buffer[n][x] = val;
		}
	}
	else 
	{
		int x;
		for (x = start; x < 64; x++) 
		{
			buffer[n][x] = val;
		}
		for (x = 0; x <= end; x++) 
		{
			buffer[n][x] = val;
		}	
	}
}

uint8_t BUFFER_getLED(uint8_t n, uint8_t x) 
{
	return buffer[n][x];
}

uint8_t* BUFFER_getArray( uint8_t n ) 
{
	return buffer[n];
}

