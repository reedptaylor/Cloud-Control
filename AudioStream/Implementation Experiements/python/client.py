import socket
import pyaudio
import time

CHUNK = 16
FORMAT = pyaudio.paInt16
CHANNELS = 1
RATE = 6000
RECORD_SECONDS = 10

HOST = '192.168.42.1'    # The remote host
PORT = 12001             # The same port as used by the server

p = pyaudio.PyAudio()

print p.get_device_info_by_index(1)['name'], p.get_device_info_by_index(1)['maxInputChannels']

stream = p.open(format=FORMAT,
                channels=CHANNELS,
                rate=RATE,
                input=True,
                input_device_index = 0,
                frames_per_buffer=CHUNK)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

stream.start_stream()


def main():
    totalLength = 0
    print("*_>recording")

    for i in range(0, int(RATE / CHUNK * RECORD_SECONDS)):
        try:
            data = stream.read(CHUNK)
        except IOError as ex:
            if ex[1] != pyaudio.paInputOverflowed:
                stream.stop_stream()
                time.sleep(2)
                stream.close()
                time.sleep(2)
                p.terminate()
                s.close()
                raise
            data = '\x00' * chunk

        # print len(data)
        totalLength += len(data)
        s.sendall(data)

    print("*_>done recording")
    print(totalLength)

    stream.stop_stream()
    time.sleep(2)
    stream.close()
    time.sleep(2)
    p.terminate()
    s.close()

    print("*_>closed")

if __name__ == '__main__':
    main()
