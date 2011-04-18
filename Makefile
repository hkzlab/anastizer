SRCDIR=src
BINDIR=bin

CFLAGS=-O2 -Isrc/ -I/usr/include/opencv/ -g -std=c99 -Wextra
LDFLAGS=  -lm -lcv -lcvaux -lhighgui

MODULES=$(BINDIR)/main.o 

PROGNAME=picdefo

all: bin/$(PROGNAME)

clean:
	rm $(BINDIR)/*.o
	rm $(BINDIR)/$(PROGNAME)

$(BINDIR)/main.o:	$(SRCDIR)/main.c
	gcc -c $(CFLAGS) $(SRCDIR)/main.c -o $(BINDIR)/main.o

bin/$(PROGNAME):	$(MODULES)
	gcc $(CFLAGS) $(MODULES) $(LDFLAGS) -o $(BINDIR)/$(PROGNAME)

