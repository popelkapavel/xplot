/*******************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include <grx20.h>

char *grx_wxh=0;
int grx_width=0,grx_height=0;

#include <math.h>
#include "vogl.h"
#include "vectfont.h"
#include "pvm.h"
/*******************************************************************/
#ifndef MAX
#define MAX(a,b) ( ((a)>=(b))? (a):(b))
#define MIN(a,b) ( ((a)>=(b))? (b):(a))
#endif
#ifndef ABS
#define ABS(x)  ( (x)>0.0? x: -(x))
#endif
#define LARGEFONT   "9x15bold"
#define SMALLFONT   "6x13bold"
#define CMAPSIZE    256
#define HSLIWIN     2
#define VSLIWIN     3
#define EV_MASK     KeyPressMask|ButtonReleaseMask|ExposureMask|\
                    ButtonPressMask|StructureNotifyMask|ButtonMotionMask

#define MAX_POPUP_STRING_LENGTH 32
/**********************************************************************/
extern char *malloc();
/**********************************************************************/
extern void save_GRX_device(), recover_GRX_device();
extern void set_initial_color_table(),make_color(), PSmake_color();
extern void use_new_material(),free_data(),make_materialn();
extern void show_picture(),rotate_box(), arb_rot(), rainbow_color();
extern void delete_all_contours();
extern void init_pvm(), count_depthcue_elts(), cpt_cnt();
extern int  title_is_picked(), do_save_setup(), do_load_setup();
/**********************************************************************/
void other_color(),draw_a_button(),check_button_box();
void pick_rotation(), setup_back_sliders(),reset_h_v_sliders();
void setup_mat_sliders(), top_window(),buttonbox(), destroy_subwin();
void draw_a_lable(), hslider(), vslider(), dialog(), canvas();
void draw_a_check(), draw_a_toggle(), update_button();
void set_up_dialog(),initialize_pspage(),draw_string();
void recover_line_single_color(), move_title(),newfonts();
int check_menu();
int pop_window();
void create_subwin(),  check_gagets();
void simple_hslider(), simple_vslider(), simple_vsliderz();
void get_initial_setup_from_file();
void remove_pendingevent(), change_title_color();
int  do_arbitary_motion(), toppm();
float get_angle();
/**********************************************************************/
extern  struct vdev    vdevice;
extern  short          RGBtable[256][3];
extern  float          sizze;
extern  int            nobjs, plot3d;
extern  int            cfonttype;
/**********************************************************************/
pssetup       pspage;
int           ps_grayscale = 0;
int           hsliderx, vslidery, vsliderz;
int           hslidox,  vslidoy,  vslidoz; /* old slider posi */

int           redraw = 1,num_cnts=10;
/**********************************************************************/
extern int       polymodeflag;
extern int       current_color_index;
extern int       not_zbuffered_wireframe;
extern int       wireframe_over_polygon, wireframe_color;
/**********************************************************************/
extern int        fixedlight, do_not_sort;
extern int        movingmode, slowmoving;
extern int        boxon, depthcue; 
extern int        lightedline, twosidedobj;
extern float      depthcue_elt_count[2],cnt_used;
/**********************************************************************/
static  char   *get_string_from_kbd();
static  int    get_out = 0, moving_title = 0;

static char    tmp_string[MAX_POPUP_STRING_LENGTH];
static char    psfile[32] = "xplot.ps",ppmfile[32]="xplot.ppm";
static char    setupfile[64] = "xplot.ini";
static char    cnt_func[64]="z";
/***********************************************************************/
static  GrContext       *winder;
static  GrContext       *theDrawable;

typedef struct {
  unsigned long foreground;
} GC;

static  GC              theGC, otherGC;
static  unsigned long   /* carray[CMAPSIZE],*/ colour;
/**********************************************************************/
/* static  char            *smallf, *largef; */
/***********************************************************************/
typedef GrContext *Drawable;
static  GrContext       *bbuf[8],*subwin[8];
static  int             back_used[8] = {1,1,1,1,1,1,1,1}; 
static  int             current_window = 0;     /* 0 or 1, for vogl graphics */

static unsigned int     width, height, owidth, oheight; /* window size */
/* static unsigned int     border_width = 1; */
static unsigned int     x, y, h, w;

static unsigned int     dim[8][4];          /* subwindow dimensions */
static int              motion_type;
/**************************************************************************/
#define BUTTON   1
#define HSLIDER  2
#define VSLIDER  3
#define LABLE    4
#define DIALOG   5
#define CANVAS   6
#define TOGGLE   7
#define CHECK    8

#define UP       0
#define DOWN     1

#define SCRN       0
#define WLD        1
#define TRANS_ROT  0
#define TRANS_SCA  1
#define TRANS_SCAU 2
#define TRANS_TRA  3

/**************************************************************************/
butboxstat bboxstat;

