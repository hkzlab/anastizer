SRCDIR=src
BINDIR=bin

CFLAGS=-O2 -Isrc/ -I/usr/include/opencv/ -g -std=c99 -Wextra
LDFLAGS=  -lm -lcv -lcvaux -lhighgui

MODULES=$(BINDIR)/main.o $(BINDIR)/utils.o $(BINDIR)/wtrap.o $(BINDIR)/spotclear.o \
		$(BINDIR)/handlers.o $(BINDIR)/globs.o $(BINDIR)/windraw.o

PROGNAME=anastizer

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

$(BINDIR)/spotclear.o:	$(SRCDIR)/spotclear/spotclear.c
	gcc -c $(CFLAGS) $(SRCDIR)/spotclear/spotclear.c -o $(BINDIR)/spotclear.o

$(BINDIR)/handlers.o:	$(SRCDIR)/gui/handlers.c
	gcc -c $(CFLAGS) $(SRCDIR)/gui/handlers.c -o $(BINDIR)/handlers.o

$(BINDIR)/windraw.o:	$(SRCDIR)/gui/windraw.c
	gcc -c $(CFLAGS) $(SRCDIR)/gui/windraw.c -o $(BINDIR)/windraw.o

$(BINDIR)/globs.o:	$(SRCDIR)/common/globs.c
	gcc -c $(CFLAGS) $(SRCDIR)/common/globs.c -o $(BINDIR)/globs.o

bin/$(PROGNAME):	$(MODULES)
	gcc $(CFLAGS) $(MODULES) $(LDFLAGS) -o $(BINDIR)/$(PROGNAME)

