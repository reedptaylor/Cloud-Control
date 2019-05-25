
/*
* teensyduino36 port of our lpc4088 code.
*/

#include <SPI.h>
#include "RF24.h"
#include <Audio.h>
//#include <SD.h>
#include <SerialFlash.h>
#include <Wire.h>

/****************** User Config ***************************/

/* WavFilePlayer uses spi too... ???????????????????????????????????? SD card */

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(26,27);

// Codec initialization
AudioPlayQueue           queue1;
AudioOutputI2S           i2s1;
AudioConnection          patchCord1(queue1, 0, i2s1, 0);
AudioConnection          patchCord2(queue1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;
uint8_t bufcount;
uint32_t buff[64];

/**********************************************************/

void setup() {
  Serial.begin(9600);

  /* Setup GND and VCC */
  pinMode(32, OUTPUT);
  pinMode(31, OUTPUT);
  digitalWrite(32, LOW); // GND
  digitalWrite(31, HIGH); // VCC

  /* Setup new pins for SPI*/
  SPI.setMISO(12); // 30 BM set this originally
  SPI.setMOSI(7); // 29 BM set this originally
  SPI.setSCK(14); // 28 BM set this originally
  
  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
 // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);

  byte pipes[][6] = {"1Node","2Node"};
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  
  // Start the radio listening for data
  radio.startListening();

  // Set up codec
  AudioMemory(512);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);
  AudioProcessorUsageMaxReset();
  AudioMemoryUsageMaxReset();
  bufcount = 0;
  Serial.print("Set Up");
}

void loop() {
  if( radio.available()){
    if ((bufcount >= 0) && (bufcount < 8)) {
      radio.read(&buff[bufcount * 8], 32);
      if (bufcount == 7) {
        bufcount = 0;
        int16_t *p = queue1.getBuffer();
        memcpy(p, &buff[0], 256);
        queue1.playBuffer();
      } else bufcount++;
    }
  }
} // Loop