/**************************************************************************/
static char *mlist[] = {
  "x,y,z",        /* bottom, scale: 2 items    */
  "Uniform",
  "Perspective",  /* top, projection: 3 items,2--4 */
  "Othogonal",
  "Eye Point", 
  "Light posit",     /* top, light: 3 items, 5--7    */
  "Light color", 
  "Background",
  "Current",      /* top, material list: 18 items, 8--26*/
  "Silver", 
  "Shine Silver",
  "Gold",
  "Shine Gold",
  "Brass",
  "Copper",
  "Shine Copper",
  "Obsidian",
  "Ruby",
  "Turquoise",
  "Jade",
  "Emerald",
  "Blue Plastic",
  "Cyan Plastic",
  "Grn Plastic",
  "Red Plastic",
  "Pink Plastic",
  "Yew Plastic",
  "Postscript",   /* save image, 2 items 27--28*/
  "   ppm  ",
  "Drawing Mode",       /* misc commands, 1 item  29--  */
  "Rainbow Map",        /* 30 */
  "SaveLoadSetup",      /* 31 */
  "Default",            /* 32 */
  "Customize",          /* 33 */
};
typedef struct { short start, end; } menu;
static menu menus[] = {{2,4},{5,7},{8,26},{27,28},{29,31},{32,33},{0,1}};
#define SCALE_UNIFORM_SCALE 6 
/**************************************************************************/
typedef struct{ 
  int  type, state;       /* type, button, lable or slider; up or down  */
  int value,ovalue;       /* current value,                             */
  unsigned int x,y,w,h;   /* position and dimension                     */
  char *name;             /* name appear on button or lable             */
  int (*callback)();     
} gitem;
/*************************************************************************/
int No_op(),portho(),ppers(),grotx(),groty(),grotz(),geye(), gref();
int light_pos(), light_color(), mat_edit(), back_color();
int to_printer(), to_psfile(), to_bmp(), obj_trans();
int miscc(), save_setup(),set_contour();
/*************************************************************************/
static char redv[7], grnv[7], bluv[7], shinev[7],cntv[8]=" 10 ";
/*************************************************************************/
static gitem gitems[]=
{                                /* Orthogonal projection  12 ; 0--11*/
  {LABLE,0,0,0, 4, 5, 40, 20, "left", No_op},
  {LABLE,0,0,0, 4,48, 40, 20, "right",No_op},
  {LABLE,0,0,0, 4,91, 40, 20, " top", No_op},
  {LABLE,0,0,0, 4,134,40, 20, "bottom",No_op},
  {LABLE,0,0,0, 4,177,40, 20, "near",No_op},
  {LABLE,0,0,0, 4,220,40, 20, " far",No_op},
  {HSLIDER,0,60,60,50, 6, 120, 18, (char *)NULL,portho},
  {HSLIDER,0,60,60,50, 49, 120, 18, (char *)NULL,portho},
  {HSLIDER,0,60,60,50, 92, 120, 18, (char *)NULL,portho},
  {HSLIDER,0,60,60,50, 133,120, 18, (char *)NULL,portho},
  {HSLIDER,0,60,60,50,178, 120, 18, (char *)NULL,portho},
  {HSLIDER,0,60,60,50, 221, 120, 18, (char *)NULL,portho},
                           /* perspective projection 8; 12--19 */
  {LABLE,0,0,0,3, 14, 40, 20, "fovy",No_op},  
  {LABLE,0,0,0,46, 14, 40, 20, "aspect",No_op},  
  {LABLE,0,0,0,89, 14, 40, 20, "near",No_op},  
  {LABLE,0,0,0,132, 14, 40, 20, "far",No_op},  
  {VSLIDER,0,90,90, 15, 50, 18, 180, (char *)NULL,ppers},  
  {VSLIDER,0,90,90, 56, 50, 18, 180, (char *)NULL,ppers},  
  {VSLIDER,0,90,90, 97, 50, 18, 180, (char *)NULL,ppers},  
  {VSLIDER,0,90,90, 141, 50, 18, 180, (char *)NULL,ppers},  
                           /* view, eye position 6; 20--21 */
  {LABLE,0,0,0, 62,14, 50, 20, "radius",No_op},  
  {VSLIDER,0,90,90,78, 50, 18, 180, (char *)NULL,geye},  
                           /* light position, 4 items,22--25  */
  {LABLE,0,0,0, 36, 14, 50, 20, "Azim",No_op},  
  {LABLE,0,0,0, 102,14, 50, 20, "incl",No_op},  
  {VSLIDER,0,90,90, 42, 50, 18, 180, (char *)NULL,light_pos},  
  {VSLIDER,0,90,90, 108, 50, 18, 180, (char *)NULL,light_pos},  
                           /* light color, 6 items,26--31  */
  {LABLE,0,0,0, 11, 14, 50, 20, " Red",No_op},  
  {LABLE,0,0,0, 67,14, 50, 20, "Green",No_op},  
  {LABLE,0,0,0, 124,14, 50, 20, "Blue",No_op},  
  {VSLIDER,0,162,162,22, 50, 18, 180, (char *)NULL,light_color},  
  {VSLIDER,0,162,162,78, 50, 18, 180, (char *)NULL,light_color},  
  {VSLIDER,0,162,162, 134, 50, 18, 180, (char *)NULL,light_color},  
                           /* material editor, 17 items,  32--48 */
  {LABLE,0,0,0, 11, 30, 50, 20, " Red",No_op},  
  {LABLE,0,0,0, 67,30, 50, 20, "Green",No_op},  
  {LABLE,0,0,0, 124,30, 50, 20, "Blue",No_op},    
  {LABLE,0,0,0, 6, 190, 50, 20, redv,No_op},  
  {LABLE,0,0,0, 62,190, 50, 20, grnv,No_op},  
  {LABLE,0,0,0, 119,190,50, 20, bluv ,No_op},    
  {LABLE,1,0,0, 4,215, 50, 20, "Shine",No_op},      
  {LABLE,0,0,0, 4,225, 50, 20, shinev,No_op},      
#define AMB_M   40
#define DIF_M   41
#define SPE_M   42
#define MAT1_M  43
#define MAT2_M  44
  {BUTTON,0,0,0, 6, 9, 50, 17, "Ambient",mat_edit},  
  {BUTTON,0,0,0, 60,9, 50, 17, "Diffuse",mat_edit},  
  {BUTTON,1,0,0, 115,9, 54, 17, "Specular",mat_edit},  
  {CHECK,1,0,0, 20,250, 20, 17, "Mat 1",mat_edit},  
  {CHECK,0,0,0, 100,250, 20, 17, "Mat 2",mat_edit},

#define MAT_SLIDER 45
  {VSLIDER,0,122,122,22, 50, 18, 140, (char *)NULL,mat_edit},
  {VSLIDER,0,122,122,78, 50, 18, 140, (char *)NULL,mat_edit},
  {VSLIDER,0,122,122,134, 50, 18, 140, (char *)NULL,mat_edit},
  {HSLIDER,0,60,60, 44, 220, 120, 18, (char *)NULL,mat_edit},
                            /* background color, 6 items, 48--53 */
  {LABLE,0,0,0, 11, 14, 50, 20, " Red",No_op},  
  {LABLE,0,0,0, 67,14, 50, 20, "Green",No_op},  
  {LABLE,0,0,0, 124,14, 50, 20, "Blue",No_op},  
#define BACK_SLIDERS 52 
  {VSLIDER,0,18,18,22, 50, 18, 180, (char *)NULL,back_color},  
  {VSLIDER,0,18,18,78, 50, 18, 180, (char *)NULL,back_color},  
  {VSLIDER,0,18,18, 134, 50, 18, 180, (char *)NULL,back_color},  
                              /* To PS_File, 12 items, 55--66 */
#define PSFILE_START  55
#define PSUP    56
#define PSDOWN  57
#define PSLEFT  58
#define PSRIGHT 59
#define PSPORTRAIT 60
#define PSLANDSCAPE 61
#define PSOK 64
#define PSCOLOR 65
#define PSGRAY  66
  {CANVAS,0,0,0,37,32,102,132,(char*)0, to_psfile}, 
  {BUTTON,0,0,0,32,177,22,18,"/\\",to_psfile},  
  {BUTTON,0,0,0,62,177,22,18,"\\/",to_psfile},  
  {BUTTON,0,0,0,92,177,22,18,"<<",to_psfile},  
  {BUTTON,0,0,0,122,177,22,18,">>",to_psfile},  
  {BUTTON,1,0,0,7,248,60,20,"Portrait",to_psfile},
  {BUTTON,0,0,0,72,248,62,20,"Landscape",to_psfile},
  {LABLE,0,0,0,55,196,80,20,"Filename", No_op},
  {DIALOG,1,0,0, 8,215,160,25, psfile,to_psfile},
  {BUTTON,0,0,0,141,248,30,20," OK", to_psfile},
  {BUTTON,1,0,0,47,5,40,18,"color", to_psfile},
  {BUTTON,0,0,0,95,5,35,18,"gray", to_psfile},
                              /* to bmp file, 3 items, 67-69 */
#define PPMFILE_START  67
  {LABLE,0,0,0,55,64,80,20,"Filename", No_op},
  {BUTTON,0,0,0,70,130,30,20," OK", to_bmp},
  {DIALOG,1,0,0, 8,90,160,25, ppmfile,to_bmp},
                              /* Polygon modes, 10 items, 70-79 */
#define DRAWINGMODE  70           
  {CHECK,1,0,0,30,10,20,17,  " Filled Polygon",miscc},
  {CHECK,0,0,0,30,37,20,17,  " Wire over Poly",miscc},
  {CHECK,0,0,0,30,64,20,17,  " WireFrame Only",miscc},
  {CHECK,0,0,0,30,91,20,17, "  Hidden Wireframe",miscc},
  {TOGGLE,0,0,0,30,118,24,20, " Box on",miscc},
#define LIGHTSRCMODE 75
  {TOGGLE,1,0,0,30,145,24,20, " Fixed Light src",miscc},
  {TOGGLE,0,0,0,30,172,24,20, " Depthcue",miscc},
  {TOGGLE,1,0,0,30,199,24,20, " Two sided polygon",miscc},
#define LIGHTEDLINE 78
  {TOGGLE,0,0,0,30,226,24,20, " Lighted line",miscc},
  {BUTTON,0,0,0,60,250,70,20,  "Reset View",miscc},
#define SETUP_STUFF 80
  {LABLE,0,0,0,55,24,80,20,"Filename", No_op},
  {DIALOG,1,0,0, 8,50,160,25, setupfile, save_setup},
  {BUTTON,0,0,0,30,90,40,20,"Save", save_setup},
  {BUTTON,0,0,0,90,90,40,20,"Load", save_setup},
#define CONTOUR_START 84
  {LABLE,0,0,0,25,24,80,20,"Contour Function", No_op},
  {DIALOG,1,0,0,8,50,160,25,cnt_func, set_contour},  
  {LABLE,0,0,0,25,90,80,20,"Number of Contours", No_op},
  {BUTTON,0,0,0,25,120,30,20,  " <<",set_contour},
  {BUTTON,0,0,0,110,120,30,20,  " >>",set_contour},
  {BUTTON,0,0,0,65,120,35,20, cntv ,No_op},
  {BUTTON,0,0,0,63,160,40,20,  " Set",set_contour},
  {BUTTON,0,0,0,30,190,106,20,  " Delete Contours",set_contour},
};
/************************************************************************/
typedef struct{          /* the widget box, */
  short len;             /* number of items */
  gitem *items;          /* list of widgets */
} gaget;

static gaget gagets[] = {
  {12, &(gitems[0])},   /* ortho      0 */
  {8,  &(gitems[12])},  /* pers       1 */
  {2,  &(gitems[20])},  /* eye        2 */
  {4,  &(gitems[22])},  /* lig pos    3 */
  {6,  &(gitems[26])},  /* lig col    4 */
  {17, &(gitems[32])},  /* mat editor 5 */
  {6,  &(gitems[49])},  /* back color 6 */
  {12, &(gitems[55])},  /* to psfile  7 */
  {3,  &(gitems[67])},  /* to bitmap  8 */
  {10, &(gitems[70])},  /* misc       9 */
  {4,  &(gitems[80])},  /* setup      10 */
  {8,  &(gitems[84])},  /* contours   11 */
};

static short gaget_map[6][3] = { { 1, 0, 2},  /* pers, orth, eye            */
				 { 3, 4, 6},  /* lig_pos,color, background  */
				 { 5, 5, 5 }, /* mat editor                 */
				 { 7, 8, 7},  /* get PS or BMP file name    */
				 { 9, 9, 10}, /* save-load setup            */
			         { 9, 11,9}}; /* contour stuff              */
/**************************************************************************/
int wwhich=4 , wwhat=0 ;
/**************************************************************************/


void remove_pendingevent() {
}

void GrFillRect(int x,int y,int w,int h,int c) {
  GrFilledBox(x,y,x+w-1,y+h-1,c);
}

void GrFillArc(int x,int y,int w,int h,int c) {
  GrFilledEllipse(x+w/2,y+h/2,w/2,h/2,c);
}

int GRX_init()
{
  int i;
  int depth;
//  int wwhi,wwhat;

  /* 
   * Open the default display, get some basic info from the server.
   */
  redraw=1;
  if(grx_wxh) sscanf(grx_wxh,"%dx%d",&grx_width,&grx_height);
  if(grx_width==0||grx_height==0) grx_width=640,grx_height=480;
  GrSetMode(GR_width_height_color_graphics,grx_width,grx_height,256);
  grx_width=GrScreenX(),grx_height=GrScreenY();
  GrMouseInit();
  GrMouseSetColors(8,15);
  for(i=0;i<256;i++)
    GrSetColor(i,i,i,i);
  GrResetColors();
  GrFilledBox(0,0,grx_width-1,grx_height-1,5);
/*  for(i=0;i<256;i++)
    GrFreeCell(i);
  for(i=0;i<256;i++)
    GrAllocCell();*/
  depth = vdevice.depth = 8;

  /*
   * set up the color map.  Will use a private colormap later.
   * The first 15 colors will be reserved.
   */
   set_initial_color_table();
   for(i = 0; i <= 15; i++)
     GrSetColor(i,RGBtable[i][0],RGBtable[i][1],RGBtable[i][2]);

  width  = grx_width, height  = grx_height;     /* size window with enough room for text */
  owidth = grx_width, oheight = grx_height;


  create_subwin(width,height,1);

  x = 0;
  y = 0;
  w = grx_width;
  h = grx_height;

    vdevice.sizeX = vdevice.sizeY = MIN(h, w);
    vdevice.sizeSx = grx_width;
    vdevice.sizeSy = grx_height;
  /*
   * show all gagets.
   */
  wwhich=4,wwhat=0;
  (void)sprintf(cntv,"%3d",num_cnts);  cnt_func[0]='z'; cnt_func[1]='\0';
  simple_hslider();  simple_vslider(); simple_vsliderz();
  set_up_dialog(wwhich,wwhat);  top_window();   buttonbox();
  initialize_pspage();
  get_initial_setup_from_file();
  return(1);
}
/*********************************************************************/
static int mouse_window=-1;

