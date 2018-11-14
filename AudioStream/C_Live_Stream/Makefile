CC = gcc
ARGS = -Wall

all: server client

server: server.c
	$(CC) $(ARGS) -o server /usr/local/lib/libportaudio.a -framework CoreServices -framework CoreFoundation -framework AudioUnit -framework AudioToolbox -framework CoreAudio server.c

client: client.c
	$(CC) $(ARGS) -o client /usr/local/lib/libportaudio.a -framework CoreServices -framework CoreFoundation -framework AudioUnit -framework AudioToolbox -framework CoreAudio client.c

clean:
	rm -f *.o server client *~
