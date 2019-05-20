#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
//#include <"SdFs.h">
#include <SD.h>
#include <SerialFlash.h>

// This program simply reads audio data from SD card (.wav files
// of 44.1kHz, 16bit, mono) port on prototype board, sends data over
// i2S to audio codec (DAC) and out the headphone out port.
// Using SPI MOSI and SCK for serial controls

AudioPlaySdWav           playWav1; // only supports 44.1kHz 16bit .wav
AudioOutputI2S           audioOutput;
//AudioOutputAnalog      audioOutput; // if sending through amp
AudioConnection          patchCord1(playWav1, 0, audioOutput, 0);
AudioConnection          patchCord2(playWav1, 1, audioOutput, 1);
AudioControlSGTL5000     sgtl5000_1;

// If using audio shield's sd port
//#define SDCARD_CS_PIN    10
//#define SDCARD_MOSI_PIN  7
//#define SDCARD_SCK_PIN   14

// If using Teensy board's sd port
#define SDCARD_CS_PIN    BUILTIN_SDCARD
#define SDCARD_MOSI_PIN  11  // not actually used
#define SDCARD_SCK_PIN   13  // not actually used

void setup() {
  Serial.begin(9600);
  AudioMemory(8);

  sgtl5000_1.enable();
  sgtl5000_1.volume(0.75); // haven't tried higher cutoffs

  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
}

void playFile(const char *filename)
{
  Serial.print("Playing file: ");
  Serial.println(filename);
  
  playWav1.play(filename);
  // delay to read file
  delay(5);
  
  while (playWav1.isPlaying()) {
    // float vol = analogRead(15);
    // vol = vol / 1024;
    // sgtl5000_1.volume(vol);
  }
}


void loop() {
  playFile("MALE.WAV"); // short clip of different male voices
  delay(500);
  playFile("FEMALE.WAV"); // short clip of different female voices
  delay(500);
  playFile("SDTEST1.WAV"); // music file 1
  delay(500);
  playFile("SDTEST2.WAV"); // music file 2
  delay(500);
  playFile("SDTEST3.WAV"); // music file 3
  delay(500);
  playFile("SDTEST4.WAV"); // music file 4
  delay(1500);
}
