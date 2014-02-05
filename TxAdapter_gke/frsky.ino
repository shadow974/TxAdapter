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
  frskySchedule = (frskySchedule + 1) % 36;
  buf[0] = 0x98;
  buf[1] = 0x10;
  switch (frskySchedule) {
  case 0: // SWR
    buf[2] = 0x05;
    buf[3] = 0xf1;
    buf[4] = 0;
    break;
  case 1: // RSSI
    buf[2] = 0x01;
    buf[3] = 0xf1;
    buf[4] = 100;
    break;
  case 2: //BATT
    buf[2] = 0x04;
    buf[3] = 0xf1;
    buf[4] = batteryVolts; //Set Range to ~25-26 Volts. Need to check maths here!
    break;
  case 3: //A2
    buf[2] = 0x03;
    buf[3] = 0xf1;
    buf[4] = 0;
    break;
  case 4: // ACC-Test  (this gives me a reading of 40.96; have no idea how it works!)   // buf5=0x10 = 4096    buf4=0x01 = 0.01   buf4=0x44 = 2.55    buf4+5=0xff = -0.01
    buf[2] = 0x00;
    buf[3] = 0x07;
    buf[4] = 0xff;
    buf[5] = 0xff;
    buf[6] = 0x00;
    buf[7] = 0x00;
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
    smartportSendFrame();
    frskyLast = now;
  }
}


