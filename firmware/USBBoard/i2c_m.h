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
 

#ifndef I2C_MAX_MESSAGE_OUT_LENGTH
	#error I2C_MAX_MESSAGE_OUT_LENGTH is not defined
#endif
#ifndef I2C_MAX_MESSAGE_IN_LENGTH
	#error I2C_MAX_MESSAGE_IN_LENGTH is not defined
#endif

#ifndef I2C_BUFFER_OUT_LENGTH
	#error I2C_BUFFER_OUT_LENGTH is not defined
#endif
#ifndef I2C_BUFFER_IN_LENGTH
	#error I2C_BUFFER_IN_LENGTH is not defined
#endif

struct i2c_message_out 
{
	uint8_t receiver;
	uint8_t length;
	uint8_t data[I2C_MAX_MESSAGE_OUT_LENGTH];
};

struct i2c_message_in 
{
	uint8_t sender;
	uint8_t length;
	uint8_t data[I2C_MAX_MESSAGE_IN_LENGTH];
};


// inits the interface
void i2c_init( void );

// appends a message to be sent
int8_t i2c_hasOutgoingMessages( void );
int8_t i2c_hasIncomingMessages( void );

int8_t i2c_canTakeNewOutgoingMessage( void );
int8_t i2c_canTakeNewIncomingMessage( void );

volatile struct i2c_message_in* i2c_getInputBufferBack( void );
volatile struct i2c_message_in* i2c_getInputBufferFront( void );
void i2c_removeInputBufferBack( void );
int8_t i2c_addInputBufferFront( volatile struct i2c_message_in m );

volatile struct i2c_message_out* i2c_getOutputBufferBack( void );
volatile struct i2c_message_out* i2c_getOutputBufferFront( void );
void i2c_removeOutputBufferBack( void );
int8_t i2c_addOutputBufferFront( volatile struct i2c_message_out m );

void i2c_trySendMessages( void );
int8_t i2c_tryReceiveMessageFrom( uint8_t slave_adress );


// internal definitions

#define I2C_clock 300000L

// Status codes for i2c
#define I2C_start_sent 0x08
#define I2C_repeated_start_sent 0x10

// Status codes for i2c master receiver mode (TWSR)
#define I2C_arbitration_lost_or_nack_received 0x38
#define I2C_SLA_R_sent_ack_received 0x40
#define I2C_SLA_R_sent_nack_received 0x48
#define I2C_data_received_ack_sent 0x50
#define I2C_data_received_nack_sent 0x58

// Status codes for i2c master transmitter mode (TWSR)
#define I2C_SLA_W_sent_ack_received 0x18
#define I2C_SLA_W_sent_nack_received 0x20
#define I2C_data_sent_ack_received 0x28
#define I2C_data_sent_nack_received 0x30
 
#define I2C_ENABLE  (1<<TWEN) | (1<<TWINT) | (1<<TWIE)
#define I2C_ACK     (1<<TWEA) | I2C_ENABLE
#define I2C_NACK    I2C_ENABLE
#define I2C_START   (1<<TWSTA) | I2C_ENABLE
#define I2C_STOP    (1<<TWSTO) | I2C_ENABLE
