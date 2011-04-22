SRCDIR=src
BINDIR=bin

CFLAGS=-O2 -Isrc/ -I/usr/include/opencv/ -g -std=c99 -Wextra
LDFLAGS=  -lm -lcv -lcvaux -lhighgui

MODULES=$(BINDIR)/main.o $(BINDIR)/utils.o $(BINDIR)/wtrap.o $(BINDIR)/gajor.o

PROGNAME=picdefo

all: bin/$(PROGNAME)

clean:
	rm $(BINDIR)/*.o
	rm $(BINDIR)/$(PROGNAME)

$(BINDIR)/main.o:	$(SRCDIR)/main.c
	gcc -c $(CFLAGS) $(SRCDIR)/main.c -o $(BINDIR)/main.o

$(BINDIR)/utils.o:	$(SRCDIR)/utils/utils.c
	gcc -c $(CFLAGS) $(SRCDIR)/utils/utils.c -o $(BINDIR)/utils.o

$(BINDIR)/wtrap.o:	$(SRCDIR)/warptrap/wtrap.c
	gcc -c $(CFLAGS) $(SRCDIR)/warptrap/wtrap.c -o $(BINDIR)/wtrap.o

$(BINDIR)/gajor.o:	$(SRCDIR)/gajor/gajor.c
	gcc -c $(CFLAGS) $(SRCDIR)/gajor/gajor.c -o $(BINDIR)/gajor.o

bin/$(PROGNAME):	$(MODULES)
	gcc $(CFLAGS) $(MODULES) $(LDFLAGS) -o $(BINDIR)/$(PROGNAME)

