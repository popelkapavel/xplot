#include <stdio.h>
#include "vogl.h"

extern char *vallocate();

static	Vstack	*vsfree = (Vstack *)NULL;

/*
 * pushviewport
 *
 * pushes the current viewport on the viewport stack
 *
 */
void pushviewport()
{
  Vstack *nvport;

  if (vsfree != (Vstack *)NULL) {
    nvport = vdevice.viewport;
    vdevice.viewport = vsfree;
    vsfree = vsfree->back;
    vdevice.viewport->back = nvport;
    vdevice.viewport->v.left = nvport->v.left;
    vdevice.viewport->v.right = nvport->v.right;
    vdevice.viewport->v.bottom = nvport->v.bottom;
    vdevice.viewport->v.top = nvport->v.top;
  } else {
    nvport = (Vstack *)vallocate(sizeof(Vstack));
    nvport->back = vdevice.viewport;
    nvport->v.left = vdevice.viewport->v.left;
    nvport->v.right = vdevice.viewport->v.right;
    nvport->v.bottom = vdevice.viewport->v.bottom;
    nvport->v.top = vdevice.viewport->v.top;
    vdevice.viewport = nvport;
  }
}

/*
 * popviewport
 *
 * pops the top viewport off the viewport stack.
 *
 */
void popviewport()
{
  Vstack *nvport;

  nvport = vdevice.viewport;
  vdevice.viewport = vdevice.viewport->back;
  nvport->back = vsfree;
  vsfree = nvport;
  
  vdevice.maxVx = vdevice.viewport->v.right * vdevice.sizeSx;
  vdevice.maxVy = vdevice.viewport->v.top * vdevice.sizeSy;
  vdevice.minVx = vdevice.viewport->v.left * vdevice.sizeSx;
  vdevice.minVy = vdevice.viewport->v.bottom * vdevice.sizeSy;

  CalcW2Vcoeffs();
}

/*
 * viewport
 *
 * Define a Viewport in Normalized Device Coordinates
 *
 * The viewport defines that fraction of the screen that the window will
 * be mapped onto. Unlike in VOGLE  the screen dimension is from 0 to sizeX
 * and 0 to sizeY.
 */
void viewport(xlow, xhigh, ylow, yhigh)
     Screencoord xlow, xhigh, ylow, yhigh;
{
  if (xhigh >= vdevice.sizeSx)
    xhigh = vdevice.sizeSx - 1;

  if (yhigh >= vdevice.sizeSy)
    yhigh = vdevice.sizeSy - 1;

  /*
   * Make sure the viewport stack knows about us.....
   */
  vdevice.viewport->v.left = xlow / (float)vdevice.sizeSx;
  vdevice.viewport->v.right = xhigh / (float)vdevice.sizeSx;
  vdevice.viewport->v.bottom = ylow / (float)vdevice.sizeSy;
  vdevice.viewport->v.top = yhigh / (float)vdevice.sizeSy;

  vdevice.minVx = xlow;
  vdevice.minVy = ylow;
  vdevice.maxVx = xhigh;
  vdevice.maxVy = yhigh;

  CalcW2Vcoeffs();
}

/*
 * getviewport
 *
 *	Returns the left, right, bottom and top limits of the current
 *	viewport.
 */
void getviewport(left, right, bottom, top)
     Screencoord *left, *right, *bottom, *top;
{
  *right = vdevice.maxVx;
  *top = vdevice.maxVy;
  *left = vdevice.minVx;
  *bottom = vdevice.minVy;
}
