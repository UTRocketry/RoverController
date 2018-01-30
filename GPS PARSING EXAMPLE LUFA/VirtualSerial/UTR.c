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

void enable_RX_int(void) {
  UCSR1B |= (1 << RXCIE1);
}

void disable_RX_int(void) {
  UCSR1B &= ~(1 << RXCIE1);
}

void lufaPrintInt(unsigned int c){
	char buffer[10] = {};
	itoa(c, buffer, 10);
	fputs(buffer, &USBSerialStream);
	fputs("\n", &USBSerialStream);
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

int main(void) {
  clock_prescale_set(clock_div_1);
  uart1_init();
  enable_RX_int();
  sei();
  SetupHardware();
  CDC_Device_CreateStream( & VirtualSerial_CDC_Interface, & USBSerialStream);
  GlobalInterruptEnable();
  while (true) {
    CDC_Device_ReceiveByte( & VirtualSerial_CDC_Interface);
    CDC_Device_USBTask( & VirtualSerial_CDC_Interface);
    USB_USBTask();
    if (gpsParseData) { //https://en.wikipedia.org/wiki/Decimal_degrees
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
        int lonDecimalPt = 0;
        for (i = 1; i < gpsBufferLength; i++) { //find comma after latitude
          if (gpsBuffer[i] == '.') { //need to find decimal because sometimes minutes notation is 5 digits, sometimes it is 4
            latDecimalPt = i;
            break;
          }
        }
        int latMinute = ((int) gpsBuffer[latDecimalPt - 2] - 48) * 10 + (int) gpsBuffer[latDecimalPt - 1] - 48;
        int latDegree;
        float latSecond;
        if (gpsBuffer[latDecimalPt - 5] != ',') { //make sure 5th digit exists
          latDegree = ((int) gpsBuffer[latDecimalPt - 5] - 48) * 100 + ((int) gpsBuffer[latDecimalPt - 4] - 48) * 10 + (int) gpsBuffer[latDecimalPt - 3] - 48;
        } else {
          latDegree = ((int) gpsBuffer[latDecimalPt - 4] - 48) * 10 + (int) gpsBuffer[latDecimalPt - 3] - 48;
        }
        latSecond = ((float) gpsBuffer[latDecimalPt + 1] - 48) * 10 + ((float) gpsBuffer[latDecimalPt + 2] - 48) + ((float) gpsBuffer[latDecimalPt + 3] - 48) * 0.1 + ((float) gpsBuffer[latDecimalPt + 4] - 48) * 0.01 + ((float) gpsBuffer[latDecimalPt + 5] - 48) * 0.001;

        float latDecimal = (float) latDegree + ((float) latMinute) / 60.0 + ((float) latSecond) / 3600.0;

        for (i = latDecimalPt + 1; i < gpsBufferLength; i++) { //find comma after longitude
          if (gpsBuffer[i] == '.') { //need to find decimal because sometimes minutes notation is 5 digits, sometimes it is 4
            lonDecimalPt = i;
            break;
          }
        }
        int lonMinute = ((int) gpsBuffer[lonDecimalPt - 2] - 48) * 10 + (int) gpsBuffer[lonDecimalPt - 1] - 48;
        int lonDegree;
        float lonSecond;
        if (gpsBuffer[lonDecimalPt - 5] != ',') { //make sure 5th digit exists
          lonDegree = ((int) gpsBuffer[lonDecimalPt - 5] - 48) * 100 + ((int) gpsBuffer[lonDecimalPt - 4] - 48) * 10 + (int) gpsBuffer[lonDecimalPt - 3] - 48;
        } else {
          lonDegree = ((int) gpsBuffer[lonDecimalPt - 4] - 48) * 10 + (int) gpsBuffer[lonDecimalPt - 3] - 48;
        }

        lonSecond = ((float) gpsBuffer[lonDecimalPt + 1] - 48) * 10 + ((float) gpsBuffer[lonDecimalPt + 2] - 48) + ((float) gpsBuffer[lonDecimalPt + 3] - 48) * 0.1 + ((float) gpsBuffer[lonDecimalPt + 4] - 48) * 0.01 + ((float) gpsBuffer[lonDecimalPt + 5] - 48) * 0.001;
        float lonDecimal = (float) lonDegree + ((float) lonMinute) / 60.0 + ((float) lonSecond) / 3600.0;
        fputs("Latitude: ", & USBSerialStream);
        lufaPrintInt((int)(latDecimal));
        fputs(".", & USBSerialStream);
        lufaPrintInt((int)((latDecimal - (int)(latDecimal)) * 10000));
        fputs("\n", & USBSerialStream);
        fputs("Longitude: ", & USBSerialStream);
        lufaPrintInt((int)(lonDecimal));
        fputs(".", & USBSerialStream);
        lufaPrintInt((int)((lonDecimal - (int)(lonDecimal)) * 10000));
        fputs("\n", & USBSerialStream);
      }
      for (i = 0; i < gpsBufferLength; i++) {
        gpsBuffer[i] = '_';
      }
    }
  }
}