int do_arbitary_motion(xi,yi)  int xi, yi;
{
  int xx,yy, butx[4], buty[4], cnt = 0;
  float ang,s,c;
  GrMouseEvent rep;
  butx[0] = xi; buty[0] = yi; cnt = 1;
  while(1)
    {
      GrMouseGetEvent(GR_M_BUTTON_DOWN|GR_M_MOTION|GR_M_BUTTON_UP,&rep);
      if((rep.buttons&(GR_M_LEFT|GR_M_RIGHT))&&(rep.flags&(GR_M_BUTTON_DOWN|GR_M_MOTION))) {
	  butx[cnt] = rep.x; buty[cnt] = rep.y; cnt++;
	  if(cnt > 1)
	    {
	      if(motion_type == 1)  /*rotation */
		{
		  yy= buty[cnt-1] -buty[0]; xx =  butx[cnt-1]-butx[0];
		  ang = get_angle( (float)yy, (float)xx);
		  if(plot3d) 
		    {
		      s = sin(ang); c = cos(ang);
		      gview.caxis[0] =gview.xaxis[0] * c + gview.yaxis[0] *s;
		      gview.caxis[1] =gview.xaxis[1] * c + gview.yaxis[1] *s;
		      gview.caxis[2] =gview.xaxis[2] * c + gview.yaxis[2] *s;
		      arb_rot(0.15);
		    }
		  else  /* for 2d plots, only the z rotation is allowed */
		    {
		      gview.caxis[0] =gview.zaxis[0]; 
		      gview.caxis[1] =gview.zaxis[1];
		      gview.caxis[2] =gview.zaxis[2];
		      arb_rot(yy >0? 0.1: -0.1); 
		    }
		}
	      else if( motion_type == 2)  /* translation */
		{
		  xx= butx[cnt-1] -butx[0]; yy = buty[0] - buty[cnt-1];
		  gview.xtranp +=  sizze * 0.005 * xx;
		  gview.ytranp +=  sizze * 0.005 * yy;
		}
	      else if(motion_type == 3) /*scaling */
		{
		  xx= butx[cnt-1] -butx[0]; 
		  ang = (float)exp(0.015 * (float)xx);	      
		  gview.xscale *=ang; gview.yscale *=ang; gview.zscale *=ang;
		}
	      else remove_pendingevent();
	      return(0);
	    }
       };
       if(rep.flags&GR_M_BUTTON_UP) {
	  movingmode = 0; redraw = 1;
	  remove_pendingevent();
	  return(1);
       }
   }
}
int find_window(int *wx,int *wy) {
  int i,x,y;
  for(i=0;i<8;i++) {
    x=*wx-dim[i][0];
    y=*wy-dim[i][1];
    if(x>=0&&y>=0&&x<dim[i][2]&&y<dim[i][3]) {
      *wx=x;
      *wy=y;
      return i;
    }
  }
  return 0;
}

/*********************************************************************/
int something()
{
  int i, j;
  GrMouseEvent report;
  int wi,wx,wy;

  GrMouseGetEvent(GR_M_BUTTON_DOWN|GR_M_MOTION|GR_M_BUTTON_UP,&report);
  if((report.buttons&(GR_M_LEFT|GR_M_RIGHT|GR_M_MIDDLE))|
     (report.flags&GR_M_BUTTON_DOWN)) {
      if(report.flags&GR_M_BUTTON_DOWN) {
        wx=report.x,wy=report.y;
        wi=mouse_window=find_window(&wx,&wy);
      } else {
        wi=mouse_window;
        if(wi>=0) wx=report.x-dim[wi][0],wy=report.y-dim[wi][1];
      }
      if(wi == 0)
	{ 
	  movingmode = 1;  redraw = 1;
	  i= wx;
	  j= wy;
	  if(report.flags & GR_M_BUTTON_DOWN)
	    {
	      moving_title = title_is_picked(i,(int)dim[0][3] - j);
	      motion_type  = 0; /* reset motion_type */
	      if( moving_title )
		{
		  int bt = report.buttons;
		  if(bt & GR_M_LEFT)
		    { 
		      change_title_color(0); /* erase the current  title */
		      if(cfonttype == 0) cfonttype =1;   else cfonttype = 0; 
		      font(cfonttype); 
		      change_title_color(1); /* redraw the title */
		      redraw = 0;  moving_title = 0;
		    }
		  else if(bt & GR_M_RIGHT)
		    {
		      titlecolor=(titlecolor+1)% 7 + 9;
		      change_title_color(1);
		      redraw = 0;  moving_title = 0;
		      /* don't have to redraw the whole thing 
		       * just change the color of the title 
		       */
		    }
		}
	      else
		{
		  int bt = report.buttons;
		  if((bt & GR_M_MIDDLE )||(bt & GR_M_RIGHT && bt & GR_M_LEFT))
		    motion_type = 3;
		  else if(bt & GR_M_LEFT) motion_type = 1;
		  else if(bt & GR_M_RIGHT)  motion_type = 2;
		  /* else if(bt & GR_M_MIDDLE)  motion_type = 3; */
		}
	      
	    }
	  if(moving_title)  move_title();
	  else if(motion_type) do_arbitary_motion(report.x,report.y);
	}
      else if(wi == 1)	{
	  rotate_box();
	}
      else if(wi == HSLIWIN)
	{   /* x-slider */  
	  i = wx;
	  if( i < 15) i = 15; 
	  else if( i > dim[HSLIWIN][2]-15) i = dim[HSLIWIN][2]-15;
	  hsliderx = i;  simple_hslider();
	  movingmode = 1;  redraw = 1;
	}
      else if(wi == VSLIWIN)
	{    /* y-slider */
	  i = wy;
	  if( i < 15) i = 15;
	  else if(i > dim[VSLIWIN][3]-15)  i =  dim[VSLIWIN][3]-15;
	  vslidery = i;	  simple_vslider(1);
	  movingmode = 1;  redraw = 1;
	}
      else if(wi == 6)
	{    /* z-slider */
	  i = wy;
	  if( i < 15) i = 15;
	  else if( i > dim[6][3]-15)  i = dim[6][3]-15;
	  vsliderz = i;  simple_vsliderz(1);
	  movingmode = 1;   redraw = 1;
	}
      else if(wi == 4)
	{   /* top menu */
	  int xx,yy,j;
	  j = check_menu(wx);
	  if(j != 0)  
	    {
	      xx = wx; yy = wy;
	      wwhich = j-1;  wwhat = pop_window(xx,yy,wwhich);
	      if(wwhich == 0)     /* projections: pers, ortho, eye */
		{
		  if(wwhat == 0)  /* perspective */
		    {
		      if(gview.proj_type ==ORTHOGONAL)
			{
			  gview.proj_type = PERSPECTIVE;
			  gview.proj_change=1; redraw=1;
			}
		    }
		  else if(wwhat == 1) /* orthogonal */
		    {
		      if(gview.proj_type == PERSPECTIVE)
			{
			  gview.proj_type =ORTHOGONAL;
			  gview.proj_change=1; redraw=1;
			}
		    }
		}                                /* back ground color */
	      else if(wwhich == 1 && wwhat == 2) setup_back_sliders();
	      else if(wwhich == 2)               /* material list */
		{  
		  if(wwhat > 0)  use_new_material(wwhat-1); wwhat = 0;
		  gitems[AMB_M].state = 0; gitems[DIF_M].state = 0;
		  gitems[SPE_M].state = 1; setup_mat_sliders(3);
		}
	      else if(wwhich == 4)
		{ if(wwhat == 1) {rainbow_color(); wwhat = 0;}}/*rainbow cmap*/
	      else if(wwhich == 5 && wwhat == 0)
		cpt_cnt(cnt_func,num_cnts);
	      if(wwhat >= 0) set_up_dialog(wwhich,wwhat);
	    }
	}
      else if(wi == 5)     /* gagets */
	{
	  int xx,yy;  
	  xx = wx;  yy = wy;
	  check_gagets(wwhich,wwhat,xx,yy,report.flags&GR_M_BUTTON_DOWN);
	  set_up_dialog(wwhich,wwhat);

	}
      else if(wi == 7)	{
	  check_button_box(wx,wy);
	}
    };
    if(report.flags&GR_M_BUTTON_UP) {
      movingmode = 0;
      remove_pendingevent();
    }
/*    case ConfigureNotify:
      width = report.xconfigure.width;
      height = report.xconfigure.height;
      if(owidth != width || oheight != height)
	{
	  redraw = 1; movingmode = 0;
	  destroy_subwin();
	  create_subwin(width,height,0);
	  oheight=height; owidth=width;
	  simple_vslider();  simple_hslider(); simple_vsliderz();
	  set_up_dialog(wwhich,wwhat); top_window();   buttonbox();
	}
      break;
    case MapNotify:
      redraw = 1; movingmode = 0;
      simple_vslider();  simple_hslider(); simple_vsliderz();
      set_up_dialog(wwhich,wwhat); top_window();   buttonbox();      
      remove_pendingevent();
      break;
    case Expose: default: 
      remove_pendingevent();
      break;
    }        */
  if(get_out)
    {
      free_data();
      gexit();
      return(0);
    }
  return(2);
}
/****************************************************************/

