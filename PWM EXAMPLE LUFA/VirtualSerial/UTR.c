#include "VirtualSerial/VirtualSerial.h"
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <util/delay.h>

USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
	{
		.Config =
			{
				.ControlInterfaceNumber   = INTERFACE_ID_CDC_CCI,
				.DataINEndpoint           =
					{
						.Address          = CDC_TX_EPADDR,
						.Size             = CDC_TXRX_EPSIZE,
						.Banks            = 1,
					},
				.DataOUTEndpoint =
					{
						.Address          = CDC_RX_EPADDR,
						.Size             = CDC_TXRX_EPSIZE,
						.Banks            = 1,
					},
				.NotificationEndpoint =
					{
						.Address          = CDC_NOTIFICATION_EPADDR,
						.Size             = CDC_NOTIFICATION_EPSIZE,
						.Banks            = 1,
					},
			},
	};

static FILE USBSerialStream;

void SetupHardware(void){
	MCUSR &= ~(1 << WDRF);
	wdt_disable();
	clock_prescale_set(clock_div_1);
	USB_Init();
}

void EVENT_USB_Device_Connect(void){
	/*PORTD |= (1 << DDD0); //set PD0 high
	_delay_ms(100);
	PORTD &= ~(1 << DDD0); //set PD0 low
	_delay_ms(100);*/
}

void EVENT_USB_Device_Disconnect(void){
	/*PORTD |= (1 << DDD0); //set PD0 high
	_delay_ms(100);
	PORTD &= ~(1 << DDD0); //set PD0 low
	_delay_ms(100);*/
}

void EVENT_USB_Device_ConfigurationChanged(void){
	bool ConfigSuccess = true;
	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);
}

void EVENT_USB_Device_ControlRequest(void){
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}

void transferData(void){
//do nothing
}

void EVENT_CDC_Device_ControLineStateChanged(USB_ClassInfo_CDC_Device_t *const CDCInterfaceInfo){
	bool HostReady = (CDCInterfaceInfo->State.ControlLineStates.HostToDevice & CDC_CONTROL_LINE_OUT_DTR) != 0;
	if (HostReady)
		transferData();
}

uint8_t USBint = 0;

void initServos(void) { //don't init until you want servos to become stiff
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

void delay_ms(uint16_t count) {
  while(count--) {
    _delay_ms(1);
  }
}

void delay_us(uint16_t count) {
  while(count--) {
    _delay_us(1);
  }
}

void setServo(int servo, int degree){
	int OCVal; //OC registers only take an int
	switch (servo){ //numbers match with schematic
	case 1:
		OCVal = 255 - ((1 + (degree / 180.0)) / 33.33) * 255;
		OCR1A = OCVal;
		break;
	case 2:
		OCVal = 255 - ((1 + (degree / 180.0)) / 33.33) * 255;
		OCR1B = OCVal;
		break;
	case 3:
		OCVal = 255 - ((1 + (degree / 180.0)) / 33.33) * 255;
		OCR0B = OCVal;
		break;
	case 4:
		OCVal = 255 - ((1 + (degree / 180.0)) / 33.33) * 255;
		OCR0A = OCVal;
		break;
	case 5:
		OCVal = 1 + (degree / 180.0) * 1000; //using delay_us below for better resolution
		PORTB |= (1 << PB6); //set PB6 high
		delay_us(OCVal); //wait
		PORTB &= ~(1 << PB6); //set PB6 low
		delay_ms(3 - OCVal / 1000); //3.33 ms became 3 ms to make output closer to 30 Hz
		break;
	}
}

int main(void){
	initServos();

	while(true){
		setServo(1, 180);
		setServo(2, 180);
		setServo(3, 180);
		setServo(4, 180);
		setServo(5, 180);
		CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
		CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
		USB_USBTask();
	}
}
