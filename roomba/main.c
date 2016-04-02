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
#include "roomba/sensor_struct.h"

unsigned int portL2_Mutex;
unsigned int portL6_Mutex;

unsigned int e1;
unsigned int e2;
roomba_sensor_data_t data;
//unsigned int PingPID;
//unsigned int PongPID;
//unsigned int IdlePID;
unsigned int InitPID;
unsigned int DrivePID;
unsigned int IdlePID ;

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


void Poll_Roomba_Data()
{ 
	//uint8_t playsong = 0;
	//Roomba_PlaySong(playsong);
	_delay_ms(200);
	lcd_xy(0,0);
	//Roomba_Drive(100, 0x8000);
	
	lcd_puts("Polling Sensor  ");
	_delay_ms(20);
	uart_putchar(149,ROOMBA_UART);
	uart_putchar(4, ROOMBA_UART);
	uart_putchar(7,ROOMBA_UART);
	uart_putchar(8,ROOMBA_UART);
	uart_putchar(27,ROOMBA_UART);
	uart_putchar(13,ROOMBA_UART);
	_delay_ms(200);
	
}

void Read_Roomba_Data(){
	
	
	
}

void Poll_Joystick(){
	char line2[16];
	char buffer[7];
	uint16_t joystick_y;
	uint16_t joystick_x;
	
	
	unsigned char send = 23;
	
	for (;;)
	{
		joystick_x = adc_read(7);
		joystick_y = adc_read(5);
		lcd_xy(0,0);
		uart_putchar(send,1);
		sprintf(line2, "ADC:%2d ", joystick_x);
		//sprintf(line2, "Fucking Kill me");
		lcd_puts(line2);
		lcd_xy(0,1);
		
		//sprintf(line2,"Jesus Fuck      ");
		sprintf(line2, "ADC:%2d ", joystick_y);
		lcd_puts(line2);
		
		sprintf(buffer, "s%04d%04de\0", (int)joystick_x, (int)joystick_y);
		
		uart_send_string(buffer, 1);
		//Poll_Roomba_Data();
		//uart_send_string(buffer, ROOMBA_UART);
		_delay_ms(20);
		
	}
	
	
}

// Application level main function
// Creates the required tasks and then terminates
void a_main() {
	char line[16];
	portL2_Mutex = Mutex_Init();
	portL6_Mutex = Mutex_Init();
	unsigned char jsBtn;
	
	
	e1 = Event_Init();
	e2 = Event_Init();
	adc_init();
	uint16_t adc_test = adc_read(7);
	//PongPID = Task_Create(Pong, 8, 1);
	//PingPID = Task_Create(Ping, 8, 1);
	//IdlePID = Task_Create(Idle, MINPRIORITY, 1);
	lcd_init(); // initialized the LCD
	lcd_xy(0,0);
	DDRB |= (1<<DDB4); // enable output mode of Digital Pin 10 (PORTB Pin 4) for backlit control
	PORTB |= (1<<DDB4); // enable back light
	itoa(adc_test, jsBtn);
	//sprintf(line, "ADC:%2d", adc_test);
	lcd_puts(line);
	//lcd_xy(0,1);
	//sprintf(line, "Laser: %s", jsBtn);
	//lcd_puts(line);
	Roomba_Init();
	//uart_init(UART_38400);
	//Roomba_Drive(100, 0x8000);
	

	
			//Roomba_PlaySong(1);
			//_delay_ms(200);
	


	
	
    InitPID = Task_Create(Poll_Joystick,0,1);
	//DrivePID = Task_Create(Init_Drive, 8, 1);
	//IdlePID = Task_Create(Idle, 8, 1);
	Task_Terminate();
}
