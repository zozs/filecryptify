CFLAGS=-O2 -Wall -pedantic -std=c99 -D_XOPEN_SOURCE=500 $(LIBSODIUM_CFLAGS)
LDFLAGS=$(LIBSODIUM_LIBS)

LIBSODIUM_CFLAGS!=pkg-config --cflags libsodium
LIBSODIUM_LIBS!=pkg-config --libs libsodium

OBJS=filecryptify.o

.PHONY: clean

all: filecryptify

filecryptify: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f filecryptify filecryptify.o
