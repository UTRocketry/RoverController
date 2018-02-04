#include "VirtualSerial/VirtualSerial.h"
//#include <avr/eeprom.h>
//#include <avr/interrupt.h>
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
	TCCR0B =  (1 << CS02);
	//Config of 16 bit PWM
	DDRC |= (1 << DDC6) | (1 << DDC5); //C6 output, OC.1A, C5 output OC.1B
	TCCR1A |= (1 << COM1A1) | (1 << COM1A0) | (1 << COM1B1) | (1 << COM1B0) | (1 << WGM00); //OC.0A & OC.0B set as PWM  //PWM phase correct
	TCCR1B = (1 << CS02);
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

//https://cdn-shop.adafruit.com/product-files/2442/FS90R-V2.0_specs.pdf
//https://servodatabase.com/servo/towerpro/sg90
#define servoCenter 1.5 //milliseconds
#define servoRange 0.6 //difference from center in ms
#define servoPeriod 16.3 //period between pulses in ms

void setServo(int servo, int speed){ //speed 0 - 10 positive or negative CW is -, CCW is +
	float OCVal; //OC registers only take an int
	switch (servo){ //numbers match with schematic
	case 4:
		OCVal = 255.0 - ((servoCenter + servoRange * speed * 0.1) / servoPeriod) * 255.0;
		OCR0A = (int) OCVal;
		break;
}}

int main(void){
	clock_prescale_set(clock_div_1);
	initServos();
	while(true){
		float i;
			for (i = -10; i < 10; i ++){
			_delay_ms(5000);
			setServo(4, i);
		}
	}
}
