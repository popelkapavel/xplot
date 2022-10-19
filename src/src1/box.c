#define OKOK
#include <stdio.h>
#include <math.h>
#include "vogl.h"
#include "vodevice.h"
#include "pvm.h"

#define D2RR 0.00174532925
#define R2DD 572.9577951

extern int redraw;
extern float  sizze;
extern Matrix rotmatrix;
float  obox[8][3],v_coor[8][3];
float  get_angle();
/*************************************************************/
extern float *get_light_position();
extern void boxlookat(),reverselookat(),boxortho();
extern void  draw_fast_picture();
void make_box(), draw_coor();
/*************************************************************/
void view_box_coor()
{
  float tg,tgas,left,right,bottom,top;

  gview.proj_change = 0;
  if(gview.proj_type == ORTHOGONAL)
    {
      v_coor[0][0] = gview.left; v_coor[0][1] = gview.bottom;
      v_coor[0][2] = - gview.near_;
      v_coor[1][0] = gview.right; v_coor[1][1] = gview.bottom;
      v_coor[1][2] = - gview.near_;
      v_coor[2][0] = gview.right; v_coor[2][1] = gview.top;
      v_coor[2][2] = - gview.near_;
      v_coor[3][0] = gview.left; v_coor[3][1] = gview.top;
      v_coor[3][2] = - gview.near_;
      v_coor[4][0] = gview.left; v_coor[4][1] = gview.bottom;
      v_coor[4][2] = - gview.far_;
      v_coor[5][0] = gview.right; v_coor[5][1] = gview.bottom;
      v_coor[5][2] = - gview.far_;
      v_coor[6][0] = gview.right; v_coor[6][1] = gview.top;
      v_coor[6][2] = - gview.far_;
      v_coor[7][0] = gview.left; v_coor[7][1] = gview.top;
      v_coor[7][2] = - gview.far_;
    }
  else if(gview.proj_type == PERSPECTIVE)
    {
      tg = tan(D2RR* gview.fovy/2.0);
      tgas = tg* gview.aspect;

      left = - gview.pnear* tgas; right = - left;
      bottom = - gview.pnear* tg; top = - bottom;
      
      v_coor[0][0] = left; v_coor[0][1] = bottom; v_coor[0][2] = - gview.pnear;
      v_coor[1][0] = right; v_coor[1][1] = bottom; v_coor[1][2]= - gview.pnear;
      v_coor[2][0] = right; v_coor[2][1] = top; v_coor[2][2] = - gview.pnear;
      v_coor[3][0] = left; v_coor[3][1] = top; v_coor[3][2] = - gview.pnear;
 
      left = - gview.pfar* tgas; right = - left;
      bottom = - gview.pfar* tg; top = - bottom;
      
      v_coor[4][0] = left; v_coor[4][1] = bottom; v_coor[4][2] = - gview.pfar;
      v_coor[5][0] = right; v_coor[5][1] = bottom; v_coor[5][2] = - gview.pfar;
      v_coor[6][0] = right; v_coor[6][1] = top; v_coor[6][2] = - gview.pfar;
      v_coor[7][0] = left; v_coor[7][1] = top; v_coor[7][2] = - gview.pfar;
    }
}
/****************************************************************/
short  tempa;
void rotate_box()
{
  tempa += 10;
}
/****************************************************************/
void mk_ov_box()
{
  float *v;
  if(gview.proj_change)  view_box_coor();
  boxortho(-2.*sizze,2.*sizze, -2.*sizze, 2.*sizze, -100. *sizze, 100.0*sizze);
  rotate(tempa,'y');
  boxlookat(0.0001*gview.vx,0.0001*gview.vy,0.0001*gview.vz,0.,0.,0.,0);

  v = get_light_position();
  color(7);  move(0.0,0.0,0.0); draw(sizze*v[0],sizze*v[1],sizze*v[2]);

  pushmatrix();
  scale(gview.xscals,gview.yscals,gview.zscals);  
  translate(gview.xtrans,gview.ytrans,gview.ztrans);
  multmatrix(rotmatrix);
  scale(gview.xscale,gview.yscale,gview.zscale);  
  draw_fast_picture(); 
  popmatrix();

  reverselookat(gview.vx,gview.vy,gview.vz,0.0,0.0,0.0,0);
  translate(-gview.xtranp,-gview.ytranp,-gview.ztranp);
  color(9);  draw_coor( sizze,1);
  color(12);  make_box(v_coor);
  color(10);  move(0.0,0.0,0.0); draw(0.0,0.0,-3.84*sizze);
}

void draw_coor(leng,aix)
     float leng; int aix;
{
  move(leng,0.,0.);  draw(0.0,0.0,0.0);  draw(0.,leng,0.);
  move(0.0,0.0,0.0);       draw(0.,0.,leng);
}
/*****************************************************************/
void make_box(temp)
     float temp[8][3];
{
  movep(temp[0]);  drawp(temp[1]);   drawp(temp[2]);   drawp(temp[3]);
  drawp(temp[0]);  drawp(temp[4]);   drawp(temp[5]);   drawp(temp[6]);
  drawp(temp[7]);  drawp(temp[4]);   movep(temp[1]);  drawp(temp[5]);
  movep(temp[2]);  drawp(temp[6]);   movep(temp[3]);  drawp(temp[7]);
}
/********************************************************************/
void geye(n,nx,ox)
     int n, nx, ox;
     /* n: 0, lable */
     /*    1: radius */
{
  float t;
  int i = nx - ox;

  if( n == 1)
    {
      t = (float) exp(0.025* (float) i);
      gview.vx *= t;
      gview.vy *= t;
      gview.vz *= t;
      redraw = 1;
    }
}
/********************************************************************/

void portho(n,nx,ox) 
     int n,nx,ox;
     /* n:  0--5, label, do nothing                  */
     /*     6--11 left,right, top, bottom, near, far */
{
  float t;
  int i = nx - ox;
  gview.proj_type=ORTHOGONAL;
  if(  n >= 6 && n <= 11) { gview.proj_change = 1; redraw = 1;}
  t = (float) exp(0.0583* (float) i);
  switch(n)
    {
    case 6:
      gview.left *= t;
      break;
    case 7:
      gview.right *= t;
      break;
    case 8:
      gview.top *= t;
      break;
    case 9:
      gview.bottom *= t;
      break;
    case 10:
      gview.near_ *= t;
      break;    
    case 11:
      gview.far_ *= t;
      break;  
    case 0: case 1: case 2: case 3: case 4: case 5:
    default:
      break;
    }
}
/******************************************************************/      
void ppers(n,nx,ox)
     int n,nx,ox;
     /* n: 0--3, lable */
     /*    4--7, fovy, aspect, near, far */
{
  float t;
  int i = nx - ox;

  gview.proj_type=PERSPECTIVE;
  if( n >= 4 && n <= 7) {redraw = 1; gview.proj_change = 1;}
  t = (float)exp(0.0193* (double)i);
  switch(n)
    {
    case 4:
      gview.fovy *= t;
      break;
    case 5:
      gview.aspect *= t;
      break;
    case 6:
      gview.pnear *= t;
      break;
    case 7:
      gview.pfar *= t;
      break;
    case 0: case 1: case 2: case 3:   default:
      gview.proj_change = 0;
      break;
    }
}
  
/******************************************************************/






