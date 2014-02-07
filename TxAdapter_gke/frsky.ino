/* 
 FrSky Telemetry for Taranis via S.Port
 
 The UART TX-Pin is connected to Pin5 of the JR-Connector. FrSky uses inverted serial levels 
 and thus a inverter is needed in between. Example circuit can be found here:
 https://github.com/openLRSng/openLRSng/wiki/Telemetry-guide
 
 Almost all of the code was originally derived from https://github.com/openLRSng/
 */


#define SMARTPORT_INTERVAL 12000
#define SMARTPORT_BAUDRATE 57600

uint32_t frskyLast = 0;

uint8_t frskySchedule = 0;


void frskyInit()
{
  frskyLast = micros();
  Serial.begin(SMARTPORT_BAUDRATE);
}


void smartportSend(uint8_t *p)
{
  uint16_t crc = 0;
  Serial.write(0x7e);
  for (int i = 0; i < 9; i++) {
    if (i == 8) {
      p[i] = 0xff - crc;
    }
    if ((p[i] == 0x7e) || (p[i] == 0x7d)) {
      Serial.write(0x7d);
      Serial.write(0x20 ^ p[i]);
    } 
    else {
      Serial.write(p[i]);
    }
    if (i>0) {
      crc += p[i]; //0-1FF
      crc += crc >> 8; //0-100
      crc &= 0x00ff;
      crc += crc >> 8; //0-0FF
      crc &= 0x00ff;
    }
  }
}


void smartportIdle()
{
  Serial.write(0x7e);
}

void smartportSendFrame()
{
  uint8_t buf[9];
  
  uint8_t * bytes;
  
  frskySchedule = (frskySchedule + 1) % 36;
  buf[0] = 0x98;
  buf[1] = 0x10;
  switch (frskySchedule) {
  case 0: // SWR (fake value = 0)
    buf[2] = 0x05;
    buf[3] = 0xf1;
    buf[4] = 0;
    break;
  case 1: // RSSI (fake value = 100)
    buf[2] = 0x01;
    buf[3] = 0xf1;
    buf[4] = 100;
    break;
  case 2: //BATT
    buf[2] = 0x04;
    buf[3] = 0xf1;
    buf[4] = batteryVolts; //Set Range to ~25-26 Volts. Need to check maths here!
    break;
  case 3: //RPM (telemetry_ahead debug)
    buf[2] = 0x00;
    buf[3] = 0x05;
    bytes = (uint8_t *) &telemetry_ahead;
    buf[4] = bytes[0];
    buf[5] = bytes[1];
    buf[6] = bytes[2];
    buf[7] = bytes[3];
    break;
  case 4: // ACC-ROLL
    buf[2] = 0x00;
    buf[3] = 0x07;
    bytes = (uint8_t *) &accData[ROLL];
    buf[4] = bytes[0];
    buf[5] = bytes[1];
    buf[6] = bytes[2];
    buf[7] = bytes[3];
    break;
  case 5: // ACC-PITCH
    buf[2] = 0x10;
    buf[3] = 0x07;
    bytes = (uint8_t *) &accData[PITCH];
    buf[4] = bytes[0];
    buf[5] = bytes[1];
    buf[6] = bytes[2];
    buf[7] = bytes[3];
    break;
  case 6: // ACC-PITCH
    buf[2] = 0x20;
    buf[3] = 0x07;
    bytes = (uint8_t *) &accData[YAW];
    buf[4] = bytes[0];
    buf[5] = bytes[1];
    buf[6] = bytes[2];
    buf[7] = bytes[3];
    break;
  default:
    smartportIdle();
    return;
  }
  smartportSend(buf);
}


void frskyUpdate()
{
  uint32_t now = micros();
  if ((now - frskyLast) > SMARTPORT_INTERVAL) {
    telemetry_ahead++;
    smartportSendFrame();
    frskyLast = now;
  }
}


