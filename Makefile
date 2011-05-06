SRCDIR=src
BINDIR=bin

CFLAGS=-O2 -Isrc/ -I/usr/include/opencv/ -g -std=c99 -Wextra -DDEBUG
LDFLAGS=  -lm -lcv -lcvaux -lhighgui

MODULES=$(BINDIR)/utils.o $(BINDIR)/wtrap.o $(BINDIR)/spotclear.o \
		$(BINDIR)/handlers.o $(BINDIR)/globs.o $(BINDIR)/windraw.o

PROG1=anastizer
PROG2=bookdet

all: bin/$(PROG1) bin/$(PROG2)

clean:
	rm $(BINDIR)/*.o
	rm $(BINDIR)/$(PROG1)
	rm $(BINDIR)/$(PROG2)

$(BINDIR)/anastizer.o:	$(SRCDIR)/anastizer.c
	gcc -c $(CFLAGS) $(SRCDIR)/anastizer.c -o $(BINDIR)/anastizer.o

$(BINDIR)/bookdet.o:	$(SRCDIR)/bookdet.c
	gcc -c $(CFLAGS) $(SRCDIR)/bookdet.c -o $(BINDIR)/bookdet.o

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

bin/$(PROG1): $(BINDIR)/anastizer.o $(MODULES)
	gcc $(CFLAGS) $(BINDIR)/anastizer.o $(MODULES) $(LDFLAGS) -o $(BINDIR)/$(PROG1)

bin/$(PROG2): $(BINDIR)/bookdet.o $(MODULES)
	gcc $(CFLAGS) $(BINDIR)/bookdet.o $(MODULES) $(LDFLAGS) -o $(BINDIR)/$(PROG2)

