### send.py by Reed Taylor, Andrew Thompson, and Brent Morada
### addapted from NRF24 drivers at https://github.com/TMRh20/RF24/blob/master/pyRF24/readme.md

from __future__ import print_function
import time
import sys
from RF24 import *
import RPi.GPIO as GPIO
import pyaudio
import time
import struct

irq_gpio_pin = None

########### USER CONFIGURATION ###########
# See https://github.com/TMRh20/RF24/blob/master/pyRF24/readme.md

# CE Pin, CSN Pin, SPI Speed

# Setup for GPIO 22 CE and CE0 CSN with SPI Speed @ 8Mhz
#radio = RF24(RPI_V2_GPIO_P1_15, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_8MHZ)

#RPi B
# Setup for GPIO 15 CE and CE1 CSN with SPI Speed @ 8Mhz
#radio = RF24(RPI_V2_GPIO_P1_15, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_8MHZ)

#RPi B+
# Setup for GPIO 22 CE and CE0 CSN for RPi B+ with SPI Speed @ 8Mhz
radio = RF24(RPI_BPLUS_GPIO_J8_15, RPI_BPLUS_GPIO_J8_24, BCM2835_SPI_SPEED_8MHZ)

# RPi Alternate, with SPIDEV - Note: Edit RF24/arch/BBB/spi.cpp and  set 'this->device = "/dev/spidev0.0";;' or as listed in /dev
#radio = RF24(22, 0);


# Setup for connected IRQ pin, GPIO 24 on RPi B+; uncomment to activate
#irq_gpio_pin = RPI_BPLUS_GPIO_J8_18
#irq_gpio_pin = 24

##########################################
def try_read_data(channel=0):
    if radio.available():
        while radio.available():
            len = radio.getDynamicPayloadSize()
            receive_payload = radio.read(len)
            print('Got payload size={} value="{}"'.format(len, receive_payload.decode('utf-8')))
            # First, stop listening so we can talk
            radio.stopListening()

            # Send the final one back.
            radio.write(receive_payload)
            print('Sent response.')

            # Now, resume listening so we catch the next packets.
            radio.startListening()

pipes = ["1Node", "2Node"]
min_payload_size = 4
max_payload_size = 32
payload_size_increments_by = 1
next_payload_size = min_payload_size
inp_role = 'none'
millis = lambda: int(round(time.time() * 1000))

# print('pyRF24/examples/pingpair_dyn/')
radio.begin()
radio.setRetries(5,15)
radio.printDetails()

radio.openWritingPipe(pipes[0])
radio.openReadingPipe(1,pipes[1])

# forever loop

payload = 1

CHUNK = 16
FORMAT = pyaudio.paInt16
CHANNELS = 1
RATE = 12000
RECORD_SECONDS = 10

p = pyaudio.PyAudio()
# print(p.get_device_info_by_index(1)['name'], p.get_device_info_by_index(1))

stream = p.open(format=FORMAT,
                channels=CHANNELS,
                rate=RATE,
                input=True,
                input_device_index = 1,
                frames_per_buffer=CHUNK)

stream.start_stream()
# print(p.get_device_info_by_index(0))

print("*_>recording")
# for i in range(5):
while(True):

    # First, stop listening so we can talk.
    radio.stopListening()

    # Take the time, and send it.  This will block until complete
    try:
        data = stream.read(CHUNK, exception_on_overflow = False)

    except IOError as ex:
        if ex[1] != pyaudio.paInputOverflowed:
            # stream.stop_stream()
            # time.sleep(2)
            stream.close()
            time.sleep(2)
            p.terminate()
            raise
        data = '\x00' * chunk

    # print('Now sending size: ' + len(data))
    ## PRINT THE DATA AS A 32-BYTE HEX STRING FOR DEBUGGING
    # hexData = ":".join("{:02x}".format(ord(c)) for c in (data))
    # print(hexData)

    result = radio.write(data) #time consuming
    if not result:
        print("failed.")

    payload += 1


print("*_>done recording")

stream.stop_stream()
time.sleep(2)
stream.close()
time.sleep(2)
p.terminate()
