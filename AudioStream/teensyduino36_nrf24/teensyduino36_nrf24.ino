
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
// Code to change I2S frequency to match specs for NRF communication
void setI2SFreq(int freq)
{
  typedef struct {
    uint8_t mult;
    uint16_t div;
  } __attribute__((__packed__)) tmclk;

  const int numfreqs = 8;
  const int samplefreqs[numfreqs] = { 8000, 11025, 16000, 22050, 32000, 44100, 44117 , 48000 };

#if (F_PLL==16000000)
  const tmclk clkArr[] = {{16, 125}, {148, 839}, {32, 125}, {145, 411}, {64, 125}, {151, 214}, {12, 17}, {96, 125} };
#elif (F_PLL==72000000)
  const tmclk clkArr[] = {{32, 1125}, {49, 1250}, {64, 1125}, {49, 625}, {128, 1125}, {98, 625}, {8, 51}, {64, 375} };
#elif (F_PLL==96000000)
  const tmclk clkArr[] = {{8, 375}, {60, 2041}, {16, 375}, {120, 2041}, {32, 375}, {147, 1250}, {2, 17}, {16, 125} };
#elif (F_PLL==120000000)
  const tmclk clkArr[] = {{32, 1875}, {29, 1233}, {64, 1875}, {89, 1892}, {128, 1875}, {89, 946}, {8, 85}, {64, 625} };
#elif (F_PLL==144000000)
  const tmclk clkArr[] = {{16, 1125}, {40, 2041}, {32, 1125}, {49, 1250}, {64, 1125}, {49, 625}, {4, 51}, {32, 375} };
#elif (F_PLL==180000000)
  const tmclk clkArr[] = {{9, 791}, {31, 1977}, {37, 1626}, {62, 1977}, {73, 1604}, {107, 1706}, {16, 255}, {128, 1875} };
#elif (F_PLL==192000000)
  const tmclk clkArr[] = {{4, 375}, {30, 2041}, {8, 375}, {60, 2041}, {16, 375}, {120, 2041}, {1, 17}, {8, 125} };
#elif (F_PLL==216000000)
  const tmclk clkArr[] = {{17, 1793}, {17, 1301}, {34, 1793}, {49, 1875}, {49, 1292}, {98, 1875}, {8, 153}, {64, 1125} };
#elif (F_PLL==240000000)
  const tmclk clkArr[] = {{16, 1875}, {24, 2041}, {32, 1875}, {29, 1233}, {64, 1875}, {89, 1892}, {4, 85}, {32, 625} };
#endif

  for (int f = 0; f < numfreqs; f++) {
    if ( freq == samplefreqs[f] ) {
      while (I2S0_MCR & I2S_MCR_DUF) ;
      I2S0_MDR = I2S_MDR_FRACT((clkArr[f].mult - 1)) | I2S_MDR_DIVIDE((clkArr[f].div - 1));
      return;
    }
  }
}
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
  radio.setPALevel(RF24_PA_MAX);

  byte pipes[][6] = {"1Node","2Node"};
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  
  // Start the radio listening for data
  radio.startListening();

  // Set up codec
  AudioMemory(512);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.75);
  AudioProcessorUsageMaxReset();
  AudioMemoryUsageMaxReset();
  bufcount = 0;
  setI2SFreq(11025);
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
