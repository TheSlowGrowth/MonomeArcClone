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
 
/**
* ======================================================================
* Messages sent from USB board to pot boards
* ======================================================================
**/

/*
 * Clear all leds to 0.
 */
#define PROT_CLEAR 0x00
#define PROT_CLEAR_LENGTH 1

/*
 * Set the value of a single led
 * 
 * Format:
 * 		100000nn-nnnnvvvv
 * 
 * where 
 * 		n = number
 * 		v = value
 */
#define PROT_SET_SIGNLE_LED 0x10
#define PROT_SET_SIGNLE_LED_LENGTH 2

/*
 * Set the value of all 64 leds
 * 
 * Format:
 * 		00100000-aaaabbbb-ccccdddd- ... -yyyyzzzz
 * 
 * where
 * 		a = value of led0
 * 		b = value of led1
 * 		c = value of led2
 * 		d = value of led3
 * 		...
 * 
 */
#define PROT_SET_ALL_LEDS 0x20
#define PROT_SET_ALL_LEDS_LENGTH 33

/*
 * Forces the ring drivers to shut down their power.
 * 
 * Format:
 * 		0101000p
 * 
 * where
 * 		p = power mode (0 = off, 1 = on)
 */
#define PROT_POWER_MODE 0x50
#define PROT_POWER_MODE_LENGTH 1

/**
* ======================================================================
* Messages sent from pot boards to USB board
* ======================================================================
**/

/*
 * The encoder has been turned by the user
 * 
 * Format:
 * 		0x30 - 0nnnnnnn - vvvvvvvv
 * 
 * where
 * 		n = adress of sender
 * 		v = number of ticks in 2's complement (>0 = clockwise)
 */
#define PROT_ENCODER_TURNED 0x30
#define PROT_ENCODER_TURNED_LENGTH 3

/*
 * The encoder has been pushed by the user
 * 
 * Format:
 * 		0x40 - bnnnnnnn
 * 
 * where
 * 		n = adress of sender
 * 		b = button state (0 = released, 1 = pressed)
 */
#define PROT_BUTTON 0x40
#define PROT_BUTTON_LENGTH 2

