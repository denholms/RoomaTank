#define F_CPU 16000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "LED_Test.h"
#include "os.h"
#include "roomba/roomba.h"
#include "uart/uart.h"
#include "adc/adc.h"
#include "lcd/lcd_drv.h"

unsigned int portL2_Mutex;
unsigned int portL6_Mutex;

unsigned int e1;
unsigned int e2;

//unsigned int PingPID;
//unsigned int PongPID;
//unsigned int IdlePID;
unsigned int InitPID;
unsigned int DrivePID;

// An idle task that runs when there is nothing else to do
// Could be changed later to put CPU into low power state
void Idle() {
	for(;;) {
	}
}

// Ping task for testing
void Init_Task() {
	
	DDRB |= (1<<PB1);	//pin 52
	PORTB |= (1<<PB1);	//pin 52 on
	/*int  x;
	
	for(;;){
		Mutex_Lock(portL6_Mutex);
		// toggle_LED(PORTL6);
		Mutex_Unlock(portL6_Mutex);

		Event_Signal(e2);
		Event_Wait(e2);

		Task_Suspend(PongPID);
		Task_Resume(PongPID);

		Task_Sleep(100);
	}*/
	Roomba_Init();
	
}

// Pong task for testing
void Init_Drive() {
	DDRB |= (1<<PB2);	//pin 51
	PORTB &= ~(1<<PB1);	//pin 51 on
	/*int  x;
	
	for(;;) {
		Mutex_Lock(portL2_Mutex);
		// toggle_LED(PORTL2);
		Mutex_Unlock(portL2_Mutex);

		Event_Signal(e1);
		Event_Wait(e1);

		Task_Suspend(PingPID);
		Task_Resume(PingPID);

		Task_Sleep(100);
	}*/
	
	Roomba_Drive(100, 0x8000);
}


// Application level main function
// Creates the required tasks and then terminates
void a_main() {
	char line[16];
	portL2_Mutex = Mutex_Init();
	portL6_Mutex = Mutex_Init();
	uint16_t adc_test;
	e1 = Event_Init();
	e2 = Event_Init();
	adc_init();
	
	//PongPID = Task_Create(Pong, 8, 1);
	//PingPID = Task_Create(Ping, 8, 1);
	//IdlePID = Task_Create(Idle, MINPRIORITY, 1);
	lcd_init(); // initialized the LCD
	DDRB |= (1<<DDB4); // enable output mode of Digital Pin 10 (PORTB Pin 4) for backlit control
	PORTB |= (1<<DDB4); // enable back light
	adc_test = adc_read(7);
	sprintf(line, "ADC: %4d", adc_test);
	lcd_puts(line);
	
	
	

	
	InitPID = Task_Create(Init_Task,8,1);
	DrivePID = Task_Create(Init_Drive, 8, 1);
	Task_Terminate();
}
