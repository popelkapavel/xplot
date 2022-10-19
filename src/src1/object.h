
/*******************************************************************/
#define QUADPOLY    1   
#define POLYGON     2   
#define POLYLINE    3
#define LINEPOINT   4
#define CNTLINE     5
#define AXESBOX     6
#define UNKNOWNTYPE 16
/*******************************************************************/
typedef struct {     /* element of the display list         */
  char type;         /* polygon,  line, triangle, text, etc */
  char  nv;          /* number of vertices involved         */
  short mat;         /* color offset or color               */
  short color;       /* color                               */
  short notused;     /*                                     */
  float **f;         /* position of vertices                */
  float depth;       /* z coor in the eye coor system       */
} elt;
/*******************************************************************/
typedef struct xlist{/* element of the display list         */
  char type;         /* polygon,  line, triangle, text, etc */
  char  nv;          /* number of vertices involved         */
  char nv1,nv2;
  short mat;         /* color offset or color               */
  short color;
  short done, pad;
  int   notused;
  float **f;         /* position of vertices                */
  float **f1;        /* position of vertices                */
  float **f2;        /* position of vertices                */
  float depth;       /* z coor in the eye coor system       */
  struct xlist *t,*tr,*r,*br,*b,*bl,*l, *tl;
} xlst;
/*******************************************************************/
typedef struct an_objectn {
  int   type;        /* surface, curve etc     */
  int   datatype;    /* data, grid, noff etc   */
  int   color   ;    /* color or material      */
  int   whatever;    /* whatever ???           */
  int   n1,n2, nv;   /* grid size,             */
  int   serial;       
  char  *filename;   /* data filename          */
  struct an_objectn  *next;
} object;
/*********************************** obj tyep ********/
#define SURFACE    30001   
#define CURVE      30002   
/************************************ data type ******/
#define UNKNOWN_DAT 81001  /* object is from a data file, need to go there
			      and figure out the data type */
#define GRID_DAT    81003
#define NOFF_DAT    81004   
#define POIN_DAT    81005

/************************************ coor box *******/
#define AXESDATA 120
/*******************************************************************/
#ifndef OBJFILE
extern int nelts;           /* length of the display list */
extern elt    **elts;    
extern elt    *elt1;        /* the display list */
extern float xmaxn,xminn,ymaxn,yminn,zmaxn,zminn;
#endif
/*******************************************************************/


