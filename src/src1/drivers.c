#include <stdio.h>
#include "vogl.h"
#include "vodevice.h"

extern char	*getenv();
extern void     _GRX_devcpy(), _PS_devcpy();
extern char     *vallocate();
extern int     strncmp();
void		gexit();

/********************************************************************/    
struct vdev	       vdevice; 
static int	       allocated = 0;
static FILE	       *voutfp = NULL;
static struct vdev     GRX_device;
int cfonttype;
/********************************************************************/    
void save_GRX_device()
{
  memcpy(&GRX_device, &vdevice, sizeof(struct vdev));
  vdevice.initialised = 0;
}
void recover_GRX_device()
{
  memcpy(&vdevice, &GRX_device, sizeof(struct vdev));
}
/********************************************************************/    
/*
 * voutput
 *
 * redirect output - only for postscript, hpgl (this is not a feature)
 */
void voutput(path)
     char *path;
{
  char	buf[128];

  if ((voutfp = fopen(path, "w")) == (FILE *)NULL) {
    sprintf(buf, "voutput: couldn't open %s", path);
    verror(buf);
  }
}
/********************************************************************/ 
/*
 * _voutfile
 *
 *	return a pointer to the current output file - designed for internal
 * use only.
 */
FILE *_voutfile()
{
  return(voutfp?voutfp:stdout);
}
/********************************************************************/ 
/*
 * verror
 *
 *	print an error on the graphics device, and then exit. Only called
 * for fatal errors. We assume that stderr is always there.
 *
 */
void verror(str)
     char *str;
{
  if(vdevice.initialised)
    gexit();

  fprintf(stderr, "%s\n", str);
	exit(1);
}
/********************************************************************/ 
void viniterror(str)
     char *str;
{
  fprintf(stderr, "%s: vogl not initialised\n", str);
  exit(1);
}
/********************************************************************/ 
/*
 * gexit
 *
 *	exit the vogl/vogle system
 *
 */
void gexit()
{
  if (!vdevice.initialised)
    verror("gexit: vogl not initialised");

  (*vdevice.dev.Vexit)();

  vdevice.devname = (char *)NULL;
  vdevice.initialised = 0;
  voutfp = stdout;
}
/********************************************************************/ 
/*
 * getdevice
 *
 *	get the appropriate device table structure
 * O well, only the X11 and the PS drivers are relevent.
 */
void getdevice(device)
     char *device;
{
  char	buf[100];
  if (strncmp(device, "GRX", 3) == 0)
    _GRX_devcpy();
  else
    if (strncmp(device, "postscript", 10) == 0) 
      {
	_PS_devcpy();
      } 
    else
      {
	sprintf(buf, "vogl: expected the enviroment variable VDEVICE to be set to the desired device.\n");
	fputs(buf, stderr);
	fprintf(stderr, "The devices compiled into this library are:\n");
	fprintf(stderr, "GRX\n");
	fprintf(stderr, "postscript\n");
	exit(1);
      }
}
/********************************************************************/ 
/*
 * vinit
 *
 * 	Just set the device name. ginit and winopen are basically
 * the same as the VOGLE the vinit function.
 *
 */
void vinit(device)
     char *device;
{
  vdevice.devname = device;
}
/********************************************************************/ 
/*
 * winopen
 *
 *	use the more modern winopen call (this really calls ginit),
 * we use the title if we can
 */
long winopen(title)
     char *title;
{
  vdevice.wintitle = title;
  ginit();
  return(1L);
}
/********************************************************************/ 
/*
 * ginit
 *
 *	by default we check the environment variable, if nothing
 * is set we use the value passed to us by the vinit call.
 */
