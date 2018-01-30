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

void uart1_init(void) { //9600 baud
  UCSR1C |= ((1 << UCSZ11) | (1 << UCSZ10));
  UCSR1B |= ((1 << RXEN1) | (1 << TXEN1));
  UBRR1L |= 51; //value 51 from Table 18-8 in ATTiny841 datasheet
}
void uart1_putchar(char c) {
  loop_until_bit_is_set(UCSR1A, UDRE1);
  UDR1 = c;
}

void uart1_putint(int c) {
  char buffer[10];

  itoa(c, buffer, 10);
	int i;
  for (i = 0; i < sizeof(c) / sizeof(int) && i < 10; i ++){
	uart1_putchar(buffer[c]);
	}
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

int gpsSM = 0;
bool gpsParseData = false;
#define gpsBufferLength 55 //$GPGLL,2447.65027,N,12100.78318,E,065500.00,A,D*6E //calculated length 50 characters
char gpsBuffer[gpsBufferLength] = {};
int gpsBufferIndex = 0;

ISR(USART1_RX_vect) {
  cli();
  loop_until_bit_is_set(UCSR1A, RXC1); /* Wait until data exists. */
  char data = UDR1;
  switch (gpsSM) {
  case 6: //put data into GPS buffer //needs flag to prevent parsing during interrupt
    if (data != 0x0D && gpsBufferIndex < gpsBufferLength) { //detect newline and prevent overflow
      gpsBuffer[gpsBufferIndex++] = data;
    } else { //set parse data flag and reset state machine
      gpsParseData = true;
      gpsBufferIndex = 0;
      gpsSM = 0;
    }
    break;
  case 5:
    if (data == 'L') {
      gpsSM++;
    } else {
      gpsSM = 0;
    }
    break;
  case 4:
    if (data == 'L') {
      gpsSM++;
    } else {
      gpsSM = 0;
    }
    break;
  case 3:
    if (data == 'G') {
      gpsSM++;
    } else {
      gpsSM = 0;
    }
    break;
  case 2:
    if (data == 'P') {
      gpsSM++;
    } else {
      gpsSM = 0;
    }
    break;
  case 1:
    if (data == 'G') {
      gpsSM++;
    } else {
      gpsSM = 0;
    }
    break;
  case 0:
    if (data == '$') {
      gpsSM++;
    } else {
      gpsSM = 0;
    }
    break;
  }
  sei();
}

int main() {
  clock_prescale_set(clock_div_1);
  uart1_init();
  enable_RX_int();
  sei();
  while (1) {
	if(gpsParseData){
	gpsParseData = false;
     bool validLock = true;
      int i;
      for (i = 0; i < gpsBufferLength; i++) { //determine valid lock
        if (gpsBuffer[i] == ',' && gpsBuffer[i + 1] == ',' && gpsBuffer[i + 2] == ',') {
          validLock = false;
        }
      }
      if (validLock) {
	int latDecimalPt = 0;
	for (i = 1; i < gpsBufferLength; i ++){ //find comma after latitude
		if (gpsBuffer[i] == '.'){ //need to find decimal because sometimes minutes notation is 5 digits, sometimes it is 4
			latDecimalPt = i;
			break;
		}
	}
	int latMinute = ((int) gpsBuffer[latDecimalPt - 2] - 48) * 10 + (int) gpsBuffer[latDecimalPt - 1] - 48;
	int latDegree;
	int latSecond;
	uart1_putint(latMinute);
	if ((int) gpsBuffer[latDecimalPt - 5] >= 0 && (int) gpsBuffer[latDecimalPt - 5] <= 9){//make sure 5th digit exists
	latDegree = ((int) gpsBuffer[latDecimalPt - 5] - 48) * 100 + ((int) gpsBuffer[latDecimalPt - 4] - 48) * 10 + (int) gpsBuffer[latDecimalPt - 3] - 48;
	} else {
	latDegree = ((int) gpsBuffer[latDecimalPt - 4] - 48) * 10 + (int) gpsBuffer[latDecimalPt - 3] - 48;
	}
	uart1_putint(latMinute);
	latSecond = ((int) gpsBuffer[latDecimalPt + 1] - 48) * 100000 + ((int) gpsBuffer[latDecimalPt + 2] - 48) * 10000 + ((int) gpsBuffer[latDecimalPt + 3] - 48) * 1000 + ((int) gpsBuffer[latDecimalPt + 4] - 48) * 100 + ((int) gpsBuffer[latDecimalPt + 5] - 48) * 10 + (int) gpsBuffer[latDecimalPt + 6] - 48;
	uart1_putint(latSecond);
/*
	int longStartPt = 0;
        for (i = 1; i < gpsBufferLength; i++) {
	if (gpsBuffer[i] != ','){
            uart1_putchar(gpsBuffer[i]);
	} else {
		longStartPt = i;
		break;
	}
          }
	uart1_putchar(' ');
	 for (i = longStartPt + 3; i < gpsBufferLength && gpsBuffer[i] != ','; i++) {
            uart1_putchar(gpsBuffer[i]);
          }*/
      }
	uart1_putchar('_');
	uart1_putint(1234);
      for (i = 0; i < gpsBufferLength; i++) {
        gpsBuffer[i] = '_';
      }
	}
  }
}
