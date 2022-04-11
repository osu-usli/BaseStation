/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 16/03/20

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/


/*******************************************************************************************************
  Program Operation -

  Serial monitor baud rate is set at 9600
*******************************************************************************************************/

#define programversion "V1.0"

#include <SPI.h>
#include "SX128XLT.h"
#include "Settings.h"

SX128XLT LT;

uint32_t endwaitmS;
uint16_t IrqStatus;
uint32_t response_sent;


void loop()
{
  Serial.flush();

  
  /*LT.receiveRanging(RangingAddress, 0, TXpower, NO_WAIT);

  endwaitmS = millis() + 300;

  while (!digitalRead(DIO1) && (millis() <= endwaitmS));          //wait for Ranging valid or timeout

  if (millis() >= endwaitmS)
  {
    Serial.print("Error - Ranging Receive Timeout!! Irq: ");
    LT.printIrqStatus();
    Serial.println();
  }
  else
  {
    IrqStatus = LT.readIrqStatus();
    digitalWrite(LED1, HIGH);

    if (~IrqStatus & IRQ_RANGING_SLAVE_RESPONSE_DONE)
    {
      Serial.print("Slave error,");
      Serial.print(",Irq,");
      Serial.print(IrqStatus, HEX);
      LT.printIrqStatus();
    } else {
      digitalWrite(LED1, LOW);
      Serial.print("Ranging success ");
      LT.printIrqStatus();
      Serial.println();
    }
  }

  return;

  LT.setPacketType(PACKET_TYPE_LORA);
  LT.setRfFrequency(Frequency, Offset);
  LT.setBufferBaseAddress(0, 0);
  LT.setModulationParams(SpreadingFactor, Bandwidth, CodeRate);
  LT.setPacketParams(12, LORA_PACKET_VARIABLE_LENGTH, 250, LORA_CRC_ON, LORA_IQ_NORMAL, 0, 0);
  LT.setDioIrqParams(IRQ_RADIO_ALL, (IRQ_TX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);
  LT.setHighSensitivity();*/

  uint8_t packet_buf[128];
  
  //int32_t packet_length = LT.receive(packet_buf, sizeof(packet_buf), rangingRXTimeoutmS, WAIT_RX);
  int packet_length = LT.receiveRangingOrData(packet_buf, sizeof(packet_buf), RangingAddress, rangingRXTimeoutmS, TXpower, WAIT_RX); 

  if (packet_length == -1) {
    Serial.print("Error: ");
    LT.printIrqStatus();
    Serial.println();
    return;
  }
  else if (packet_length == 0) {
    Serial.println("Ranging success");
    return;
  }
      
  struct Packet {
    uint32_t marker;
    uint32_t time;
    int32_t pressure;
    double acc;
    int16_t temp;
    double distance;
  };
  if (packet_length != sizeof(struct Packet)) {
    Serial.println("Length mismatch");
    return;
  }
  
  struct Packet *p = (struct Packet*)packet_buf;
  if (p->marker != 0x5555AAAA) {
    Serial.println("Marker mismatch");
  }

  Serial.print("t: "); Serial.print(p->time);
  Serial.print("\tp: "); Serial.print(p->pressure);
  Serial.print("\ttemp: "); Serial.print(p->temp);
  
  Serial.print("\tacc: "); Serial.print(p->acc);

  Serial.print("\t\trange: "); Serial.println(p->distance);
}


void led_Flash(unsigned int flashes, unsigned int delaymS)
{
  //flash LED to show board is alive
  unsigned int index;

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(LED1, HIGH);
    delay(delaymS);
    digitalWrite(LED1, LOW);
    delay(delaymS);
  }
}


void setup()
{
  Serial.begin(115200);            //setup Serial console ouput
  Serial.println();
  Serial.println(__FILE__);
  Serial.print(F("Compiled "));
  Serial.print(__TIME__);
  Serial.print(F(" "));
  Serial.println(__DATE__);
  Serial.println(F(programversion));
  Serial.println(F("Stuart Robinson"));
  Serial.println();

  Serial.println("55_Ranging_Slave Starting");

  pinMode(LED1, OUTPUT);
  led_Flash(2, 125);

  SPI.begin();

  if (LT.begin(NSS, NRESET, RFBUSY, DIO1, LORA_DEVICE))
  {
    Serial.println(F("Device found"));
    led_Flash(2, 125);
  }
  else
  {
    Serial.println(F("No device responding"));
    while (1)
    {
      led_Flash(50, 50);                                 //long fast speed flash indicates device error
    }
  }

  //The function call list below shows the complete setup for the LoRa device for ranging using the information
  //defined in the Settings.h file.
  //The 'Setup LoRa device for Ranging' list below can be replaced with a single function call, note that
  //the calibration value will be loaded automatically from the table in the library;
  //LT.setupRanging(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, RangingAddress, RangingRole);

  LT.setupRanging(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, RangingAddress, RANGING_SLAVE);

  //***************************************************************************************************
  //Setup LoRa device for Ranging Slave
  //***************************************************************************************************
  LT.setMode(MODE_STDBY_RC);
  LT.setPacketType(PACKET_TYPE_RANGING);
  LT.setModulationParams(SpreadingFactor, Bandwidth, CodeRate);
  LT.setPacketParams(12, LORA_PACKET_VARIABLE_LENGTH, 0, LORA_CRC_ON, LORA_IQ_NORMAL, 0, 0);
  LT.setRfFrequency(Frequency, Offset);
  LT.setTxParams(TXpower, RADIO_RAMP_02_US);
  LT.setRangingMasterAddress(RangingAddress);
  LT.setRangingSlaveAddress(RangingAddress);
  LT.setRangingCalibration(LT.lookupCalibrationValue(SpreadingFactor, Bandwidth));
  LT.setRangingRole(RANGING_SLAVE);
  LT.writeRegister(REG_RANGING_FILTER_WINDOW_SIZE, 8); //set up window size for ranging averaging
  LT.setHighSensitivity();
  //***************************************************************************************************


  LT.setRangingCalibration(11300);               //override automatic lookup of calibration value from library table

  Serial.print(F("Calibration,"));
  Serial.println(LT.getSetCalibrationValue());           //reads the calibratuion value currently set
}
