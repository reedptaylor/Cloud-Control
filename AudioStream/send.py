### send.py by Reed Taylor, Andrew Thompson, and Brent Morada
### addapted from NRF24 drivers at https://github.com/TMRh20/RF24/blob/master/pyRF24/readme.md

from __future__ import print_function
import time
import sys
from RF24 import *
import RPi.GPIO as GPIO
import pyaudio
import struct
from Tkinter import *
import thread

stopRecording = True
failedPackets = 0

def startRecording():
    global radio
    global failedPackets
    global app
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
    while(True):

        # First, stop listening so we can talk.
        radio.stopListening()

        if (not stopRecording):
            # Take the time, and send it.  This will block until complete
            try:
                data = stream.read(CHUNK, exception_on_overflow = False)

            except IOError as ex:
                if ex[1] != pyaudio.paInputOverflowed:
                    stream.close()
                    p.terminate()
                    raise
                data = '\x00' * chunk

            # print('Now sending size: ' + len(data))
            ## PRINT THE DATA AS A 32-BYTE HEX STRING FOR DEBUGGING
            # hexData = ":".join("{:02x}".format(ord(c)) for c in (data))
            # print(hexData)

            result = radio.write(data) #time consuming
            if not result:
                # print("failed.")
                failedPackets = failedPackets + 1
                app.setFailed()

            payload += 1
        else:
            print("*_>done recording")
            stream.stop_stream()
            stream.close()
            p.terminate()
            return


class Application(Frame):

    def start_recording(self):
        global failedPackets
        global stopRecording
        stopRecording = False
        failedPackets = 0
        self.failed["text"] = "Failed packets: " + str(failedPackets)
        thread.start_new_thread(startRecording, ())
        self.record["text"] = "Stop recording"
        self.record["command"] = self.stop_recording

    def stop_recording(self):
        global stopRecording
        stopRecording = True
        self.record["text"] = "Start recording"
        self.record["command"] = self.start_recording

    def createWidgets(self):
        global failedPackets
        self.QUIT = Button(self)
        self.QUIT["text"] = "QUIT"
        self.QUIT["fg"]   = "red"
        self.QUIT["command"] =  self.quit
        self.QUIT["height"] = 2
        self.QUIT["width"] = 6
        self.QUIT.config(font=("Lato", 20))
        self.QUIT.grid(row=2,column=1)

        self.record = Button(self)
        self.record["text"] = "Start recording"
        self.record["command"] = self.start_recording
        self.record["height"] = 7
        self.record["width"] = 20
        self.record.config(font=("Lato", 25))
        self.record.grid(row=0,column=0)

        self.failed = Label(self)
        self.failed["text"] = "Failed packets: " + str(failedPackets)
        self.failed.config(font=("Lato", 15))
        self.failed.grid(row=2,column=0)

        self.failed = Label(self)
        self.failed["text"] = "Failed packets: " + str(failedPackets)
        self.failed.config(font=("Lato", 15))
        self.failed.grid(row=2,column=0)

        self.logo = PhotoImage(file="/home/pi/Desktop/Cloud-Control/logo/CloudControl.png")
        self.logo = self.logo.subsample(6, 6)
        self.logolabel = Label(self, image=self.logo)
        self.logolabel.config(background='#B8D7FD')
        self.logolabel.grid(row=0,column=1)

    def setFailed(self):
        global failedPackets
        self.failed["text"] = "Failed packets: " + str(failedPackets)

    def __init__(self, master=None):
        Frame.__init__(self, master)
        self.pack()
        self.createWidgets()


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

root = Tk()
root.attributes("-fullscreen", True)
root.configure(background='#B8D7FD')
app = Application(master=root)
app.mainloop()
root.destroy()
