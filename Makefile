.PHONY: check clean install

PREFIX?=/usr/local
DESTDIR?=
MANDIR?=man

.ifndef VERSION
VERSION!=git describe --tags --always --abbrev=4 --dirty 2> /dev/null || true
.endif

.if empty(VERSION)
.error VERSION must either be given, or be possible to deduce with git describe!
.endif

CFLAGS=-O2 -Wall -pedantic -std=c99 -D_XOPEN_SOURCE=500 $(LIBSODIUM_CFLAGS) -DFILECRYPTIFY_VERSION=$(VERSION)
LDFLAGS=$(LIBSODIUM_LIBS)

LIBSODIUM_CFLAGS!=pkg-config --cflags libsodium
LIBSODIUM_LIBS!=pkg-config --libs libsodium

OBJS=filecryptify.o

all: filecryptify

filecryptify: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

check: filecryptify
	cd tests && ./tests.sh

clean:
	rm -f filecryptify filecryptify.o tests/tmp*

install: filecryptify
	mkdir -p $(DESTDIR)$(PREFIX)/bin $(DESTDIR)$(PREFIX)/$(MANDIR)/man1
	install filecryptify $(DESTDIR)$(PREFIX)/bin
	install filecryptify.1 $(DESTDIR)$(PREFIX)/$(MANDIR)/man1
