/*******************************************************************/
#include <stdio.h>
#include <math.h>
#include "vogl.h"
#define  GVIEWSETHERE
#include "pvm.h"
#include "object.h"
/*******************************************************************/
#ifndef MAX
#define MAX(a,b) ( (a)> (b)? (a): (b))
#define MIN(a,b) ( (a)< (b)? (a): (b))
#endif
#ifndef ABS
#define ABS(x)   ( (x)>0 ? (x) : -(x))
#endif
/*******************************************************************/
extern void read_data(), remove_pendingevent();
extern void view_box_coor(), mk_ov_box(), message();
extern void winset(),ozdepth(),pzdepth(), initialize_colors();
extern void use_new_material(),qsort(), initialize_screen_axes();
extern void  draw_title_string(), make_dpy_list();
extern int something(),do_load_setup();
extern void getmatrixVM();
extern float *get_light_position();
extern char *strcpy();
/*******************************************************************/
void setup_trans(), show_picture(), draw_fast_picture();
void sort_depth(), show_title();
void set_fast_light_polygon_only(), set_fast_light_all();
void set_fast_light_polygon_only_S(), set_fast_light_all_S();
void set_light_polygon_only(), set_light_all();
void set_light_polygon_only_S(), set_light_all_S();
void depthcue_draw_picture(), light_draw_picture();
void init_pvm(), minvert(),copyvec3(),pick_rotation();
void normalize3();
void rot_about_v(), updaterot(), printmatrix();
void initialize_screen_axis();
void arb_rot();
/*******************************************************************/
extern int hsliderx, vslidery, vsliderz;
extern int hslidox, vslidoy, vslidoz;
extern int redraw;
extern butboxstat bboxstat;
extern struct vdev vdevice;
/************************************************* from polyogns.c */
extern int current_color_index;
extern int not_zbuffered_wireframe;
extern int wireframe_over_polygon;
extern int  polymodeflag;
extern float cut_off_size; 
/*******************************************************************/
Matrix vm_matrix;
/********************************************************************/
float   rotaxis[3];               /* current rotation axis */
float   sizze;                    /* object size           */
Matrix  rotmatrix;                /* cumulative rotation M */
int      movingmode = 0;
int      slowmoving = 0;
int      fixedlight = 1;            /* light is fixed        */
int      do_not_sort = 0;   
int      boxon = 0, depthcue = 0;
int      lightedline = 0;
int      twosidedobj = 1;
float    depthcue_elt_count[2] = {0.0,0.0};

/********************************************************************/
int     plot3d = 1;       
/********************************************************************/
typedef void (* VFUNC)();
VFUNC zdepth, draw_picture, set_light, set_fast_light;
/********************************************************************/
void init_pvm()
{
  gview.xmax = xmaxn;
  gview.ymax = ymaxn;
  gview.zmax = zmaxn;
  gview.xmin = xminn;
  gview.ymin = yminn;
  gview.zmin = zminn;
  
  sizze = MAX( MAX(ABS(gview.xmax - gview.xmin),
		   ABS(gview.ymax - gview.ymin)), 
	      ABS(gview.zmax - gview.zmin));

  gview.vx = 0.5*(gview.xmax + gview.xmin) +2. * sizze;
  gview.vy = 0.5*(gview.ymax + gview.ymin ) +2. * sizze;
  gview.vz = 0.5*(gview.zmax + gview.zmin ) +1. * sizze;

  gview.proj_type = PERSPECTIVE;

  gview.left = gview.bottom = -0.6* sizze; 
  gview.right = gview.top =  0.6 * sizze;
  gview.near_ = 1.7 * sizze; gview.far_ = 4.3 * sizze;

  gview.fovy =250.0; gview.aspect =1.0;
  gview.pnear= 1.7 * sizze; gview.pfar = 4.3*sizze;

  gview.xscale = gview.yscale = gview.zscale = 1.0;
  gview.xscals = gview.yscals = gview.zscals = 1.0;
  gview.xtrans = gview.ytrans = gview.ztrans = 0.0;
  gview.xtranp = gview.ytranp = gview.ztranp = 0.0;

  identmatrix(rotmatrix);    
  if(plot3d == 0)
    {
      gview.proj_type = ORTHOGONAL;
      gview.vx -= 2. * sizze; 
      gview.vy -= 2. * sizze; 
      gview.vz += 2. * sizze; 
      gview.near_ = 1.5 * sizze;
      gview.far_ = 4.5 * sizze;
    }   
  else
    {
      rotmatrix[0][0] = 0.237426;
      rotmatrix[1][1] = 0.237426;
      rotmatrix[0][1] =  0.191122;
      rotmatrix[0][2] =  0.952418;
      rotmatrix[1][0] =  0.930309;
      rotmatrix[1][2] =  -0.279559;
      rotmatrix[2][0] =  -0.279559;
      rotmatrix[2][1] =  0.952418;
      rotmatrix[2][2] =  -0.121432;
    }
  gview.proj_change = 1;
  initialize_screen_axis();
  titlepos[0] = 0.0; titlepos[1] = 0.9;
  titlecolor = 14;
}

