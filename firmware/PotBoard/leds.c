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
 

#define TRUE 1
#define FALSE 0

#include <inttypes.h>
#include <avr/io.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <math.h>

#include "leds.h"
#include "power.h"

#include "hardware.h"



#define OUTPUT_BUFFER_SIZE 25
uint8_t output_buffer[OUTPUT_BUFFER_SIZE];
uint8_t output_progress = 0;
uint8_t column = 0;

// look-up table for gamma correction
uint16_t lights_lut[16] = { 0, 14, 35, 62, 99, 150, 219, 313, 441, 614, 849, 1170, 1605, 2197, 3002, 4095 };

void setLED(uint8_t led, uint8_t value) 
{
	light_values[led] = lights_lut[value];
}

void setAllLEDs(uint8_t value) 
{
	int i;
	for (i = 0; i < 64; i++) 
	{
		light_values[i] = lights_lut[value];
	}
}

void SPI_Init( void ) 
{
	// set SPI pins to output
	DDRB |= (1<<PB3) | (1<<PB5)| (1<<PB2);
	
	// Enable SPI with interrupt, master, fast mode
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPIE);
	SPSR = (1<<SPI2X);
	
	// Set pullup on SS pin to avoid uC going to SPI slave mode
	PORTB |= (1<<PB2);
	
	// Set RCK pin to output, and low
	OUT_RCK;	
	RCK_HIGH;
}

void CLOCK_Init(void) 
{
	// Set grayscale-clock pin to output
	OUT_GSCLK;
	
	////////////////////////////////////////////////////////////////////
	// counter0 will trigger, when 4096 clock pulses have been fired on
	// the GSCLK pin
	
	// configure counter 0 as normal mode, prescaler 64.
	TCCR0B = (1<<CS00) | (1<<CS01);
	// configure interrupt on overflow (will occur every f_clk/(64*256))
	TIMSK0 = (1<<TOIE0);
	
	
	////////////////////////////////////////////////////////////////////
	// counter1 will generate the gs-clock for the tlc5940
	// it will run 4096x as fast as counter0
	
	// configure counter 1 as fast-pwm mode, prescaler 8.
	// when both counters are started simultanioulsly, counter0 will 
	// generate an interrupt after counter1 has done 4096 clock pulses.
	TCCR1B |= (1<<WGM12);
	
	
	// set prescaler to 1
	TCCR1B |= (1<<CS10);
	
	// set overflow to zero, so the GSCLK pin will be toggled with 
	// a frequency of f_clk/(4*1) = f_clk/4
	OCR1A = 1;
}

void LEDs_Init(void) 
{
	
	OUT_VPRG;
	OUT_XLAT;

	VPRG_HIGH;
	
		// initialize brightness values
	uint8_t i;
	for (i = 0; i < 64; i++) 
	{
		light_values[i] = 0;
	}
	
	
	column = 0;
	output_progress = OUTPUT_BUFFER_SIZE + 1;
	
	SPI_Init();

	// set dot correction
	uint8_t j;
	for (j = 0; j < 25; j++) 
	{
		output_buffer[j] = 0xFF;
	}
	output_progress = 1;
	// starts transmission
	SPDR = output_buffer[0];
	
	sei();
	_delay_ms(100);
	
	XLAT_HIGH;
	XLAT_LOW;
	
	// prepare for GS mode
	VPRG_LOW;
	
	CLOCK_Init();
}

