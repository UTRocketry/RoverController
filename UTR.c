#include "VirtualSerial/VirtualSerial.h"
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#define POWERREG_WRITE = 0x082
#define POWERDOWN = 0b01100000
#define DEEPSLEEP = 0b01100001
#define STANDBY   = 0b01100101
#define FIFOON    = 0b01100111
#define SYNTHRX   = 0b01101000
#define FULLRX    = 0b01101001
#define WORRX     = 0b01101011
#define SYNTHTX   = 0b01101100
#define FULLTX    = 0b01101101
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
	PORTD |= (1 << DDD0); //set PD0 high
	_delay_ms(100);
	PORTD &= ~(1 << DDD0); //set PD0 low
	_delay_ms(100);
}

void EVENT_USB_Device_Disconnect(void){
	PORTD |= (1 << DDD0); //set PD0 high
	_delay_ms(100);
	PORTD &= ~(1 << DDD0); //set PD0 low
	_delay_ms(100);
}

void EVENT_USB_Device_ConfigurationChanged(void){
	bool ConfigSuccess = true;
	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);
}

void EVENT_USB_Device_ControlRequest(void){
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}

void transferData(void){
		/*PORTD |= (1 << DDD0); //set PD0 high
		uint16_t data = eeprom_read_word (( uint16_t *) 46) ;
		char dataLSB =  data & 0xFF;
		char dataMSB = data >> 8;
		fputs(dataMSB, &USBSerialStream);
		fputs(dataLSB, &USBSerialStream);
		PORTD &= ~(1 << DDD0); //set PD0 low*/
}

void EVENT_CDC_Device_ControLineStateChanged(USB_ClassInfo_CDC_Device_t *const CDCInterfaceInfo){
	bool HostReady = (CDCInterfaceInfo->State.ControlLineStates.HostToDevice & CDC_CONTROL_LINE_OUT_DTR) != 0;
	if (HostReady)
		transferData();
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

char SPI_RW_8(char reg_A, char reg_D){
	PORTB &= ~(1<<DDB0); //SS low
	SPDR = reg_A;
	while(!(SPSR & (1<<SPIF)));
	SPDR = reg_D;
	while(!(SPSR & (1<<SPIF)));
	PORTB |= (1<<DDB0); //SS high
	return SPDR;
}
uint8_t USBint = 0;
void initRaido(int freq){
    SPI_RW_8(POWERREG_WRITE,STANDBY); // Put radio into stand by 
    
}
int main(void){
	TCCR0B |= ((1 << CS02) | (1 << CS00)); //Table 15-9 clk/1024 prescale
	TIMSK0 |= (1 << TOIE0); //timer 0 overflow interrupt enable
	DDRB |= (1 << PB0); //set PB0 output
	DDRD |= (1 << DDD0); //set PD0 output
	PORTB |= (1 << PB0); //SS high
	sei();
	SetupHardware();
	CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream);
	GlobalInterruptEnable();
	SPI_MasterInit();
	char fifo[32];char fifo_dat;
	_delay_ms(1000);
	SPI_RW_8(POWERREG_WRITE,FULLRX);// Bring the power registers to Full RX
	while(true){
		char rec = SPI_RW_8(0x028, 0b00000000);
		//fputs(rec, &USBSerialStream);
		/*if(!(rec & 0b00000001)){
			fifo_dat=SPI_RW_8(0x029,0b00000000); 
			CDC_Device_SendString(&VirtualSerial_CDC_Interface, fifo_dat);
			_delay_ms(1000);
			}*/
		CDC_Device_SendString(&VirtualSerial_CDC_Interface, rec);
		_delay_ms(1000);
	}
}


ISR(TIMER0_OVF_vect){ //moved from main loop to timer .1 second / (8Mhz / 1024 prescale) = 12.8
	/*HANDLE USB COMMUNICATIONS*/
	cli();
	USBint ++;
	//if (USBint > 3){
	//USBint = 0;
	CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
	CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
	USB_USBTask();
	//}
	sei();
}
