#include <stdio.h>
#include "vogl.h"
extern int quickclip(), clip();

/*
 * draw
 *
 * draw a line form the logical graphics position to the
 * the world coordinates x, y, z.
 *
 */
void draw(x, y, z)
     float x, y, z;
{
  Vector res;

  if (!vdevice.cpVvalid)
    multvector(vdevice.cpWtrans, vdevice.cpW, vdevice.transmat->m);

  vdevice.cpW[V_X] = x;
  vdevice.cpW[V_Y] = y;
  vdevice.cpW[V_Z] = z;
  multvector(res, vdevice.cpW, vdevice.transmat->m);

  if (vdevice.cpVvalid) quickclip(vdevice.cpWtrans, res);
  else clip(vdevice.cpWtrans, res);
  
  vdevice.cpWtrans[V_X] = res[V_X];
  vdevice.cpWtrans[V_Y] = res[V_Y];
  vdevice.cpWtrans[V_Z] = res[V_Z];
  vdevice.cpWtrans[V_W] = res[V_W];
}
/**********************************************************************/
extern float *vts; /* the vertices */
extern int current_color_index;
void my_line(f)
     float **f;
{
  float *flt;
  Vector res;

  color(current_color_index);
  flt = *f;
  vdevice.cpW[V_X] = *flt++;
  vdevice.cpW[V_Y] = *flt++;
  vdevice.cpW[V_Z] = *flt;

  multvector(vdevice.cpWtrans, vdevice.cpW, vdevice.transmat->m);
  flt = *(f+1);
  vdevice.cpW[V_X] = *flt++;
  vdevice.cpW[V_Y] = *flt++;
  vdevice.cpW[V_Z] = *flt;
  multvector(res, vdevice.cpW, vdevice.transmat->m);

  clip(vdevice.cpWtrans, res);

  vdevice.cpWtrans[V_X] = res[V_X];
  vdevice.cpWtrans[V_Y] = res[V_Y];
  vdevice.cpWtrans[V_Z] = res[V_Z];
  vdevice.cpWtrans[V_W] = res[V_W];
}
/******************************************************************/
extern void ptclip();

void my_point(f)
     float **f;
{
  float *flt;
  Vector res;

  color(current_color_index);
  flt = *f;
  vdevice.cpW[V_X] = *flt++;
  vdevice.cpW[V_Y] = *flt++;
  vdevice.cpW[V_Z] = *flt;

  multvector(vdevice.cpWtrans, vdevice.cpW, vdevice.transmat->m);
  copyvector(res,vdevice.cpWtrans);

  ptclip(vdevice.cpWtrans, res);

  vdevice.cpWtrans[V_X] = res[V_X];
  vdevice.cpWtrans[V_Y] = res[V_Y];
  vdevice.cpWtrans[V_Z] = res[V_Z];
  vdevice.cpWtrans[V_W] = res[V_W];
}
/******************************************************************/
void drawp(x)
     float *x;
{
  Vector res;

  if (!vdevice.cpVvalid)
    multvector(vdevice.cpWtrans, vdevice.cpW, vdevice.transmat->m);

  vdevice.cpW[V_X] = *x++;
  vdevice.cpW[V_Y] = *x++;
  vdevice.cpW[V_Z] = *x;
  multvector(res, vdevice.cpW, vdevice.transmat->m);

  if (vdevice.cpVvalid)  quickclip(vdevice.cpWtrans, res);
  else  clip(vdevice.cpWtrans, res);
  
  vdevice.cpWtrans[V_X] = res[V_X];
  vdevice.cpWtrans[V_Y] = res[V_Y];
  vdevice.cpWtrans[V_Z] = res[V_Z];
  vdevice.cpWtrans[V_W] = res[V_W];
}
/*********************************************************************/
void pnta(x, y, z)
     float x, y, z;
{
  Vector res;

  if (!vdevice.cpVvalid)
    multvector(vdevice.cpWtrans, vdevice.cpW, vdevice.transmat->m);

  vdevice.cpW[V_X] = x;
  vdevice.cpW[V_Y] = y;
  vdevice.cpW[V_Z] = z;
  multvector(res, vdevice.cpW, vdevice.transmat->m);

  if(vdevice.cpVvalid) quickclip(vdevice.cpWtrans, res);
  else  clip(vdevice.cpWtrans, res);

  vdevice.cpWtrans[V_X] = res[V_X];
  vdevice.cpWtrans[V_Y] = res[V_Y];
  vdevice.cpWtrans[V_Z] = res[V_Z];
  vdevice.cpWtrans[V_W] = res[V_W];
}
/*******************************************************************/
/*
 * draws
 *
 * draw a line form the logical graphics position to the
 * the world coordinates x, y, z expressed as a short integer data type.
 *
 */