/*******************************************************/
char *desfile, *datfile;
int xplot_show_main(argc, argv)
	int	argc;
	char	**argv;
{
  if(argc < 2) 
    {
      (void) fprintf(stderr,"Usage: %s desfile, datfile\n",argv[0]);
      exit(1);
    }
  desfile = argv[0]; datfile = argv[1];

  /* 
   * First initialize the device ...
   */
  vinit("GRX");  ginit();
  initialize_colors();
  /*
   * Then read the data file.
   */
  message(0,"Reading data");
  read_data();  if(!plot3d) boxon = 1;
  /* 
   * initialize the viewing transformations
   */
  init_pvm();
  /*
   * and finally make the display list.
   */
  message(1,"Building display list");
  make_dpy_list();
  winset(1);   doublebuffer(); 
  winset(0);  doublebuffer();
  if(argc >= 3)
    { 
      if(do_load_setup( argv[2]) == 0)
	(void)fprintf(stderr,"Cannot load setup file %s\n", argv[2]);
    }
  else  if( do_load_setup("xplot.ini") == 0)
    do_load_setup("xplot.ini");
  
  while(something())
    {
      setup_trans();
      if(redraw)
	{
	  show_picture(!movingmode);
	  swapbuffers();
	  if(!movingmode) { remove_pendingevent();  redraw = 0;}
	}
      /*
       * the order here is very important. mk_ov_box needs to know
       * the display_list setup by show_picture !
       */
      winset(1); color(BLACK);   clear();  mk_ov_box(); swapbuffers(); 
    }
  titlepos[0]=0.0;   titlepos[1]=0.9;
  return(0);
}
/*************************************************************/
void show_picture(notmoving)
     int notmoving;
{
  /*
   *   projection 
   */
  if(gview.proj_type ==PERSPECTIVE)
    {
      zdepth = &pzdepth;
      perspective( (short)gview.fovy, gview.aspect, gview.pnear,gview.pfar);
    }
  else
    {
      ortho(gview.left,gview.right,gview.bottom,gview.top,
	    gview.near_,gview.far_);
      zdepth = &ozdepth;
    }
  /*
   *   screen translation and scaling.
   */
  translate(gview.xtranp,gview.ytranp,gview.ztranp);
  scale(gview.xscals,gview.yscals,gview.zscals);  
  /*
   *    viewing transformation
   */
  lookat(gview.vx, gview.vy, gview.vz,0.,0.,0.,0);
  /*
   *    modeling transformation. Screen rotation is
   *    encoded in rotmatrix by setting the rotation
   *    axis to be the screen axis
   */
  translate(gview.xtrans,gview.ytrans,gview.ztrans);
  multmatrix(rotmatrix);
  getmatrixVM(vm_matrix,1./gview.xscals,1./gview.yscals,1./gview.zscals);
  scale(gview.xscale,gview.yscale,gview.zscale);  

  /*****************************************************************
   * setup display routine
   */
  if(depthcue) draw_picture = &depthcue_draw_picture;
  else   draw_picture = &light_draw_picture;
  if(lightedline)
    {
      if(twosidedobj)
	{
	  set_light = & set_light_all;
	  set_fast_light = & set_fast_light_all;
	}
      else
	{
	  set_light = & set_light_all_S;
	  set_fast_light = & set_fast_light_all_S;
	}
    }
  else
    {
      if(twosidedobj)
	{
	  set_light = & set_light_polygon_only;
	  set_fast_light = & set_fast_light_polygon_only;  
	}
      else
	{
	  set_light = & set_light_polygon_only_S;
	  set_fast_light = & set_fast_light_polygon_only_S;  
	}
    }
  /*****************************************************************/

  if(fixedlight)
    { if(notmoving || slowmoving) set_light(); else set_fast_light();}

  winset(0);
  color(0); clear();
  if( notmoving || slowmoving) draw_picture();
  else draw_fast_picture();
  if(titleset) { color(titlecolor); draw_title_string(titlestring);}
}
/**************************************************************/
extern int icmp();
void sort_depth()
{
  int i,  k,l; float tmp; 
  elt **et = elts, *t;
  float ff[3], *vt1;

  for(i = 0; i < nelts; i++)
    {
      t = *et;
      switch(t->type)
	{
	case QUADPOLY:
	  ff[0]=(t->f)[0][0]+(t->f)[1][0]+(t->f)[2][0]+(t->f)[3][0];
	  ff[1]=(t->f)[0][1]+(t->f)[1][1]+(t->f)[2][1]+(t->f)[3][1];
	  ff[2]=(t->f)[0][2]+(t->f)[1][2]+(t->f)[2][2]+(t->f)[3][2];
	  zdepth( &(tmp),ff,vm_matrix); t->depth = tmp;
	  break;
	case POLYGON:
	  ff[0] = 0.0; ff[1]=0.0; ff[2]=0.0;
	  k = (t->nv); 
	  for(l=0; l<k; l++)
	    {
	      vt1 = (t->f)[l];
	      ff[0] += vt1[0];   ff[1] += vt1[1];   ff[2] += vt1[2];  
	    }
	  zdepth( &(tmp), ff , vm_matrix); tmp = tmp+tmp;
	  t->depth = (tmp+tmp)/(float)k; 
	  break;
	case POLYLINE:
	case AXESBOX:
	  vt1 = (t->f)[0];
	  ff[0] = vt1[0];   ff[1] = vt1[1];   ff[2] = vt1[2];  
	  vt1 = (t->f)[1];
	  ff[0] += vt1[0];   ff[1] += vt1[1];   ff[2] += vt1[2];  
	  zdepth( &(tmp), ff , vm_matrix); t->depth = tmp+tmp;
	  break;
	case CNTLINE:
	  vt1 = (t->f)[0];
	  ff[0] = vt1[0];   ff[1] = vt1[1];   ff[2] = vt1[2];  
	  vt1 = (t->f)[1];
	  ff[0] += vt1[0];   ff[1] += vt1[1];   ff[2] += vt1[2];  
	  zdepth( &(tmp), ff , vm_matrix);
	  t->depth = tmp+tmp-cut_off_size;
	  break;
	case LINEPOINT:
	  vt1 = (t->f)[0];
	  ff[0] = vt1[0];   ff[1] = vt1[1];   ff[2] = vt1[2];  
	  zdepth( &(tmp), ff , vm_matrix); tmp = tmp+tmp;
	  t->depth = tmp+tmp;
	  break;
	default:
	  break;
	}
      et++;
    }
  qsort(elts, nelts, sizeof(elt *), icmp);
}
/**************************************************************************/
void light_draw_picture()
{
  int i;     elt **t, *t1;
  if(do_not_sort == 0 || slowmoving) sort_depth();
  t = elts;
  for(i=0; i<nelts; i++)
    {
      t1 = *t;
      current_color_index = t1->color;
      switch( t1->type )
	{
	case POLYGON:
	case QUADPOLY:
	  my_poly(t1->nv,t1->f);
	  break;
	case POLYLINE:
	case CNTLINE:
	  my_line(t1->f);
	  break;
	case LINEPOINT:
	  my_point(t1->f);
	  break;
	case AXESBOX:
	  if(boxon)
	    {
	      current_color_index = 4;	  
	      my_line(t1->f);
	    }
	  break;

	default:
	  break;
	}
      t++;
    }
}
/******************************************************************/
void depthcue_draw_picture()
{
  int i;     elt **t, *t1;
  int c1 = 0, c2 = 0;  /* count of elt in each of the two colors */
  
  if(do_not_sort == 0 || slowmoving) sort_depth();

  t = elts;

  for(i=0; i<nelts; i++)
    {
      t1 = *t;
      if(t1->color > 135)
	{
	  c2++; 
	  current_color_index= 136 +(int)(depthcue_elt_count[1] * (float) c2);
	}
      else
	{
	  c1++; 
	  current_color_index= 16+ (int)(depthcue_elt_count[0] * (float) c1);
	}
      switch(t1->type)
	{
	case POLYGON:
	case QUADPOLY:
	  my_poly(t1->nv,t1->f);
	  break;
	case POLYLINE:
	case CNTLINE:
	  my_line(t1->f);
	  break;
	case LINEPOINT:
	  my_point(t1->f);
	  break;
	case AXESBOX:
	  if(boxon)
	    {
	      current_color_index = 4;	  
	      my_line(t1->f);
	    }
	  break;

	default:
	  break;
	}
      t++;
    }
}
/*********************************************************************/
void draw_fast_picture()
{
  int i, oldpolymode = polymodeflag;
  int incr, total;
  elt **t, *t1;


  polymodeflag = 0;
  t = elts;
  if(nelts > 100) { incr = nelts/100; total = 100;}
  else { incr = 1; total = nelts;}

  for(i=0; i < total; i++)
    {
      t1 = *t;
      current_color_index = t1->color;
      switch(t1->type)
	{
	case POLYGON:
	case QUADPOLY:
	  my_poly(t1->nv,t1->f);
	  break;
	case POLYLINE:
	case CNTLINE:
	  my_line(t1->f);
	  break;
	case LINEPOINT:
	  my_point(t1->f);
	  break;
	case AXESBOX:
	  if(boxon)
	    {
	      current_color_index = 14;	  /* white */
	      my_line(t1->f);
	    }
	  break;
	default:
	  break;
	}
      t += incr;
    }
  polymodeflag = oldpolymode;
}

