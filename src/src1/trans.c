#include <math.h>
#include "vogl.h"


/*
 * translate
 * 
 * Set up a translation matrix and premultiply it and 
 * the top matrix on the stack.
 *
 */
void translate(x, y, z)
     float 	x, y, z;
{
  /*
   * Do the operations directly on the top matrix of
   * the stack to speed things up.
   */
  vdevice.transmat->m[3][0] += x * vdevice.transmat->m[0][0]
    + y * vdevice.transmat->m[1][0]
      + z * vdevice.transmat->m[2][0];

  vdevice.transmat->m[3][1] += x * vdevice.transmat->m[0][1]
    + y * vdevice.transmat->m[1][1]
      + z * vdevice.transmat->m[2][1];

  vdevice.transmat->m[3][2] += x * vdevice.transmat->m[0][2]
    + y * vdevice.transmat->m[1][2]
      + z * vdevice.transmat->m[2][2];

  vdevice.transmat->m[3][3] += x * vdevice.transmat->m[0][3]
    + y * vdevice.transmat->m[1][3]
      + z * vdevice.transmat->m[2][3];
}

/*
 * rot
 * 
 * Set up a rotate matrix and premultiply it with 
 * the top matrix on the stack.
 *
 */
void rot(r, axis)
	float	r; /* in degrees */
	char	axis;
{
  float	costheta, sintheta, tmp;
  
  /*
   * Do the operations directly on the top matrix of
   * the stack to speed things up.
   */
/*  mysincos( (int)(r*10.0), &sintheta, &costheta);*/
  costheta = cos((double)(D2R * r));
  sintheta = sin((double)(D2R * r));

  switch(axis) {
  case 'x':
  case 'X':
    tmp = vdevice.transmat->m[1][0];
    vdevice.transmat->m[1][0] = costheta * tmp
      + sintheta * vdevice.transmat->m[2][0];
    vdevice.transmat->m[2][0] = costheta * vdevice.transmat->m[2][0]
      - sintheta * tmp;

    tmp = vdevice.transmat->m[1][1];
    vdevice.transmat->m[1][1] = costheta * tmp
      + sintheta * vdevice.transmat->m[2][1];
    vdevice.transmat->m[2][1] = costheta * vdevice.transmat->m[2][1]
      - sintheta * tmp;
    tmp = vdevice.transmat->m[1][2];
    vdevice.transmat->m[1][2] = costheta * tmp
      + sintheta * vdevice.transmat->m[2][2];
    vdevice.transmat->m[2][2] = costheta * vdevice.transmat->m[2][2]
      - sintheta * tmp;

    tmp = vdevice.transmat->m[1][3];
    vdevice.transmat->m[1][3] = costheta * tmp
      + sintheta * vdevice.transmat->m[2][3];
    vdevice.transmat->m[2][3] = costheta * vdevice.transmat->m[2][3]
      - sintheta * tmp;
    break;
  case 'y':
  case 'Y':
    tmp = vdevice.transmat->m[0][0];
    vdevice.transmat->m[0][0] = costheta * tmp
      - sintheta * vdevice.transmat->m[2][0];
    vdevice.transmat->m[2][0] = sintheta * tmp
      + costheta * vdevice.transmat->m[2][0];
    tmp = vdevice.transmat->m[0][1];
    vdevice.transmat->m[0][1] = costheta * tmp
      - sintheta * vdevice.transmat->m[2][1];
    vdevice.transmat->m[2][1] = sintheta * tmp
      + costheta * vdevice.transmat->m[2][1];
    tmp = vdevice.transmat->m[0][2];
    vdevice.transmat->m[0][2] = costheta * tmp
      - sintheta * vdevice.transmat->m[2][2];
    vdevice.transmat->m[2][2] = sintheta * tmp
      + costheta * vdevice.transmat->m[2][2];
    tmp = vdevice.transmat->m[0][3];
    vdevice.transmat->m[0][3] = costheta * tmp
      - sintheta * vdevice.transmat->m[2][3];
    vdevice.transmat->m[2][3] = sintheta * tmp
      + costheta * vdevice.transmat->m[2][3];
    break;
  case 'z':
  case 'Z':
    tmp = vdevice.transmat->m[0][0];
    vdevice.transmat->m[0][0] = costheta * tmp
      + sintheta * vdevice.transmat->m[1][0];
    vdevice.transmat->m[1][0] = costheta * vdevice.transmat->m[1][0]
      - sintheta * tmp;
    
    tmp = vdevice.transmat->m[0][1];
    vdevice.transmat->m[0][1] = costheta * tmp
      + sintheta * vdevice.transmat->m[1][1];
    vdevice.transmat->m[1][1] = costheta * vdevice.transmat->m[1][1]
      - sintheta * tmp;
    
    tmp = vdevice.transmat->m[0][2];
    vdevice.transmat->m[0][2] = costheta * tmp
      + sintheta * vdevice.transmat->m[1][2];
    vdevice.transmat->m[1][2] = costheta * vdevice.transmat->m[1][2]
      - sintheta * tmp;
    
    tmp = vdevice.transmat->m[0][3];
    vdevice.transmat->m[0][3] = costheta * tmp
      + sintheta * vdevice.transmat->m[1][3];
    vdevice.transmat->m[1][3] = costheta * vdevice.transmat->m[1][3]
      - sintheta * tmp;
    break;
  default:
    verror("rot: illegal axis of rotation");
  }
}

/*
 * rotate
 * 
 * Set up an old style, I've got this real fast way of doing
 * it providing I use ints, rotate.
 *
 */
void rotate(r, axis)
     Angle	r;
     char	axis;
{
  rot((float)r * 0.1, axis);
}