void draws(x, y, z)
     Scoord x, y, z;
{
  draw((Coord)x, (Coord)y, (Coord)z);
}


/*
 * drawi
 *
 * draw a line form the logical graphics position to the
 * the world coordinates x, y, z expressed as an integer data type.
 *
 */
void drawi(x, y, z)
     Icoord x, y, z;
{
  draw((Coord)x, (Coord)y, (Coord)z);
}



/*
 * draw2
 *
 * draw a line from the logical graphics position  to the
 * the world coordinates x, y.
 *
 */
void draw2(x, y)
     float x, y;
{
  draw(x, y, 0.0);
}

/*
 * draw2s
 *
 * draw a line from the logical graphics position  to the
 * the world coordinates x, y expressed as a short integer data type.
 *
 */
void draw2s(x, y)
     Scoord x, y;
{
  draw2((Coord)x, (Coord)y);
}


/*
 * draw2i
 *
 * draw a line from the logical graphics position  to the
 * the world coordinates x, y expressed as an integer data type.
 *
 */
void draw2i(x, y)
     Icoord x, y;
{
  draw2((Coord)x, (Coord)y);
}

/*
 * rdr
 *
 * 3D relative draw from the logical graphics position by dx, dy, dz.
 * This is the same as the VOGLE routine rdraw.
 *
 */
void rdr(dx, dy, dz)
	float dx, dy, dz;
{
  draw((vdevice.cpW[V_X]+dx), (vdevice.cpW[V_Y]+dy), (vdevice.cpW[V_Z]+dz));
}

/*
 * rdrs
 *
 * 3D relative draw from the logical graphics position by dx, dy, dz
 * expressed as a short integer data type.
 *
 */
void rdrs(dx, dy, dz)
     Scoord dx, dy, dz;
{
  rdr((Coord)dx, (Coord)dy, (Coord)dz);
}

/*
 * rdri
 *
 * 3D relative draw from the logical graphics position by dx, dy, dz
 * expressed as an integer data type.
 *
 */
void rdri(dx, dy, dz)
     Icoord dx, dy, dz;
{
  rdr((Coord)dx, (Coord)dy, (Coord)dz);
}


/*
 * rdr2
 *
 * 2D relative draw from the logical graphics position by dx, dy.
 * This is the same as the VOGLE routine rdraw2.
 *
 */
void rdr2(dx, dy)
     float dx, dy;
{
  draw((vdevice.cpW[V_X] + dx), (vdevice.cpW[V_Y] + dy), 0.0);
}


/*
 * rdr2s
 *
 * 3D relative draw from the logical graphics position by dx, dy
 * expressed as a short integer data type.
 *
 */
void rdr2s(dx, dy)
     Scoord dx, dy;
{
  rdr2((Coord)dx, (Coord)dy);
}

/*
 * rdr2i
 *
 * 3D relative draw from the logical graphics position by dx, dy
 * expressed as an integer data type.
 *
 */
void rdr2i(dx, dy)
     Icoord dx, dy;
{
  rdr2((Coord)dx, (Coord)dy);
}


/*
 * bgnline
 *
 * 	Flags that all v*() routine points will be building up a line list.
 */
void bgnline()
{
  vdevice.bgnmode = VLINE;
  vdevice.save = 1;
  /*  
   sync = vdevice.sync;
    vdevice.sync = 0;
    */
}

/*
 * endline
 *
 * 	Flags that all v*() routine points will be simply setting the 
 * 	current position.
 */
void endline()
{
  vdevice.bgnmode = NONE;
  vdevice.save = 0;
  /*
     if (sync) {
     vdevice.sync = 1;
     (*vdevice.dev.Vsync)();
     }
     */
}

/*
 * bgnclosedline
 *
 * 	Flags that all v*() routine points will be building up a line list.
 */
void bgnclosedline()
{
  vdevice.bgnmode = VCLINE;
  vdevice.save = 1;
  /*
     sync = vdevice.sync;
     vdevice.sync = 0;
     */
}

/*
 * endclosedline
 *
 * 	Flags that all v*() routine points will be simply setting the 
 * 	current position.
 */
void endclosedline()
{
  vdevice.bgnmode = NONE;
  /*
     pvdevice.sync = sync;
     draw(vdevice.savex, vdevice.savey, vdevice.savez);
     */
}


