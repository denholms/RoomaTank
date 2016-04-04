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
uint8_t bumper;
uint8_t virtualwall;
uint8_t IRwalls;


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
			velocity = (velocity > 100) ? 0 : -200;
			break;
		case 52:
			//Up
			velocity = (velocity <= -200) ? 0 : 200;
			break;
		default:
			break;
			
	}
	Roomba_Drive(velocity, radius);
	_delay_ms(200);
	return;
}

void Get_Roomba_Data(){
	
	//char command[50] = {149, 3, 7, 13, 45, '\0'};
	//uart_send_string(command, ROOMBA_UART);
	for (;;){
		uart_putchar(SENSOR, ROOMBA_UART);
		uart_putchar(AMOUNT, ROOMBA_UART);
		uart_putchar(BUMP, ROOMBA_UART);
		uart_putchar(VIRTUALWALL, ROOMBA_UART);
		uart_putchar(IRWALLS, ROOMBA_UART);
		_delay_ms(20);
	
		while(!(UCSR0A & (1<<RXC0))){
			uart_putchar('#', BT_UART);	
			uart_putchar(SENSOR, ROOMBA_UART);
			uart_putchar(AMOUNT, ROOMBA_UART);
			uart_putchar(BUMP, ROOMBA_UART);
			uart_putchar(VIRTUALWALL, ROOMBA_UART);
			uart_putchar(IRWALLS, ROOMBA_UART);
			_delay_ms(20);
		}
		bumper = UDR0;
		while(!(UCSR0A & (1<<RXC0)));
		virtualwall = UDR0;
		while(!(UCSR0A & (1<<RXC0)));
		IRwalls = UDR0;
		uart_putchar('%', BT_UART);
		uart_putchar(bumper, BT_UART);
		uart_putchar(virtualwall, BT_UART);
		uart_putchar(IRwalls, BT_UART);
		uart_putchar('$', BT_UART);
		uart_reset_receive(ROOMBA_UART);
		
	}
	
	bumper = uart_get_byte(0,ROOMBA_UART);
	virtualwall = uart_get_byte(1, ROOMBA_UART);
	IRwalls = uart_get_byte(2,ROOMBA_UART);
	uart_reset_receive(ROOMBA_UART);
}

void Auto_Drive() {
	//uart_putchar(CLEAN, ROOMBA_UART);
	
	Get_Roomba_Data();
	
	if (bumper &= 0b00000001) // bump right
	{
		Roomba_Drive(-100, 0x8000);
		_delay_ms(50);
		Roomba_Drive(0, -1);
		_delay_ms(50);
	} else if (bumper &= 0b00000010) // bump left
	{
		Roomba_Drive(-100, 0x8000);
		_delay_ms(50);
		Roomba_Drive(0, 1);
		_delay_ms(50);
		
	} else if (virtualwall){
		
		Roomba_Drive(0, -1);
		_delay_ms(100);
		Roomba_Drive(100, 0x8000);
	}
	PORTG |= (1<<PG2);
	_delay_ms(20);
	PORTG &= ~(1<<PG2);
	_delay_ms(20);
	return;
}

void Sense(){
	uint16_t photo_resist = 0;
	uint8_t start = 255;
	uint8_t dir = 0;
	uint8_t laser_btn = 255;
	uint8_t end = 255;
	int count = 0;
	
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
				if (dir == 48){
					count += 1;
					if (count >= 50){
						count = 0;
						Auto_Drive();
					}
				} else {
					Man_Drive(dir);
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