void ginit()
{
  int	i;

  if(vdevice.devname == (char *)NULL) 
    verror("Use vinit first to set the device name, X11 or postscript.");
  
  getdevice(vdevice.devname);

  if(vdevice.initialised)
    gexit();

  if (!allocated) 
    {
      allocated = 1;

      vdevice.transmat = (Mstack *)vallocate(sizeof(Mstack));
      vdevice.transmat->back = (Mstack *)NULL;
      vdevice.attr = (Astack *)vallocate(sizeof(Astack));
      vdevice.attr->back = (Astack *)NULL;
      vdevice.viewport = (Vstack *)vallocate(sizeof(Vstack));
      vdevice.viewport->back = (Vstack *)NULL;
      vdevice.bases = (Matrix *)vallocate(sizeof(Matrix) * 10);
      vdevice.enabled = (char *)vallocate(MAXDEVTABSIZE);
    }

  for (i = 0; i < MAXDEVTABSIZE; i++)
    vdevice.enabled[i] = 0;
  vdevice.alreadyread = FALSE;
  vdevice.data = 0;
  vdevice.devno = 0;
  vdevice.kbdmode = vdevice.mouseevents = vdevice.kbdevents = 0;

  vdevice.concave = 0;
  vdevice.clipoff = 0;
  vdevice.sync = 0;
  vdevice.cpW[V_W] = 1.0;			/* never changes */
  
  vdevice.maxfontnum = 2;

  vdevice.attr->a.lw = 1;
  vdevice.attr->a.fontnum = 0;
  vdevice.attr->a.mode = 0;
  vdevice.attr->a.backface = 0;

  if ((*vdevice.dev.Vinit)()) {
    vdevice.initialised = 1;

    vdevice.inobject = 0;
    vdevice.inpolygon = 0;

    viewport((Screencoord)0, (Screencoord)vdevice.sizeSx - 1,
	     (Screencoord)0, (Screencoord)vdevice.sizeSy - 1);

    ortho2(0.0, (Coord)(vdevice.sizeSx - 1), 0.0, (Coord)(vdevice.sizeSy - 1));

    move(0.0, 0.0, 0.0);

    font(0);	/* set up default font */

  } else {
    fprintf(stderr, "vogl: error while setting up device\n");
    exit(1);
  }
}
/************************************************************************/

/*
 * vnewdev
 *
 * reinitialize vogl to use a new device but don't change any
 * global attributes like the window and viewport settings.
 */
void vnewdev(device)
     char *device;
{
  if (!vdevice.initialised)
    verror("vnewdev: vogl not initialised\n");

  pushviewport();	

  (*vdevice.dev.Vexit)();

  vdevice.initialised = 0;

  getdevice(device);

  (*vdevice.dev.Vinit)();

  vdevice.initialised = 1;

  /*
   * Need to update font for this device...
   */
  font(vdevice.attr->a.fontnum);

  popviewport();
}
/********************************************************************/
/*
 * vgetdev
 *
 *	Returns the name of the current vogl device 
 *	in the buffer buf. Also returns a pointer to
 *	the start of buf.
 */
char *vgetdev(buf)
     char *buf;
{
  /*
   * Note no exit if not initialized here - so that gexit
   * can be called before printing the name.
   */
  if (vdevice.dev.devname)
    strcpy(buf, vdevice.dev.devname);
  else
    strcpy(buf, "(no device)");

  return(&buf[0]);
}

/*
 * getvaluator
 *
 *	similar to the VOGLE locator only it returns either x (MOUSEX) or y (MOUSEY).
 */
long getvaluator(dev)
     Device dev;
{
  int a, b, c;

  c = (*vdevice.dev.Vlocator)(&a, &b);

  if (c != -1) {
    if (dev == MOUSEX)
      return((long)a);
    else 
      return((long)b);
  }
  return(-1);
}

/*
 * getbutton
 *
 *	returns the up (or down) state of a button. 1 means down, 0 up,
 * -1 invalid.
 */
