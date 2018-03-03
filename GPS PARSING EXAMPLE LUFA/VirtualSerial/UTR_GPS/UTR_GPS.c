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

void UTR_GPS_enable_RX_int(void) {
  UCSR1B |= (1 << RXCIE1);
}

void UTR_GPS_disable_RX_int(void) {
  UCSR1B &= ~(1 << RXCIE1);
}

int gpsSM = 0;
bool UTR_GPS_parseData = false;
#define gpsBufferLength 55
char gpsBuffer[gpsBufferLength] = {};
int gpsBufferIndex = 0;
float latDecimal = 0;
float lonDecimal = 0;

void UTR_GPS_parseGPS(void){
      UTR_GPS_parseData = false;
      //bool validLock = true;
      /*int i;
      for (i = 0; i < gpsBufferLength; i++) { //determine valid lock
        if (gpsBuffer[i] == ',' && gpsBuffer[i + 1] == ',' && gpsBuffer[i + 2] == ',') {
          validLock = false;
        }
      }
      if (validLock) {*/
        int latDecimalPt = 0;
        int lonDecimalPt = 0;
	int i;
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

        latDecimal = (float) latDegree + ((float) latMinute) / 60.0 + ((float) latSecond) / 3600.0;

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
        lonDecimal = (float) lonDegree + ((float) lonMinute) / 60.0 + ((float) lonSecond) / 3600.0;
      //}
      for (i = 0; i < gpsBufferLength; i++) {
        gpsBuffer[i] = '_';
      }
    }

ISR(USART1_RX_vect) { //GNGLL for some, GPGLL for others
  cli();
  loop_until_bit_is_set(UCSR1A, RXC1);
  char data = UDR1;
  switch (gpsSM) {
  case 6: //put data into GPS buffer //needs flag to prevent parsing during interrupt
    if (data != 0x0D && gpsBufferIndex < gpsBufferLength) { //detect newline and prevent overflow
      gpsBuffer[gpsBufferIndex++] = data;
    } else { //set parse data flag and reset state machine
	UTR_GPS_parseData = true;
	int i;
	for (i = 0; i < gpsBufferLength; i++) { //determine valid lock
        if (gpsBuffer[i] == ',' && gpsBuffer[i + 1] == ',' && gpsBuffer[i + 2] == ',') {
          UTR_GPS_parseData = false;
        }
      }
     
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
    if (data == 'N') {
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
