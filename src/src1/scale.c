#include "vogl.h"

/*
 * scale
 * 
 * Set up a scale matrix and premultiply it and 
 * the top matrix on the stack.
 *
 */
void scale(x, y, z)
     float 	x, y, z;
{
  /*
   * Do the operations directly on the top matrix of
   * the stack to speed things up.
   */

  vdevice.transmat->m[0][0] *= x;
  vdevice.transmat->m[0][1] *= x;
  vdevice.transmat->m[0][2] *= x;
  vdevice.transmat->m[0][3] *= x;

  vdevice.transmat->m[1][0] *= y;
  vdevice.transmat->m[1][1] *= y;
  vdevice.transmat->m[1][2] *= y;
  vdevice.transmat->m[1][3] *= y;

  vdevice.transmat->m[2][0] *= z;
  vdevice.transmat->m[2][1] *= z;
  vdevice.transmat->m[2][2] *= z;
  vdevice.transmat->m[2][3] *= z;
}
