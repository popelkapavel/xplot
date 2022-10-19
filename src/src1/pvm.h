#define PERSPECTIVE      1
#define ORTHOGONAL       2


typedef  struct {
  float xmax,xmin,ymax,ymin,zmax,zmin;       /* object bounds           */ 
  float vx,vy,vz;                            /* eye position, look at O */
  float left,right,top,bottom,near_,far_;      /* orthogonal projection   */
  float pnear,pfar,aspect, fovy;             /* perspective projection  */
  float xscals,yscals,zscals;                /* scaling in screen space */
  float xscale,yscale,zscale;                /* object scaling factors  */
  float xtranp, ytranp,ztranp;               /* object translation, SCR */
  float xtrans,ytrans,ztrans;                /* object translation, BDY */
  float xaxis[3],yaxis[3],zaxis[3],caxis[3]; /* rotation axis           */
  short  proj_type, proj_change;
} oview;

typedef struct 
{ 
  char mode,transmode,movingmode,notused; 
} butboxstat;
#define CENTER_OS   1
#define MOVINGMODE  2
#define GET_OUT     3

typedef struct {
  float ambient[3];
  float diffuse[3];
  float specular[3];
  float shininess;
} mat_entry;

typedef struct {
  float azim, incl, x[3];
  float red,grn,blu;
} light;

#define LANDSCAPE 0
#define PORTRAIT  1
typedef struct {
  short filemode;  /* ps-land, ps-por */
  short background;
  int x,y,w,h;   
} pssetup;

 
#ifdef GVIEWSETHERE
oview       gview;
float       titlepos[2];
char        titlestring[256];
short       titleset, titlecolor;
#else
extern oview       gview;
extern float       titlepos[2];
extern char        titlestring[256];
extern short       titleset, titlecolor;
#endif

#define OXTRANS 1
#define OYTRANS 2
#define OZTRANS 3
#define OSCALE  4
