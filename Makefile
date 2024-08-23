CC = gcc
CFLAGS = -std=c11 -Wall -Werror -Wno-unused-function -Wno-unused-parameter -g -O0
LDFLAGS = 

DESTDIR ?= 
PREFIX ?= /usr

TARGET0 = dbmaker
OBJECT0 = $(SOURCE0:%.c=%.c.o)
SOURCE0 = main.c database.c

all: $(TARGET0)

clean:
	rm -f *.o

distclean: clean
	rm -f $(TARGET0)

dist: distclean
	tar cvf ../$(DIRNAME)-latest.txz ../$(DIRNAME)

install: $(TARGET0)
	cp $(TARGET0) $(DESTDIR)$(PREFIX)/bin

uninstall:
	rm -f $(TARGET0)

$(TARGET0): $(OBJECT0)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.c.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