void buttonbox()
{
  unsigned int w =  dim[7][2], h = dim[7][3];
  Drawable tmp=bbuf[7];

  GrSetContext(tmp);
  GrFillRect(0,0,w,h,2);
  GrFillRect(6,0,102,25,11);
  GrFillRect(116,0,150,25,11);
  GrLine(107,13,116,13,11);
  GrLine(107,14,116,14,11);
  GrLine(111,10,116,13,11);
  GrLine(111,18,116,14,11);

  draw_a_button(tmp, 10,4,45,17,"Screen",bboxstat.mode==SCRN);
  draw_a_button(tmp, 58,4,45,17,"World",bboxstat.mode==WLD);
  draw_a_button(tmp, 120,4,45,17,"Rotate",bboxstat.transmode==TRANS_ROT);
  draw_a_button(tmp, 168,4,45,17,"Transl",bboxstat.transmode==TRANS_TRA);
  draw_a_button(tmp, 215,4,45,17,"Scale",bboxstat.transmode==TRANS_SCA ||
		bboxstat.transmode==TRANS_SCAU);

  draw_a_button(tmp, 280,4,55,17,"C Slider", bboxstat.notused == CENTER_OS);
  draw_a_button(tmp, 340,4,45,17," Fast",  !slowmoving);
  draw_a_button(tmp, 390,4,45,17," Quit",  bboxstat.notused == GET_OUT);
  bboxstat.notused = 0;
  GrBitBlt(subwin[7],0,0,bbuf[7],0,0,w-1,h-1,0);
}

void check_button_box(x,y)
     int x,y;
{
  if(x >= 10 && x <= 55) 
    {
      bboxstat.mode = SCRN;
      if(bboxstat.transmode == TRANS_SCAU)
	bboxstat.transmode = TRANS_SCA;
    }
  else if(x >= 58 && x <=103) {bboxstat.mode = WLD;}
  else if(x >= 120 && x <=165) {bboxstat.transmode = TRANS_ROT;}
  else if(x >= 168 && x <=213) {bboxstat.transmode = TRANS_TRA;}
  else if(x >= 215 && x <= 260)
    {
      if(bboxstat.mode == SCRN) bboxstat.transmode = TRANS_SCA;
      else
	{
	  int t = pop_window(x-20,y+dim[7][1]-50,SCALE_UNIFORM_SCALE);
	  if( t != 0)  bboxstat.transmode = TRANS_SCAU;
	  else  bboxstat.transmode = TRANS_SCA;
	}
    }
  else if(x >= 280 && x <=335)
    { bboxstat.notused = CENTER_OS; buttonbox(); 
      reset_h_v_sliders();  bboxstat.notused = -1; }
  else if(x >= 340 && x <=385)
    {  slowmoving = !slowmoving; buttonbox(); 
     }
  else if(x >= 390 && x <=435)
    {  bboxstat.notused = GET_OUT; buttonbox(); get_out = 1;  }
  buttonbox();
}

void reset_h_v_sliders()
{
  hsliderx = hslidox = dim[2][2]/2;
  vslidery = vslidoy = dim[3][3]/2;
  vsliderz = vslidoz = dim[6][3]/2;
  simple_hslider();
  simple_vslider();
  simple_vsliderz();
}

/****************************************************************/
void set_up_dialog(which,what) int which, what; 
{
  unsigned int w = dim[5][2], h = dim[5][3];
  GrContext *tmp=bbuf[5];
  
  GrSetContext(tmp);
  GrFillRect(0,0,w,h,3);
  GrLine(1,1,w,1,1);
  GrLine(1,1,1,h,1);
  GrLine(1,h-1,w,h-1,5);
  GrLine(w-1,1,w-1,h,5);
  {
    gaget *tg = &(gagets[gaget_map[which][what]]);
    gitem tgi;
    int i, len= tg->len;
    
    for (i = 0; i < len; i++)
      {
	tgi = tg->items[i];
	switch(tgi.type)
	  {
	  case LABLE:
	    draw_a_lable(tmp,tgi.x,tgi.y, tgi.w,tgi.h, tgi.name,tgi.state);
	    break;
	  case BUTTON:
	    draw_a_button(tmp,tgi.x,tgi.y, tgi.w,tgi.h, 
			  tgi.name, tgi.state);
	    break;
	  case VSLIDER:
	    vslider(tmp, tgi.x,tgi.y,tgi.w,tgi.h, (int *)&(tgi.value));
	    break;
	  case HSLIDER:
	    hslider(tmp, tgi.x,tgi.y,tgi.w,tgi.h, (int *)&(tgi.value));
	    break;
	  case DIALOG:
	    dialog(tmp, tgi.x,tgi.y,tgi.w,tgi.h, tgi.name);
	    break;
	  case CHECK:
	    draw_a_check(tmp,tgi.x,tgi.y,tgi.w,tgi.h, tgi.name, tgi.state);
	    break;
	  case TOGGLE:
	    draw_a_toggle(tmp,tgi.x,tgi.y,tgi.w,tgi.h, tgi.name, tgi.state);
	    break;
	  case CANVAS:
	    canvas(tmp,tgi.x,tgi.y,tgi.w,tgi.h, tgi.name);
	    break;
	  default:
	    break;
	  }
      }
  }
  GrBitBlt(subwin[5],0,0,tmp,0,0,w,h,0);
//  GrSetContext(GrScreenContext());
//  XCopyArea(display, tmp, subwin[5],otherGC,0,0, w+1, h+1,0,0);
}
/**********************************************************************/
void check_gagets(which,what,xx,yy,down)
     int which,what,xx,yy,down;
{
  gaget *tg = &(gagets[gaget_map[which][what]]);
  gitem *tgi;
  int i, len= tg->len;
  static int on_item=-1;

  if(down||on_item<0) {
    on_item=-1;
    for (i = 0; i < len; i++)
      {
        tgi = &(tg->items[i]);
        if( xx >= tgi->x && xx <= tgi->x + tgi->w &&
    	   yy  >= tgi->y && yy <= tgi->y + tgi->h)
	  on_item = i;
        continue;
      }
  }
  if( on_item >= 0)
    {
      tgi = &(tg->items[on_item]);
      switch(tgi->type)
	{
	case BUTTON:
	case TOGGLE:
	case CHECK:
	  movingmode = 1;
	  (*(tgi->callback))(on_item,0,1);
	  break;
	case VSLIDER:
	  movingmode = 1;
	  i = yy - tgi->y;
	  if(i < 18) i= 18; else if(i > tgi->h -18) i = tgi->h -18;
	  (*(tgi->callback))(on_item,i, tgi->ovalue);
	  tgi->value = tgi->ovalue = i;
	  break;
	case HSLIDER:
	  movingmode = 1;
	  i = xx - tgi->x;
	  if(i < 18) i= 18; else if(i > tgi->w -18) i = tgi->w -18;
	  (*(tgi->callback))(on_item,i,tgi->ovalue);
	  tgi->value = tgi->ovalue= i;
	  break;
	case DIALOG:
	  (*(tgi->callback))(on_item,0,1);
	  (void) get_string_from_kbd(tgi);
	  (*(tgi->callback))(on_item,0,2);
	  break;
	case CANVAS:
	  (*(tgi->callback))(on_item,xx,yy);
	  break;
	case LABLE:
	default:
	  break;
	}
    }
}
/*******************************************************************/
void canvas(thewin, xo,yo,wo,ho, name)
     Drawable thewin; int xo,yo,wo,ho; char *name;
{ 
  int x,y,w,h, bh;
  Drawable tmp = thewin;               /* factor 1.2 */
  pssetup *s = &pspage;

  bh = gitems[PSFILE_START].y + gitems[PSFILE_START].h;
  x = s->x + xo; y = bh -(s->y);
  w = x + s->w;  h = y - s->h;
  GrSetContext(tmp);
  GrFillRect(xo,yo,wo,ho,5);
  
  GrLine(x,y,w,y,1);
  GrLine(w,y,w,h,1);
  GrLine(w,h,x,h,1);
  GrLine(x,h,x,y,1);
  GrLine(x,y,w,h,1);
  GrLine(w,y,x,h,1);
}

void dialog(thewin, x,y,w,h, name)
     Drawable thewin; int x,y,w,h; char *name;
{
  Drawable tmp = thewin;
  int tx = x-2,ty = y-2, tw = w+4, th = h+4;

  GrSetContext(thewin);
  GrFillRect(tx,ty,tw,th,5);


  GrLine(tx,ty,  tx+tw,ty,1);
  GrLine(tx,ty,  tx,ty+th,1);
  GrLine(tx,ty+1,  tx+tw,ty+1,1);
  GrLine(tx+1,ty,  tx+1,ty+th,1);
  GrLine(x,y+h,  x+w,y+h,6);
  GrLine(x+w,y,  x+w,y+h,6);
  draw_string(tmp,x+2,y+17,name,1);  
}
/**********************************************************************/

