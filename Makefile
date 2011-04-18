SRCDIR=src
BINDIR=bin

CFLAGS=-O2 -Isrc/ -I/usr/include/opencv/ -g -std=c99 -Wextra
LDFLAGS=  -lm -lcv -lcvaux -lhighgui

MODULES=$(BINDIR)/main.o $(BINDIR)/spotclear.o $(BINDIR)/utils.o

PROGNAME=ltresh

all: bin/$(PROGNAME)

clean:
	rm $(BINDIR)/*.o
	rm $(BINDIR)/$(PROGNAME)

$(BINDIR)/main.o:	$(SRCDIR)/main.c
	gcc -c $(CFLAGS) $(SRCDIR)/main.c -o $(BINDIR)/main.o

$(BINDIR)/spotclear.o:	$(SRCDIR)/spotclear/spotclear.c
	gcc -c $(CFLAGS) $(SRCDIR)/spotclear/spotclear.c -o $(BINDIR)/spotclear.o

$(BINDIR)/utils.o:	$(SRCDIR)/utils/utils.c
	gcc -c $(CFLAGS) $(SRCDIR)/utils/utils.c -o $(BINDIR)/utils.o

bin/$(PROGNAME):	$(MODULES)
	gcc $(CFLAGS) $(MODULES) $(LDFLAGS) -o $(BINDIR)/$(PROGNAME)