/********************************************************************/
void set_light_polygon_only()
{
  Matrix tmpa;
  int i;
  elt **t = elts, *t1;
  float lig1[4],lig[4],val;
  float *ligp, *vt1;
  
  if(depthcue) return;

  ligp = get_light_position();
  minvert(vm_matrix, tmpa,1);

  for(i = 0; i < nelts; i++)  /* assuming that lig and norm are normalized */
    {
      t1 = *t;
      if(t1->type <= POLYGON)
	{
	  vt1 = (t1->f)[0];
	  lig[0] = vt1[3];   lig[1] = vt1[4];  lig[2] = vt1[5];
	  multvector3(lig1, lig, tmpa);
	  val = ligp[0]* lig1[0] + ligp[1]* lig1[1] +ligp[2]*lig1[2];
	  if(val < 0.0) val = -val; 
	  t1->color = t1->mat + (int)(119.5* val);
	}
      t++;
    }
}
void set_light_polygon_only_S()
{
  Matrix tmpa;
  int i;
  elt **t = elts, *t1;
  float lig1[4],lig[4],val;
  float *ligp, *vt1;
  
  if(depthcue) return;

  ligp = get_light_position();
  minvert(vm_matrix, tmpa,1);

  for(i = 0; i < nelts; i++)  /* assuming that lig and norm are normalized */
    {
      t1 = *t;
      if(t1->type <= POLYGON)
	{
	  vt1 = (t1->f)[0];
	  lig[0] = vt1[3];   lig[1] = vt1[4];  lig[2] = vt1[5];
	  multvector3(lig1, lig, tmpa);
	  val = ligp[0]* lig1[0] + ligp[1]* lig1[1] +ligp[2]*lig1[2];
	  t1->color = t1->mat + (int)(59.5* (val+1.0));
	}
      t++;
    }
}


