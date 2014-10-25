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
 
#include <avr/pgmspace.h>
#include "led_buffer.h"

// length of one frame in multiples of 3ms (min: 4)
#define SUAN_FRAME_LENGTH 4

uint8_t startup_anim_frame;
uint8_t startup_anim_frame_subpos;
uint8_t startup_anim_done;

void initStartupAnimation( void ) {
	startup_anim_done = 0;
	startup_anim_frame = 0;
	startup_anim_frame_subpos = 0;
}

void runStartupAnimation( void ) 
{
	if (startup_anim_done > 0)
		return;
	
	if (startup_anim_frame_subpos >= SUAN_FRAME_LENGTH)
	{
		startup_anim_frame_subpos = 0;
		
		if (startup_anim_frame == 7)
		{
			BUFFER_setAllInEncoder(0, 1);
		}
		else if (startup_anim_frame == 8)
		{
			BUFFER_setAllInEncoder(0, 2);
		}
		else if (startup_anim_frame == 9)
		{
			BUFFER_setAllInEncoder(0, 3);
		}
		else if (startup_anim_frame == 10)
		{
			BUFFER_setAllInEncoder(0, 4);
		}
		//===========
		else if (startup_anim_frame == 16)
		{
			BUFFER_setAllInEncoder(1, 1);
		}
		else if (startup_anim_frame == 17)
		{
			BUFFER_setAllInEncoder(1, 2);
		}
		else if (startup_anim_frame == 18)
		{
			BUFFER_setAllInEncoder(1, 3);
		}
		//===========
		else if (startup_anim_frame == 26)
		{
			BUFFER_setAllInEncoder(2, 1);
		}
		else if (startup_anim_frame == 27)
		{
			BUFFER_setAllInEncoder(2, 2);
		}
		//===========
		else if (startup_anim_frame == 40)
		{
			BUFFER_setAllInEncoder(3, 1);
		}
		//===========
		else if (startup_anim_frame == 100)
		{
			BUFFER_setAllInEncoder(0, 3);
			BUFFER_setAllInEncoder(1, 2);
			BUFFER_setAllInEncoder(2, 1);
			BUFFER_setAllInEncoder(3, 0);
		}
		else if (startup_anim_frame == 101)
		{
			BUFFER_setAllInEncoder(0, 2);
			BUFFER_setAllInEncoder(1, 1);
			BUFFER_setAllInEncoder(2, 0);
		}
		else if (startup_anim_frame == 102)
		{
			BUFFER_setAllInEncoder(0, 1);
			BUFFER_setAllInEncoder(1, 0);
		}
		else if (startup_anim_frame == 103)
		{
			BUFFER_setAllInEncoder(0, 0);/*
		}
		else if (startup_anim_frame == 104)
		{
			BUFFER_setAll(3);
		}
		else if (startup_anim_frame == 105)
		{
			BUFFER_setAll(2);
		}
		else if (startup_anim_frame == 106)
		{
			BUFFER_setAll(1);
		}
		else if (startup_anim_frame == 107)
		{
			BUFFER_setAll(0);*/
			startup_anim_done = 1;
		}
		
		startup_anim_frame++;
	}
	else 
		startup_anim_frame_subpos++;
}
