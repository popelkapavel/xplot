

/************************************************************************
 * 
 *  Code is modified from the code for Gnuplot by Zou Maorong
 */
/*
 *  G N U P L O T  --  plot.h
 *  Copyright (C) 1986, 1987  Thomas Williams, Colin Kelley
 *  You may use this code as you wish if credit is given and this message
 *  is retained.
 */
/****************************************************************************/

#define PROGRAM          "X P L O T"
#define SHELL            "/bin/sh"		
#define ZERO             1e-15		
#define TRUE             1
#define FALSE            0
#define Pi               3.141592653589793
#define MAX_LINE_LEN     2048
#define MAX_TOKENS       1024
#define MAX_ID_LEN       128
#define MAX_AT_LEN       600		
#define STACK_DEPTH      600
#define NO_CARET         (-1)
#define	IO_SUCCESS	 0
#define	IO_ERROR	 1

#include "const.h"

/*********************************************************************/

#define putcc            putc
#define END_OF_COMMAND   (c_token >= num_tokens || equals(c_token,";"))
#define NOMEMORY  int_error("not enough memory",NO_CARET)
#define memcpy(d,s,l)    bcopy(s,d,l)
#define top_of_stack     stack[s_p]


typedef int              BOOLEAN;
typedef int              (*FUNC_PTR)();

enum operators
{
  PUSH, PUSHC, PUSHD,PUSHD1,PUSHD2,PUSHD3,CALL,LNOT, BNOT, UMINUS, LOR, LAND, 
  BOR, XOR, BAND, EQ, NE, GT, LT, GE, LE, PLUS, MINUS, MULT, DIV, MOD, POWER,
  FACTORIAL,BOOL, JUMP, JUMPZ, JUMPNZ, JTERN, SF_START
};

#define is_jump(operator) ((operator) >=(int)JUMP && (operator) <(int)SF_START)

enum DATA_TYPES
{
  INT, CMPLX
};

struct cmplx 
{
  double real, imag;
};

struct value 
{
  enum DATA_TYPES type;
  union
    {
      int int_val;
      struct cmplx cmplx_val;
    } v;
};

struct lexical_unit
{	
  BOOLEAN is_token;              /* true if token, false if a value */ 
  struct value l_val;
  int start_index;               /* index of first char in token    */
  int length;                    /* length of token in chars        */
};

struct ft_entry 
{		                /* standard/internal function table entry */
  char *f_name;                 /* pointer to name of this function       */
  FUNC_PTR func;                /* address of function to call            */
};

struct udft_entry 
{                                  /* user-defined function table entry   */
  struct udft_entry     *next_udf; /* pointer to next udf in linked list  */
  char                  udf_name[MAX_ID_LEN+1]; /* name of function entry */
  struct at_type        *at;                    /* pointer to action table */
  char                  *definition;            /* definition of the func  */
  int                   n_arg;                  /* number of arguments     */
  struct value          dummy_value;            /* value of dummy variable */
  struct value          dummy_value1;           /* the 2nd dummy value */
  struct value          dummy_value2;           /* the 3nd dummy value */
  struct value          dummy_value3;           /* the 3nd dummy value */
};

struct udvt_entry 
{               		/* user-defined value table entry       */
  struct udvt_entry *next_udv;  /* pointer to next value in linked list */
  char udv_name[MAX_ID_LEN+1];  /* name of this value entry             */
  BOOLEAN udv_undef;            /* true if not defined yet              */
  struct value udv_value;       /* value it has                         */
};

union argument 
{	                		/* p-code argument */
  int j_arg;				/* offset for jump */
  struct value v_arg;		        /* constant value  */
  struct udvt_entry *udv_arg;           /* pointer to dummy variable */
  struct udft_entry *udf_arg;           /* pointer to udf to execute */
};

struct at_entry 
{                         /* action table entry       */
  enum operators index;	  /* index of p-code function */
  union argument arg;
};

