#ifndef _main_h_
#define _main_h_

/***************************************************************************/
/*                       define some helper functions:                     */
/***************************************************************************/
#define 	sbi(port, bit)   (port) |= (1 << (bit))
#define 	cbi(port, bit)   (port) &= ~(1 << (bit))

#include <avr/io.h>
#include <stdio.h>
#include <avr/sfr_defs.h>
#include "mydefs.h"

#define F_CPU 16000000L
#include <util/delay.h>

/***************************************************************************/
/*                       define the button values:                         */
/***************************************************************************/
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

/***************************************************************************/
/*                       define the wanted LCD type:                       */
/***************************************************************************/
//#define LCD_1X8
//#define LCD_1X16
//#define LCD_1X20
//#define LCD_1X40
//#define LCD_2X8       // some 1x16 are wired as 2x8
//#define LCD_2X12
#define LCD_2X16
//#define LCD_2X20
//#define LCD_2X24
//#define LCD_2X40
//#define LCD_4X16
//#define LCD_4X20

/***************************************************************************/
/*                       define the LCD connections:                       */
/***************************************************************************/
#define	LCD_D4		SBIT( PORTG, 5 )
#define	LCD_DDR_D4	SBIT( DDRG, 5 )

#define	LCD_D5		SBIT( PORTE, 3 )
#define	LCD_DDR_D5	SBIT( DDRE, 3 )

#define	LCD_D6		SBIT( PORTH, 3 )
#define	LCD_DDR_D6	SBIT( DDRH, 3 )

#define	LCD_D7		SBIT( PORTH, 4 )
#define	LCD_DDR_D7	SBIT( DDRH, 4 )

#define	LCD_RS		SBIT( PORTH, 5 )
#define	LCD_DDR_RS	SBIT( DDRH, 5 )

#define	LCD_E0		SBIT( PORTH, 6 )
#define	LCD_DDR_E0	SBIT( DDRH, 6 )

#endif