void set_light_all()
{
  Matrix tmpa;
  int i, k;
  elt **t = elts, *t1;
  float lig1[4],lig[4],val;
  float *ligp, *vt1;
  
  if(depthcue) return;

  ligp = get_light_position();
  minvert(vm_matrix, tmpa,1);

  for(i = 0; i < nelts; i++)  /* assuming that lig and norm are normalized */
    {
      t1 = *t;
      if(t1->type <= POLYGON) k = t1->mat; else k = 136;
      vt1 = (t1->f)[0];
      lig[0] = vt1[3];   lig[1] = vt1[4];  lig[2] = vt1[5];
      multvector3(lig1, lig, tmpa);
      val = ligp[0]* lig1[0] + ligp[1]* lig1[1] +ligp[2]*lig1[2];
      if(val < 0.0) val = -val; 
      t1->color = k + (int)(119.5* val);
      t++;
    }
}
void set_light_all_S()
{
  Matrix tmpa;
  int i, k;
  elt **t = elts, *t1;
  float lig1[4],lig[4],val;
  float *ligp, *vt1;
  
  if(depthcue) return;

  ligp = get_light_position();
  minvert(vm_matrix, tmpa,1);

  for(i = 0; i < nelts; i++)  /* assuming that lig and norm are normalized */
    {
      t1 = *t;
      if(t1->type <= POLYGON) k = t1->mat; else k = 136;
      vt1 = (t1->f)[0];
      lig[0] = vt1[3];   lig[1] = vt1[4];  lig[2] = vt1[5];
      multvector3(lig1, lig, tmpa);
      val = ligp[0]* lig1[0] + ligp[1]* lig1[1] +ligp[2]*lig1[2];
      t1->color = k + (int)(59.50* (val+1.0));
      t++;
    }
}


