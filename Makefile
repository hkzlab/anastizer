SRCDIR=src
BINDIR=bin

CFLAGS=-O2 -Isrc/ -I/usr/include/opencv/ -g -std=c99 -Wextra -DDEBUG
LDFLAGS=  -lm -lcv -lcvaux -lhighgui

MODULES=$(BINDIR)/utils.o $(BINDIR)/wtrap.o $(BINDIR)/spotclear.o $(BINDIR)/imc.o
		
PR1MODULES=$(BINDIR)/handlers_anastizer.o $(BINDIR)/windraw_anastizer.o $(BINDIR)/anastizer.o $(BINDIR)/globs_anastizer.o
PR2MODULES=$(BINDIR)/bookdet.o

PROG1=anastizer
PROG2=bookdet

all: bin/$(PROG1) bin/$(PROG2)

clean:
	rm $(BINDIR)/*.o
	rm $(BINDIR)/$(PROG1)
	rm $(BINDIR)/$(PROG2)


# Programs
bin/$(PROG1): $(MODULES) $(PR1MODULES)
	gcc $(CFLAGS) $(MODULES) $(PR1MODULES) $(LDFLAGS) -o $(BINDIR)/$(PROG1)

bin/$(PROG2): $(MODULES) $(PR2MODULES)
	gcc $(CFLAGS) $(MODULES) $(PR2MODULES) $(LDFLAGS) -o $(BINDIR)/$(PROG2)


# *** specific modules (ANASTIZER)
$(BINDIR)/anastizer.o:	$(SRCDIR)/anastizer.c
	gcc -c $(CFLAGS) $(SRCDIR)/anastizer.c -o $(BINDIR)/anastizer.o

$(BINDIR)/handlers_anastizer.o:	$(SRCDIR)/gui/handlers_anastizer.c
	gcc -c $(CFLAGS) $(SRCDIR)/gui/handlers_anastizer.c -o $(BINDIR)/handlers_anastizer.o

$(BINDIR)/windraw_anastizer.o:	$(SRCDIR)/gui/windraw_anastizer.c
	gcc -c $(CFLAGS) $(SRCDIR)/gui/windraw_anastizer.c -o $(BINDIR)/windraw_anastizer.o

$(BINDIR)/globs_anastizer.o:	$(SRCDIR)/common/globs_anastizer.c
	gcc -c $(CFLAGS) $(SRCDIR)/common/globs_anastizer.c -o $(BINDIR)/globs_anastizer.o


# *** specific modules (BOOKDET)
$(BINDIR)/bookdet.o:	$(SRCDIR)/bookdet.c
	gcc -c $(CFLAGS) $(SRCDIR)/bookdet.c -o $(BINDIR)/bookdet.o


# generic modules
$(BINDIR)/utils.o:	$(SRCDIR)/utils/utils.c
	gcc -c $(CFLAGS) $(SRCDIR)/utils/utils.c -o $(BINDIR)/utils.o

$(BINDIR)/wtrap.o:	$(SRCDIR)/warptrap/wtrap.c
	gcc -c $(CFLAGS) $(SRCDIR)/warptrap/wtrap.c -o $(BINDIR)/wtrap.o

$(BINDIR)/spotclear.o:	$(SRCDIR)/spotclear/spotclear.c
	gcc -c $(CFLAGS) $(SRCDIR)/spotclear/spotclear.c -o $(BINDIR)/spotclear.o

$(BINDIR)/imc.o:	$(SRCDIR)/imc/imc.c
	gcc -c $(CFLAGS) $(SRCDIR)/imc/imc.c -o $(BINDIR)/imc.o


