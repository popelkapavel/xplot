
#include <stdio.h>
#include "vogl.h"
#include "pvm.h"
#define MIN(x,y) ( (x)> (y)? (y): (x))
/**************************************************************/
extern pssetup pspage;
extern int ps_grayscale;
/**************************************************************/
typedef struct 
{
  short r, g, b;
} Coltab;

#define I255 0.0039215686  /*   1/255  */

static Coltab	coltab[256];
extern char	*vallocate();
extern FILE	*_voutfile();

static int ps_first_time = 1, drawn = 0,
           curcol = 0,			/* black */
           pslstx = -1, pslsty = -1,	/* last (x, y) drawn */
           page;

static FILE	*fp;

/*
 * noop
 *
 *	do nothing but return -1
 */
static int noop() {  return(-1); }
/*
 * PS_color
 *
 *	change the grey value of the ink
 */
static int PS_color(col)
     int col;
{
  curcol = col;
  curcol %= 256;

  if(!ps_grayscale) 
    (void)fprintf(fp, "%3.2f %3.2f %3.2f c\n",
		  (float) (coltab[curcol].r)* I255, 
		  (float) (coltab[curcol].g)* I255, 
		  (float) (coltab[curcol].b)* I255 );
  else /*       0.299 * r + 0.584 * g + 0.114 * b          */
    (void)fprintf(fp, "%3.2f g\n", 
		  0.001172549 * (float) (coltab[curcol].r) +
		  0.0023019608* (float) (coltab[curcol].g) +
		  0.0004470588235* (float) (coltab[curcol].b)) ;
  return(0);
}

/*
 * PS_mapcolor
 *
 *	Set our values in our pseudo colour map.
 */
static int PS_mapcolor(indx, r, g, b)
     int indx, r, g, b;
{
  if (indx < 256 && indx >= 0) {
    coltab[indx].r = (short) r;
    coltab[indx].g = (short) g; 
    coltab[indx].b = (short) b;
  }
  return(0);
}

/*
 * PS_common_init
 *
 *	 Initialization that is common to both layouts
 */
static int PS_common_init()
{
  vdevice.depth = 8;
  /*
   * Set other line drawing parameters	
   */
  (void)fprintf(fp, "2 setlinewidth\n1 setlinejoin\n1 setlinecap\n");
  /*
   * Speed up symbol font handling	
   */
  (void)fprintf(fp, "/sf /TimesRoman findfont def\n");

  /*
   * Move	
   */
  (void)fprintf(fp, "/m /moveto load def\n");
  /*
   * Draw
   */
  (void)fprintf(fp, "/d { lineto currentpoint stroke moveto } def\n");
  /*
   * Polygon Draw	
   */
  (void)fprintf(fp, "/p /lineto load def\n");
  /*
   * Set character height	
   */
  (void)fprintf(fp, "/h { sf exch scalefont setfont } def\n");
  /*
   * Show character string
   */
  (void)fprintf(fp, "/s /show load def\n");
  /*
   * Set gray scale	
   */
  (void)fprintf(fp, "/g /setgray load def\n");
  (void)fprintf(fp, "/c /setrgbcolor load def\n");
  PS_mapcolor(0, 0, 0, 0);
  PS_mapcolor(1, 255, 0, 0);
  PS_mapcolor(2, 0, 255, 0);
  PS_mapcolor(3, 255, 255, 0);
  PS_mapcolor(4, 0, 0, 255);
  PS_mapcolor(5, 255, 0, 255);
  PS_mapcolor(6, 0, 255, 255);
  PS_mapcolor(7, 255, 255, 255);
  /*	
   * Set a default font height
   */
  (void)fprintf(fp, "45 h\n");
  return(1);
}

/*
 * PS_init
 *
 *	set up the postcript environment. Returns 1 on success.
 */
