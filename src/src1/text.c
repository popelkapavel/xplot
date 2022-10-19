#include <stdio.h>
/* #include <strings.h>*/
#include "vogl.h"

extern int   strlen();
extern char *strncpy();
static	Vector	cpos;	/* Current character position */
static	int	 sc_x, sc_y;

/*
 * font
 * 	assigns a font.
 */
void font(id)
     short	id;
{
  if (id < 0 || id >= vdevice.maxfontnum)
    verror("font: font number is out of range");
	
  if (id == 1) {
    if (!(*vdevice.dev.Vfont)(vdevice.dev.large)) 
      verror("font: unable to open large font");
  } else if (id == 0) {
    if (!(*vdevice.dev.Vfont)(vdevice.dev.small))
      verror("font: unable to open small font");
  }
  vdevice.attr->a.fontnum = id;
}

/*
 * charstr
 *
 * Draw a string from the current character position.
 *
 */
void charstr(str)
     char 	*str;
{
  int	cx, cy;
  char	c;
  int	sl = strlen(str);
  cx = vdevice.cpVx;
  cy = vdevice.cpVy;

  vdevice.cpVx = sc_x;
  vdevice.cpVy = sc_y;

  { /* Check if string is within viewport */
    /* Could clip in Z maybe? */
    int	left_s = vdevice.cpVx;
    int	bottom_s = vdevice.cpVy - (int)vdevice.hheight;
    int	top_s = bottom_s + (int)vdevice.hheight;
    int	right_s = left_s + (int)((sl + 1) * vdevice.hwidth);

    if (left_s > vdevice.minVx &&
	bottom_s < vdevice.maxVy &&
	top_s > vdevice.minVy &&
	right_s < vdevice.maxVx) {
      (*vdevice.dev.Vstring)(str);
    } else {
      while(c = *str++) {
	if (vdevice.cpVx > vdevice.minVx &&
	    vdevice.cpVx < vdevice.maxVx - (int)vdevice.hwidth) {
	  (*vdevice.dev.Vchar)(c);
	}
	vdevice.cpVx += (int)vdevice.hwidth;
      }
    }
  }
  
  sc_x += sl * (int)vdevice.hwidth;

  /* Put our original device graphics position back... */
  vdevice.cpVx = cx;
  vdevice.cpVy = cy;
}

/*
 * cmov
 *
 *	Sets the current character position.
 */
void cmov(x, y, z)
     float	x, y, z;
{
  Vector	res;
  cpos[V_X] = x;
  cpos[V_Y] = y;
  cpos[V_Z] = z;
  cpos[V_W] = 1.0;

  multvector(res, cpos, vdevice.transmat->m);

  sc_x = WtoVx(res);
  sc_y = WtoVy(res);
}

 
/*
 * cmov2
 *
 *	Sets the current character position. Ignores the Z coord.
 *	
 *
 */
void cmov2(x, y)
     float	x, y;
{
  cmov(x, y, 0.0);
}

/*
 * cmovi
 *
 *	Same as cmov but with integer arguments....
 */
void cmovi(x, y, z)
     Icoord	x, y, z;
{
  cmov((Coord)x, (Coord)y, (Coord)z);
}

/*
 * cmovs
 *
 *	Same as cmov but with short integer arguments....
 */
void cmovs(x, y, z)
     Scoord	x, y, z;
{
  cmov((Coord)x, (Coord)y, (Coord)z);
}

/*
 * cmov2i
 *
 *	Same as cmov2 but with integer arguments....
 */
void cmov2i(x, y)
     Icoord	x, y;
{
  cmov((Coord)x, (Coord)y, 0.0);
}

/*
 * cmov2s
 *
 *	Same as cmov but with short integer arguments....
 */
void cmov2s(x, y)
     Scoord	x, y;
{
  cmov((Coord)x, (Coord)y, 0.0);
}


/*
 * strwidth
 *
 * Return the length of a string in pixels
 *
 */
long strwidth(s)
     char	*s;
{
#ifdef SUN_CC
	/*
	 * SUN's ANSI CC compiler bitches here sating to use an explicit
	 * cast for strlen... it's only a warning, but this fixes it...
    	 */
	return((long)((size_t)strlen(s) * vdevice.hwidth));
#else
	return((long)(strlen(s) * vdevice.hwidth));
#endif
}

/* 
 * getheight
 *
 * Return the maximum Height of the current font
 */
long  getheight()
{
  return((long)vdevice.hheight);
}

/*
 * getcpos
 *
 * Return the current character position in screen coords 
 */
void getcpos(cx, cy)
     Scoord	*cx, *cy;
{
  *cx = sc_x;
  *cy = sc_y;
}
/************************************************************/ 
extern void myNW2V();
extern float titlepos[2];
void draw_title_string(title)
     char *title;
{
  int sl, left_s, right_s, top_s, bottom_s;
  int  titlexy[2], htwidth;
  int old_vcpx = vdevice.cpVx;
  int old_vcpy = vdevice.cpVy;
  char *s = title;

  sl = strlen(s) +1;
  htwidth = (int) ( sl * vdevice.hwidth * 0.5);
  myNW2V(titlepos, titlexy);
  titlexy[0]  -=  htwidth;
  vdevice.cpVx = titlexy[0];  vdevice.cpVy = titlexy[1];  

  left_s = titlexy[0];  right_s = left_s + 2 * htwidth + (int) vdevice.hwidth;
  top_s = titlexy[1];   bottom_s = titlexy[1] - (int)vdevice.hheight;

  if (left_s > vdevice.minVx &&  bottom_s < vdevice.maxVy &&
      top_s > vdevice.minVy &&  right_s < vdevice.maxVx) 
    (*vdevice.dev.Vstring)(s);
  else /* clipping, may off by a char_height */
    {
      char tmp_string[256]; int len;
      if(left_s < vdevice.minVx)
	{
	  len = (int)(  (vdevice.minVx - left_s)/vdevice.hwidth );
	  s += len;   sl -= len; if(sl < 1) sl = 1;
	  left_s = vdevice.minVx;  vdevice.cpVx = vdevice.minVx;
	}
      if(right_s > vdevice.maxVx)
	{
	  len = (int) ((right_s - vdevice.maxVx)/vdevice.hwidth);
	  sl -= len; if(sl < 1) sl = 1;
	}
      (void) strncpy(tmp_string, s, sl); tmp_string[sl-1]='\0';
      (*vdevice.dev.Vstring)(tmp_string); 
    }
  vdevice.cpVx = old_vcpx;
  vdevice.cpVy = old_vcpy;
}
/******************************************************************/