void set_fast_light_polygon_only()
{
  Matrix tmpa;
  int i, incr, total;
  elt **t = elts, *t1;
  float lig1[4],lig[4],val;
  float *ligp, *vt1;
  
  if(nelts > 100) {incr = nelts/100; total =100;}
  else {incr = 1; total= nelts;}

  ligp = get_light_position();
  minvert(vm_matrix, tmpa,1);

  for(i = 0; i < total; i++)  /* assuming that lig and norm are normalized */
    {
      t1 = *t;
      if(t1->type <= POLYGON)
	{
	  vt1 = (t1->f)[0];
	  lig[0] = vt1[3];   lig[1] = vt1[4];  lig[2] = vt1[5];
	  multvector3(lig1, lig, tmpa);
	  val = ligp[0]* lig1[0] + ligp[1]* lig1[1] +ligp[2]*lig1[2];
	  if(val < 0.0) val = -val; 
	  t1->color = t1->mat + (int)(119.5* val);
	}
      t += incr;
    }
}


void set_fast_light_polygon_only_S()
{
  Matrix tmpa;
  int i, incr, total;
  elt **t = elts, *t1;
  float lig1[4],lig[4],val;
  float *ligp, *vt1;
  
  if(nelts > 100) {incr = nelts/100; total =100;}
  else {incr = 1; total= nelts;}

  ligp = get_light_position();
  minvert(vm_matrix, tmpa,1);

  for(i = 0; i < total; i++)  /* assuming that lig and norm are normalized */
    {
      t1 = *t;
      if(t1->type <= POLYGON)
	{
	  vt1 = (t1->f)[0];
	  lig[0] = vt1[3];   lig[1] = vt1[4];  lig[2] = vt1[5];
	  multvector3(lig1, lig, tmpa);
	  val = ligp[0]* lig1[0] + ligp[1]* lig1[1] +ligp[2]*lig1[2];
	  t1->color = t1->mat + (int)(59.5* (val+1.0));
	}
      t += incr;
    }
}

void set_fast_light_all()
{
  Matrix tmpa;
  int i, k, incr, total;
  elt **t = elts, *t1;
  float lig1[4],lig[4],val;
  float *ligp, *vt1;
  
  if(nelts > 100) {incr = nelts/100; total =100;}
  else {incr = 1; total= nelts;}

  ligp = get_light_position();
  minvert(vm_matrix, tmpa,1);

  for(i = 0; i < total; i++)  /* assuming that lig and norm are normalized */
    {
      t1 = *t;
      if(t1->type <= POLYGON) k = t1->mat; else k = 136;
      vt1 = (t1->f)[0];
      lig[0] = vt1[3];   lig[1] = vt1[4];  lig[2] = vt1[5];
      multvector3(lig1, lig, tmpa);
      val = ligp[0]* lig1[0] + ligp[1]* lig1[1] +ligp[2]*lig1[2];
      if(val < 0.0) val = -val; 
      t1->color = k + (int)(119.5* val);
      t += incr;
    }
}

void set_fast_light_all_S()
{
  Matrix tmpa;
  int i, k, incr, total;
  elt **t = elts, *t1;
  float lig1[4],lig[4],val;
  float *ligp, *vt1;
  
  if(nelts > 100) {incr = nelts/100; total =100;}
  else {incr = 1; total= nelts;}

  ligp = get_light_position();
  minvert(vm_matrix, tmpa,1);

  for(i = 0; i < total; i++)  /* assuming that lig and norm are normalized */
    {
      t1 = *t;
      if(t1->type <= POLYGON) k = t1->mat; else k = 136;
      vt1 = (t1->f)[0];
      lig[0] = vt1[3];   lig[1] = vt1[4];  lig[2] = vt1[5];
      multvector3(lig1, lig, tmpa);
      val = ligp[0]* lig1[0] + ligp[1]* lig1[1] +ligp[2]*lig1[2];
      t1->color = k + (int)(59.5* (val+1.0));
      t += incr;
    }
}
/*******************************************************************/