void prepareOutputForColumn(uint8_t c) 
{
	// if power is down, never switch any led on!
	if (getPowerState() != ON)
	{
		int i;
		// select first column
		output_buffer[0] = 0b11011101;
		// all leds off.
		for (i = 1; i < OUTPUT_BUFFER_SIZE; i++) 
		{
			output_buffer[i] = 0;
		}
		return;
	}
	
	int i;
	// every second column has to be in reverse order (see schematic)
	switch (c) 
	{
		case 0: 
		{
			// select first column
			output_buffer[0] = 0b11011101;
			
			for (i = 0; i <8; i++) 
			{
				output_buffer[3*i+3] = light_values[14-2*(7-i)+1] & 0x00ff;
				output_buffer[3*i+2] = ((light_values[14-2*(7-i)]<<4)&(0xf0)) | ((light_values[14-2*(7-i)+1]>>8)&(0x0f));
				output_buffer[3*i+1] = (light_values[14-2*(7-i)]>>4) & 0x00ff;
			}
			break;
		}
		case 1: 
		{
			// select second column
			output_buffer[0] = 0b10111011;
			
			for (i = 0; i < 8; i++) 
			{
				output_buffer[3*i+3] = light_values[16+2*(7-i)] & 0x00ff;
				output_buffer[3*i+2] = ((light_values[16+2*(7-i)+1]<<4)&(0xf0)) | ((light_values[16+2*(7-i)]>>8)&(0x0f));
				output_buffer[3*i+1] = (light_values[16+2*(7-i)+1]>>4) & 0x00ff;
			}
			break;
		}
		case 2: 
		{
			// select third column
			output_buffer[0] = 0b01110111;
			
			for (i = 0; i < 8; i++) 
			{
				output_buffer[3*i+3] = light_values[46-2*(7-i)+1] & 0x00ff;
				output_buffer[3*i+2] = ((light_values[46-2*(7-i)]<<4)&(0xf0)) | ((light_values[46-2*(7-i)+1]>>8)&(0x0f));
				output_buffer[3*i+1] = (light_values[46-2*(7-i)]>>4) & 0x00ff;
			}
			break;
		}
		case 3: 
		{
			// select fourth column
			output_buffer[0] = 0b11101110;
			
			for (i = 0; i < 8; i++) 
			{
				output_buffer[3*i+3] = light_values[48+2*(7-i)] & 0x00ff;
				output_buffer[3*i+2] = ((light_values[48+2*(7-i)+1]<<4)&(0xf0)) | ((light_values[48+2*(7-i)]>>8)&(0x0f));
				output_buffer[3*i+1] = (light_values[48+2*(7-i)+1]>>4) & 0x00ff;
			}
			break;
		}
		default: 
		{
			// select no column
			output_buffer[0] = 0b11111111;
		}
	}
}

void startTransmission(void) 
{
	SPDR = output_buffer[0];
	output_progress = 1;
}

ISR(SPI_STC_vect) 
{
	if (output_progress < OUTPUT_BUFFER_SIZE) 
	{
		SPDR = output_buffer[output_progress];
		output_progress++;
	}
}

ISR(TIMER0_OVF_vect) 
{
	if (getPowerState() == OFF)
		return;
		
	/* how this works:
	 * 
	 * This timer is called at the end of a gs-cycle.
	 * The data for the next cycle has been clocked out to the tlc5940 
	 * and the 74hc595 in the previous timer interrupt. By pulsing
	 * XLT and RCK, the data is now latched into the internal buffers
	 * of the TLC5940 and is output on the 74hc595's output pins
	 * respectively.
	 * Then the gs clock is started again.
	 * while the new gs cycle is running, the gs data for the next cycle
	 * as pushed out to both chips.
	 */
	
	// stop interrupts now.
	cli();
	// stop gs-clock
	TCCR1A &= ~(1<<COM1A0);
	
	// wait for last transmission to complete
	//while(output_progress < OUTPUT_BUFFER_SIZE) {};
	
	// Trigger next GS cycle with the data sent before
	// by pulsing XLT and RCK

	
	XLAT_HIGH;
	asm volatile("nop\n\t""nop\n\t"::); 
	asm volatile("nop\n\t""nop\n\t"::); 
	asm volatile("nop\n\t""nop\n\t"::); 
	RCK_HIGH;
	
	// allow counter1 to toggle the grayscale-pin
	// => gs-cycle is started
	TCCR1A |= (1<<COM1A0);

	// now shift out the next values for the next cycle
	prepareOutputForColumn(column);
	
	RCK_LOW;
	XLAT_LOW;

	// be sure, VPRG is low to set the TLC5940 in grayscale-mode
	VPRG_LOW;
	
	// start clocking out grayscale data to the TLC5940
	startTransmission();
	
	// increase column
	column++;
	if (column > 3)
		column = 0;
	
	// enable interrupts again.
	sei();
}
