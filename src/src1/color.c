/********************************************************************
 *
 *    Xplot, colro.c
 *
 ********************************************************************/
#include <stdio.h>
#include <math.h>
#include "vogl.h"
#include "pvm.h"
/*********************************************************************/
light lig = { 0.78, 2.05, 0.630837921, 0.6240636912, -0.4610726914,
		       1.0,1.0,1.0};
/*static light lig = { 0.3398015543, 0.7853981643,
		       0.6666666667, 0.66666666667, 0.3333333333333,
		       1.0,1.0,1.0};
*/
/*********************************************************************/
extern int redraw;
/*********************************************************************/
extern void mapcolor();
extern void ignore_everything();
void set_light_position(), light_pos(), light_color();
void make_materialn(), use_new_material(), make_color(),PSmake_color();
float *get_light_position();
extern int nmats;                         /* number of materials used */
extern int fixedlight;
int is_rainbow[2] = {0,0};
/*********************************************************************/
mat_entry mat_list[] =
{
  { 0.19225, 0.19225, 0.19225,
      0.50754, 0.50754, 0.50754,
      0.508273, 0.508273, 0.508273,
      14.0},
#define SILVER 0
  { 0.23125, 0.231125, 0.23125,
      0.2775, 0.2775, 0.2775,
      0.773911,0.773911,0.773911,
      10.0},
#define SILVER_S 1
  {0.24725, 0.1995, 0.0745,
     0.75164, 0.60648, 0.22648,
     0.628281, 0.555802, 0.366065,
     18.0},
#define GOLD 2
  {0.24725, 0.2245, 0.0645,
     0.34615, 0.3143, 0.0903,
     0.797357, 0.723991, 0.208006,
     12.0},
#define GOLD_S 3
  {0.0329412, 0.023529, 0.017451,
     0.780392, 0.568627, 0.113725,
     0.992157, 0.941176, 0.807843,
     20.0},
#define BRASS 4
  {0.2295, 0.08825, 0.0275,
     0.5508, 0.2118, 0.066,
       0.580594, 0.223257, 0.0695701,
    16.0},
#define COPPER 5    
  {0.19125, 0.0735, 0.0225,
   0.256777, 0.137622, 0.086014,
   0.7038, 0.27048, 0.0828,
     10.0},
#define COPPER_S 6
  {0.05375, 0.05, 0.06625,
     0.18275, 0.17, 0.22525,
     0.332741, 0.328634, 0.346435,
      20.3},
#define OBSIDIAN 7
  { 0.1745, 0.01175, 0.01175,
      0.61424, 0.04136, 0.04136,
      0.727811, 0.626959, 0.626959,
      10.0},
#define RUBY 8
  {0.1, 0.18725, 0.1745,
     0.297254, 0.30829, 0.306678,
     0.396, 0.74151, 0.69102,
     3.0},
#define TURQUOISE 9  
  {0.0135, 0.02225, 0.01575,
     0.316228, 0.316228, 0.316228,
     0.54, 0.89, 0.63,
     2.0},
#define JADE 10
  {0.0215, 0.1745, 0.0215,
     0.07568, 0.61424, 0.07568,
     0.633, 0.727811, 0.633,
     6.0},
#define EMERALD 11
  {0.0,0.0,0.0,
     0.12,0.12,0.45,
     0.45,0.45,0.55,
     2.0},
#define PLASTIC_BLUE 12
  {0.0, 0.1, 0.06,
     0.0, 0.50980392, 0.50980392,
     0.50196078, 0.50196078, 0.50196078,
     3.0},
#define PLASTIC_CYAN 13
  {0.0, 0.0, 0.0,
     0.1, 0.35, 0.1,
     0.45, 0.65, 0.45,
     3.0},
#define PLASTIC_GREEN 14
  {0.0,0.0,0.0,
     0.5,0.0,0.0,
     0.7,0.4,0.4,
     2.0},
#define PLASTIC_RED 15
  {0.1, 0.0, 0.06,
     0.47058824, 0.0, 0.19607843,
     0.50,0.40, 0.50,
     2.0},
#define PLASTIC_PINK 16
  {0.0,0.0,0.0,
     0.5,0.5,0.0,
     0.6,0.6,0.5,
     3.0},
#define PLASTIC_YELLOW 17
};
/**********************************************************************/
short RGBtable[256][3];
mat_entry cmat[2];
int current_mat;
/**********************************************************************/
void set_initial_color_table()  /* the 1st 16 colors. 0: background  */
{                               /* 1--6: different shades of grey.   */ 
  int i,j;                      /* 7; light color, WIHTE, 8, BLACK   */
                                /* 9--14: r g b y c m                */
  for(i = 0; i <=7; i++)
    {
      j = 32 * (i+1);  if(j > 255) j = 255;
      RGBtable[i][0]=j;RGBtable[i][1]=j;RGBtable[i][2]=j;
    }
  RGBtable[8][0] = 0;    RGBtable[8][1] = 0;   RGBtable[8][2] = 0;

  RGBtable[9][0] = 255;  RGBtable[ 9][1] = 0;   RGBtable[ 9][2] = 0;
  RGBtable[10][0] = 0;   RGBtable[10][1] = 255; RGBtable[10][2] = 0;
  RGBtable[11][0] = 0;   RGBtable[11][1] = 0;   RGBtable[11][2] = 255;
  RGBtable[12][0] = 255; RGBtable[12][1] = 255; RGBtable[12][2] = 0;
  RGBtable[13][0] = 255; RGBtable[13][1] = 0;   RGBtable[13][2] = 255;
  RGBtable[14][0] = 0;   RGBtable[14][1] = 255; RGBtable[14][2] = 255;
  RGBtable[15][0] = 255; RGBtable[15][1] = 255; RGBtable[15][2] = 255;
}
/**********************************************************************/
void use_new_material(index)
     int index;
{
  mat_entry *t = &(cmat[current_mat]);
  mat_entry *o = &(mat_list[index]);
  int i;
  for (i = 0; i < 3; i++)
    {
      t->ambient[i] = o->ambient[i];
      t->diffuse[i] = o->diffuse[i];
      t->specular[i] = o->specular[i];
    }
  t->shininess = o->shininess;
  make_materialn();
}
/******************************************************************/
void make_materialn() 
{
  float r,g,b,ra,ga,ba,rd,gd,bd,rs,gs,bs;
  float df, sf, dff, sff, sf1;
  int i, offset;
  mat_entry *t = &(cmat[current_mat]);

  if(current_mat == 0) offset = 16;
  else offset = 136;
  ra = t->ambient[0];
  ga = t->ambient[1];
  ba = t->ambient[2];
  rd = t->diffuse[0];
  gd = t->diffuse[1];
  bd = t->diffuse[2];
  rs = t->specular[0];
  gs = t->specular[1];
  bs = t->specular[2];

  df =  0.008333333333;  sf =  0.0083333333333;
  dff = 0.0;   sff = 0.0000001;
  for(i = 0; i < 120; i++)
    {
      sf1 = pow(sff,t->shininess);
      r = (ra + (1.0)* dff * rd + sf1 * rs)*lig.red; if(r > 1.0) r = 1.0;
      g = (ga + (1.0)* dff * gd + sf1 * gs)*lig.grn; if(g > 1.0) g = 1.0;
      b = (ba + (1.0)* dff * bd + sf1 * bs)*lig.blu; if(b > 1.0) b = 1.0;
      dff += df; sff += sf;
      RGBtable[offset + i][0] = (int ) ( r * 255.0);
      RGBtable[offset + i][1] = (int ) ( g * 255.0);
      RGBtable[offset + i][2] = (int ) ( b * 255.0);
    }
  make_color(0);
}
/***********************************************************/
void make_color(type) int type;
{
  int i, offset;

  if(current_mat == 0) offset= 16;
  else offset = 136;
  for(i = offset; i < offset+120; i++)
    mapcolor(i, RGBtable[i][0],RGBtable[i][1],RGBtable[i][2]);
  is_rainbow[current_mat] = type;
}

