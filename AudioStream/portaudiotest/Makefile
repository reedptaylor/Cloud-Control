CC = gcc
ARGS = -Wall

all: test

test: paex_record.c
	$(CC) $(ARGS) -o test /usr/local/lib/libportaudio.a -framework CoreServices -framework CoreFoundation -framework AudioUnit -framework AudioToolbox -framework CoreAudio paex_record.c

clean:
	rm -f *.o test *~
