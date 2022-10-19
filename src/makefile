#
#   Xplot Makefile
#
# BINDIR is the place you want to put the binaries, should be in
#         your search path. 
# MANDIR is the place for the manpage. 
#
#BINDIR=/usr/local/bin

GRXDIR=E:\tmp\contrib\grx249
GRXLIBDIR=$(GRXDIR)/lib/win32
GRXINCDIR=$(GRXDIR)/include
GRXLIB=grx20

BINDIR=/public/bin
#MANDIR=

#CC = cc

CC = gcc -g -O9 -m486 -I$(GRXINCDIR)


# first run make in src1 and src2 directory

xplot.exe: 
	$(CC) src1/*.o src2/*.o -o xplot.exe -L$(GRXLIBDIR) -lm -l$(GRXLIB) -lgdi32 

objs:
	cd src1; make -f Makefile CC="$(CC)";cd .. 
	cd src2; make -f Makefile CC="$(CC)";cd ..


all: xplot.exe
	cd doc; make all 

install: xplot.exe
	strip xplot.exe
#	cp xplot $(BINDIR)
#	cd doc; make install MANDIR="$(MANDIR)";cd ..

clean:
	-rm xplot.exe
	-cd src1; make clean;cd ..
	-cd src2; make clean;cd ..
	-cd doc;  make clean;cd ..




