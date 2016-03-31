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
#include <string.h>

unsigned int portL2_Mutex;
unsigned int portL6_Mutex;

unsigned int e1;
unsigned int e2;

//unsigned int PingPID;
//unsigned int PongPID;
//unsigned int IdlePID;
unsigned int InitPID;
unsigned int DrivePID;
unsigned int PollPID;
unsigned int IdlePID;
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
	lcd_blank(8);
	lcd_puts("Init     ");
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
	//_delay_ms(2000);
	Roomba_Init();
	
	
	//Task_Terminate();
	
	
}

// Pong task for testing
void Init_Drive() {
	DDRB |= (1<<PB2);	//pin 51
	PORTB |= (1<<PB1);	//pin 51 on
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
	lcd_puts("drive");
}

void Poll_Roomba_Data(){
	DDRB |= (1<<PB2);	//pin 51
	PORTB &= ~(1<<PB1);	//pin 51 on
	//Roomba_Init();
	char toroomba[30];
	char fromroomba[30];
	char processedstring[30];
	//sprintf(toroomba, "148113");
	lcd_puts("Play song");
	//uart0_init(UART_19200);
	
	//uart0_putc(148);
	//uart0_putc(1);
	//uart0_putc(13);
	Roomba_PlaySong(2);
	
	
	/*while (uart0_available() > 0)
	{
		int index = 0;
		fromroomba [index] = uart0_getc();
		index ++;
		
		if (index == 29)
		{
			fromroomba[29] = '\0';
		}
		
	}*/
	
	//strcpy(processedstring, fromroomba);
	//fromroomba[0] = '\0';
	
	
	//lcd_puts(processedstring);
	
	
	
	
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
	//sprintf(line, "ADC: %4d", adc_test);
	//lcd_puts(line);
	
	
	Init_Task();
	Roomba_Drive(100, 0x8000);
	//Poll_Roomba_Data();
	
	for (;;)
	{
		Roomba_PlaySong(1);
		_delay_ms(200);
		PORTB &= ~(1<<PB1);
		_delay_ms(200);
		PORTB |= (1<<PB1);
	}

	
	//InitPID = Task_Create(Init_Task,8,1);
	//PollPID = Task_Create(Poll_Roomba_Data, 8, 1);
	//IdlePID = Task_Create(Idle, 9, 1);
	//DrivePID = Task_Create(Init_Drive, 8, 1);
	Task_Terminate();
}
