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
connectionPackets = 0
DOWNLINK_COUNT = 5

def startRecording():
    global radio
    global failedPackets
    global connectionPackets
    global app

    payload = 1

    CHUNK = 16
    FORMAT = pyaudio.paInt16
    CHANNELS = 1
    RATE = 12000

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
                connectionPackets = connectionPackets + 1
                app.setFailed()
                if connectionPackets >= DOWNLINK_COUNT:
                    app.setDown()
            else:
                if (connectionPackets >= DOWNLINK_COUNT):
                    app.setUp()
                connectionPackets = 0

            payload += 1
        else:
            print("*_>done recording")
            stream.stop_stream()
            stream.close()
            p.terminate()
            return

def notRecording():
    global radio
    global connectionPackets
    global app

    while(stopRecording):
        radio.stopListening()
        result = radio.write('\x00' * 16)
        if not result:
            connectionPackets = connectionPackets + 1
            if connectionPackets >= DOWNLINK_COUNT:
                app.setDown()
        else:
            connectionPackets = 0
            app.setUp()
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
        self.record["image"] = self.square
        self.record["command"] = self.stop_recording

    def stop_recording(self):
        global stopRecording
        stopRecording = True
        thread.start_new_thread(notRecording, ())
        self.record["text"] = "Start recording"
        self.record["image"] = self.circle
        self.record["command"] = self.start_recording

    def createWidgets(self):
        global failedPackets
        self.QUIT = Button(self)
        self.QUIT["text"] = "QUIT"
        self.QUIT["fg"]   = "red"
        self.QUIT["command"] =  self.quit
        self.QUIT["height"] = 2
        self.QUIT.config(font=("Lato", 20), background='#B8D7FD', activebackground='#B8D7FD')
        self.QUIT.grid(row=3,column=2, sticky=N+S+E+W)

        self.square = PhotoImage(file="/home/pi/Desktop/Cloud-Control/AudioStream/red.png")
        self.square = self.square.subsample(4, 4)
        self.circle = PhotoImage(file="/home/pi/Desktop/Cloud-Control/AudioStream/circle-xxl.png")
        self.circle = self.circle.subsample(2, 2)
        self.record = Button(self)
        self.record["image"] = self.circle
        self.record["text"] = "Start recording"
        self.record["compound"] = "top"
        self.record["command"] = self.start_recording
        self.record.config(background='#B8D7FD')
        self.record.config(activebackground='#B8D7FD')
        self.record["height"] = 310
        self.record["width"] = 310
        self.record.config(font=("Lato", 25), relief=FLAT)
        self.record.grid(row=1,column=0, rowspan=2, sticky=N+S+E+W)

        self.failed = Label(self)
        self.failed["text"] = "Failed packets: " + str(failedPackets)
        self.failed.config(font=("Lato", 18), background='#B8D7FD')
        self.failed.grid(row=1,column=1, columnspan=2, sticky=N+S+E+W)

        self.desciption = Text(self)
        self.desciption.tag_configure("center", justify='center')
        self.desciption.insert(INSERT, "Welcome to the CloudControl base station application! To begin recording, press the start recording option and press again to stop. The failed packets counter shows any drops in communication and Uplink status shows if the drone unit is connected.")
        self.desciption.tag_add("center", "1.0", "end")
        self.desciption["height"] = 1
        self.desciption["width"] = 55
        self.desciption.config(font=("Lato", 18), background='#B8D7FD', pady=100, padx=10, state=DISABLED,wrap=WORD, exportselection=0, relief=FLAT)
        self.desciption.grid(row=0,column=0, sticky=N+S+E+W)

        self.connection = Label(self)
        self.connection["text"] = "Uplink Status:"
        self.connection.config(font=("Lato", 18), background='#B8D7FD')
        self.connection.grid(row=2,column=1, columnspan=2, sticky=N+S+E+W)

        self.logo = PhotoImage(file="/home/pi/Desktop/Cloud-Control/logo/CloudControl.png")
        self.logo = self.logo.subsample(6, 6)
        self.logolabel = Label(self, image=self.logo)
        self.logolabel.config(background='#B8D7FD')
        self.logolabel.grid(row=0,column=1, columnspan=2, sticky=N+S+E+W)

        self.spacer = Label(self)
        self.spacer.config(background='#B8D7FD')
        self.spacer.grid(row=3,column=0, columnspan=2, sticky=N+S+E+W)

    def setFailed(self):
        global failedPackets
        self.failed["text"] = "Failed packets: " + str(failedPackets)

    def setUp(self):
        self.connection["text"] = "Uplink Status: Up"
        self.connection.configure(foreground="green")

    def setDown(self):
        self.connection["text"] = "Uplink Status: Down"
        self.connection.configure(foreground="red")

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
pipes = ["1Node", "2Node"]

radio.begin()
radio.setRetries(5,15)
radio.printDetails()

radio.openWritingPipe(pipes[0])
radio.openReadingPipe(1,pipes[1])

root = Tk()
root.geometry("1280x720")
root.attributes("-fullscreen", True)
root.configure(background='#B8D7FD')
app = Application(master=root)
app.stop_recording() #start checking for connection
app.mainloop()
root.destroy()
