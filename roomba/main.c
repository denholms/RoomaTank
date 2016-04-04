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
unsigned int IdlePID;
unsigned int InitPID;
unsigned int Man_DrivePID;
unsigned int Auto_DrivePID;
unsigned int SensePID;

int16_t velocity = 0;
int16_t radius = 0x8000;
uint16_t light_threshold;

// An idle task that runs when there is nothing else to do
// Could be changed later to put CPU into low power state
void Idle() {
	for(;;) {
	}
}

// Ping task for testing
/*
void Init_Task() {
	
	DDRB |= (1<<PB1);	//pin 52
	PORTB |= (1<<PB1);	//pin 52 on
	int  x;
	
	for(;;){
		Mutex_Lock(portL6_Mutex);
		// toggle_LED(PORTL6);
		Mutex_Unlock(portL6_Mutex);

		Event_Signal(e2);
		Event_Wait(e2);

		Task_Suspend(PongPID);
		Task_Resume(PongPID);

		Task_Sleep(100);
	}
	Roomba_Init();
	Task_Terminate();
}*/

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

void Man_Drive(uint8_t dir){
	switch ((int)dir){
		case 48:
			//No movement
			radius = 0x8000;
			break;
		case 49:
			//Left
			radius = (velocity == 0) ? -1 : -200;
			break;
		case 50:
			//Right
			radius = (velocity == 0) ? 1 : 200;
			break;
		case 51:
			//Down
			velocity = (velocity > 0) ? 0 : -200;
			break;
		case 52:
			//Up
			velocity = (velocity < 0) ? 0 : 200;
			break;
		default:
			break;
			
	}
	Roomba_Drive(velocity, radius);
	return;
}

void Auto_Drive() {
	uart_putchar(CLEAN, ROOMBA_UART);
	_delay_ms(20);
	return;
}

void Sense(){
	uint16_t photo_resist = 0;
	uint8_t start = 255;
	uint8_t dir = 0;
	uint8_t laser_btn = 255;
	uint8_t end = 255;
	
	for (;;) {
		photo_resist = 0;
		
		// Read photo-resistor
		photo_resist = adc_read(7);

		// Hit by laser
		if (photo_resist > light_threshold){

			//Light up LED
			PORTG |= (1<<PG2);
			Idle();
		}
		
		// If enough data received
		if (uart_bytes_received(BT_UART) >= 5) {
			//uart_reset_receive(BT_UART);
			
			start = uart_get_byte(1, BT_UART);
			end = uart_get_byte(4, BT_UART);
			uart_putchar(uart_bytes_received(BT_UART), BT_UART);
			uart_putchar(start, BT_UART);
			uart_putchar(end, BT_UART);
			// Validate framing
			if (start == (uint8_t)'s' && end == (uint8_t)'e'){
				dir = uart_get_byte(2, BT_UART);
				laser_btn = uart_get_byte(3, BT_UART);
				uart_putchar(dir, BT_UART);
				uart_putchar(laser_btn, BT_UART);
				
				// On == 'o'
				if (laser_btn == 111){
					uart_putchar('o', BT_UART);
					//FIRE LASER
					PORTD |= (1<<PD7);
				}
				// Direction input (joystick) results in manual drive
				if (dir != 48){
					Man_Drive(dir);
				} else {
					Auto_Drive();
				}
			}
			uart_reset_receive(BT_UART);
		}
		
		//Turn laser off
		PORTD &= ~(1<<PD7);
	}
	Task_Terminate();
}

void Init_Task(){
	Roomba_Init();
	adc_init();
	portL2_Mutex = Mutex_Init();
	portL6_Mutex = Mutex_Init();
	e1 = Event_Init();
	e2 = Event_Init();
	DDRD |= (1<<PD7);
	DDRG |= (1<<PG2);
	PORTD &= ~(1<<PD7);
	PORTG &= ~(1<<PG2);
	
	int sum = 0;
	for (int i = 0; i< 10; i++){
		sum += adc_read(7);
	}
	light_threshold = (int)((sum/10)*1.4);
	
	Task_Terminate();
}

// Application level main function
// Creates the required tasks and then terminates
void a_main() {
	//Init_Task();
	InitPID		= Task_Create(Init_Task,0,1);
	SensePID	= Task_Create(Sense, 1, 1);
	IdlePID		= Task_Create(Idle, 8, 1);
	//Man_DrivePID = Task_Create(Man_Drive, 1, 1);
	//Auto_DrivePID = Task_Create(Auto_Drive, 3, 1);
	
	/*char line[16];
	uint16_t adc_test;
	adc_test = adc_read(7);
	sprintf(line, "%04d\0", adc_test);
	uart_send_string(line, BT_UART);
	_delay_ms(50);*/
	
	Task_Terminate();
}
