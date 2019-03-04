# Cloud-Control
Senior Capstone Project

## Abstract
Communicating with groups of people during an emergency situation is an extremely important and challenging task. The Cloud Control project aims to create a crowd control solution using a hexacopter drone. The project consists of a ground control application (GCS) which records the user and transmits their voice data to the drone. The drone is equipped with our custom chip and a speaker system which will amplify the voice messages to a target on the ground. The target application is to be able to fly the drone over a single person or a group of people and relay a message to them from the GCS through a microphone. The GCS sends the captured voice data to the chip on the drone over a wireless connection, and the chip processes the data and outputs it through the attached speaker. The potential applications of this project are very broad and include search and rescue operations, active shooter scenarios, and even recreational events like concerts and festivals.

## Devices
1. Raspberry Pi acts as the GCS. Sends data over NRF24 wireless module.
2. LPC 4088 acts as reciever and is connected to drone. Recieves data over NRF24 wireless module.


## Prerequisites
1. Install pyaudio on Raspberry Pi
2. Turn on SPI controls and install spidev on Raspberry Pi
3. Configure USB microphone thorugh `/etc/asound.conf` on Raspberry Pi
