#!/usr/bin/env python

#
# Example using Dynamic Payloads
#
#  This is an example of how to use payloads of a varying (dynamic) size.
#

from __future__ import print_function
import time
import sys
from RF24 import *
import RPi.GPIO as GPIO
import pyaudio
import time

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
send_payload = b'ABCDEFGHIJKLMNOPQRSTUVWXYZ789012'
millis = lambda: int(round(time.time() * 1000))

print('pyRF24/examples/pingpair_dyn/')
radio.begin()
radio.setRetries(5,15)
radio.printDetails()

print(' ************ Role Setup *********** ')
while (inp_role !='0') and (inp_role !='1'):
    inp_role = str(input('Choose a role: Enter 0 for receiver, 1 for transmitter (CTRL+C to exit) '))

if inp_role == '0':
    print('Role: Pong Back, awaiting transmission')
    if irq_gpio_pin is not None:
        # set up callback for irq pin
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(irq_gpio_pin, GPIO.IN, pull_up_down=GPIO.PUD_UP)
        GPIO.add_event_detect(irq_gpio_pin, GPIO.FALLING, callback=try_read_data)

    radio.openWritingPipe(pipes[1])
    radio.openReadingPipe(1,pipes[0])
    radio.startListening()
else:
    print('Role: Ping Out, starting transmission')
    radio.openWritingPipe(pipes[0])
    radio.openReadingPipe(1,pipes[1])

# forever loop

payload = 1

CHUNK = 16 #256 ideal (512 bytes)
FORMAT = pyaudio.paInt16
CHANNELS = 1
RATE = 6000
RECORD_SECONDS = 1

p = pyaudio.PyAudio()

print(p.get_device_info_by_index(1)['name'], p.get_device_info_by_index(1))

stream = p.open(format=FORMAT,
                channels=CHANNELS,
                rate=RATE,
                input=True,
                input_device_index = 1,
                frames_per_buffer=CHUNK)

stream.start_stream()
# print(p.get_device_info_by_index(0))

print("*_>recording")
for i in range(0, int(RATE / CHUNK * RECORD_SECONDS)):
    if inp_role == '1':   # ping out
        # The payload will always be the same, what will change is how much of it we send.

        # First, stop listening so we can talk.
        radio.stopListening()

        # Take the time, and send it.  This will block until complete
        try:
            data = stream.read(CHUNK, exception_on_overflow = False) #time consuming

        except IOError as ex:
            if ex[1] != pyaudio.paInputOverflowed:
                # stream.stop_stream()
                # time.sleep(2)
                stream.close()
                time.sleep(2)
                p.terminate()
                raise
            data = '\x00' * chunk

        # print('Now sending  ' + str(send_payload[:16]) + " size: " + str(sys.getsizeof(send_payload[:16])))
        # result = radio.write(send_payload[:16])
        # print('Now sending  ' + str(data) + " size: " + str(sys.getsizeof(data)))
        # print('Now sending size: ' + len(data))
        # PRINT THE DATA AS A 32-BYTE HEX STRING FOR DEBUGGING
        # print(hexData)
        # hexData = ":".join("{:02x}".format(ord(c)) for c in (data))
        # print(hexData)

        result = radio.write(data)
        # if not result:
        #     print("failed.")

        payload += 1

    else:
        # Pong back role.  Receive each packet, dump it out, and send it back

        # if there is data ready
        if irq_gpio_pin is None:
            # no irq pin is set up -> poll it
            try_read_data()
        else:
            # callback routine set for irq pin takes care for reading -
            # do nothing, just sleeps in order not to burn cpu by looping
            time.sleep(1000)

print("*_>done recording")

stream.stop_stream()
time.sleep(2)
stream.close()
time.sleep(2)
p.terminate()
