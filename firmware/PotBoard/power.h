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
 

#ifndef POWER_H
#define POWER_H

#include <inttypes.h>

#define ON 1
#define OFF 0

// inits the power savong mechanisms.
void powerInit( void );

// tells the cpu to enable the power again.
void power_on( void );

// tells the cpu to go to sleep and stop updating the leds at the next
// call to try_go_to_sleep()
void power_off( void );

// trys to go to sleep if power_off() was called previously.
void try_go_to_sleep( void );

// returns whether the cpu should sleep
uint8_t getPowerState( void );

#endif
