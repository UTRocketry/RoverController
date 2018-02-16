#include "AX5043.h"
#define AX_SS_PIN  PB5
int testdat = 0;
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

char SPI_RW_8(unsigned char reg_A,unsigned char reg_D, int read){
	PORTB &= ~(1<<DDB5); //SS low
	if(read==0){
		SPDR = reg_A;
	}else{
		
		SPDR = reg_A | 0b1000000;
	}
	while(!(SPSR & (1<<SPIF)));
	SPDR = reg_D;
	while(!(SPSR & (1<<SPIF)));
	PORTB |= (1<<DDB5); //SS high
	return SPDR;
}
char SPI_RW_A16_R8(uint16_t reg_A,unsigned char reg_D, int read){
	uint8_t reg_A_upper = reg_A >> 8;
	uint8_t reg_A_lower = reg_A;
    PORTB &= ~(1<<DDB5); //SS low
	if(read==0){
		SPDR = reg_A_upper;
		while(!(SPSR & (1<<SPIF)));
		SPDR = reg_A_lower;
		while(!(SPSR & (1<<SPIF)));
	}else{
		SPDR = reg_A_upper | 0x80;
		while(!(SPSR & (1<<SPIF)));
		SPDR = reg_A_lower;
		while(!(SPSR & (1<<SPIF)));
	}
	SPDR = reg_D;
	while(!(SPSR & (1<<SPIF)));
    PORTB |= (1<<DDB5); //SS high
	return SPDR;
}
int ii=0;
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

void lufaPrintInt(unsigned int c){
    char buffer[10] = {};
    itoa(c, buffer, 10);
    fputs(buffer, &USBSerialStream);
    fputs("\n", &USBSerialStream);
}
void lufaPrintUint20_t(uint32_t c) {
  int temp = c / 10000;
  lufaPrintInt(temp);
  c -= temp * 10000;
  if (c == 0) {
    fputs("0000", & USBSerialStream);
  } else {
    lufaPrintInt(c);
  }
  fputs("\n", & USBSerialStream);
}
void UTR_GPS_uart1_init(void) { //9600 baud
  UCSR1C |= ((1 << UCSZ11) | (1 << UCSZ10));
  UCSR1B |= ((1 << RXEN1) | (1 << TXEN1));
  UBRR1L |= 51; //value from datasheet
}

void UTR_GPS_uart1_putchar(char c) {
  loop_until_bit_is_set(UCSR1A, UDRE1);
  UDR1 = c;
}

char UTR_GPS_uart1_getchar(void) {
  loop_until_bit_is_set(UCSR1A, RXC1);
  return UDR1;
}
void UTR_printInt(int c){
  char buffer[5] = {}; //max size of int is 65535 or 5 digits
  itoa(c, buffer, 10);
    int i;
  for (i = 0; i < 5; i ++){
        if (buffer[i])
        UTR_GPS_uart1_putchar(buffer[i]);
  }
}
void uart0_printUint20_t(uint32_t c) {
  int temp = c / 10000;
  UTR_printInt(temp);
  c -= temp * 10000;
  if (c == 0) {
    UTR_GPS_uart1_putchar('0');
        UTR_GPS_uart1_putchar('0');
        UTR_GPS_uart1_putchar('0');
        UTR_GPS_uart1_putchar('0');
  } else {
    UTR_printInt(c);
  }
  UTR_GPS_uart1_putchar('\n');
}

int main(void){
	//INIT CODE 
	TCCR0B |= ((1 << CS02) | (1 << CS00)); //Table 15-9 clk/1024 prescale
	TIMSK0 |= (1 << TOIE0); //timer 0 overflow interrupt enable
	DDRB |= (1 << AX_SS_PIN); //set PB0 output
	DDRD |= (1 << DDD5); //set PD0 output
	PORTB |= (1 << AX_SS_PIN); //SS high
	sei(); // Set interputs 
	SetupHardware(); //USB init 
    UTR_GPS_uart1_init();
	CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream); //Init USB stream
    _delay_ms(3000);
    fputs('b',&USBSerialStream);
	GlobalInterruptEnable();
	SPI_MasterInit(); // Turns AVR device into SPI Master
    ax_bootup();
    power_usart1_disable();
    power_timer0_disable();
    power_timer1_disable();
    UTR_GPS_uart1_putchar('Y');
	//END OF INIT CODE
    while(ax_check_comms()==0){ fputs("Shit broke..",&USBSerialStream); ax_bootup();}
    fputs('i',&USBSerialStream);
	_delay_ms(1000);
    SPI_RW_8(0x25,0x04,0);
    _delay_ms(200);
    SPI_RW_8(AX_REG_PWRMODE,PWRMODE_STANDBY,0);
    uint8_t radio;
    uint32_t status;
    while(true){
        switch(UTR_GPS_uart1_getchar()){
            case 's':
                       //ax_send_data();
            radio = SPI_RW_8(0x1C,0x00,1);
            status = AX_getStatusBits();
            //fputs("Data: ",&USBSerialStream);
            //CDC_Device_SendString(&VirtualSerial_CDC_Interface,status);
            //lufaPrintUint20_t(status);
            //fputs("\n",&USBSerialStream);
            //sendSerial(status);
            //char fifofree = SPI_RW_8(AX_REG_FIFO);
            //fputs(ax_read_packet(),&USBSerialStream);
            //int *datai = (int) datac;
            //for(int i =3; i<3;i++){
        
            //lufaPrintInt(status);
            //lufaPrintInt(radio);
            //lufaPrintInt(testdat);
            uart0_printUint20_t(status);
            UTR_printInt(radio);
            //}
                break;
            case 't':
                
                break;
            
        }
		
	}
	//END IF TEST CODE 
}


