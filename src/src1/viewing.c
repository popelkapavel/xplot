
#define PMI
#include <math.h>
#include "vogl.h"

#define	SQ(a)	((a)*(a))
#define COT(a)	((float)(cos((double)(a)) / sin((double)(a))))

static Matrix pmi; /* the inverse of the projection matrix */
static Matrix viewa, viewb;

/*
 * polarview
 *
 * Specify the viewer's position in polar coordinates by giving
 * the distance from the viewpoint to the world origin, 
 * the azimuthal angle in the x-y plane, measured from the y-axis,
 * the incidence angle in the y-z plane, measured from the z-axis,
 * and the twist angle about the line of sight. Note: that unlike
 * in VOGLE we are back to tenths of degrees.
 *
 */
void polarview(dist, azim, inc, twist)
     Coord	dist;
     Angle	azim, inc, twist;
{
  translate(0.0, 0.0, -dist);
  rotate(-twist, 'z');
  rotate(-inc, 'x');
  rotate(-azim, 'z');
}

/*
 * normallookat
 *
 *	do the standard lookat transformation.
 */
static void normallookat(vx, vy, vz, px, py, pz)
     float  vx, vy, vz, px, py, pz;

{
  float	l2, l3, sintheta, sinphi, costheta, cosphi;

  l2 = sqrt((double)(SQ((px - vx)) + SQ((pz - vz))));
  l3 = sqrt((double)(SQ((px - vx)) + SQ((py - vy)) + SQ((pz - vz))));

  if (l3 != 0.0) {
    sinphi = (vy - py) / l3;
    cosphi = l2 / l3;

    /*
     * Rotate about X by phi
     */
    identmatrix(viewa);
    viewa[1][1] = viewa[2][2] = cosphi;
    viewa[1][2] = sinphi;
    viewa[2][1] = -sinphi;
    multmatrix(viewa);
    viewa[1][2] = -sinphi;
    viewa[2][1] = sinphi;    
  }

  if (l2 != 0.0) {
    sintheta = (px - vx) / l2;
    costheta = (vz - pz) / l2;

    /*
     * Rotate about Y by theta
     */
    identmatrix(viewb);
    viewb[0][0] = viewb[2][2] = costheta;
    viewb[0][2] = -sintheta;
    viewb[2][0] = sintheta;
    multmatrix(viewb);
    viewb[0][2] = sintheta;
    viewb[2][0] = -sintheta;
  }
}
/*
 * lookat
 *
 * Specify the viewer's position by giving a viewpoint and a 
 * reference point in world coordinates. A twist about the line
 * of sight may also be given.  Note that unlike in VOGLE we are
 * back to tenths of degrees.
 */

void lookat(vx, vy, vz, px, py, pz, twist)
     Coord  vx, vy, vz, px, py, pz;
     Angle	twist;
{
  rotate(-twist, 'z');
  normallookat(vx, vy, vz, px, py, pz);
  translate(-vx, -vy, -vz);
}
/*******************************************************************
 *
 *  Reverse the transformation produced by lookat
 *
 */
void reverselookat(vx, vy, vz, px, py, pz, twist)
     Coord  vx, vy, vz, px, py, pz;
     Angle  twist;
{
  translate(vx, vy, vz);
  multmatrix(viewb);  /* vx,vy,vz,px,py,pz must be really different */
  multmatrix(viewa);
  rotate(twist, 'z');
}
/*******************************************************************/

static void boxnormallookat(vx, vy, vz, px, py, pz)
     float  vx, vy, vz, px, py, pz;

