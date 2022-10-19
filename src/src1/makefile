#
# Make the vogl library.
#
# Below are our defaults for compliers etc. These are set in the
# main Makefile.
#

CC = gcc -O6 -IE:\tmp\contrib\grx249\include

X11SCR = grx.c # X11.c
X11OBJ = grx.o # X11.o
X11LIB = # -lX11

CSRC =	$(X11SRC) ps.c  buffer.c clip.c draw.c drivers.c \
	viewing.c viewp.c mapping.c matrix.c move.c \
	polygons.c text.c  trans.c scale.c valloc.c\
	object.c color.c box.c main.c intxn.c save.c

COBJS = $(X11OBJ) ps.o  buffer.o clip.o draw.o drivers.o \
	viewing.o viewp.o mapping.o matrix.o move.o \
	polygons.o text.o  trans.o scale.o valloc.o\
	object.o color.o box.o main.o intxn.o save.o


all: $(COBJS)


#$(COBJS): vogl.h

#object.c: pvm.h object.h 

#X11.c: vectfont.h pvm.h


.c.o:
	$(CC) -c $<


clean:
	rm -f $(COBJS) 

