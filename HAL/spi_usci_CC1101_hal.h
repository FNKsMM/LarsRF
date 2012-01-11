/*
* Author and copyright 2011: Lars Kristian Roland
* LarsRF by Lars Kristian Roland is licensed under a Creative Commons 
* Attribution-ShareAlike 3.0 Unported License.
* Based on a work at github.com.
* Permissions beyond the scope of this license may be available at http://lars.roland.bz/.
*/

#include "various.h"

#define WRITE	            0x00
#define BURST               0x40
#define READ                0x80

#define CS_low P2OUT &= ~BIT7;
#define CS_high P2OUT |= BIT7;

uint8 SPI_read_single( uint8 addr );
uint8 SPI_write_single( uint8 addr, uint8 value );

void SPI_write_burst(uint8 addr, uint8 *buffer, uint8 count);
void SPI_read_burst(uint8 addr, uint8 *buffer, uint8 count);

void SPI_strobe( uint8 strobe );
uint8 SPI_read_status( uint8 addr );
void SPI_init( void );