char *get_string_from_kbd(g)
     gitem *g;
{
//  GrContext         popwin;
  GrMouseEvent         report;
  int            length;

//  GrCreateSubContext(g->x,g->y,g->x+g->w,g->y+g->h,subwin[5],&popwin);
  draw_string(subwin[5],g->x+2,g->y+17,tmp_string,0);
  
  while(1)
    {
      GrMouseGetEvent(GR_M_KEYPRESS,&report);
      if(report.flags&GR_M_KEYPRESS) {
              length=strlen(tmp_string);
         if(report.key==13) break;
         if(report.key>=32&&report.key<127) {
              if(length<MAX_POPUP_STRING_LENGTH-1) {
                tmp_string[length]=report.key;
                tmp_string[length+1]=0;
              }
	  }
          if(report.key==8) {
            if(length>0) tmp_string[length-1]=0;
          }
          GrSetContext(subwin[5]);
          GrFillRect(g->x,g->y,g->w,g->h,5);
	  draw_string(subwin[5],g->x+2,g->y+17,tmp_string,0);
       }
    }
   return tmp_string;
}
/*******************************************************************/
int to_psfile(n,i,b) int n,i,b;
{
  pssetup *s = &pspage;
  int WW,HH; 

  gitems[PSUP].state = UP;
  gitems[PSDOWN].state = UP;
  gitems[PSLEFT].state = UP;
  gitems[PSRIGHT].state = UP;
  if(b == 1) (void)strcpy(tmp_string, psfile);

  WW= s->filemode == PORTRAIT? 102: 132;
  HH = s->filemode == PORTRAIT? 132: 102;

  switch(n)
    {
    case 0: /* canvas */
      {
	s->x =  (i -  gitems[PSFILE_START].x) - s->w/2;
	s->y = (gitems[PSFILE_START].y +
		gitems[PSFILE_START].h - b) - s->h/2;
 
	if(s->x + s->w > WW) s->x = WW  - s->w;
	if(s->y + s->h > HH) s->y = HH - s->h;

	if(s->x < 0) s->x = 0;
	if(s->y < 0) s->y = 0;
      }
      break;
    case 1:
      gitems[PSUP].state = DOWN;
      s->h = (int)  ( (float) (s->h) * 1.05);
      if( s->h + s->y > HH) s->h = HH - s->y;
      break;
    case 2:
      gitems[PSDOWN].state = DOWN;
      s->h = (int) ( (float) (s->h) * 0.95);
      if(s->h < 20) s->h = 20;
      break;
    case 3:
      gitems[PSLEFT].state = DOWN;
      s->w = ( (float) (s->w) * 0.95);
      if(s->w < 20) s->w = 20;
      break;
    case 4:
      gitems[PSRIGHT].state = DOWN;
      s->w=  ( (float) (s->w) * 1.05);
      if(s->x + s->w > WW) s->w = WW - s->x;
      break;
    case 5: /* portrait */
      if(s->filemode == LANDSCAPE)
	{
	  int k = s->x;
	  s->x = s->y; s->y = k;
	  k = s->w; s->w = s->h; s->h = k;

	  gitems[PSFILE_START].x = 37;
	  gitems[PSFILE_START].y = 32;
	  gitems[PSFILE_START].w = 102;
	  gitems[PSFILE_START].h = 132;
	  s->filemode = PORTRAIT;
	}
      gitems[PSPORTRAIT].state = DOWN;
      gitems[PSLANDSCAPE].state = UP;

      break;
    case 6: /* landscape */
      if(s->filemode == PORTRAIT)
	{
	  int k = s->x;
	  s->x = s->y; s->y = k;
	  k = s->w; s->w = s->h; s->h = k;

	  gitems[PSFILE_START].x = 19;
	  gitems[PSFILE_START].y = 63;
	  gitems[PSFILE_START].w = 132;
	  gitems[PSFILE_START].h = 102;
	  s->filemode = LANDSCAPE;
	}
      gitems[PSPORTRAIT].state = UP;
      gitems[PSLANDSCAPE].state = DOWN;
      break;
    case 8: /* get file name */
      (void) strcpy(psfile, tmp_string);
      break;
    case 9:  /* ok, go save to file */
      gitems[PSOK].state = DOWN;
      set_up_dialog(wwhich,wwhat);
      save_GRX_device();
      voutput(psfile);
      vinit("postscript");
      ginit();
      font(cfonttype);
      PSmake_color();
      show_picture(1);
      gexit();
      gitems[PSOK].state = UP;
      recover_GRX_device();
      set_up_dialog(wwhich,wwhat);
      break;
    case 10:
      gitems[PSCOLOR].state = 1;  gitems[PSGRAY].state = 0; ps_grayscale = 0;
      break;
    case 11:
      gitems[PSCOLOR].state = 0; gitems[PSGRAY].state = 1;  ps_grayscale = 1;
      break;
    case 7:
    default:
      break;
    }
  return(0);
}
/************************************************************/
int to_bmp(n,i,b)   int n,i,b;
{
  if(b == 1)  (void)strcpy(tmp_string, ppmfile);
  switch(n)
    {
    case 1: /* button ok */
      gitems[PPMFILE_START+1].state = DOWN;
      set_up_dialog(wwhich,wwhat);
      if(toppm() == 0)
	fprintf(stderr,"Cannot save to ppmfile\n");
      gitems[PPMFILE_START+1].state = UP;
      set_up_dialog(wwhich,wwhat);
      break;
    case 2: 
      (void) strcpy(ppmfile, tmp_string);
      break;
    case 0:
    default:
      break;
    }
  return(0);
}
/*******************************************************************/
int pop_window(x,y,ind) 
     int ind; unsigned int x,y;
{
  GrContext tmp_win,back;
  Drawable       pmp;  
  int  yy, i, ans, oldans;
  int length = (int)(menus[ind].end -menus[ind].start+1);
  unsigned int ww,hh;

  ww = 90; hh = 24 * length+8;

  GrCreateSubContext(x,y,x+ww-1,y+hh-1,GrScreenContext(),&tmp_win);
  GrCreateContext(ww,hh,0,&back);
  GrBitBlt(&back,0,0,&tmp_win,0,0,ww,hh,0);

  pmp = &tmp_win;
  GrSetContext(pmp);
  GrFillRect(0,0,ww,hh,3);
  GrLine(0,0,ww,0,5);
  GrLine(1,1,ww,1,5);
  GrLine(0,0,0,hh,5);
  GrLine(1,1,1,hh-1,5);
  GrLine(ww,2,ww,hh,1);
  GrLine(ww-1,2,ww-1,hh,1);
  GrLine(ww,hh-1,ww,hh-1,1);
  GrLine(3,hh,ww,hh,1);

  for(i = 0; i < length; i++)
    {
      yy = menus[ind].start+i;
      draw_a_button(pmp,5, 5+i*24, 80,20, mlist[yy], 0);
    }
  ans = 0; oldans = -1;
  while(1)
    {
      GrMouseEvent report;
      GrMouseGetEvent(~0,&report);
      if(report.flags&(GR_M_BUTTON_DOWN|GR_M_MOTION)) {
	  yy = report.y-y;
	    {
	      ans = ((yy +5 )/ 24);
	      if(ans >= length) ans = length+1; if( ans < 0) ans = 0;
	      if( ans != oldans)
		{
		  if(oldans >=0)
		    { update_button(pmp,5, 5+oldans*24,80,20,0); }
		  update_button(pmp,5, 5+ans*24,80,20,1);
		  oldans = ans;
		}
	    }
        }
        if(report.flags&(GR_M_BUTTON_UP)) {

          GrBitBlt(&tmp_win,0,0,&back,0,0,ww,hh,0);
          GrDestroyContext(&back);
          GrDestroyContext(&tmp_win);

	  if(ans < 0 || ans >= length) return(-1);
	  return(ans);
        }
    }
}

/****************************************************************/
void simple_vslider()
{
  Drawable tmp = bbuf[VSLIWIN];
  unsigned int w = dim[VSLIWIN][2], h = dim[VSLIWIN][3];
  vslider(tmp, 0,0,w,h,&vslidery);
  GrBitBlt(subwin[VSLIWIN],0,0,tmp,0,0,w+1,h+1,0);
}

void simple_vsliderz() 
{
  Drawable tmp = bbuf[6];
  unsigned int w = dim[6][2], h = dim[6][3];
  vslider(tmp, 0,0,w,h,&vsliderz);
  GrBitBlt(subwin[6],0,0,tmp,0,0,w+1,h+1,0);
}

void simple_hslider() 
{
  Drawable tmp = bbuf[2];
  unsigned int w = dim[2][2], h = dim[2][3];
  hslider(tmp, 0,0,w,h,&hsliderx);
  GrBitBlt(subwin[HSLIWIN],0,0,tmp,0,0,w+1,h+1,0);
}
/*******************************************************************/
void hslider(thewin, x,y,w,h,pos) 
     Drawable thewin; unsigned int x,y,w,h; int *pos;
{
  Drawable tmp = thewin;

  GrSetContext(tmp);
  GrFillRect(x,y,w,h,3);
  GrFillRect(x+*pos-15,y+5, 30,10,4);

  GrLine( x+2,y+2,  x+w-2,y+2,1);
  GrLine( x+2,y+2,  x+2,y+h-2,1);
  GrLine( x+ *pos-15,y+h-4,  x+ *pos+15,y+h-4,1);
  GrLine( x+ *pos+15,y+5, x+ *pos+15,y+h-4,1);
  GrLine( x+ *pos,y+5,x+ *pos,y+h-5,1);
  GrLine( x+2,y+h-2, x+w-2,y+h-2,5);
  GrLine( x+w-2 ,y+2, x+ w-2 ,y+h-2,5);
  GrLine( x+ *pos-15,y+4,x+ *pos+14,y+4,5);
  GrLine( x+ *pos-15,y+4, x+ *pos-15,y+h-5,5);
  GrLine( x+ *pos+1,y+5,x+ *pos+1,y+h-5,5);
}

void vslider(thewin, x,y,w,h,pos) 
     Drawable thewin; unsigned int x,y,w,h; int *pos;
{
  Drawable tmp = thewin;
  GrSetContext(tmp);
  GrFillRect(x,y,w,h,3);
  GrFillRect(x+5, y+ *pos-15, 10,30,4);
  GrLine( x+2,y+2,x+w-2,y+2,1);
  GrLine( x+2,y+2,x+2,y+h-2,1);
  GrLine( x+4,y+ *pos+15,x+ w-4,y+ *pos+15,1);
  GrLine( x+w-4,y+ *pos-15,x+ w-4,y+ *pos+15,1);
  GrLine( x+5,y+ *pos,x+ w-5,y+ *pos,1);

  GrLine(x+2, y+ h-2,x+ w-2,y+ h-2,5);
  GrLine(x+w-2 ,y+2,  x+ w-2 ,y+h-2,5);
  GrLine(x+4,y+ *pos-15,x+ w-5,y+ *pos-15,5);
  GrLine(x+4,y+ *pos-15,x+ 4,y+ *pos+15,5);
  GrLine(x+5,y+ *pos+1,x+ w-5,y+ *pos+1,5);
}

