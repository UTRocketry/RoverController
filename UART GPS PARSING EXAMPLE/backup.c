      int startPt = 0;
      int j;
      for (j = 0; j < gpsBufferSize; j++) {
        int i;
        for (i = 0; i < startMsgLen; i++) {
          if (gpsBuffer[j] == startMsg[i] && i == startMsgLen - 1) {
            startPt = j + 2;
            j = gpsBufferSize; //break out of both loops
          }
        }
      }
      int endPt = 0;
      for (j = startPt + 3; j < gpsBufferSize; j++) {
        if (gpsBuffer[j] == endMsg) {
          endPt = j;
          j = gpsBufferSize; //break out of both loops
          uart1_putchar('n');
        }
      }
      if (startPt != 0 && endPt != 0) {
        bool validLock = true;
        int i;
        for (i = startPt; i < endPt; i++) {
          if (gpsBuffer[i] == ',' && gpsBuffer[i + 1] == ',' && gpsBuffer[i + 2] == ',' && i < startPt + 10) {
            validLock = false;
          }
        }
        if (validLock) {
          uart1_putchar('l');
          int i;
          for (i = startPt; i < endPt && gpsBuffer[i] != 0x0A; i++) {

            if (gpsBuffer[i] == ',' && (gpsBuffer[i + 1] == 'N' || gpsBuffer[i + 1] == 'S') && gpsBuffer[i + 2] == ',') { //fix N and S notation
              uart1_putchar(' ');
              uart1_putchar(gpsBuffer[i + 1]);
              uart1_putchar('l');
              i = i + 3;
            }
            if (gpsBuffer[i] == ',' && (gpsBuffer[i + 1] == 'E' || gpsBuffer[i + 1] == 'W') && gpsBuffer[i + 2] == ',') { //fix E and W notation
              uart1_putchar(' ');
              uart1_putchar(gpsBuffer[i + 1]);
              i = endPt; //break from loop
            }
            if (gpsBuffer[i] != ',') { //don't print commas
              uart1_putchar(gpsBuffer[i]);
            }
          }
        } else {
          uart1_putchar('!'); //Serial.print("NO LOCK");
        }
        //int i;
        /*for (i = 0; i < gpsBufferSize; i++) { //clean buffer
          gpsBuffer[i] = ' ';
        }*/
        uart1_putchar('\n');
      }
    }
