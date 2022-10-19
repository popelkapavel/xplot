#include "vogl.h"
/*
 * move
 *
 * Move the logical graphics position to the world coordinates x, y, z.
 *
 */
void move(x, y, z)
     Coord x, y, z;
{
  vdevice.cpW[V_X] = x;
  vdevice.cpW[V_Y] = y;
  vdevice.cpW[V_Z] = z;
  
  vdevice.cpVvalid = 0;

  if (vdevice.clipoff) {		/* update device coords as well */
    multvector(vdevice.cpWtrans, vdevice.cpW, vdevice.transmat->m);
    vdevice.cpVx = WtoVx(vdevice.cpWtrans);
    vdevice.cpVy = WtoVy(vdevice.cpWtrans);
  }
}
/****************************************************************/
void movep(x)
     Coord *x;
{
  vdevice.cpW[V_X] = *x++;
  vdevice.cpW[V_Y] = *x++;
  vdevice.cpW[V_Z] = *x;
  vdevice.cpVvalid = 0;
}
/****************************************************************/
/*
 * moves
 *
 * Move the logical graphics position to the world coordinates x, y, z.
 * expressed as a short integer data type.
 *
 */
void moves(x, y, z)
     Scoord 	x, y, z;
{
  move((Coord)x, (Coord)y, (Coord)z);
}


/*
 * movei
 *
 * Move the logical graphics position to the world coordinates x, y, z.
 * expressed as an integer data type.
 *
 */
void movei(x, y, z)
     Icoord 	x, y, z;
{
  move((Coord)x, (Coord)y, (Coord)z);
}


/*
 * move2
 *
 * Move the logical graphics position to the world coords x, y, 0.0
 * (I.e. a 2D move is defined as a 3D move with the Z-coord set to zero)
 *
 */
void move2(x, y)
     Coord	x, y;
{
  move(x, y, 0.0);
}

/*
 * move2s
 *
 * Move the logical graphics position to the world coordinates x, y.
 * expressed as a short integer data type.
 *
 */
void move2s(x, y)
     Scoord 	x, y;
{
  move((Coord)x, (Coord)y, 0.0);
}


/*
 * move2i
 *
 * Move the logical graphics position to the world coordinates x, y.
 * expressed as an integer data type.
 *
 */
void move2i(x, y)
     Icoord 	x, y;
{
  move((Coord)x, (Coord)y, 0.0);
}


/*
 * rmv
 *
 * move the logical graphics position from the current world 
 * coordinates by dx, dy, dz 
 *
 */
void rmv(dx, dy, dz)
     Coord dx, dy, dz;
{
  move((vdevice.cpW[V_X]+dx), (vdevice.cpW[V_Y]+dy), (vdevice.cpW[V_Z]+dz));
}

/*
 * rmvs
 *
 * move the logical graphics position from the current world 
 * coordinates by dx, dy, dz expressed as a short integer data type.
 *
 */
void rmvs(dx, dy, dz)
     Scoord	dx, dy, dz;
{
  rmv((Coord)dx, (Coord)dy, (Coord)dz);
}

/*
 * rmvi
 *
 * move the logical graphics position from the current world 
 * coordinates by dx, dy, dz expressed as an integer data type.
 *
 */
void rmvi(dx, dy, dz)
     Icoord	dx, dy, dz;
{
  rmv((Coord)dx, (Coord)dy, (Coord)dz);
}

/*
 * rmv2
 *
 * Move Relative in 2D.
 *
 */
void rmv2(dx, dy)
     float	dx, dy;
{
  move((vdevice.cpW[V_X] + dx), (vdevice.cpW[V_Y] + dy), 0.0);
}

/*
 * rmv2s
 *
 * move the logical graphics position from the current world 
 * coordinates by dx, dy expressed as a short integer data type.
 *
 */
void rmv2s(dx, dy)
	Scoord	dx, dy;
{
  move((vdevice.cpW[V_X] + (Coord)dx), (vdevice.cpW[V_Y] + (Coord)dy), 0.0);
}

/*
 * rmv2i
 *
 * move the logical graphics position from the current world 
 * coordinates by dx, dy expressed as an integer data type.
 *
 */
void rmv2i(dx, dy)
     Icoord dx, dy;
{
  move((vdevice.cpW[V_X] + (Coord)dx), (vdevice.cpW[V_Y] + (Coord)dy), 0.0);
}

