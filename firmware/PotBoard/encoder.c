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
 
#include <inttypes.h>
#include <avr/io.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <math.h>
#include "encoder.h"
#include "hardware.h"

uint8_t enc_state;
int16_t turns;
uint8_t enc_button_state;
uint8_t enc_button_sent_to_master;

void EncoderInit( void ) 
{
	ENC_SWITCH_PULLUP;
	enc_state = 0;
	enc_button_state = 0;
	enc_button_sent_to_master = 1;
	turns = 0;
}

int16_t getTurns( void ) 
{
	return turns;
}

uint8_t newButtonEvent( void ) 
{
	return enc_button_sent_to_master == 0;
}

uint8_t getButtonState( void ) 
{
	return enc_button_state;
}

void buttonEventSent( void ) 
{
	enc_button_sent_to_master = 1;
}

void resetTurns( void ) 
{
	turns = 0;
}

void checkEncoder( void ) 
{
	
	if ((ENC_SWITCH_IS_HIGH) && (enc_button_state != 0)) 
	{
		enc_button_state = 0;
		enc_button_sent_to_master = 0;
	}
	else if ((!ENC_SWITCH_IS_HIGH) && (enc_button_state != 1)) 
	{
		enc_button_state = 1;
		enc_button_sent_to_master = 0;
	}
	
	
	switch (enc_state) 
	{
		case 0: 
		{
			if ((ENC_CHAN_A_IS_HIGH) && (ENC_CHAN_B_IS_HIGH)) 
			{
				enc_state = 1;
				turns++;
			} 
			else if ((!ENC_CHAN_A_IS_HIGH) && (!ENC_CHAN_B_IS_HIGH)) 
			{
				enc_state = 3;
				turns--;
			} 
			break;
		}
		case 1: 
		{
			if ((!ENC_CHAN_A_IS_HIGH) && (ENC_CHAN_B_IS_HIGH)) 
			{
				enc_state = 2;
				turns++;
			} 
			else if ((ENC_CHAN_A_IS_HIGH) && (!ENC_CHAN_B_IS_HIGH)) 
			{
				enc_state = 0;
				turns--;
			} 
			break;
		}
		case 2: 
		{
			if ((!ENC_CHAN_A_IS_HIGH) && (!ENC_CHAN_B_IS_HIGH)) 
			{
				enc_state = 3;
				turns++;
			} 
			else if ((ENC_CHAN_A_IS_HIGH) && (ENC_CHAN_B_IS_HIGH)) 
			{
				enc_state = 1;
				turns--;
			} 
			break;
		}
		case 3: 
		{
			if ((ENC_CHAN_A_IS_HIGH) && (!ENC_CHAN_B_IS_HIGH)) 
			{
				enc_state = 0;
				turns++;
			} 
			else if ((!ENC_CHAN_A_IS_HIGH) && (ENC_CHAN_B_IS_HIGH)) 
			{
				enc_state = 2;
				turns--;
			} 
			break;
		}
	}
}
