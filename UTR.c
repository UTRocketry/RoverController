#include "VirtualSerial/VirtualSerial.h"
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#define POWERMODE_W   0x082
#define POWERMODE_R   0x002
#define REVISION      0X000
#define	MODULATION_R  0X010
#define MODULATION_W  0X090
#define ENCODING_R    0X011
#define ENCODING_W    0X091
#define FRAMING_R     0X012
#define FRAMING_W     0X092
#define RADIOSTATE    0x01C
#define FIFOCMD       0x0A8 //Five bit command to commit data to the FIFO to send 0bXXX00000
#define FIFOSTAT      0X028
#define FIFODATA_R    0X029
#define FIFOCOUNT1_R  0X02A
#define FIFOCOUNT1_W  0X0AA
#define FIFOCOUNT0_W  0X0AB
#define FIFOCOUNT0_R  0X02B
#define FIFOFREE1     0X02C
#define FIFOFREE0     0X02D
#define BGNDRSSI      0x041
#define RSSI          0x040
#define POWERDOWN  0b01100000
#define DEEPSLEEP  0b01100001
#define STANDBY    0b01100101
#define FIFOON     0b01100111
#define SYNTHRX    0b01101000
#define FULLRX     0b01101001
#define WORRX      0b01101011
#define SYNTHTX    0b01101100
#define FULLTX     0b01101101
#define EMPTY      0b00000000


uint8_t USBint = 0;
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface = {
	.Config =
{
	.ControlInterfaceNumber = INTERFACE_ID_CDC_CCI,
	.DataINEndpoint =
{
	.Address = CDC_TX_EPADDR,
	.Size = CDC_TXRX_EPSIZE,
	.Banks = 1,
},
.DataOUTEndpoint =
{
	.Address = CDC_RX_EPADDR,
	.Size = CDC_TXRX_EPSIZE,
	.Banks = 1,
},
.NotificationEndpoint =
{
	.Address = CDC_NOTIFICATION_EPADDR,
	.Size = CDC_NOTIFICATION_EPSIZE,
	.Banks = 1,
},
},
};

static FILE USBSerialStream;
void transferData(void) {
	/*PORTD |= (1 << DDD0); //set PD0 high
	uint16_t data = eeprom_read_word (( uint16_t *) 46) ;
	char dataLSB =  data & 0xFF;
	char dataMSB = data >> 8;
	fputs(dataMSB, &USBSerialStream);
	fputs(dataLSB, &USBSerialStream);
	PORTD &= ~(1 << DDD0); //set PD0 low*/
}
void SetupHardware(void) {
	MCUSR &= ~(1 << WDRF);
	wdt_disable();
	clock_prescale_set(clock_div_1);
	USB_Init();
}
void EVENT_USB_Device_Connect(void) {
	PORTD |= (1 << DDD0); //set PD0 high
	_delay_ms(100);
	PORTD &= ~(1 << DDD0); //set PD0 low
	_delay_ms(100);
}

void EVENT_USB_Device_Disconnect(void) {
	PORTD |= (1 << DDD0); //set PD0 high
	_delay_ms(100);
	PORTD &= ~(1 << DDD0); //set PD0 low
	_delay_ms(100);
}

void EVENT_USB_Device_ConfigurationChanged(void) {
	bool ConfigSuccess = true;
	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);
}

void EVENT_USB_Device_ControlRequest(void) {
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}

void EVENT_CDC_Device_ControLineStateChanged(USB_ClassInfo_CDC_Device_t *const CDCInterfaceInfo) {
	bool HostReady = (CDCInterfaceInfo->State.ControlLineStates.HostToDevice & CDC_CONTROL_LINE_OUT_DTR) != 0;
	if (HostReady) { transferData(); }
}

void sendSerial(char rec) {
	CDC_Device_SendString(&VirtualSerial_CDC_Interface, &rec);
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

bool initRaido(unsigned char freq) {
	SPI_RW_8(POWERMODE_W, STANDBY); // Put radio into stand by 
	_delay_ms(50);
	if (SPI_RW_8(RADIOSTATE, EMPTY) == 0) { return true; }
	else { return false; }
	/*
	The Radio can have 11 possibale states
	0 - Idle
	1 - Poweredown
	4 - Tx PLL Settings
	6 - TX
	7 - Tx Tail
	8 - Rx PLL Settings
	9 - Rx Antenna Selection
	12 - Rx Preamble 1	13 - Rx Preamble 2
	14 - Rx Preamble 3
	15 - RX
	*/
}
void txRadio(unsigned char message) {
	SPI_RW_8(FIFOCOUNT0_W, message);

}
void rssiSignal() {
	//SPI_RW_8();
	//return(0);
	_delay_ms(1);
}
void restartRadio() {
	_delay_ms(1);
}
ISR(TIMER0_OVF_vect) { //moved from main loop to timer .1 second / (8Mhz / 1024 prescale) = 12.8
					   /*HANDLE USB COMMUNICATIONS*/
	cli();
	USBint++;
	//if (USBint > 3){
	//USBint = 0;
	CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
	CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
	USB_USBTask();
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
	CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream); //Init USB stream
	GlobalInterruptEnable();
	SPI_MasterInit(); // Turns AVR device into SPI Master
	initRaido(0);//NEEDS freq 
	//END OF INIT CODE

	_delay_ms(1000);
	sendSerial(initRadio());
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