/*******************************************************************/
static char *top_items[] = {
  "Projection", "  Light","Materials", "SaveImage"," Misc", "Contour" };

void top_window()
{
  int i, w = dim[4][2], h = dim[4][3];
  Drawable tmp = bbuf[4];

  GrSetContext(tmp);
  GrFillRect(0,0,w,h,6);

  for(i = 0; i < 6; i++)
    draw_a_lable(tmp,70*i+10,0,70,18, top_items[i],0);
  GrBitBlt(subwin[4],0,0,tmp,0,0,w+1,h+1,0);
}
/*******************************************************************/
int check_menu(x) unsigned int x;
{
  if( x < 70) return(1);
  if( x <140) return(2);
  if( x <210) return(3);
  if( x <280) return(4);
  if( x <350) return(5);
  if( x <420) return(6);
  return(0);
}
/********************************************************************/
void draw_a_lable(win,x,y, w,h, name, mode)
     Drawable win; int x,y,w,h, mode;
     char *name;
{
  draw_string(win,x+5,y+14,name,mode);
}

void draw_a_button(win,x,y, w,h, name, mode)
     Drawable win; int x,y,w,h, mode;
     char *name;
{
  int c;
  GrSetContext(win);
  GrFillRect(x,y,w,h,4);
  c=mode?5:1;
  GrLine(x+2, y+h, x+w, y+h,c);
  GrLine(x+w, y+1, x+w, y+h,c);
  GrLine(x+1, y+h-1, x+w, y+h-1,c);
  GrLine(x+w-1, y+1, x+w-1, y+h,c);

  c=mode?1:5;
  GrLine(x, y, x+w-2, y,c);
  GrLine(x, y+1, x+w-2, y+1,c);
  GrLine(x, y, x, y+h-2,c);
  GrLine(x+1, y, x+1, y+h-2,c);
  draw_string(win,x+5,y+13,name,mode);
}

void update_button(win,x,y, w,h,mode)
     Drawable win; int x,y,w,h, mode; 
{ int c;
  c=mode?6:1;
  GrLine( x+2, y+h, x+w, y+h,c);
  GrLine(x+w, y+1, x+w, y+h,c);
  GrLine(x+1, y+h-1, x+w, y+h-1,c);
  GrLine(x+w-1, y+1, x+w-1, y+h,c);
  c=mode?2:5;
  GrLine(x, y, x+w-2, y,c);
  GrLine(x, y+1, x+w-2, y+1,c);
  GrLine(x, y, x, y+h-2,c);
  GrLine(x+1, y, x+1, y+h-2,c);
}

/*******************************************************************/
void draw_a_check(win,x,y, w,h, name, mode)
     Drawable win; int x,y, w,h, mode;   char *name;
{
  int c;
  GrSetContext(win);
  draw_string(win,x+30,y+13,name,0);

  GrFillRect(x,y,w,h,4);

  GrLine(x+1, y+h, x+w, y+h,5);
  GrLine(x+w, y+1, x+w, y+h,5);

  GrLine(x, y, x+w, y,1);
  GrLine(x, y, x, y+h,1);
  if(mode)
    {
      if(mode == wireframe_color) 
	{
	  draw_string(win,x+30,y+13,name,wireframe_color);
	  c=wireframe_color;
	}
      else c=9;
      GrLine(x+5, y+8, x+9, y+13,c);
      GrLine(x+9, y+13, x+17, y+5,c);
    }
}

void draw_a_toggle(win,x,y, w,h, name, mode)
     Drawable win; int x,y, w,h, mode;   char *name;
{
  draw_string(win,x+30,y+13,name,0);

  GrSetContext(win);
  GrFillArc(x-1,y-1,w+1,h+1,1);
  GrFillArc(x,y,w,h,4);
  if(mode)    {
      int ww = w/2, hh= h/2, i = ww/2, j = hh/2;
      GrFillArc(x+i,y+j,ww,hh,3);
    }
}


/*******************************************************************
 *
 * Create 8 subwindows;
 *  0: the main drawing window,
 *  1: top right window, graphic status,
 *  2: horizontal slider,
 *  3: vertical slider,
 *  4: top window, for menus,
 *  5: bottom right, for menu icons etc.
 *  6: left vertical slider bar,
 *  7: the bottom buttonbox
 */

void create_subwin(w,h,n)
     int w,h,n;
{
  int i;
  
  dim[0][0] = 20;    dim[0][1]=23;   dim[0][2]= w -236; dim[0][3]=h-75;
  dim[1][0] = w-185; dim[1][1]=5;    dim[1][2]= 175;    dim[1][3]=175;
  dim[2][0] = 20;    dim[2][1]=h-51; dim[2][2]= w -236; dim[2][3]=20;
  dim[3][0] = w-215; dim[3][1]=23;   dim[3][2]= 20;     dim[3][3]=h-75;
  dim[4][0] = 0;     dim[4][1]=0;    dim[4][2]= w -195; dim[4][3]=20;
  dim[5][0] = w-185; dim[5][1]=195;  dim[5][2]= 175;    dim[5][3]=h-205;
  dim[6][0] = 0; dim[6][1]=23;   dim[6][2]= 20;    dim[6][3]=h-75;
  dim[7][0] = 0; dim[7][1]=h-28;   dim[7][2]= w-195;    dim[7][3]=25;

  if(n != 0)
    {
      vslidery = vslidoy = dim[3][3]/2;
      hsliderx = hslidox = dim[2][2]/2;
      vsliderz = vslidoz = dim[6][3]/2;
    }

  for(i=0; i<8; i++)
    { subwin[i]=GrCreateSubContext(dim[i][0],dim[i][1],
        dim[i][0]+dim[i][2],dim[i][1]+dim[i][3],GrScreenContext(),0);
      if (back_used[i]) {
            bbuf[i]=GrCreateContext(dim[i][2],dim[i][3],0,0);
	}
    }
  winder=subwin[0];
  if(back_used[current_window])
    theDrawable = (Drawable) bbuf[current_window];
  else
    theDrawable = (Drawable) subwin[current_window];
}

/*******************************************************************/
void destroy_subwin()
{
  int i; 
  for(i = 0; i < 8; i++)
    {
      if(back_used[i])  GrDestroyContext(bbuf[i]);
    }
}
/**********************************************************************/      
void GRX_color(ind)
     int     ind;
{
  colour = ind ;
  theGC.foreground=ind;
}

void other_color(ind) int ind;
{ 
  colour= ind;
  otherGC.foreground=ind;
}
/*******************************************************************/

int GRX_winset(i)
     int i;
{
  x = dim[i][0];
  y = dim[i][1];
  w = dim[i][2];
  h = dim[i][3];
  winder=subwin[i];
  theDrawable=bbuf[i];

  vdevice.sizeX = vdevice.sizeY = MIN(h, w);
  vdevice.sizeSx = w+1;
  vdevice.sizeSy = h+1;
  viewport((Screencoord)0, (Screencoord)vdevice.sizeSx -1,
	   (Screencoord)0, (Screencoord)vdevice.sizeSy -1 );
  return(0);
}
/************************************************************************/
int GRX_exit()
{

  destroy_subwin();
  GrSetMode(GR_default_text);
  
  get_out = 0;  moving_title = 0;
  depthcue_elt_count[0] = 0.0;
  return(1);
}
/************************************************************************/
int GRX_font(fontfile)
     char *fontfile;
{
  vdevice.hheight = 14;
  vdevice.hwidth = 8;

  return 1;     // how easy
}

/************************************************************************/
void GRX_draw(x, y)
     int x, y;
{
  GrSetContext(theDrawable);
  GrLine(vdevice.cpVx,vdevice.sizeSy-vdevice.cpVy,x,vdevice.sizeSy-y,theGC.foreground);
}

/************************************************************************/
void GRX_pnt(x, y)
     int x, y;
{
  GrSetContext(theDrawable);
  GrPlot(x,vdevice.sizeSy-y,theGC.foreground);
}

/************************************************************************/
void GRX_clear()
{
  GrSetContext(theDrawable);
  GrFilledBox(vdevice.minVx,vdevice.minVy,
    vdevice.maxVx,vdevice.maxVy,colour);
}
/************************************************************************/

void GRX_char(c)
     char c;
{ char c2[2]={c,0};
  GrSetContext(theDrawable);
  GrTextXY(vdevice.cpVx,(int)(vdevice.sizeSy - vdevice.cpVy),c2,theGC.foreground,0);
}
/*************************************************************************/
void GRX_string(s)
        char	s[];
{
  GrSetContext(theDrawable);
  GrTextXY(vdevice.cpVx, (int)(vdevice.sizeSy - vdevice.cpVy),s,theGC.foreground,0);
}

/*************************************************************************/
void GRX_fill(n, x, y)
     int n, x[], y[];
{  int plist[128][2];
  int	i;

  for (i = 0; i < n; i++) 
    {
      plist[i][0] = x[i];
      plist[i][1] = vdevice.sizeSy - y[i];
    }
  GrSetContext(theDrawable);
  GrFilledPolygon(n,plist,theGC.foreground);
  vdevice.cpVx = x[n-1];
  vdevice.cpVy = y[n-1];
}
/*************************************************************************/

int GRX_backbuf()
{
  if (!back_used[current_window]) 
    {
      bbuf[current_window] =
        GrCreateContext((unsigned int)vdevice.sizeSx,(unsigned int)vdevice.sizeSy,0,0);
    }

  theDrawable = (Drawable)bbuf[current_window];

  back_used[current_window] = 1;
  GrSetContext(theDrawable);
  return(1);
}

/*************************************************************************/
int GRX_swapbuf()
{
  GrBitBlt(winder,0,0,theDrawable,0,0,
    (unsigned int)vdevice.sizeSx, (unsigned int)vdevice.sizeSy,0);
  return 0;
}
/*************************************************************************/
void GRX_frontbuf()
{
  theDrawable = (Drawable)winder;
}

