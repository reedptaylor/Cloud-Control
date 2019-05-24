
/*
* teensyduino36 port of our lpc4088 code.
*/

#include <SPI.h>
#include "RF24.h"

/****************** User Config ***************************/

/* WavFilePlayer uses spi too... ???????????????????????????????????? */

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(26,27);

/**********************************************************/

void setup() {
  Serial.begin(115200);

  /* Setup GND and VCC */
  pinMode(32, OUTPUT);
  pinMode(31, OUTPUT);
  digitalWrite(32, LOW); // GND
  digitalWrite(31, HIGH); // VCC

  /* Setup new pins for SPI*/
  SPI.setMISO(12); // 30 BM set this originally
  SPI.setMOSI(11); // 29 BM set this originally
  SPI.setSCK(13); // 28 BM set this originally
  
  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
 // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);

  byte pipes[][6] = {"1Node","2Node"};
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  
  // Start the radio listening for data
  radio.startListening();
}

void loop() {
//  Serial.println("In Loop");
  uint32_t read_payload[8];
  if( radio.available()){
    radio.read( &read_payload, sizeof(uint32_t)*8);
    Serial.print("read_payload: ");
    Serial.println(read_payload[0]);     
  }
} // Loop
