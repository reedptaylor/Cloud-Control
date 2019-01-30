import socket
import pyaudio

CHUNK = 16
FORMAT = pyaudio.paInt16
CHANNELS = 1
RATE = 6000
RECORD_SECONDS = 10

HOST = 'localhost'    # The remote host
PORT = 12000             # The same port as used by the server

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

p = pyaudio.PyAudio()

stream = p.open(format=FORMAT,
                channels=CHANNELS,
                rate=RATE,
                input=True,
                frames_per_buffer=CHUNK)

stream.start_stream()


def main():
    totalLength = 0
    print("*_>recording")

    for i in range(0, int(RATE / CHUNK * RECORD_SECONDS)):
        try:
            data = stream.read(CHUNK)
        except Exception as e:
            print e
            data = '\x00' * CHUNK

        # print len(data)
        totalLength += len(data)
        s.sendall(data)

    print("*_>done recording")
    print(totalLength)

    stream.stop_stream()
    stream.close()
    p.terminate()
    s.close()

    print("*_>closed")

if __name__ == '__main__':
    main()
