/*
atmega16u2

^^^ may need to change this ^^^
*/

#define F_CPU 8000000UL

#include <avr/io.h> 
#include <util/delay.h> 
#include <stdbool.h>
#include <avr/power.h>
#include <avr/interrupt.h>

#define startMsgLen 6
//#define endMsgLen 1
#define gpsBufferSize 300 //larger buffer sizes dont work
//#define gpsBufferSize 120
#define startMsg "$GPGLL"
#define endMsg '$'
#define outputBufferSize 50
char outputBuffer[outputBufferSize] = {};
int outputBufferIndex = 0;
//#define endMsg 0x0D
int bufferIndex = 0;
char gpsBuffer[gpsBufferSize] = {};
//char gpsBuffer[] = "$GPGSV,4,4,13,50,51,133,33*44$GPGLL,2447.65027,N,12100.78318,E,065500.00,A,D*6E$GPGSV,4,4,13,50,51,133,33*44";

void uart1_init(void) { //9600 baud
  UCSR1C |= ((1 << UCSZ11) | (1 << UCSZ10));
  UCSR1B |= ((1 << RXEN1) | (1 << TXEN1));
  UBRR1L |= 51; //value 51 from Table 18-8 in ATTiny841 datasheet
}
void uart1_putchar(char c) {
  loop_until_bit_is_set(UCSR1A, UDRE1);
  UDR1 = c;
}

char uart1_getchar(void) {
  loop_until_bit_is_set(UCSR1A, RXC1); /* Wait until data exists. */
  return UDR1;
}

void delay_ms(uint16_t count) {
  while (count--) {
    _delay_ms(1);
  }
}

void delay_us(uint16_t count) {
  while (count--) {
    _delay_us(1);
  }
}

void enable_RX_int() {
  UCSR1B |= (1 << RXCIE1);
}

void disable_RX_int() {
  UCSR1B &= ~(1 << RXCIE1);
}

ISR(USART1_RX_vect) {
  cli();
  loop_until_bit_is_set(UCSR1A, RXC1); /* Wait until data exists. */
  char data = UDR1;
  gpsBuffer[bufferIndex] = data;
  if (bufferIndex < gpsBufferSize) {
    bufferIndex++;
  } else {
    //bufferIndex = 0;
  }
  sei();
}

int main() {
  clock_prescale_set(clock_div_1);
  uart1_init();
  enable_RX_int();
  sei();
  PORTB |= (1 << PB0); //pull up resistor
  while (1) {
    if (!PINB & (1 << PB0)) { //only parse when PB0 is low
      /*int z;
      for (z = 0; z < gpsBufferSize; z++) {
        uart1_putchar(gpsBuffer[z]);
      }*/
      int startPt = 0;
      int j;
      for (j = 0; j < gpsBufferSize; j++) {
        int i;
        for (i = 0; i < startMsgLen; i++) {
          if (gpsBuffer[j] == startMsg[i] && i == startMsgLen - 1) {
            startPt = j + 2;
            j = gpsBufferSize; //break out of both loops
          }
        }
      }
      int endPt = 0;
      for (j = startPt + 3; j < gpsBufferSize; j++) {
        if (gpsBuffer[j] == endMsg) {
          endPt = j;
          j = gpsBufferSize; //break out of both loops
        }
      }
      if (startPt != 0 && endPt != 0) {
        bool validLock = true;
        int i;
        for (i = startPt; i < endPt; i++) {
          if (gpsBuffer[i] == ',' && gpsBuffer[i + 1] == ',' && gpsBuffer[i + 2] == ',' && i < startPt + 10) {
            validLock = false;
          }
        }
        if (validLock) {
          //uart1_putchar('l');
          int i;
          for (i = startPt; i < endPt && gpsBuffer[i] != 0x0A; i++) {

            /*if (gpsBuffer[i] == ',' && (gpsBuffer[i + 1] == 'N' || gpsBuffer[i + 1] == 'S') && gpsBuffer[i + 2] == ',') { //fix N and S notation
              uart1_putchar(' ');
              uart1_putchar(gpsBuffer[i + 1]);
              uart1_putchar('l');
              i = i + 3;
            }
            if (gpsBuffer[i] == ',' && (gpsBuffer[i + 1] == 'E' || gpsBuffer[i + 1] == 'W') && gpsBuffer[i + 2] == ',') { //fix E and W notation
              uart1_putchar(' ');
              uart1_putchar(gpsBuffer[i + 1]);
              i = endPt; //break from loop
            }*/
            if (gpsBuffer[i] != ',') { //don't print commas
              //uart1_putchar(gpsBuffer[i]);
		outputBuffer[outputBufferIndex ++] = gpsBuffer[i];
            }
          }
        } else {
          //uart1_putchar('!'); //Serial.print("NO LOCK");
        }
        //int i;
	int y = 0;
	for (y = 0; y < outputBufferSize && outputBuffer[y]; y ++){
		uart1_putchar(outputBuffer[y]);
	}
	bufferIndex = 0; //clean buffer
	outputBufferIndex = 0;
        for (i = 0; i < gpsBufferSize; i++) {
          gpsBuffer[i] = ' ';
        }
        uart1_putchar('\n');
      }
    }
  }
}
