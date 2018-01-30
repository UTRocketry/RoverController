/*
atmega16u2

^^^ may need to change this ^^^

Quick PWM reference:
https://www.google.com/url?sa=i&rct=j&q=&esrc=s&source=images&cd=&cad=rja&uact=8&ved=2ahUKEwjS7fG28-LYAhVLQq0KHYCvDycQjRx6BAgAEAY&url=https%3A%2F%2Fwww.rcgroups.com%2Fforums%2Fattachment.php%3Fattachmentid%3D2442218&psig=AOvVaw0Jkq-_pxMajvk5OGPhEv7j&ust=1516412637706417

30 Hz Servo Calculations:

1 ms = 0 deg
1.5 ms = 90 deg
2 ms = 180 deg

At 30 Hz,
30 Hz = 30 pulses per second, 1 pulse = 1 / 30 second
1 / 30 second = .0333 second = 33.33 ms
Ex: 2ms duty cycle = 2 ms / 33.33 ms = 6%

OC = output compare
OC_register = 255 - ((1 + (deg / 180)) / 33.33) * 255

PS don't forget to restart the board after programming
*/

#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>

void initServos() { //don't init until you want servos to become stiff
	//Config of 8 bit PWM
	DDRB |= (1 << DDB7) | (1 << DDB6); //B7 output, OC.0A, B6 output, bit banged PWM
	DDRD |= (1 << DDD0); //D0 output, OC.0B
	TCCR0A |= (1 << COM0A1) | (1 << COM0A0) | (1 << COM0B1) | (1 << COM0B0) | (1 << WGM00); //OC.0A & OC.0B set as PWM  //PWM phase correct
	TCCR0B = (1 << CS00) | (1 << CS01); //Clk / 64 ~ 30Hz checked with meter
	//Config of 16 bit PWM
	DDRC |= (1 << DDC6) | (1 << DDC5); //C6 output, OC.1A, C5 output OC.1B
	TCCR1A |= (1 << COM1A1) | (1 << COM1A0) | (1 << COM1B1) | (1 << COM1B0) | (1 << WGM00); //OC.0A & OC.0B set as PWM  //PWM phase correct
	TCCR1B = (1 << CS10) | (1 << CS11); //Clk / 64 same as 8 bit ~ 30 Hz
}

void setServo(int servo, float degree){
	uint8_t OCVal; //OC registers only take an int
	switch (servo){ //numbers match with schematic
	case 1:
		OCVal = 255.0 - ((1.0 + (degree / 180.0)) / 33.33) * 255.0;
		OCR1A = OCVal;
		break;
	case 2:
		OCVal = 255.0 - ((1.0 + (degree / 180.0)) / 33.33) * 255.0;
		OCR1B = OCVal;
		break;
	case 3:
		OCVal = 255.0 - ((1.0 + (degree / 180.0)) / 33.33) * 255.0;
		OCR0B = OCVal;
		break;
	case 4:
		OCVal = 255.0 - ((1.0 + (degree / 180.0)) / 33.33) * 255.0;
		OCR0A = OCVal;
		break;
	case 5:
		OCVal = 1.0 + (degree / 180.0) * 1000.0; //using delay_us below for better resolution
		PORTB |= (1 << PB6); //set PB6 high
		_delay_us(OCVal); //wait
		PORTB &= ~(1 << PB6); //set PB6 low
		_delay_ms(3.0 - OCVal / 1000.0); //3.33 ms became 3 ms to make output closer to 30 Hz
		break;
	}
}

int main() {
	initServos();

	while (1) {
		int i;
		for (i = 0; i < 180; i = i + 5){
		setServo(1, i);
		setServo(2, i);
		setServo(3, i);
		setServo(4, i);
		//setServo(5, i);
		_delay_ms(100);
		}
		for (i = 180; i > 0; i = i - 5){
		setServo(1, i);
		setServo(2, i);
		setServo(3, i);
		setServo(4, i);
		//setServo(5, i);
		_delay_ms(100);
		}
	  }
}
