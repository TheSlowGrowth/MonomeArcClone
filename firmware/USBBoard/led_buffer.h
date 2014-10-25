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
 

void BUFFER_init( void );
void BUFFER_setLED(uint8_t n, uint8_t x, uint8_t val);
void BUFFER_setAll(uint8_t val);
void BUFFER_setAllInEncoder(uint8_t n, uint8_t val);
void BUFFER_setCompleteEncoder(uint8_t n, uint8_t* vals);
void BUFFER_setRange(uint8_t n, uint8_t start, uint8_t end, uint8_t val);
uint8_t BUFFER_getLED(uint8_t n, uint8_t x);
uint8_t* BUFFER_getArray( uint8_t n );