void minvert(mat, result,w) int w; /* w = 0 give the inverse */
     Matrix mat, result;           /* w = 1 for inverse transpose */
{
        int i, j, k;
        double temp;
        double m[8][4];
	
        identmatrix(result);
        for (i = 0;  i < 4;  i++) {
                for (j = 0;  j < 4;  j++) {
                        m[i][j] = mat[i][j];
                        m[i+4][j] = result[i][j];
                }
        }
        for (i = 0;  i < 4;  i++) {
                for (j = i;  (m[i][j] == 0.0) && (j < 4);  j++)
                                ;
                if (j == 4) {
                        fprintf (stderr, "error:  cannot do inverse matrix\n");
                        exit (2);
                } 
                else if (i != j) {
                        for (k = 0;  k < 8;  k++) {
                                temp = m[k][i];   
                                m[k][i] = m[k][j];   
                                m[k][j] = temp;
                        }
                }

                for (j = 7;  j >= i;  j--)
                        m[j][i] /= m[i][i];

                for (j = 0;  j < 4;  j++)
                        if (i != j)
                                for (k = 7;  k >= i;  k--)
                                        m[k][j] -= m[k][i] * m[i][j];
        }
	if(w)
	  {
	    for (i = 0;  i < 4;  i++)
	      for (j = 0;  j < 4;  j++)
		result[j][i] = m[i+4][j];
	  }
	else
	  {
	    for (i = 0;  i < 4;  i++)
	      for (j = 0;  j < 4;  j++)
		result[i][j] = m[i+4][j];
	  }
}


/***************************************************************/
void rot_about_v(v,angle,m)      /* assume that v != 0       */
     float v[3]; Matrix m;       /* m is the resulted matrix */
     float angle;          
{
  float a[3], b[3];
  float norm, n1, tmp,tmp1,sina, cosa;

  n1 = v[0]*v[0] + v[1] * v[1];
  if(v[0] != 0.0 || v[1] != 0.0)
    {
      a[0] = v[1]; a[1] = -v[0]; a[2]=0.0;
      norm =(float)sqrt((double) (n1 + v[2]*v[2]));
      v[0] /= norm; v[1] /= norm; v[2] /= norm;
      norm = (float)sqrt((double)n1);
      a[0] /= norm; a[1] /= norm;
      b[0] = - a[1] * v[2]; b[1] = a[0] * v[2]; b[2] = v[0] *a[1]- v[1]* a[0];
    }
  else 
    { v[2] = 1.0;
      a[0] = 1.0; a[1] = 0.0; a[2] = 0.0;
      b[1] = 0.0; b[1] = 1.0; b[2] = 0.0;
    }
  /* now v, a, b are orthonormal */
  cosa = cos(angle); sina = sin(angle);
  tmp = v[0]*v[0];
  m[0][0] = (1.0-tmp)*cosa + tmp; 
  tmp = v[1]*v[1];
  m[1][1] = (1.0-tmp)*cosa + tmp; 
  tmp = v[2]*v[2];
  m[2][2] = (1.0-tmp)*cosa + tmp; 

  tmp= v[0]*v[1] *( 1.0 - cosa);
  tmp1=  v[2] * sina; 
  m[0][1] = tmp + tmp1;
  m[1][0] = tmp - tmp1;
  tmp=  v[0]*v[2] *(1.0 - cosa);
  tmp1=  v[1] * sina; 
  m[0][2] = tmp - tmp1;
  m[2][0] = tmp + tmp1;
  tmp= v[2]*v[1] *(1.0 - cosa);
  tmp1= v[0] * sina; 
  m[1][2] = tmp + tmp1;
  m[2][1] = tmp - tmp1;

  m[0][3] = m[1][3] = m[2][3] = 0.0;
  m[3][0] = m[3][1] = m[3][2] = 0.0;
  m[3][3] = 1.0;
}

void updaterot(m)
     Matrix m;
{
  Matrix tmp;
  copymatrix(tmp,rotmatrix);
  mult4x4(rotmatrix,m,tmp);
}
/*************************************************************************/

#define SCRN    0
#define WLD     1
#define TRANS_ROT  0
#define TRANS_SCA  1
#define TRANS_SCAU 2
#define TRANS_TRA  3
#define ROTFAC 0.01

