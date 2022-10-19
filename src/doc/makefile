#
#   
#

MANDIR= 

all: doc

doc: tutor.dvi

tutor.dvi: tutor.tex
	latex tutor.tex

install: 
	cp xplot.1 $(MANDIR)

clean:
	rm *.dvi *.log *.aux *.toc