{
  float	l2, l3, sintheta, sinphi, costheta, cosphi;
  Matrix tmp;

  l2 = sqrt((double)(SQ((px - vx)) + SQ((pz - vz))));
  l3 = sqrt((double)(SQ((px - vx)) + SQ((py - vy)) + SQ((pz - vz))));

  if (l3 != 0.0) {
    sinphi = (vy - py) / l3;
    cosphi = l2 / l3;

    /*
     * Rotate about X by phi
     */
    identmatrix(tmp);
    tmp[1][1] = tmp[2][2] = cosphi;
    tmp[1][2] = sinphi;
    tmp[2][1] = -sinphi;
    multmatrix(tmp);
  }

  if (l2 != 0.0) {
    sintheta = (px - vx) / l2;
    costheta = (vz - pz) / l2;

    /*
     * Rotate about Y by theta
     */
    identmatrix(tmp);
    tmp[0][0] = tmp[2][2] = costheta;
    tmp[0][2] = -sintheta;
    tmp[2][0] = sintheta;
    multmatrix(tmp);
  }
}

void boxlookat(vx, vy, vz, px, py, pz, twist)
     Coord  vx, vy, vz, px, py, pz;
     Angle	twist;
{
  rotate(-twist, 'z');
  boxnormallookat(vx, vy, vz, px, py, pz);
  translate(-vx, -vy, -vz);
}

/*
 * perspective
 *
 * Specify a perspective viewing pyramid in world coordinates by
 * giving a field of view, aspect ratio, and the locations of the 
 * near(hither) and far(yon) clipping planes in the z direction. Note
 * that unlike in VOGLE we are back to tenths of degrees.
 */
void perspective(ifov, aspect, hither, yon)
     Angle 	ifov;
     float	aspect;
     Coord	hither, yon;
{
  Matrix		mat;
  float		fov;
/*
  if (aspect == 0.0)
    verror("perspective: can't have zero aspect ratio!");

  if ((yon - hither) == 0.0)
    verror("perspective: near clipping plane same as far one.");

  if (ifov == 0 || ifov == 1800)
    verror("perspective: bad field of view passed.");
*/
  fov = ifov * 0.1;
  identmatrix(mat);
  identmatrix(pmi);

  mat[1][1] = COT((D2R * fov / 2.0));  pmi[1][1]=1.0/mat[1][1];
  mat[0][0] = mat[1][1] / aspect;      pmi[0][0] = 1.0/mat[0][0];
  mat[2][2] = -(yon + hither) / (yon - hither);
  mat[2][3] = -1;
  mat[3][2] = -2.0 * yon * hither / (yon - hither);
  mat[3][3] = 0;
  /*  find pmi */
  fov= 1.0/mat[3][2];
  pmi[2][3]=fov; pmi[3][3]=mat[2][2]*fov;
  pmi[2][2]=0.0; pmi[3][2]=1.0; 
  /* end of pmi */

  loadmatrix(mat);
}

/*
 * window
 *
 * Specify a perspective viewing pyramid in world coordinates by
 * giving a rectangle at the near clipping plane and the location
 * of the far clipping plane.
 *
 */
void window(left, right, bottom, top, hither, yon)
     Coord 	left, right, bottom, top, hither, yon;
{
  Matrix mat;

  if ((right - left) == 0.0)
    verror("window: left clipping plane same as right one.");

  if ((top - bottom) == 0.0)
    verror("window: bottom clipping plane same as top one.");
  
  if ((yon - hither) == 0.0)
    verror("window: near clipping plane same as far one.");

  identmatrix(mat);
  identmatrix(pmi);

  mat[0][0] = 2.0 * hither / (right - left);
  mat[1][1] = 2.0 * hither / (top - bottom);
  mat[2][0] = (right + left) / (right - left);
  mat[2][1] = (top + bottom) / (top - bottom);
  mat[2][2] = -(yon + hither) / (yon - hither);
  mat[2][3] = -1.0;
  mat[3][2] = -2.0 * yon * hither / (yon - hither);
  mat[3][3] = 0.0;

	/* pmi  */
  pmi[0][0] = 1.0/mat[0][0];
  pmi[1][1] = 1.0/mat[1][1];
  pmi[2][2] = 0.0;
  pmi[3][0] = mat[2][0]/mat[0][0];
  pmi[3][1] = mat[2][1]/mat[1][1];
  pmi[3][2]=-1.0; pmi[3][3]=mat[2][2]/mat[3][2];
  pmi[2][3]=1.0/mat[3][2];
  /* end pmi */
  loadmatrix(mat);
}

