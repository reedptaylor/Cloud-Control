import Queue
import socket
import wave

import pyaudio

CHUNK = 16
FORMAT = pyaudio.paInt16
CHANNELS = 1
RATE = 6000
WAVE_OUTPUT_FILENAME = "server_output.wav"
WIDTH = 2

HOST = 'localhost'     # Symbolic name meaning all available interfaces
PORT = 12001              # Arbitrary non-privileged port

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
s.listen(1)
conn, addr = s.accept()
print 'Connected by', addr

p = pyaudio.PyAudio()
stream = p.open(format=p.get_format_from_width(WIDTH),
                channels=CHANNELS,
                rate=RATE,
                output=True,
                output_device_index = 1,
                frames_per_buffer=CHUNK)
q = Queue.Queue()

# frames = []

stream.start_stream()


def main():
    data = conn.recv(CHUNK)

    while data != '':
        q.put(data)
        if not q.empty():
            stream.write(q.get())

        # stream.write(data)
        data = conn.recv(CHUNK)
    #     frames.append(data)
    #
    # wf = wave.open(WAVE_OUTPUT_FILENAME, 'wb')
    # wf.setnchannels(CHANNELS)
    # wf.setsampwidth(p.get_sample_size(FORMAT))
    # wf.setframerate(RATE)
    # wf.writeframes(b''.join(frames))
    # wf.close()

    stream.stop_stream()
    stream.close()
    p.terminate()
    conn.close()

if __name__ == '__main__':
    main()
