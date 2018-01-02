#pragma once
void SetupHardware(void);
void EVENT_USB_Device_Connect(void);
void EVENT_USB_Device_ControlRequest(void);
void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_Disconnect(void);
void EVENT_CDC_Device_ControLineStateChanged(USB_ClassInfo_CDC_Device_t *const CDCInterfaceInfo);
void USBStreamInit(void);
void USBInterruptjob(void);
void sendSerial(char rec);