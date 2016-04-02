#define F_CPU 16000000L

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

int16_t velocity = 0;
int16_t radius = 0x8000;

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
	Task_Terminate();
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
	Task_Terminate();
}

void move(char jsX[], char jsY[]){
	//X high - left
	if (jsX[0] || jsX[1] > 7){
		radius = radius > -1800 ? radius - 200 : -2000;
	} else if (jsX[1] < 3) {
	//X low - right
		radius = radius < 1800 ? radius + 200 : 2000;
	}
	//Y high - down
	if (jsY[0] || jsY[1] > 7) {
		velocity = velocity > 0 ? 0 : -100;
	} else if (jsY[1] < 3) {
	//Y low - up
		velocity = velocity < 0 ? 0 : 100;
	}
	
	Roomba_Drive(velocity, radius);
}

// Application level main function
// Creates the required tasks and then terminates
void a_main() {
	Roomba_Init();
	char line[16];
	uint16_t adc_test;
	portL2_Mutex = Mutex_Init();
	portL6_Mutex = Mutex_Init();
	e1 = Event_Init();
	e2 = Event_Init();
	adc_init();
	
	char start = 'z';
	char jsX[4];
	char jsY[4];
	char end = 'z';
	
	uint8_t song = 50;
	Roomba_PlaySong(song);
	
	for (;;){
		Roomba_Drive(-500, 0x8000);
		while (uart_bytes_received(BT_UART) < 30){
			Roomba_Drive(100,0x8000);
			_delay_ms(1000);
			Roomba_Drive(0, 0x8000);
			_delay_ms(1000);
			Roomba_Drive(-100, 0x8000);
			_delay_ms(1000);
			Roomba_Drive(0, 0x8000);
			_delay_ms(1000);
		}
		
		int i = 0;
		while (start != 115) {
			start = uart_get_byte(i, BT_UART);
			i++;
			if (i == 23){
				uart_reset_receive(BT_UART);
				i = 0;
			}
		}
		jsX[0] = uart_get_byte(i++, BT_UART);
		jsX[1] = uart_get_byte(i++, BT_UART);
		jsX[2] = uart_get_byte(i++, BT_UART);
		jsX[3] = uart_get_byte(i++, BT_UART);
		jsY[0] = uart_get_byte(i++, BT_UART);
		jsY[1] = uart_get_byte(i++, BT_UART);
		jsY[2] = uart_get_byte(i++, BT_UART);
		jsY[3] = uart_get_byte(i++, BT_UART);
		end = uart_get_byte(i++, BT_UART);
		uart_putchar(start, BT_UART);
		uart_putchar(end, BT_UART);
		if (start != 115 || end != 101) {
			uart_putchar(start, BT_UART);
			uart_putchar(end, BT_UART);
			Roomba_Drive(500, 0x8000);
			_delay_ms(1000);
			continue;
		}
		move(jsX, jsY);
		uart_reset_receive(BT_UART);
		Roomba_Drive(200, 2000);
		_delay_ms(1000);
		
	}
	//PongPID = Task_Create(Pong, 8, 1);
	//PingPID = Task_Create(Ping, 8, 1);
	//IdlePID = Task_Create(Idle, MINPRIORITY, 1);
	
	lcd_init();												// initialized the LCD
	DDRB |= (1<<DDB4);										// enable output mode of Digital Pin 10 (PORTB Pin 4) for backlit control
	PORTB |= (1<<DDB4);										// enable back light
	
	adc_test = adc_read(7);
	sprintf(line, "ADC: %4d", adc_test);
	lcd_puts(line);
	
	InitPID = Task_Create(Init_Task,8,1);
	DrivePID = Task_Create(Init_Drive, 8, 1);
	Task_Terminate();
}