/*
 * ortho
 *
 * Define a three dimensional viewing box by giving the left,
 * right, bottom and top clipping plane locations and the distances
 * along the line of sight to the near and far clipping planes.
 *
 */
void ortho(left, right, bottom, top,hither, yon)
     Coord  left, right, bottom, top, hither, yon;
{
  Matrix mat;
/*
  if ((right - left) == 0.0)
    verror("ortho: left clipping plane same as right one.");
  
  if ((top - bottom) == 0.0)
    verror("ortho: bottom clipping plane same as top one.");

  if ((yon - hither) == 0.0)
    verror("ortho: near clipping plane same as far one.");
*/
  identmatrix(mat);
  identmatrix(pmi);

  mat[0][0] = 2.0 / (right - left);
  mat[1][1] = 2.0 / (top - bottom);
  mat[2][2] = -2.0 / (yon - hither);
  mat[3][0] = -(right + left) / (right - left);
  mat[3][1] = -(top + bottom) / (top - bottom);
  mat[3][2] = -(yon + hither) / (yon - hither);
  /* pmi */
  pmi[0][0] = 1.0/mat[0][0];pmi[1][1] = 1.0/mat[1][1];
  pmi[2][2] = 1.0/mat[2][2];
  pmi[3][0] = - mat[3][0] * pmi[0][0];
  pmi[3][1] = - mat[3][1] * pmi[1][1];
  pmi[3][2] = - mat[3][2] * pmi[2][2];
  /* end pmi */

  loadmatrix(mat);
}

void boxortho(left, right, bottom, top,hither, yon)
     Coord  left, right, bottom, top, hither, yon;
{
  Matrix mat;
/*
  if ((right - left) == 0.0)
    verror("ortho: left clipping plane same as right one.");
  
  if ((top - bottom) == 0.0)
    verror("ortho: bottom clipping plane same as top one.");

  if ((yon - hither) == 0.0)
    verror("ortho: near clipping plane same as far one.");
*/
  identmatrix(mat);

  mat[0][0] = 2.0 / (right - left);
  mat[1][1] = 2.0 / (top - bottom);
  mat[2][2] = -2.0 / (yon - hither);
  mat[3][0] = -(right + left) / (right - left);
  mat[3][1] = -(top + bottom) / (top - bottom);
  mat[3][2] = -(yon + hither) / (yon - hither);
  loadmatrix(mat);
}



/*
 * ortho2
 *
 * Specify a two dimensional viewing rectangle. 
 *
 */
void ortho2(left, right, bottom, top)
     Coord	left, right, bottom, top;
{
  Matrix	mat;

  identmatrix(mat);
  identmatrix(pmi);

  if ((right - left) == 0.0)
    verror("ortho2: left clipping plane same as right one.");

  if ((top - bottom) == 0.0)
    verror("ortho2: bottom clipping plane same as top one.");

  mat[0][0] = 2.0 / (right - left);
  mat[1][1] = 2.0 / (top - bottom);
  mat[2][2] = -1.0;
  mat[3][0] = -(right + left) / (right - left);
  mat[3][1] = -(top + bottom) / (top - bottom);
  /* pmi */
  pmi[0][0] = 1.0/mat[0][0]; pmi[1][1] = 1.0/mat[1][1];
  pmi[2][2] = -1.0;
  pmi[3][0] = - mat[3][0]/mat[0][0];
  pmi[3][1] = - mat[3][1]/mat[1][1];
  pmi[3][2] = 0.0;
  /* end pmi */

  loadmatrix(mat);
}
/***********************************************************************/
void getmatrixVM(m,a,b,c)
     Matrix m; float a,b,c;
{
  Matrix tmp;
  getmatrix(tmp);
  mult4x4(m,tmp,pmi);
  m[0][0] *= a; m[1][0] *= a; m[2][0]*=a; m[3][0] *= a;
  m[0][1] *= b; m[1][1] *= b; m[2][1]*=b; m[3][1] *= b;
  m[0][2] *= c; m[1][2] *= c; m[2][2]*=c; m[3][2] *= c;
}
/***********************************************************************/



