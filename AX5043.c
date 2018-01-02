#include "VirtualSerial/VirtualSerial.h"
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include "AX5043.h"


bool initRaido(unsigned char freq) {
	SPI_RW_8(POWERMODE_W, STANDBY); // Put radio into stand by 
	return true;
}

void restartRadio() {

}