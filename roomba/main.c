/*
 * roomba.c
 *
 * Created: 2016-03-27 6:47:38 PM
 * Author : denholms
 */ 
#define F_CPU 16000000UL

#include <avr/io.h>
#include "roomba/roomba.h"
#include "uart/uart.h"

int main(void)
{
    Roomba_Init();
	Roomba_Drive(100, 0x8000);
	for(;;){}
	return 0;
}