void GRX_SetColor(short i,short r,short g,short b) {
//  GrSetContext(GrScreenContext());
  GrSetColor(i,r,g,b);
}
/*************************************************************************/
int No_op()
{ return 0;
}
/*************************************************************************/
typedef int (*ifunction)();

static DevEntry GRXdev =
{ 
  "GRX",
  "large",
  "small",
  (ifunction)GRX_backbuf,
  (ifunction)GRX_char,
  (ifunction)No_op, /* X11_checkkey, */
  (ifunction)GRX_clear,
  (ifunction)GRX_color,
  (ifunction)GRX_draw,
  (ifunction)GRX_pnt,
  (ifunction)GRX_exit,
  (ifunction)GRX_fill,
  (ifunction)GRX_font,
  (ifunction)GRX_frontbuf,
  (ifunction)No_op, /*X11_getkey,*/
  (ifunction)GRX_init,
  (ifunction)No_op, /* X11_locator,*/
  (ifunction)GRX_SetColor,
  (ifunction)No_op, /* X11_setls,*/
  (ifunction)No_op, /* X11_setlw,*/
  (ifunction)GRX_string,
  (ifunction)GRX_swapbuf,
  (ifunction)No_op,
  (ifunction)GRX_winset,
};
/*************************************************************************/
void _GRX_devcpy()
{
  vdevice.dev = GRXdev;
}

/*************************************************************************/
void draw_string(win,x,y,str,color)
     Drawable win;
     int x,y,color;
     char *str;
{
  register int    i, j;
  int    ox, oy, nx, ny;
  int clr;

  GrSetContext(win);
  if(color) 
    clr=((color == wireframe_color? wireframe_color: 9));
  else clr=(8);
  ox = x; oy = y;
  while (*str) {
    if (vectfonttable[(int)(*str)][0][0]) {
      i = 0;
      while ((j = vectfonttable[(int)(*str)][i][0])) {
	switch (j) {
	case 3:
	  ox += vectfonttable[(int)(*str)][i][1];
	  oy -= vectfonttable[(int)(*str)][i][2];
	  break;
	case 2:
	  nx = ox + vectfonttable[(int)(*str)][i][1];
	  ny = oy -vectfonttable[(int)(*str)][i][2];
	  GrLine(ox, oy, nx,ny,clr);
	  ox = nx; oy = ny;
	  break;
	}
	i++;
      }
    }
    str++;
  }
}
/****************************************************************/
float get_angle(x,y)
     float x,y;
{
  float t, xx, yy;
  if(x==0.0)
    {
      if(y >0.0) return(1.570796327);
      else  return(-1.570796327);
    }
  xx = fabs(x); yy = fabs(y);
  t = atan( yy/xx);
  if(x >0.0) 
    {
      if(y >= 0) return(t);
      else return(-t);
    }
  else
    {
      if(y>=0) return(3.1415926535 - t);
      else return(3.1415926535 + t);
    }
}
/********************************************************************/  
extern mat_entry cmat[2];
extern int current_mat;
/*******************************************************************/
static float *being_edited;
/*******************************************************************/
int mat_edit(n,nx,ox)
     int n,nx,ox;
     /* n:  0--6, lable        */
     /*     7--11, button: amb, dif, spec, mat1, mat2 */
     /*     12--15, slider, r g b shine */
{
  float f;
  switch(n)
    {
    case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
      break;
    case 8:  /* ambient */
      gitems[AMB_M].state = 1;
      gitems[DIF_M].state = 0;
      gitems[SPE_M].state = 0;
      setup_mat_sliders(1);
      break;
    case 9:
      gitems[AMB_M].state = 0;
      gitems[DIF_M].state = 1;
      gitems[SPE_M].state = 0;
      setup_mat_sliders(2);
      break;
    case 10:
      gitems[AMB_M].state = 0;
      gitems[DIF_M].state = 0;
      gitems[SPE_M].state = 1;
      setup_mat_sliders(3);
      break;
    case 11:
      gitems[MAT1_M].state = 1;
      gitems[MAT2_M].state = 0;
      gitems[AMB_M].state = 0;
      gitems[DIF_M].state = 0;
      gitems[SPE_M].state = 1;
      current_mat = 0;
      setup_mat_sliders(3);
      break;
    case 12:
      gitems[MAT1_M].state = 0;
      gitems[MAT2_M].state = 1;
      gitems[AMB_M].state = 0;
      gitems[DIF_M].state = 0;
      gitems[SPE_M].state = 1;
      current_mat = 1;
      setup_mat_sliders(3);
      break;
    case 13:  /* red */
      f = 0.00962 *(float)(nx - 18);
      if(f > 1.0) f = 1.0;
      being_edited[0] = f;
      (void)sprintf(redv,"%1.4f",f);
      make_materialn();
      break;
    case 14:  /* grean */
      f = 0.00962 *(float)(nx - 18);
      if(f > 1.0) f = 1.0;
      being_edited[1] = f;
      (void)sprintf(grnv,"%1.4f",f);
      make_materialn();
      break;
    case 15:  /* blue */
      f = 0.00962 *(float)(nx - 18);
      if(f > 1.0) f = 01.0;
      being_edited[2] = f;
      (void)sprintf(bluv,"%1.4f",f);
      make_materialn();
      break;
    case 16:  /* red */
      cmat[current_mat].shininess = 0.015 *(float)((nx- 18)*(nx- 18));
      (void)sprintf(shinev, "%2.2f", cmat[current_mat].shininess);
      make_materialn();
      break;
    default:
      break;
    }  
  /*
   * do not need to redraw, just change the color map
   */
  return(0);
}
/*********************************************************************/
void setup_mat_sliders(i)
     int i; /* 1-amb, 2-dif, 3-spec */
{

  mat_entry *t = &(cmat[current_mat]);
  
  if( i == 1)      being_edited = (t->ambient);
  else if( i == 2) being_edited = (t->diffuse);
  else if( i == 3) being_edited = (t->specular);

  (void)sprintf(redv, "%1.4f", being_edited[0]);
  (void)sprintf(grnv, "%1.4f", being_edited[1]);
  (void)sprintf(bluv, "%1.4f", being_edited[2]);
  (void)sprintf(shinev, "%2.2f", t->shininess);
  
  gitems[MAT_SLIDER].value = 18+(int)(104.0* being_edited[0]);
  gitems[MAT_SLIDER+1].value =18+ (int)(104.0* being_edited[1]);
  gitems[MAT_SLIDER+2].value =18+ (int)(104.0* being_edited[2]);
  gitems[MAT_SLIDER+3].value =
    (int)( 18.0+ sqrt( 66.67* cmat[current_mat].shininess));
}

/*********************************************************************/

void setup_back_sliders()
{ 
  gitems[BACK_SLIDERS].value =   18 + (int) (0.4078 * (float)RGBtable[0][0]);
  gitems[BACK_SLIDERS+1].value = 18 + (int) (0.4078 * (float)RGBtable[0][1]);
  gitems[BACK_SLIDERS+2].value = 18 + (int) (0.4078 * (float)RGBtable[0][2]);
}
/*********************************************************************/

void initialize_pspage()
{
  pspage.filemode = PORTRAIT;
  pspage.background = BLACK;
  pspage.x = 12;             /* one scr pixel <-> 6 ps pixel */
  pspage.y = 12;             /* 6.5 x 9  <-> 78 x 108  */
  pspage.w = 78;
  pspage.h = 78;
}
/*********************************************************************/
int miscc(n,i,b) int n,i,b;
{  static int n2=0;
  switch(n)
    {
    case 0:
      polymodeflag = 1;  not_zbuffered_wireframe = 1;      
      wireframe_over_polygon = 0; do_not_sort = 0;
      if(gitems[DRAWINGMODE].state != 1) redraw = 1;
      gitems[DRAWINGMODE].state = 1;
      gitems[DRAWINGMODE+1].state = 0;
      gitems[DRAWINGMODE+2].state = 0;
      gitems[DRAWINGMODE+3].state = 0;
      break;
    case 1:
      if(n2==n)
        wireframe_color =  (wireframe_color+1)%8 + 8;
      polymodeflag = 1;   not_zbuffered_wireframe = 1;      
      wireframe_over_polygon = 1;  do_not_sort = 0;
      redraw = 1; /* always redraw here */
      gitems[DRAWINGMODE].state = 0;
      gitems[DRAWINGMODE+1].state = wireframe_color;
      gitems[DRAWINGMODE+2].state = 0;
      gitems[DRAWINGMODE+3].state = 0;  
      break;
    case 2:
      polymodeflag = 0; do_not_sort = 1;
      if(gitems[DRAWINGMODE+2].state != 1) redraw = 1;
      gitems[DRAWINGMODE].state = 0;
      gitems[DRAWINGMODE+1].state = 0;
      gitems[DRAWINGMODE+2].state = 1;
      gitems[DRAWINGMODE+3].state = 0;  
      break;
    case 3:
      polymodeflag = 1;  not_zbuffered_wireframe = 0;  do_not_sort = 0;
      if(gitems[DRAWINGMODE+3].state != 1) redraw = 1;
      gitems[DRAWINGMODE].state = 0;
      gitems[DRAWINGMODE+1].state = 0;
      gitems[DRAWINGMODE+2].state = 0;
      gitems[DRAWINGMODE+3].state = 1;  
      break;
    case 4:
      redraw = 1;
      boxon = !boxon;
      gitems[DRAWINGMODE+4].state = boxon;
      break;
    case 5:
      redraw = 1;   fixedlight = !fixedlight; depthcue = 0;
      gitems[LIGHTSRCMODE].state = fixedlight;  
      gitems[LIGHTSRCMODE+1].state = 0;  /*depthcue */
      break;
    case 6:
      if( !depthcue )
	{ /* do the counting only once */
	  if( cnt_used || depthcue_elt_count[0] == 0.0) 
	    count_depthcue_elts();
	  redraw = 1;
	}
      fixedlight = 0; depthcue = 1;
      gitems[LIGHTSRCMODE].state = 0;  
      gitems[LIGHTSRCMODE+1].state = 1;  
      break;
    case 7:
      if(!depthcue)
	{
	  redraw = 1;  twosidedobj = !twosidedobj;
	  gitems[LIGHTSRCMODE+2].state = twosidedobj;
	}
      break;
    case 8:
      if(!depthcue)
	{
	  redraw = 1; lightedline = !lightedline;
	  if(!lightedline) recover_line_single_color();
	  gitems[LIGHTEDLINE].state = lightedline;
	}
      break;

    case 9:
      gitems[LIGHTEDLINE+1].state = 1;
      /*
      if(bboxstat.mode == SCRN)
	{ if(gview.xtranp != 0. || gview.ytranp != 0. || gview.ztranp != 0.)
	    redraw = 1;
	  gview.xtranp = gview.ytranp = gview.ztranp = 0.0;  
	}
      else
	{ if(gview.xtrans != 0. || gview.ytrans != 0. || gview.ztrans != 0.)
	    redraw = 1;
	  gview.xtrans = gview.ytrans = gview.ztrans = 0.0;  
	}
	*/
      init_pvm(); redraw = 1;
      break;
    default:
      break;
    }
  if(n==9) 
    { 
      set_up_dialog(wwhich,wwhat); 
      gitems[LIGHTEDLINE+1].state = 0;
    }
  set_up_dialog(wwhich,wwhat);
  n2=n;
  return(0);
}
/******************************************************************/
#define XPLOTRC ".xplotrc"
/*extern char *strncpy(), *strcat(), getenv();*/