static int PS_init()
{
  int psx, psy, psw, psh;
  fp = _voutfile();

  if (!ps_first_time)  return(1);
  page = 1;
  /*
   *  set up the page layout
   */
  psx = pspage.x * 6; psy = pspage.y * 6;
  psw = pspage.w * 6;  psh = pspage.h * 6;
  
  fputs("%!PS-Adobe-2.0 EPSF-1.2\n", fp);
  /*
   * Re compute all this
   */
  (void)fprintf(fp, "%%%%BoundingBox: %d %d %d %d \n", psx,psy,psw+psx,psh+psy);
  (void)fprintf(fp, "%%%%Page: %d %d\n", page, page);
  fputs("%%EndComments\n", fp);
  PS_common_init();
  if(pspage.filemode == PORTRAIT)
    (void)fprintf(fp, "%d %d translate\n", psx,psy);
  else
    (void)fprintf(fp,"90 rotate\n %d %d  translate\n", psx,-612 + psy );
  
  vdevice.sizeSy = (psh * 300)/72;
  vdevice.sizeSx = (psw * 300)/72;
  vdevice.sizeX = vdevice.sizeY = MIN( vdevice.sizeSx, vdevice.sizeSy);
  (void)fprintf(fp, "72 300 div dup scale\n");

  return (1);
}

/*
 * PS_exit
 *
 *	do a showpage and close the output file if neccessary.
 */
static int PS_exit()
{
  fputs("showpage\n", fp);
  fputs("%%Trailer\n", fp);
  fflush(fp);

  if (fp != stdout)
    fclose(fp);

  /*
   * Bug fix from brett@kirk.es.go.dlr.de (Bernward Bretthauer)
   */
  ps_first_time = 1;
  drawn = 0;
  curcol = 0;			/* black */
  pslstx = pslsty = -1;

  return(0);
}

/*
 * PS_draw
 *
 *	draw to an x, y point.
 */
static int PS_draw(x, y)
     int x, y;
{
  (void)fprintf(fp, "%d %d m\n", vdevice.cpVx, vdevice.cpVy);
  (void)fprintf(fp, "%d %d d\n", x, y);
  pslstx = x;
  pslsty = y;
  drawn = 1;

  return(0);
}

static int PS_pnt(x, y)
     int x, y;
{
  (void)fprintf(fp, "%d %d m\n", x, y);
  (void)fprintf(fp, "%d %d d\n", x, y);
  
  return(0);
}


/*
 * PS_font
 *
 * load in small or large - could be improved.
 */
static int PS_font(font)
     char *font;
{
  if (strcmp(font, "small") == 0) {
    vdevice.hwidth = 30.0;
    vdevice.hheight = vdevice.hwidth * 1.833;
    (void)fprintf(fp, "%d h\n", (int)vdevice.hheight);
  } else if (strcmp(font, "large") == 0) {
    vdevice.hwidth = 40.0;
    vdevice.hheight = vdevice.hwidth * 1.833;
    (void)fprintf(fp, "%d h\n", (int)vdevice.hheight);
  } else    return(0);
  return(1);
}

/*
 * PS_clear
 *
 *	flush the current page without resetting the graphics state of the
 * laser printer.
 */
static int PS_clear()
{
  if(drawn)
    {
      (void)fprintf(fp, "gsave showpage grestore\n");
      /* This is the end of the page, not of the document. */
      /*  ralf@physik3.gwdg.de (Ralf Fassel) */
      fputs("%%PageTrailer\n", fp);
      page++;
      (void)fprintf(fp, "%%%%Page: %d %d\n", page, page);
    }
  else
    {
      (void)fprintf(fp, "newpath\n"); 
      (void)fprintf(fp, " %d %d moveto \n", 0,0);
      (void)fprintf(fp, " %d %d lineto \n", vdevice.sizeSx,0);
      (void)fprintf(fp, " %d %d lineto \n", vdevice.sizeSx,vdevice.sizeSy);
      (void)fprintf(fp, " %d %d lineto \n", 0,vdevice.sizeSy);
      (void)fprintf(fp, " closepath\n fill \n");
    }
  drawn = 0;
  return(0);
}

	
/*
 * PS_char
 *
 *	output a character making sure that a '\' is sent first when
 * appropriate.
 */
