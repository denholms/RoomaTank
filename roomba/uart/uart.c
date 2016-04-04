/**
 * @file   uart.c
 * @author Justin Tanner
 * @date   Sat Nov 22 21:32:03 2008
 *
 * @brief  UART Driver targetted for the AT90USB1287
 *
 */
#include "uart.h"

#define F_CPU 16000000UL

#ifndef F_CPU
#warning "F_CPU not defined for uart.c."
#define F_CPU 11059200UL
#endif

static volatile uint8_t uart_buffer[UART_BUFFER_SIZE];
static volatile uint8_t uart_buffer_1[UART_BUFFER_SIZE];

static volatile uint8_t uart_buffer_index;
static volatile uint8_t uart_buffer_1_index;

/**
 * Initalize UART
 * 
 */
void uart_init(UART_BPS bitrate)
{
	UCSR0A = _BV(U2X1);									// Double speed (async) control (On when U2Xn = 1, set to 0 when doing synchronous transfer)
	UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);		// Activate receiver, transmitter, and receive complete flag (0 when buffer empty)
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);					// USART Control and Status Register C (selects async/sync operation), sets frame format

	UCSR1A = _BV(U2X1);
	UCSR1B = _BV(RXEN1) | _BV(TXEN1) | _BV(RXCIE1);
	UCSR1C = _BV(UCSZ11) | _BV(UCSZ10);

	//UBRR0H = 0;											// Baud Rate register - for any speed >= 9600 bps, the UBBR value fits in the low byte.
	//UBRR1H = 0;
	//UBRR1L = 207;	//103
	UBRR1 = 207;
	//UBRR0L = 103;
	UBRR0 = 103;
/*
	// See the appropriate AVR hardw                                             \\\\\\\\\\\\\\\\\\are specification for a table of UBBR values at different
	// clock speeds.
	switch (bitrate)
	{
#if F_CPU==8000000UL
	case UART_19200:
		UBRR0L = 51;
		//UBRR1L = 51;
		break;
	case UART_38400:
		UBRR0L = 25;
		//UBRR1L = 25;
		break;
	case UART_57600:
		UBRR0L = 16;
		//UBRR1L = 16;
		break;
	default:
		UBRR0L = 51;
		//UBRR1L = 51;
#elif F_CPU==16000000UL
	case UART_19200:
		UBRR0L = 103;
		//UBRR1L = 103;
		break;
	case UART_38400:
		UBRR0L = 51;
		//UBRR1L = 51;
		break;
	case UART_57600:
		UBRR0L = 34;
		//UBRR1L = 34;
		break;
	default:
		UBRR0L = 103;
		//UBRR1L = 103;
#elif F_CPU==18432000UL
	case UART_19200:
		UBRR0L = 119;
		//UBRR1L = 119;
		break;
	case UART_38400:
		UBRR0L = 59;
		//UBRR1L = 59;
		break;
	case UART_57600:
		UBRR0L = 39;
		//UBRR1L = 39;
		break;
	default:
		UBRR0L = 119;
		//UBRR1L = 119;
		break;
#else
#warning "F_CPU undefined or not supported in uart.c."
	default:
		UBRR0L = 71;
		//UBRR1L = 71;
		break;
#endif
	}
*/
    uart_buffer_index = 0;
    uart_buffer_1_index = 0;
}

/**
 * Transmit one byte
 * NOTE: This function uses busy waiting
 *
 * @param byte data to trasmit
 * @param uart channel
 */
void uart_putchar(uint8_t byte, int uart)
{
	if (uart) { 								// Channel 1

		/* wait for empty transmit buffer */
		while (!( UCSR1A & (1 << UDRE1)));		// UDRE = Data Register Empty

	 	/* Put data into buffer, sends the data */
    	UDR1 = byte;

	} else {

	    /* wait for empty transmit buffer */
	    while (!( UCSR0A & (1 << UDRE0)));

	    /* Put data into buffer, sends the data */
	    UDR0 = byte;
	}
}

/**
 * Receive a single byte from the receive buffer
 *
 * @param index
 * @param uart channel
 *
 * @return
 */
uint8_t uart_get_byte(int index, int uart)
{
  	if (index < UART_BUFFER_SIZE) {

  		if (uart) {
  			return uart_buffer_1[index];
  		} else {
  			return uart_buffer[index];
  		}
  	}
  	return 0;
}



/**
 * Get the number of bytes received on UART
 *
 * @param uart channel
 *
 * @return number of bytes received on UART
 */
uint8_t uart_bytes_received(int uart)
{
	if (uart == 1) {
    	return uart_buffer_1_index;
    } else {
    	return uart_buffer_index;
    }
}

/**
 * Prepares UART to receive another payload
 *
 * @ param uart channel
 */
void uart_reset_receive(int uart)
{
	if (uart == 1) {
		uart_buffer_1_index = 0;
	} else {
		uart_buffer_index = 0;
	}
}
void uart_send_string(char *string, int uart){
	
	while (*string != '\0')
	{
		uart_putchar(*string, uart);
		string++;
	}
	
	
}
/**
 * UART receive byte ISR
 */
ISR(USART0_RX_vect)
{
	while(!(UCSR0A & (1<<RXC0)));
    uart_buffer[uart_buffer_index] = UDR0;
    uart_buffer_index = (uart_buffer_index + 1) % UART_BUFFER_SIZE;
}

/**
 * UART receive byte ISR
 */
ISR(USART1_RX_vect)
{
	while(!(UCSR1A & (1<<RXC1)));
    uart_buffer_1[uart_buffer_1_index] = UDR1;
    uart_buffer_1_index = (uart_buffer_1_index + 1) % UART_BUFFER_SIZE;
}





