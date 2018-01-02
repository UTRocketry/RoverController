#pragma once
#define POWERMODE_W  = 0x082
#define POWERMODE_R  = 0x002
#define REVISION     = 0X000
#define	MODULATION_R = 0X010
#define MODULATION_W = 0X090
#define ENCODING_R   = 0X011
#define ENCODING_W   = 0X091
#define FRAMING_R    = 0X012
#define FRAMING_W    = 0X092
#define FIFOSTAT     = 0X028
#define FIFODATA_R   = 0X029
#define FIFOCOUNT1_R = 0X02A
#define FIFOCOUNT1_W = 0X0AA
#define FIFOCOUNT0_W = 0X0AB
#define FIFOCOUNT0_R = 0X02B
#define FIFOFREE1    = 0X02C
#define FIFOFREE0    = 0X02D
#define BGNDRSSI     = 0x041
#define RSSI         = 0x040
#define POWERDOWN = 0b01100000
#define DEEPSLEEP = 0b01100001
#define STANDBY   = 0b01100101
#define FIFOON    = 0b01100111
#define SYNTHRX   = 0b01101000
#define FULLRX    = 0b01101001
#define WORRX     = 0b01101011
#define SYNTHTX   = 0b01101100
#define FULLTX    = 0b01101101

bool initRaido(unsigned char freq);
void restartRadio();