Boolean getbutton(dev)
     Device dev;
{
  int a, b, c;

  if (dev < 256) 
    {
      c = (*vdevice.dev.Vcheckkey)();
      if (c >= 'a' && c <= 'z')
	c = c - 'a' + 'A';
      if (c == dev)
	return(1);
      return(0);
    }
  else if (dev < 261) 
    {
      c = (*vdevice.dev.Vlocator)(&a, &b);
      if (c & 0x01 && dev == MOUSE3)
	return(1);
      if (c & 0x02 && dev == MOUSE2)
	return(1);
      if (c & 0x04 && dev == MOUSE1)
	return(1);
      return(0);
    }
  return(-1);
}

/*
 * Get the values of the valuators in devs and put them into vals
 */
void getdev(n, devs, vals)
     long n;
     Device devs[];
     short vals[];
{
  int	i;

  for( i=0; i < n; i++)
    vals[i] = (short)getvaluator(devs[i]);
}


/*
 * clear
 *
 *	clears the screen to the current colour, excepting devices
 * like a laser printer where it flushes the page.
 *
 */
void clear()
{
  (*vdevice.dev.Vclear)();
}

/*
 * colorf
 *
 *	set the current colour to colour index given by
 * the rounded value of f.
 *
 */
void colorf(f)
     float	f;
{
  color((int)(f + 0.5));
}

/*
 * color
 *
 *	set the current colour to colour index number i.
 *
 */
void color(i)
     int i;
{
  vdevice.attr->a.color = i; 
  (*vdevice.dev.Vcolor)(i);
}

long getcolor()
{
  return((long)vdevice.attr->a.color);
}

/*
 * mapcolor
 *
 *	set the color of index i.
 */
void
mapcolor(i, r, g, b)
     Colorindex	i;
     short r, g, b;
{
  (*vdevice.dev.Vmapcolor)(i, r, g, b);
}

/*
 * getplanes
 *
 *	Returns the number if bit planes on a device.
 */
long getplanes()
{
  return((long)vdevice.depth);
}

/*
 * reshapeviewport
 *
 *	Simply sets the viewport to the size of the current window
 */
void reshapeviewport()
{
  viewport(0, vdevice.sizeSx - 1, 0, vdevice.sizeSy - 1);
}

/*
 * winconstraints
 *		- does nothing
 */
void winconstraints()
{
}

/*
 * keepaspect
 *		- does nothing
 */
void keepaspect()
{
}

/*
 * shademodel
 *		- does nothing
 */
void shademodel(model)
     long model;
{
}

/*
 * getgdesc
 *
 *	Inquire about some stuff....
 */
long getgdesc(inq)
     long inq;
{	
  /*
   * How can we know before the device is inited??
   */

  switch (inq) {
  case GD_XPMAX:
    if (vdevice.initialised)
      return((long)vdevice.sizeSx);
    else
      return(500L);	/* A bullshit number */
  case GD_YPMAX:
    if (vdevice.initialised)
      return((long)vdevice.sizeSy);
    else
      return(500L);
  default:
    return(-1L);
  }
}

/*
 * foregound
 * 		Dummy - does nothing.
 */
void
foreground()
{
}

/*
 * vsetflush
 *
 * Controls flushing of the display - we can get considerable
 * Speed up's under X11 using this...
 */
void vsetflush(yn)
     int yn;
{
  vdevice.sync = yn;
}

/*
 * vflush
 *
 * Explicitly call the device flushing routine...
 * This is enabled for object so that you can force an update
 * in the middle of an object, as objects have flushing off
 * while they are drawn anyway.
 */
void vflush()
{
  (*vdevice.dev.Vsync)();
}


/* 
 * getorigin
 *
 *	Returns the origin of the window. This is a dummy.
 */
void getorigin(x, y)
     long *x, *y;
{
  *x = *y = 0;
}

/*
 * getsize
 *
 *	Returns the approximate size of the window (some window managers
 *	stuff around with your borders).
 */
void getsize(x, y)
     long *x, *y;
{
  *x = (long)vdevice.sizeSx;
  *y = (long)vdevice.sizeSy;
}

void winattach()
{}

void winset(i)
     int i;
{
  (*vdevice.dev.Vwinset)(i);
}