static int PS_char(c)
     char c;
{
  if (pslstx != vdevice.cpVx || pslsty != vdevice.cpVy)
    (void)fprintf(fp, "%d %d m\n", vdevice.cpVx, vdevice.cpVy);

  (void)fprintf(fp, "(");

  switch(c) {
  case '(':
    (void)fprintf(fp, "\\(");
    break;
  case ')':
    (void)fprintf(fp, "\\)");
    break;
  case '\\':
    (void)fprintf(fp, "\\");
    break;
  default:
    (void)fprintf(fp, "%c",c);
  }
  
  (void)fprintf(fp,") s \n");

  drawn = 1;
  pslstx = pslsty = -1;
  return(0);
}

/*
 * PS_string
 *
 *	output a string one char at a time.
 */
static int PS_string(s)
     char *s;
{
  char	c;

  if (pslstx != vdevice.cpVx || pslsty != vdevice.cpVy)
    (void)fprintf(fp, "%d %d m\n", vdevice.cpVx, vdevice.cpVy);
  
  (void)fprintf(fp, "(");
  while ((c = *s++))
    switch(c) {
    case '(':
      (void)fprintf(fp, "\\(");
      break;
    case ')':
      (void)fprintf(fp, "\\)");
      break;
    case '\\':
      (void)fprintf(fp, "\\");
      break;
    default:
      (void)fprintf(fp, "%c",c);
    }
  
  (void)fprintf(fp,") s \n");
  drawn = 1;
  pslstx = pslsty = -1;

  return(0);
}

/*
 * PS_fill
 *
 *      fill a polygon
 */
static int PS_fill(n, x, y)
     int   n, x[], y[];
{
  int     i;


  (void)fprintf(fp, "newpath \n");

  (void)fprintf(fp, "%d %d m\n", x[0], y[0]);
  
  for (i = 1; i < n; i++)
    (void)fprintf(fp, "%d %d p\n", x[i], y[i]);
  
  (void)fprintf(fp, "closepath\n");

  (void)fprintf(fp, "fill\n");

  vdevice.cpVx = x[n - 1];
  vdevice.cpVy = y[n - 1];

  pslstx = pslsty = -1;		/* fill destroys current path */
 
  return(0);
}
static int PS_setlw(w)
     int w;
{
  (void)fprintf(fp, "%d setlinewidth\n", w * 2 + 1);

  return(0);
}

/*
 * Set the line style...
 */
static int PS_setls(lss)
     int lss;
{
  unsigned ls = lss;
  int	i, d, a, b, offset;

  if (ls == 0xffff) {
    (void)fprintf(fp, "[] 0 setdash\n");
    return(0);
  }

  fputc('[', fp);

  for (i = 0; i < 16; i++)	/* Over 16 bits */
    if ((ls & (1 << i)))
      break;

  offset = i;

#define	ON	1
#define	OFF	0
		
  a = b = OFF;
  if (ls & (1 << 0))
    a = b = ON;

  d = 0;
  for (i = 0; i < 16; i++) {	/* Over 16 bits */
    if (ls & (1 << i))
      a = ON;
    else
      a = OFF;
    
    if (a != b) {
      b = a;
      (void)fprintf(fp, "%d ", d * 2 + 1);
      d = 0;
    }
    d++;
  }

  (void)fprintf(fp, "] %d setdash\n", offset);

  return(0);
}

int PS_swapbuf()
{
  return (1) ;
}

static DevEntry psdev = {
	"postscript",
	"large",
	"small",
	noop,
	PS_char,
	noop,
	PS_clear,
	PS_color,
	PS_draw,
	PS_pnt,
	PS_exit,
	PS_fill,
	PS_font,
	noop,
	noop,
	PS_init,
	noop,
	PS_mapcolor,
	PS_setls,
	PS_setlw,
	PS_string,
	PS_swapbuf,
	noop,
	noop,   /* winset */
};

/*
 * _CPS_devcpy
 *
 *	copy the postscript device into vdevice.dev.
 * 	Set it so we can use colours.
 */
int _CPS_devcpy()
{
  vdevice.dev = psdev;
  vdevice.dev.devname = "cps";

  return(0);
}

/*
 * _PS_devcpy
 *
 *	copy the postscript device into vdevice.dev.
 */
int _PS_devcpy()
{
  vdevice.dev = psdev;

  return(0);
}
/********************************************************/
