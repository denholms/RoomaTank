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
#define BTDDR DDRE
#define BTPORT PORTD
#define BTPIN PIND
#define BT  PD7
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
char sensordata[2];
char wall;
char virtualwall;


// An idle task that runs when there is nothing else to do
// Could be changed later to put CPU into low power state
void Idle() {
	for(;;) {
	}
}
void ButtonInit(){
	
	//PIN 38 value is 0 when pressed, 128 when not
	//as input
	BTDDR &= ~(1<<BT);
	//enable internal pullup
	BTPORT |= (1<<BT);
}

uint8_t ButtonRead(){
	
	
	return (1<<BT)&BTPIN;
	
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
	uart_putchar(2, ROOMBA_UART);
	uart_putchar(8,ROOMBA_UART);
	uart_putchar(13,ROOMBA_UART);
	_delay_ms(200);
	
}

void Read_Roomba_Data(){
	
	
	int index = 0;
	
	if (uart_bytes_received(BT_UART) >= 2)
	{
		while (index < 2)
		{
			sensordata[index] =  uart_get_byte(index, BT_UART);
			index ++;
		}
		uart_reset_receive(BT_UART);
		
		
		wall = sensordata[0];
		virtualwall = sensordata[1];
		
		
	}
	
	
}

void Poll_Joystick(){
	char line2[16];
	char buffer[5];
	uint16_t joystick_y;
	uint16_t joystick_x;
	uint8_t joystickpress;
	char button = 'f';
	
	//X high - left
	ButtonInit();
	
	//unsigned char send = 23;
	
	for (;;)
	{
		joystick_x = adc_read(7);
		joystick_y = adc_read(5);
		//uart_putchar('s', BT_UART);
		joystickpress = ButtonRead();
		
		if (joystickpress < 128)
		{
			button = 'o';
			
			
		}
		if (joystick_x > 700){
			//radius > -1800 ? radius - 200 : -2000;
			
			sprintf(buffer, "s4%ce\0", button);
			//uart_putchar((uint8_t)'4', BT_UART);
			
			} else if (joystick_x < 300) {
			//X low - right
			//radius = radius < 1800 ? radius + 200 : 2000;
			sprintf(buffer, "s3%ce\0", button);
			//uart_putchar('3', BT_UART);
		}
		//Y high - down
		if (joystick_y > 700) {
			//velocity = velocity > 0 ? 0 : -100;
			sprintf(buffer, "s1%ce\0", button);
			//uart_putchar('1', BT_UART);
			
			
			} else if (joystick_y < 300) {
			//Y low - up
			//velocity = velocity < 0 ? 0 : 100;
			sprintf(buffer, "s2%ce\0", button);
			//uart_putchar('2', BT_UART);
		}
		lcd_xy(0,0);
		//uart_putchar(send,1);
		sprintf(line2, "ADC:%2d ", joystick_x);
		//sprintf(line2, "Fucking Kill me");
		//lcd_puts(line2);
		lcd_puts(line2);
		lcd_xy(0,1);
		
		//sprintf(line2,"Jesus Fuck      ");
		sprintf(line2, "ADC:%2d ", joystickpress);
		lcd_puts(line2);
		
		//sprintf(buffer, "s%04d%04de\0", (int)joystick_x, (int)joystick_y);
		
		uart_send_string(buffer, BT_UART);
		//uart_putchar('e', BT_UART);
		sprintf(buffer, "s0e\0");
		
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
	//itoa(adc_test, jsBtn);
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
