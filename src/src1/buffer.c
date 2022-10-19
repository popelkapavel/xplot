#include "vogl.h"

static	int	sync = 1;
/*
 * backbuffer
 *
 *	swap drawing to backbuffer - returns -1 if no
 * backbuffer is available.
 */
void backbuffer(yes)
     int yes;
{
  if (vdevice.attr->a.mode == SINGLE)
    return;

  if (yes) {
    if ((*vdevice.dev.Vbackb)() < 0)
      verror("device doesn't support double buffering\n");
		
    vdevice.inbackbuffer = 1;
    vdevice.sync = 0;
  } else
    vdevice.inbackbuffer = 0;
}

/*
 * frontbuffer
 *
 *	start drawing in the front buffer again. This
 * will always work!
 */
void frontbuffer(yes)
     int yes;
{
  if (vdevice.attr->a.mode == SINGLE)
    return;

  if (yes) {
    (*vdevice.dev.Vfrontb)();
    vdevice.inbackbuffer = 0;
    vdevice.sync = sync;
  } else
    backbuffer(1);
}

/*
 * swapbuffers
 *
 *	swap the back and front buffers - returns -1 if
 * no backbuffer is available.
 */
void swapbuffers()
{
  if ((*vdevice.dev.Vswapb)() < 0)
    verror("swapbuffers device doesn't support double buffering\n");
}

/*
 * doublebuffer()
 *
 *	Flags our intention to do double buffering....
 *	tries to set it up etc etc...
 */
void doublebuffer()
{
  if ((*vdevice.dev.Vbackb)() < 0)
    verror("device doesn't support double buffering\n");

  vdevice.inbackbuffer = 1;
  sync = vdevice.sync;
  vdevice.sync = 0;
}

/*
 * singlebuffer()
 *
 *	Goes back to singlebuffer mode....(crock)
 */
void singlebuffer()
{ 
  if (vdevice.attr->a.mode == SINGLE)
    return;

  (*vdevice.dev.Vfrontb)();

  vdevice.inbackbuffer = 0;
}