void PSmake_color()
{
  int i;
  for(i = 0; i < 256; i++)
    mapcolor(i, RGBtable[i][0],RGBtable[i][1],RGBtable[i][2]);
}
/******************************************************************/  
void set_current_material(i) int i;
{
  if( i== 0 || i ==1) current_mat = i;
}
/******************************************************************/  
void light_pos(n,nx,ox)
     int n,nx,ox;
     /* n:  0--1 lable,   */
     /*     2--3 azim inc */
{
  if (n == 2 || n == 3) {redraw = 1; fixedlight = 1;}
  switch(n)
    {
    case 2:  /*azim */
      lig.azim = 0.05 *( (float) (nx - 18));
      set_light_position();
      break;
    case 3:
      lig.incl = 0.025 *( (float)( nx - 18));
      set_light_position();
      break;
    case 0: case 1:  default:
      break;
    }
}
/*********************************************************************/
void light_color(n,nx,ox)
     int n,nx,ox;
     /* n: 0--2 lable  */
     /*    3--5 r g b  */
{
  float t = 0.007 * (float) (nx-18);
  if( n >= 3 && n <= 5) fixedlight = 1;
  if(t > 1.0) t = 1.0;
  switch(n)
    {
    case 3:
      lig.red = t;
      break;
    case 4:
      lig.grn = t;
      break;
    case 5:
      lig.blu = t;
      break;
    case 0: case 1: case 2:  default:
      break;
    }
  mapcolor(7, (int) (255.0* lig.red), (int) (255.0* lig.grn),
	    (int) (255.0* lig.blu));
  if(n >= 3)
    {
      if(nmats)
	{
	  int i = current_mat;
	  current_mat = 0; make_materialn();
	  current_mat = 1; make_materialn();
	  current_mat = i;
	}
      else make_materialn();
    }
}
/*********************************************************************/
extern float sizze;
void set_light_position(v)
     float *v;
{
  float t;
  t =  sin(lig.incl);

  lig.x[0] = t * cos(lig.azim);
  lig.x[1] = t * sin(lig.azim);
  lig.x[2] =     cos(lig.incl);
}
float *get_light_position()
{
  return(lig.x);
}
/*********************************************************************/
void back_color(n,nx,ox)
     int n,nx,ox;
     /* n: 0--2 lable  */
     /*    3--5 r g b  */
{
  float t = 0.007 * (float) (nx-18);
  if(t > 1.0) t = 1.0;
  switch(n)
    {
    case 0: case 1: case 2:
      break;
    case 3:
      RGBtable[0][0] = (int) (t* 255.0);
      break;
    case 4:
      RGBtable[0][1] = (int) (t* 255.0);
      break;
    case 5:
      RGBtable[0][2] = (int) (t* 255.0);
      break;
    default:
      break;
    }
  mapcolor(0, RGBtable[0][0],RGBtable[0][1],RGBtable[0][2]);
}
/**********************************************************************/
void torgbtable(i,r,g,b)
     int i;  Colorindex r,g,b;
{
  RGBtable[i][0]=r;  RGBtable[i][1]=g; RGBtable[i][2]=b;
}
void rainbow_color()
{
  Colorindex blue = 0 , green = 0 , red = 0 ;
  int i; 

  if(current_mat == 0) i = 16;
  else i = 136;
  red = 120;
  while (red < 244) 
    {
      torgbtable(i,red,green,blue) ; i++ ; 
      red += 15 ; 
    }
  red = 255; torgbtable(i,red,green,blue) ; i++ ; 
  while(green < 244)
    {
      green += 13;
      torgbtable(i, red, green, blue) ; i++ ; 
    }
  green = 255;  torgbtable(i,red,green,blue) ; i++ ; 
  while (red >= 15) 
    {
      red -= 15 ;      
      torgbtable(i, red, green, blue) ; i++ ;
    }
  red = 0 ;  
  while (blue < 244) 
    {
      blue += 13;
      torgbtable(i, red, green, blue) ; i++ ;  
   }
  blue = 255;   torgbtable(i,red,green,blue) ; i++ ; 
  while(green >= 15) 
    {
      green -= 15;
      torgbtable(i, red, green, blue) ; i++ ; 
    }
  green = 0;  torgbtable(i,red,green,blue) ; i++ ; 
  while (red < 244) 
    {
      red += 15;
      torgbtable(i, red, green, blue) ;  i++ ;
    }
  red = 255 ;  torgbtable(i,red,green,blue) ; i++ ; 
  while (green < 244) 
    {
      green += 15;
      torgbtable(i, red, green, blue) ; i++ ;
    }
  make_color(1);
}

void random_color()
{
  int i, start;
  short r,g,b;
  if(current_mat == 0) start = 16;
  else start = 136;  
  
  for(i = start; i < start+120; i++)
    {
      r =  (rand()>>5)%255; g =  (rand()>>5)%255; b =  (rand()>>5)%255;
      torgbtable(i, r,g,b);
    }
  make_color();
}
/********************************************************************/
