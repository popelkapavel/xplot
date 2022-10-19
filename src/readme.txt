Latest windows version on http://popelkapavel.sweb.cz/xplot/
Original version ftp://ftp.ma.utexas.edu/pub/mzou/

Original readme is at file readme2.

You need GRX graphics library to compile xplot for djgpp, set correct GRX
path in makefile. Then run "make" and you should create xplot.exe.
In doc dir is ascii man page, html and latex tutorial. "make install" only
strips xplot.exe.

Versions:
0.12	corrected syntax in html file
0.11	creating tmp files in 8.3 format in %TEMP% dir, not as 
	/tmp/xplot$PID.d??, which couse trables
0.10	first release

What is new in dos port:

Very primitive html version of LaTeX manual.

Some command line parametres was added.

xplot [-i] [-r WIDTHxHEIGHT] [filename] ...

-i		- plot do not check polygon intersection, this speed up
		preparing for plotting, at the cost of some chaos in
		intersecting polygons

-r WIDTHxHEIGHT - sets video mode resolution, typical 640x480, 800x600, ...
		resolutions higher than 640x480 may work strange under NT.

-h		short help

filename	- xplot process file as it do with load command,
		more filenames can be put, you can edit your file in editor
		and then "compile" it with xplot, to see results.

nice plotting.

			Pavel