void setup_trans()
{
  float aix[3], factor;
  Matrix tmp;

  if( hsliderx - hslidox + vslidery - vslidoy + vsliderz - vslidoz == 0)
    return;
     
  if(bboxstat.mode == SCRN)
    {                             /* transformation in screen coor */
      switch(bboxstat.transmode)
	{
	case TRANS_ROT:
	  {                              /* screen rotation */
	    if(hsliderx != hslidox)
	      {
		copyvec3(aix, gview.yaxis);
		minvert(rotmatrix, tmp,0);
		multvector3(rotaxis,aix,tmp); 
		rot_about_v(rotaxis,(float)(hsliderx -hslidox)*ROTFAC,tmp);
		hslidox = hsliderx;
		updaterot(tmp);
	      }
	    if(vslidoy != vslidery)
	      {
		copyvec3(aix, gview.xaxis);
		minvert(rotmatrix, tmp,0);
		multvector3(rotaxis,aix,tmp);
		rot_about_v(rotaxis,(float)(vslidery- vslidoy)*ROTFAC, tmp);
		vslidoy = vslidery;
		updaterot(tmp);
	      }
	    if(vslidoz != vsliderz)
	      {
		copyvec3(aix, gview.zaxis);
		minvert(rotmatrix, tmp,0);
		multvector3(rotaxis,aix,tmp); 
		rot_about_v(rotaxis,(float)(vsliderz-vslidoz)*ROTFAC,tmp);
		vslidoz = vsliderz;
		updaterot(tmp);
	      }
	  }
	  break;
	case TRANS_TRA:
	  {                                  /* screen translation */
	    factor = sizze * 0.005;
	    if(hslidox != hsliderx)	      
	      {
		gview.xtranp += factor * (float)(hsliderx-hslidox);
		hslidox = hsliderx;
	      }
	    if(vslidoy != vslidery)	      
	      {
		gview.ytranp -= factor * (float)(vslidery-vslidoy);
		vslidoy = vslidery;
	      }
	    if(vslidoz != vsliderz)	      
	      {
		gview.ztranp += factor * (float)(vsliderz-vslidoz);
		vslidoz = vsliderz;
	      }
	  }
	  break;
	case TRANS_SCA:
	  {
	    if(hsliderx != hslidox)
	      {
		gview.xscals *= (float)exp(0.01*(float)(hsliderx-hslidox));
		hslidox = hsliderx;
	      }
	    if(vslidery != vslidoy)
	      {
		gview.yscals *= (float)exp(0.01*(float) (vslidery - vslidoy));
		vslidoy = vslidery;
	      }
	    if(vsliderz != vslidoz)
	      {
		gview.zscals *= (float)exp(0.01 *(float)(vsliderz-vslidoz));
		vslidoz = vsliderz;
	      }
	  }
	  break;
	default:
	  break;
	}
    }
  else if(bboxstat.mode == WLD)    
    {
      switch(bboxstat.transmode)
	{
	case TRANS_ROT:
	  if(hslidox != hsliderx)
	    {
	      rotaxis[0]=1.0; rotaxis[1] = 0.0; rotaxis[2]=0.0;
	      rot_about_v(rotaxis,(float)(hsliderx-hslidox)*ROTFAC,tmp);
	      hslidox = hsliderx;
	      updaterot(tmp);
	    }
	  if(vslidoy != vslidery)
	    {
	      rotaxis[0]=0.0; rotaxis[1] = 1.0; rotaxis[2]=0.0;
	      rot_about_v(rotaxis,(float)(vslidery-vslidoy)*ROTFAC,tmp);
	      vslidoy = vslidery;
	      updaterot(tmp);
	    }
	  if(vslidoz != vsliderz)
	    {
	      rotaxis[0]=0.0; rotaxis[1] = 0.0; rotaxis[2]=1.0;
	      rot_about_v(rotaxis,(float)(vsliderz-vslidoz)*ROTFAC,tmp);
	      vslidoz = vsliderz;
	      updaterot(tmp);
	    }
	  break;
	case TRANS_TRA:
	  {
	    float factor = sizze * 0.04;
	    if(hslidox != hsliderx)	      
	      {
		gview.xtrans += factor * (float)(hsliderx-hslidox);
		hslidox = hsliderx;
	      }
	    if(vslidoy != vslidery)	      
	      {
		gview.ytrans += factor * (float)(vslidery-vslidoy);
		vslidoy = vslidery;
	      }
	    if(vslidoz != vsliderz)	      
	      {
		gview.ztrans += factor * (float)(vsliderz-vslidoz);
		vslidoz = vsliderz;
	      }
	  }
	  break;
	case  TRANS_SCAU:
	  {
	    if( hsliderx != hslidox || vslidery != vslidoy || 
	       vsliderz != vslidoz)
	      {
		factor = (float) exp( 0.01 * (float) (hsliderx - hslidox + 
						      vslidery -vslidoy
						      + vsliderz - vslidoz) );
		gview.xscale *= factor;
		gview.yscale *= factor;
		gview.zscale *= factor;
		hslidox = hsliderx; vslidoy = vslidery; vslidoz = vsliderz;
	      }
	  }
	  break;
	case TRANS_SCA:
	  {
	    if(hsliderx != hslidox)
	      {
		gview.xscale *= (float) exp( 0.01 *(float)(hsliderx-hslidox));
		hslidox = hsliderx;
	      }
	    if(vslidery != vslidoy)
	      {
		gview.yscale *= (float) exp(0.01 * (float)(vslidery-vslidoy));
		vslidoy = vslidery;
	      }
	    if(vsliderz != vslidoz)
	      {
		gview.zscale *= (float)exp(0.01 *(float)(vsliderz - vslidoz));
		vslidoz = vsliderz;
	      }
	  }
	  break;
	default:
	  break;
	}
    }	   
}
/*************************************************************************/
void copyvec3(a,b)
     float *a,*b;
{
  a[0] = b[0]; a[1] = b[1]; a[2] = b[2];
}
/*************************************************************************/
void initialize_screen_axis()
{
  /*
   *  initialize  screen axis, assuming that twist = 0
   */
  if(gview.vx == 0.0 && gview.vz == 0.0)
    {
      gview.xaxis[0]= 1.0; gview.xaxis[1]= 0.0; gview.xaxis[2]=0.0;
      gview.yaxis[0]= 0.0; gview.yaxis[1]= 0.0; gview.yaxis[2]= -1.0;
      gview.zaxis[0]= 0.0; gview.zaxis[1]= 1.0; gview.zaxis[2]=0.0;
      return;
    }
  gview.xaxis[0] =  gview.vz;
  gview.xaxis[1] = 0.0;
  gview.xaxis[2] = -gview.vx;
  
  gview.yaxis[0] = - gview.vx * gview.vy;
  gview.yaxis[1] =  gview.vx * gview.vx + gview.vz * gview.vz;
  gview.yaxis[2] = - gview.vz * gview.vy;
  
  gview.zaxis[0] = gview.vx;
  gview.zaxis[1] = gview.vy;
  gview.zaxis[2] = gview.vz;
  normalize3(gview.xaxis);
  normalize3(gview.yaxis);
  normalize3(gview.zaxis);
}
/*************************************************************************/
void normalize3(v)
     float *v;
{
  float n = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
  v[0] /= n;  v[1] /= n;  v[2] /= n;
}
/*************************************************************************/
void pick_rotation(speed)
     float speed;
{
  float aix[4];
  Matrix tmp;
  copyvec3(aix, gview.caxis);
  minvert(rotmatrix, tmp,0);
  multvector3(rotaxis,aix,tmp); 
  rot_about_v(rotaxis,speed,tmp);
  updaterot(tmp);
}
void printmatrix(m)
     Matrix m;
{
  printf("%f %f %f %f\n",m[0][0],m[0][1],m[0][2],m[0][3]);
  printf("%f %f %f %f\n",m[1][0],m[1][1],m[1][2],m[1][3]);
  printf("%f %f %f %f\n",m[2][0],m[2][1],m[2][2],m[2][3]);
  printf("%f %f %f %f\n\n",m[3][0],m[3][1],m[3][2],m[3][3]);
}
/********************************************************************/
void arb_rot(ang) float ang;
{
  float aix[3];
  Matrix tmp;

  copyvec3(aix, gview.caxis);
  minvert(rotmatrix,tmp,0);
  multvector3(rotaxis,aix,tmp); 
  rot_about_v(rotaxis,ang,tmp);
  updaterot(tmp);
}
/********************************************************************/
/*
 * count the number of elts using each of the two possible materials
 */
void count_depthcue_elts()
{
  elt **t = elts, *t1;  
  int i, c, c1=0, c2=0;

  depthcue_elt_count[0] = 1.0;
  for(i = 0; i < nelts; i++)
    {
      t1 = *t;
      c = t1->color;
      if( c> 135) c2++;
      else c1++;
      t++;
    }
  if(c2>0) depthcue_elt_count[1] = 119.0/(float)c2;  
  if(c1>0) depthcue_elt_count[0] = 119.0/ (float)c1;
}
/****************************************************************/


  
  

