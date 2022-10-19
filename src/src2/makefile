#
#

CC = gcc  -O6 -c 



COBJS = eval.o internal.o misc.o parse.o plot.o scanner.o\
             standard.o util.o  init.o new5.o new6.o\
             new0.o new1.o new2.o new3.o new4.o help.o ncnt.o

CSRC = $(COBJS:.o=.c)


all: $(COBJS) 
	

$(CSRC): plot.h const.h


#util.c:  version
#	touch util.c

.c.o:
	$(CC) $(CFLAG) -c $<


clean:
	rm $(COBJS)