struct at_type
{
  int a_count;				/* count of entries in .actions[] */
  struct at_entry actions[MAX_AT_LEN];
  /* will usually be less than MAX_AT_LEN is malloc()'d copy */
};

struct a_record
{
  int  index, erase;
  char *definition;
  struct a_record *next_record;
};

struct a_string
{
  char *name;
  char *contents;
  struct a_string *next;
};

/*
 *  User defined axes
 */
struct uaxes
{
  float ox,oy,oz;
  float xx,xy,xz,yx,yy,yz,zx,zy,zz;
  int ticmarks;
};

struct surface 
{
  struct udft_entry *fx,*fy,*fz,*fr1,*fr2;
  int    sample,sample1,which_first;
  int    is_function,is_wire,oren;
  double xmin,ymin,xmax,ymax;
  struct uaxes *uaxes;
};

struct curve
{
  struct udft_entry *fx,*fy,*fz;
  int    sample,sample1;
  int    color;
  int    style;
  double xmin,xmax;
  struct uaxes *uaxes;
};

struct tube
{
  struct udft_entry *fx,*fy,*fz, *radf;
  int    sample,sample1;
  int    color;
  double xmin,xmax;
  struct uaxes *uaxes;
};

struct contour
{
  struct udft_entry *fx;
  int    sample,sample1;
  int    threed;
  int    n_cs, **nconts;
  double *c_vs;
  double xmin,ymin,xmax,ymax;
  struct uaxes *uaxes;
};

struct map
{
  struct udft_entry *fx,*fy,*fz,*fw;
  int sample;
  int color;
  int dimension;
  int style;
  double x0,y0,z0,w0;
  struct uaxes *uaxes;
};

struct eqn
{
  struct udft_entry *fx,*fy,*fz,*fw;
  int sample;
  int color;
  int dimension;
  int method;
  int skip;
  double start_time,end_time;
  double x0,y0,z0,w0;
  double step, small;
  double x1,y1,z1,w1;
  struct uaxes *uaxes;
};

struct tmesh
{
  int sample, sample1;
  int oren;
  struct uaxes *uaxes;
};

struct a_graph
{
  int index;
  int plot_type;
  int defined;
  int is_data,data_check,data_type;
  char *name;
  char *definition;
  char data_file[MAX_ID_LEN];
  union
    {
      struct surface surface;
      struct curve   curve;
      struct tmesh   tmesh;
      struct contour contour;
      struct map     map;
      struct eqn     eqn;
      struct tube    tube;
    }  picture;
  struct a_graph *next;
};

struct a_material
{
  int   index;
  int   defined;
  char  *name;
  float token[14];
  struct a_material *next;
};

struct an_action
{
  float entry;
  struct an_action *next;
};

struct an_object
{
  char *name;
  int  index;
  int  defined;
  char *menu_string;
  struct an_action  *actions;
  struct an_object *next;
};

struct dummy_obj
{
  struct an_object *obj;
  struct dummy_obj *next;
};

struct plot_history
{
  int index;
  int flag;
  struct plot_history *next;
};

struct  oview
{
  float vx,vy,vz,px,py,pz; 
  int   window_position[4];
  short twist,proj_type;
  union 
    {
      struct ortho
	{
	  float   left,right,top,bottom,near,far;
	}  o;
      struct pers
	{
	  float   pnear,pfar,aspect; short fovy;
	} p;
    } projections;
};

struct light
{
  int   index;
  char  *name;
  float lambient[3];
  float lcolor[3];
  float lposition[4];
  float lspotdir[3];
  float lspotlight[2];
  struct light *next;
};

struct lmodel
{
  int   index;
  char  *name;
  float ambient[3];
  float atten[2];
  float atten2;
  float twoside;
  float local;
  struct lmodel *next;
};

struct final
{
  int light_off;
  int zbuffer_off;
  int double_buffer_off;
  int depthcue_on;
  int box_on;
  int slice_on;
  int save_image;
  int bulb_flag;
  int coor_flag;
  int big_box;
  int base_box;
  int wire;
  int normal;
};

#define STDERRR stderr

/****************************************************************************/



