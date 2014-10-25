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
 

// LED commands
#define COMM_I_LED_SINGLE				0x90
#define COMM_I_LED_SINGLE_LENGTH		4

#define COMM_I_LED_ALL 					0x91
#define COMM_I_LED_ALL_LENGTH			3

#define COMM_I_LED_COMPLETE 			0x92
#define COMM_I_LED_COMPLETE_LENGTH		34

#define COMM_I_LED_RANGE 				0x93
#define COMM_I_LED_RANGE_LENGTH			5

// Encoder Commands
#define COMM_O_ENCODER					0x50
#define COMM_O_ENCODER_LENGTH			3

#define COMM_O_ENCODER_UP				0x51
#define COMM_O_ENCODER_UP_LENGTH		2

#define COMM_O_ENCODER_DOWN				0x52
#define COMM_O_ENCODER_DOWN_LENGTH		2


// System Commands IN
#define COMM_I_QUERY 					0x00
#define COMM_I_QUERY_LENGTH				1

#define COMM_I_QUERY_ID 				0x01
#define COMM_I_QUERY_ID_LENGTH			1

#define COMM_I_WRITE_ID 				0x02
#define COMM_I_WRITE_ID_LENGTH			33

#define COMM_I_REQ_GRID_OFFSET 			0x03
#define COMM_I_REQ_GRID_OFFSET_LENGTH	1

#define COMM_I_SET_GRID_OFFSET 			0x04
#define COMM_I_SET_GRID_OFFSET_LENGTH	4

#define COMM_I_REQ_GRID_SIZE 			0x05
#define COMM_I_REQ_GRID_SIZE_LENGTH		1

#define COMM_I_SET_GRID_SIZE 			0x06
#define COMM_I_SET_GRID_SIZE_LENGTH		3

#define COMM_I_GET_ADDR 				0x07
#define COMM_I_GET_ADDR_LENGTH			1

#define COMM_I_SET_ADDR 				0x08
#define COMM_I_SET_ADDR_LENGTH			3

#define COMM_I_GET_FIRMWARE 			0x0F
#define COMM_I_GET_FIRMWARE_LENGTH		1

// SystemCommands OUT

#define COMM_O_QUERY_RESP 				0x00
#define COMM_O_QUERY_RESP_LENGTH		3

#define COMM_O_QUERY_ID_RESP 			0x01
#define COMM_O_QUERY_ID_RESP_LENGTH		33

#define COMM_O_REP_GRID_OFFSET			0x02
#define COMM_O_REP_GRID_OFFSET_LENGTH	4

#define COMM_O_REP_GRID_SIZE			0x03
#define COMM_O_REP_GRID_SIZE_LENGTH		3

#define COMM_O_REP_ADDR					0x04
#define COMM_O_REP_ADDR_LENGTH			3

#define COMM_O_REP_FIRMWARE				0x0F
#define COMM_O_REP_FIRMWARE_LENGTH		9

