import pyaudio

CHUNK = 16
FORMAT = pyaudio.paInt16
CHANNELS = 1
RATE = 6000
RECORD_SECONDS = 5

p = pyaudio.PyAudio()

stream = p.open(format=FORMAT,
                channels=CHANNELS,
                rate=RATE,
                input=True,
                frames_per_buffer=CHUNK)

stream.start_stream()

def main():
    print("*_>recording")

    for i in range(0, int(RATE / CHUNK * RECORD_SECONDS)):
        try:
            data = stream.read(CHUNK)
        except Exception as e:
            print e
            data = '\x00' * CHUNK

        # PRINT THE DATA AS A 32-BYTE HEX STRING FOR DEBUGGING
        # hexData = ":".join("{:02x}".format(ord(c)) for c in data)
        # print hexData

        ### send data here
        # nrfsend(data)

    print("*_>done recording")

    stream.stop_stream()
    stream.close()
    p.terminate()

    print("*_>closed")

if __name__ == '__main__':
    main()
