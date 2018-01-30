#include "UTR_GPS.c"

int gpsSM;
bool UTR_GPS_parseData;
//#define gpsBufferLength 55
//char gpsBuffer[gpsBufferLength] = {};
int gpsBufferIndex;
float latDecimal;
float lonDecimal;

void UTR_GPS_uart1_init(void);

void UTR_GPS_uart1_putchar(char c);

char UTR_GPS_uart1_getchar(void);

void UTR_GPS_parseGPS(void);

void UTR_GPS_enable_RX_int(void);

void UTR_GPS_disable_RX_int(void);