void get_initial_setup_from_file()
{
/*  FILE *fp, *fopen();
  char home[128];
  int i;
  
  if( (fp = fopen(XPLOTRC,"r")) == (FILE *)NULL)
    {
      (void) strcat(strncpy(home, getenv("HOME"), 128),"/");
      fp = fopen(strcat(home,XPLOTRC),"r");
      if(fp)
	{

	}
    }
*/
}
/******************************************************************/  
static void getcolors();
static int readwindow();
/********************************************************************/
int toppm() 
{
  short R[256], G[256], B[256];
  int *buf, dx, dy, dxdy, i, j, k, r;

  FILE *fp;
  
  dx = dim[0][2], dy = dim[0][3];  /* dimension of the main window */
  dxdy = dx*dy;
  
  if((buf = (int*) malloc(sizeof (int) * dxdy)) == NULL )
    { fprintf(stderr, "Out of memory.\n");   exit(1);   }

  r = readwindow(0, 0, dx-1, dy-1, buf);
  getcolors(0,255,R,G,B);

  if((fp = fopen(ppmfile, "w")) == NULL) 
    {
      fprintf(stderr, "gl2ppm: cannot open file");
      return(0);
    }

  fprintf(fp,"P6\n%d %d\n255\n",dx,dy); /* ppm header */
  for(j = dy-1; j >= 0; j--) for (k = 0; k < dx; k++) 
    {
      i = j * dx + k;
      putc( (char)R[buf[i]],fp);
      putc( (char)G[buf[i]],fp);
      putc( (char)B[buf[i]],fp);
    }
  fclose(fp); free(buf);
  return( r == dxdy );
}
/***********************************************************************/
void getcolors (ind1,ind2,r,g,b)
     short ind1, ind2, *r, *g, *b;
{
  short i;
  int n = ind2 - ind1 + 1;
  int r2,g2,b2;

  for(i = 0; i < n; i++)     {
      GrQueryColor(i+ind1,&r2,&g2,&b2);
      r[i]=r2;
      g[i]=g2;
      b[i]=b2;
    }
}
/***********************************************************************/
static int readwindow(x1,y1,x2,y2, data)
     short x1, y1, x2, y2;   int *data;
{
  unsigned long x, y;
  short  width  = x2-x1+1, height = y2-y1+1;

  GrSetContext(subwin[0]);
  for(y = height-1; y != -1UL; y--)
    {
      for(x=0; x < width; x++)
	*(data++) = (long) GrPixel(x, y);
    }
  x = width * height;
  return(x);
}
/***********************************************************************/
static float titledelta[2];

int title_is_picked(x,y)
     int x,y;
{
  int cx[2], htwidth;

  if(titleset == 0) return(0);
  htwidth =  (int)(strlen(titlestring) *vdevice.hwidth * 0.5);
  cx[0] = (int)( 0.5* (float) dim[0][2] *(titlepos[0]+1.0) + 0.5);
  cx[1] = (int)( 0.5* (float) dim[0][3] *(titlepos[1]+1.0) + 0.5);
  if(cx[0] - htwidth <= x &&  x <= cx[0] + htwidth &&
     cx[1] <= y && y <= (int)(cx[1] + vdevice.hheight) )
    {           /* pointer is on the text */
      titledelta[0] =  (float)(x+x)/ (float) dim[0][2] - 1.0;
      titledelta[1] =  (float)(y+y)/ (float) dim[0][3] - 1.0;
      titledelta[0] = titlepos[0] - titledelta[0] -1.0;
      titledelta[1] = titlepos[1] - titledelta[1] -1.0;
      return(1);
    }
  return(0);
}
/***********************************************************************/
void move_title(){
}
/***********************************************************************/
extern void draw_title_string(),winset();
void change_title_color(flag)
     int flag;
{
  winset(0); /* this is important, after each frame, the window is
	      * set to the status window 1
	      */
  theDrawable = (Drawable)subwin[0];  /* draw directly to the window. */
  if(flag) color(titlecolor);   else  color(0); 
  draw_title_string(titlestring);
  theDrawable = (Drawable) bbuf[0];   /* doing nothing but safe       */
}
/***********************************************************************/
void message(i,str) int i; char *str;
{
  GrSetContext(subwin[0]);
  if(i == 0) {
      GrFillRect(0,0,dim[0][2],dim[0][3],0);
    }
  GrTextXY(20,40*(i+2),str,9+i%7,0);
}

/***********************************************************************/
int save_setup(n,i,b)   int n,i,b;
{
  if(b == 1)  (void)strcpy(tmp_string, setupfile);
  switch(n)
    {
    case 2: /* button save */
      gitems[SETUP_STUFF+2].state = DOWN;
      set_up_dialog(wwhich,wwhat);
      if(do_save_setup(setupfile) == 0)
	(void) fprintf(stderr,"Cannot save to %s\n",setupfile);
      gitems[SETUP_STUFF+2].state = UP;
      set_up_dialog(wwhich,wwhat);
      break;
    case 3: /* button load */
      gitems[SETUP_STUFF+3].state = DOWN;
      set_up_dialog(wwhich,wwhat);
      if(do_load_setup(setupfile) == 0)
	(void) fprintf(stderr,"Cannot load from %s\n",setupfile);
      gitems[SETUP_STUFF+3].state = UP;
      set_up_dialog(wwhich,wwhat);
      break;
    case 1:
      (void) strcpy(setupfile, tmp_string);
      break;
    case 0:  default:
      break;
    }
  return(0);
}
/******************************************************************/
int set_contour(n,i,b)   int n,i,b;
{
  if(b == 1)  (void)strcpy(tmp_string, cnt_func);
  switch(n)
    {
    case 1: /* get the contour function */
      (void) strcpy(cnt_func, tmp_string);
      break;
    case 3: /* decrease num_cnt */
      gitems[CONTOUR_START+3].state = DOWN;
      set_up_dialog(wwhich,wwhat);
      num_cnts -= 1; if(num_cnts <0) num_cnts = 0;
      (void)sprintf(cntv,"%3d",num_cnts);
      gitems[CONTOUR_START+3].state = UP;
      set_up_dialog(wwhich,wwhat);
      break;
    case 4:
      gitems[CONTOUR_START+4].state = DOWN;
      set_up_dialog(wwhich,wwhat);
      num_cnts += 1; if(num_cnts >99) num_cnts =99;
      (void)sprintf(cntv,"%3d",num_cnts);
      gitems[CONTOUR_START+4].state = UP;
      set_up_dialog(wwhich,wwhat);
      break;
    case 6:
      /* find the contours */
      gitems[CONTOUR_START+6].state = DOWN;
      set_up_dialog(wwhich,wwhat);
      cpt_cnt(cnt_func,num_cnts);
      gitems[CONTOUR_START+6].state = UP;
      set_up_dialog(wwhich,wwhat);
      break;
    case 7:  /* delete all contours */
      gitems[CONTOUR_START+7].state = DOWN;
      set_up_dialog(wwhich,wwhat);
      delete_all_contours();
      gitems[CONTOUR_START+7].state = UP;
      set_up_dialog(wwhich,wwhat);
      break;
    default:
      break;
    }
  return(0);
}
/******************************************************************/

void setup_plotmodes(cbuf) char *cbuf;
{
  int aa[9], i;
  (void)sscanf(cbuf,"%d %d %d %d %d %d %d %d %d",
	       aa,aa+1,aa+2,aa+3,aa+4,aa+5,aa+6,aa+7,aa+8);
  for(i = DRAWINGMODE; i <= DRAWINGMODE+8; i++)
    gitems[i].state = aa[i-DRAWINGMODE];

  if(depthcue) if( depthcue_elt_count[0] == 0.0) count_depthcue_elts();
  set_up_dialog(wwhich,wwhat); 
}

void save_plotmodes(fp) FILE *fp;
{
  int  i;
  for(i = DRAWINGMODE; i <= DRAWINGMODE+8; i++)
    (void)fprintf(fp," %d", gitems[i].state);
  (void)fprintf(fp,"\n");
}

/******************************************************************/

