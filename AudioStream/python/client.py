import socket
import pyaudio

CHUNK = 512
FORMAT = pyaudio.paFloat32
CHANNELS = 2
RATE = 44100
RECORD_SECONDS = 5

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
    print("*_>recording")

    for i in range(0, int(RATE / CHUNK * RECORD_SECONDS)):
        try:
            data = stream.read(CHUNK)
        except Exception as e:
            print e
            data = '\x00' * CHUNK

        # print len(data)
        s.sendall(data)

    print("*_>done recording")

    stream.stop_stream()
    stream.close()
    p.terminate()
    s.close()

    print("*_>closed")

if __name__ == '__main__':
    main()
