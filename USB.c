#include "VirtualSerial/VirtualSerial.h"
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include "USB.h"
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
void USBStreamInit(){
	CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream); //Init USB stream
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
	if (HostReady)
		transferData();
}

void USBInterruptjob() {
	USBint++;
	//if (USBint > 3){
	//USBint = 0;
	CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
	CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
	USB_USBTask();
	//}
}
void sendSerial(char rec) {
	CDC_Device_SendString(&VirtualSerial_CDC_Interface, char rec);
}