#include "VirtualSerial/VirtualSerial.h"
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include "AX5043.h"
#include "USB.h"



void transferData(void){
		/*PORTD |= (1 << DDD0); //set PD0 high
		uint16_t data = eeprom_read_word (( uint16_t *) 46) ;
		char dataLSB =  data & 0xFF;
		char dataMSB = data >> 8;
		fputs(dataMSB, &USBSerialStream);
		fputs(dataLSB, &USBSerialStream);
		PORTD &= ~(1 << DDD0); //set PD0 low*/
}

void SPI_MasterInit(void){
	DDRB |= (1<<DDB1)|(1<<DDB2)|(1<<DDB0);
	SPCR |= (1<<SPE)|(1<<MSTR)|(1<<SPR0);
}

/*
MSB of reg_A is RW bit of that register

when writing to reg_A: reg_D is data to be written
when reading from reg_A: reg_D should be 0x00

this function is only valid for the 8 bit registers (0x00 to 0x70)

adding 128 to an int sets register to write instead of read
*/

char SPI_RW_8(unsigned char reg_A,unsigned char reg_D){
	PORTB &= ~(1<<DDB0); //SS low
	SPDR = reg_A;
	while(!(SPSR & (1<<SPIF)));
	SPDR = reg_D;
	while(!(SPSR & (1<<SPIF)));
	PORTB |= (1<<DDB0); //SS high
	return SPDR;
}

ISR(TIMER0_OVF_vect) { //moved from main loop to timer .1 second / (8Mhz / 1024 prescale) = 12.8
					   /*HANDLE USB COMMUNICATIONS*/
	cli();
	USBInterruptjob();
	sei();
}


int main(void){
	//INIT CODE 
	TCCR0B |= ((1 << CS02) | (1 << CS00)); //Table 15-9 clk/1024 prescale
	TIMSK0 |= (1 << TOIE0); //timer 0 overflow interrupt enable
	DDRB |= (1 << PB0); //set PB0 output
	DDRD |= (1 << DDD0); //set PD0 output
	PORTB |= (1 << PB0); //SS high
	sei(); // Set interputs 
	SetupHardware(); //USB init 
	USBStreamInit(); //USB stream init
	GlobalInterruptEnable();
	SPI_MasterInit(); // Turns AVR device into SPI Master
	initRaido(0);//NEEDS freq 
	//END OF INIT CODE


	//TEST CODE  
	//char fifo[32];
	//char fifo_dat; 
	_delay_ms(1000);
	//SPI_RW_8(POWERMODE_W,FULLRX);  // Bring the power registers to Full RX
	while(true){
		char rec = SPI_RW_8(0x028, 0b00000000);
		//fputs(rec, &USBSerialStream);
		/*if(!(rec & 0b00000001)){
			fifo_dat=SPI_RW_8(0x029,0b00000000); 
			CDC_Device_SendString(&VirtualSerial_CDC_Interface, fifo_dat);
			_delay_ms(1000);
			}*/
		sendSerial(rec);
		_delay_ms(1000);
	}
	//END IF TEST CODE 
}


