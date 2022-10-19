/*******************************************************************/
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "object.h"
/*******************************************************************/
#define SM   0.0000001
#define BSM  0.0000001
#define Mmax(a,b) ( (a) > (b)? (a): (b))
#define Mmin(a,b) ( (a) < (b)? (a): (b))
#define Abs(a) ((a)>=0.0 ? (a):(-a))
/*******************************************************************/
extern void qsort(),free(),message(),add_axes();
extern char *malloc();
extern float **new_indices();
extern float *new_floats();
int calc_intersections=1;
static int  simple_intersect();
static int  dcmp(), intersectline();
static void pushstck(), popstck();
static void zsort_depth();
static void intersect();
static void initial_stack();
static void clean_stack();
static xlst *next_quad();
static int  no_intersection();
static int extenduv();
float cut_off_size;                  /* max size of all polygons */
/*****************************************************************/
extern float sizze;
/*****************************************************************/
extern xlst  **dlst;                 /* the pre-display-list     */
extern xlst  *dlst1; 
extern int   plot3d;
static int   npolys;                 /* number of polygons       */
static int   e1_int_e2, extra_count;              
/*******************************************************************/
typedef struct dstack {
  xlst *f,*s;
  struct dstack *p;
} dstck;
static dstck *stack_top = (dstck *)0;
static dstck *next_stck_entry();
/*******************************************************************/
void  make_dpy_list()
{
  int length, i, j;    elt **tptr, *t;     xlst *xt;
  /*
   * read_data() has built a pre-dpy-list. Sort them according to
   * their z-coors. Put all the possible intersecting pairs on
   * a stack and trace out all the intersections. Subdivide the
   * corresponding polygons accordingly.
   */
  extra_count = 0;
  if(plot3d)
    {
      zsort_depth();
      if(calc_intersections) {
        if(npolys > 0) message(2,"Eliminating polygon intersections ...");
        initial_stack();
        /*
         * find all intersections and subdivide the corresponding
         * polygons
         */
        while(stack_top != (dstck *)0)  intersect();
      }
    }
  length = extra_count + nelts;
  elts = (elt **)malloc( length * sizeof(elt *));
  elt1 = (elt *)malloc( length * sizeof(elt));
  if( (elts == (elt **)0) || (elt1 == (elt *)0) )
    {
      (void)fprintf(stderr, "Out of memory :-(\n");
      exit(123);
    }
  for(i = 0; i < length; i++)
    { elts[i] = elt1 + i; elts[i]->type = UNKNOWNTYPE;}

  tptr = elts;
  for(i = 0; i < nelts; i++)
    {
      t = *tptr;
      xt = dlst[i];
      if( xt->nv1 != 0)
	{
	  if(xt->nv1 == 4)  t->type = QUADPOLY;
	  else   t->type = POLYGON;
	  t->mat = xt->mat;
	  t->nv = xt->nv1;
	  t->f = xt->f1;
	  tptr++; t = *tptr;
	  if(xt->nv2 == 4)  t->type = QUADPOLY;
	  else   t->type = POLYGON;
  	  t->mat = xt->mat;
	  t->nv = xt->nv2;
	  t->f = xt->f2;
	}
      else
	{
	  j = xt->type; 
	  if( j >= QUADPOLY && j <=AXESBOX)
	    {
	      t->type =j; t->mat = xt->mat;  t->color = xt->color;
	      t->nv = xt->nv;   t->f = xt->f;
	    }
	}
      tptr++;
    }
  nelts = length;
  /*
   * Finally, add in the part for coordinate box. Note: it must
   * be done here or before find_intersections.
   */
  add_axes();
  (void) free( (char *) dlst);  dlst = (xlst **)0;
  (void) free( (char *) dlst1);  dlst1 = (xlst *)0;
  clean_stack();
  cut_off_size = sizze * 0.1;
}
/*******************************************************************/
static void initial_stack()
{
  int i, j, c=0;
  xlst *t, *s;
  
  for(i = 0; i < npolys-1; i++)
    {
      t = dlst[i];      e1_int_e2 = 1; 
      for(j = i+1; e1_int_e2 && j < npolys; j++)
	{ 
	  s = dlst[j]; 
	  if(simple_intersect(t,s))
	    {
	      pushstck(t,s);  pushstck(s,t); 
	      c += 2;
	      if(c == 1000)
		{
		  message(3,"Lots of suspicious intesections,");
		  message(4, "              may take a while");
		}
	    }
	}
    }
}
/*******************************************************************/
static void pushstck(t,s) xlst *t,*s;
{
  dstck *tmp = next_stck_entry();
  tmp->p =  stack_top;   tmp->f = t; tmp->s = s; 
  stack_top = tmp;
}
static void popstck()
{
  if(stack_top == (dstck *)0) 
    {(void) fprintf(stderr, "Empty stack !\n");  return;}
  stack_top = stack_top->p;

}
/*******************************************************************/
static void zsort_depth()
{
  int i, count=0;
  xlst *t, *s;
  float ff;

  for(i = 0; i < nelts; i++)
    {
      t = dlst[i];
      if(t->type == QUADPOLY)
	t->depth = (t->f)[0][2]+(t->f)[1][2]+(t->f)[2][2]+(t->f)[3][2];
      else if(t->type == POLYGON) {
        int j;
        t->depth=0;
        for(j=0;j<t->nv;j++)
          t->depth+=(t->f)[j][2];
        t->depth*=4/t->nv;
      } else
	{ count++;   t->depth = -72345678.0;}
    }
  npolys = nelts - count; if(npolys < 0) { npolys = 0; return; }
  qsort(dlst, nelts, sizeof(xlst *), dcmp);
  cut_off_size = 0.0; 
  for(i = 0; i < npolys-1; i++)
    {
      t = dlst[i];  s = dlst[i+1];
      ff =  t->depth - s->depth;
      if(ff > cut_off_size)  cut_off_size= ff;
    }
  cut_off_size *= 0.25; 
}
/*******************************************************************/
static int dcmp(a,b) xlst **a,**b;
{
  return( ((*a)->depth >= (*b)->depth)? -1: 1);
}
/*******************************************************************/
static int simple_intersect(e1,e2) xlst *e1, *e2;
{
  float axx,axn,ayx,ayn,azx,azn,bxx,bxn,byx,byn,bzx,bzn;
  float ftx,ftn, gtx,gtn;
  float *f1, *f2, *f3, *f4;
  int answer = 0;

  /*
   * figure out the rectangular boxes that enclose e1 or e2, if
   * they do not intersect then e1 cannot intersect e2. Or if
   * these two boxes are of at least cut_off_size away in the
   * z-direction, then all polygons below e2 will not intersect
   * e1. (set e1_int_e2 = 0).
   */
  f1 = (e1->f)[0];  f2 = (e1->f)[1];  f3 = (e1->f)[2];   f4 = (e1->f)[3];
  if(f1[0]>f2[0]){ftx=f1[0]; ftn=f2[0];} else {ftx=f2[0]; ftn=f1[0];} 
  if(f3[0]>f4[0]){gtx=f3[0]; gtn=f4[0];} else {gtx=f3[0]; gtn=f4[0];}   
  axx = Mmax(gtx, ftx); axn = Mmin(gtn, ftn);
  if(f1[1]>f2[1]){ftx=f1[1]; ftn=f2[1];} else {ftx=f2[1]; ftn=f1[1];} 
  if(f3[1]>f4[1]){gtx=f3[1]; gtn=f4[1];} else {gtx=f3[1]; gtn=f4[1];}   
  ayx = Mmax(gtx, ftx); ayn = Mmin(gtn, ftn);
  if(f1[2]>f2[2]){ftx=f1[2]; ftn=f2[2];} else {ftx=f2[2]; ftn=f1[2];} 
  if(f3[2]>f4[2]){gtx=f3[2]; gtn=f4[2];} else {gtx=f3[2]; gtn=f4[2];}   
  azx = Mmax(gtx, ftx); azn = Mmin(gtn, ftn); 

  f1 = (e2->f)[0];  f2 = (e2->f)[1];  f3 = (e2->f)[2];   f4 = (e2->f)[3];
  if(f1[0]>f2[0]){ftx=f1[0]; ftn=f2[0];} else {ftx=f2[0]; ftn=f1[0];} 
  if(f3[0]>f4[0]){gtx=f3[0]; gtn=f4[0];} else {gtx=f3[0]; gtn=f4[0];}   
  bxx = Mmax(gtx, ftx); bxn = Mmin(gtn, ftn);
  if(f1[1]>f2[1]){ftx=f1[1]; ftn=f2[1];} else {ftx=f2[1]; ftn=f1[1];} 
  if(f3[1]>f4[1]){gtx=f3[1]; gtn=f4[1];} else {gtx=f3[1]; gtn=f4[1];}   
  byx = Mmax(gtx, ftx); byn = Mmin(gtn, ftn);
  if(f1[2]>f2[2]){ftx=f1[2]; ftn=f2[2];} else {ftx=f2[2]; ftn=f1[2];} 
  if(f3[2]>f4[2]){gtx=f3[2]; gtn=f4[2];} else {gtx=f3[2]; gtn=f4[2];}   
  bzx = Mmax(gtx, ftx); bzn = Mmin(gtn, ftn); 

  if(azn >= bzx + cut_off_size) { e1_int_e2 = 0; return(0);}
    {
      axx -= axn; ayx -= ayn; azx -= azn; 
      bxx -= axn; byx -= ayn; bzx -= azn;  
      bxn -= axn; byn -= ayn; bzn -= azn;
  
      if( bxn >= 0.0 && bxn <= axx) 
	{ 
	  if( (byn >= 0.0 && byn <= ayx) || (byx >= 0.0 && byx <= ayx) ||
	     (byn <= 0.0 && byx >= ayx))
	    {
	      if( (bzn >= 0.0 && bzn <= azx)|| (bzx >= 0.0 &&  bzx <= azx) 
		 || (bzn <= 0.0 && bzx >= azx) ) answer = 1;
	      else answer = 0;
	    }
	  else answer = 0;
	}
      else if( bxx >= 0.0 && bxx <= axx )
	{ 
	  if( (byn >= 0.0 && byn <= ayx) || (byx >= 0.0 && byx <= ayx) ||
	     (byn <= 0.0 && byx >= ayx))
	    {
	      if( (bzn >= 0.0 && bzn <= azx)|| (bzx >= 0.0 &&  bzx <= azx) 
		 || (bzn <= 0.0 && bzx >= azx) ) answer = 1;
	      else answer = 0;
	    }
	  else answer = 0;
	}
      else if( bxn <= 0.0 && bxx >= axx )
	{ 
	  if( (byn >= 0.0 && byn <= ayx) || (byx >= 0.0 && byx <= ayx) ||
	     (byn <= 0.0 && byx >= ayx))
	    {
	      if( (bzn >= 0.0 && bzn <= azx)|| (bzx >= 0.0 &&  bzx <= azx) 
		 || (bzn <= 0.0 && bzx >= azx) ) answer = 1;
	      else answer = 0;
	    }
	  else answer = 0;
	}
      else   answer = 0;
    }
  /**************************************************
   * neighboring polygons do not intersect each other 
   **************************************************/
  if(answer) 
    {
      int i,j;
      for(i = 0; i < 4; i++) for(j = 0; j < 4; j++)
	if( i != j && (e1->f)[i] == (e2->f)[j]) answer = 0;
    }
  if(answer)
    {
      xlst  *p1, *p2;
      float *n1, *n2, *ref,*ref2,*v1;
      float d1,d2,v;
      int   i,pos,neg,zeo;

      p1 = e1; p2 = e2; 
      n1 = (p1->f)[0] +3; n2 = (p2->f)[0] +3;  /* the normals          */
      ref= (p1->f)[0]; 
      d1 = n1[0]*ref[0]+n1[1]*ref[1]+n1[2]*ref[2];
      pos = neg = zeo = 0;
      for(i=0; i<4; i++)
	{
	  v1 = (p2->f)[i]; 
	  v = n1[0]*v1[0]+n1[1]*v1[1]+n1[2]*v1[2] - d1;
	  if(v > BSM)         pos++;
	  else if (v < -BSM)  neg++; 
	  else zeo++;
	}
      if( pos == 4 || neg == 4 || (zeo==1 && (pos==3 || neg == 3)) ||
	 zeo >= 3 ) return(0);
      /******************************************************************/
      ref2= (p2->f)[0];  
      d2 = n2[0]*ref2[0]+n2[1]*ref2[1]+n2[2]*ref2[2];
      pos = neg = zeo = 0;
      for(i=0; i<4; i++)
	{
	  v1 = (p1->f)[i]; 
	  v = n2[0]*v1[0]+n2[1]*v1[1]+n2[2]*v1[2] - d2;
	  if(v > BSM)          pos++;
	  else if (v < -BSM)   neg++;
	  else  zeo++; 
	}
      if( pos == 4 || neg == 4 || (zeo==1 && (pos==3 || neg == 3)) ||
	 zeo >= 3) return(0);
    }
  return(answer);
}
/*******************************************************************
 *
 *  Trace out the intersection of p1 with p2. Work at one polygon,
 *  namely p1, a time. There are a lot of redundant computations and
 *  still, it cannot handle all the cases. Eg when the intersection
 *  happen exactly at the one side of p2.
 *  
 *******************************************************************/
static int loop_count = -923456789;
static void intersect() 
{
  xlst  *p1, *p2, *first, *second, *middle, *first1, *second1;
  float *n1, *n2, *ref,*v1,*v2,*uu,*vv,*abc;
  float d1,v, num,den, lambda;
  float intxs1[4][3],intxs2[4][3],ftmp[3];
  float fleft[30][3], fright[30][3];
  int   left, right, needfix = 0;
  int   i,j,k,l,m,n,pos,neg,zeo,flag1[5];
  int   flag[5], ind1,ind2, okok,ok1;

  p1 = stack_top->f; p2 = stack_top->s;      /* the polygons         */
  if(p1->done != 0) { popstck(); return; } /* already computed     */

  first = second = (xlst *)0;
  first1 = second1 = (xlst *)0;

  n1 = (p1->f)[0] +3; n2 = (p2->f)[0] +3;  /* the normals          */
  ref= (p1->f)[0];   
  d1 = n1[0]*ref[0]+n1[1]*ref[1]+n1[2]*ref[2];
  pos = neg = zeo = 0;
  /*
   * pos, neg and zeo count the number of vertices of p2 which are
   * on the +, - side of p1 or exactly on p1.   If there are two
   * vertices of p2 on p1, we still need to subdivide p1.
   */
  for(i=0; i<4; i++)
    {
      v1 = (p2->f)[i]; 
      v = n1[0]*v1[0]+n1[1]*v1[1]+n1[2]*v1[2] - d1;
      if(v > SM)        { pos++; flag1[i] = 1; }
      else if (v < -SM) { neg++; flag1[i] = -1;}
      else { zeo++; flag1[i] = 0;}
    }
  flag1[4] = flag1[0]; ind1 = -1; ind2 = -1; okok = 0;
  switch(zeo)
    {
    case 0: 
      /* 
       * there are vertices above p1 and below p1 but none
       * is on p1
       */
      i = 0; while(flag1[i]*flag1[i+1]>0 ) i++;
      j = i+1; while(flag1[j]*flag1[j+1]> 0) j++;
      switch(i)
	{
	case 0: first = p2->l; break;
	case 1: first = p2->t; break;
	case 2: first = p2->r; break;
	default: break;
	}
      switch(j)
	{
	case 1: second = p2->t; break;
	case 2: second = p2->r; break;
	case 3: second = p2->b; break;
	default: break;
	}
      break;
    case 1: 
      /*
       * one vertex is on p1. Also some are above, others are
       * below.
       */
      i = 0; while(flag1[i] != 0) i++;
      j = 0; while(flag1[j]*flag1[j+1]>= 0) j++;
      switch (i) 
	{
	case 0: first = p2->bl; break;
	case 1: first = p2->tl; break;
	case 2: first = p2->tr; break;
	case 3: first = p2->br; break;
	default: break;
	}
      switch (j) 
	{
	case 0: second = p2->l; break;
	case 1: second = p2->t; break;
	case 2: second = p2->r; break;
	case 3: second = p2->b; break;
	default: break;
	}
      v1 = (p2->f)[i];
      intxs1[i][0] =v1[0]; intxs1[i][1] =v1[1]; intxs1[i][2] =v1[2];
      if(i > j) {middle = first; first=second; second=middle; ind2 =i;}
      else ind1 = i;
      break;
    case 2:
      /*
       * two vertices are on p1. Separete two cases, diagnal 
       * and adjacent
       */
      i = 0; while(flag1[i] != 0) i++;
      j = i+1; while(flag1[j] != 0) j++;
      ind1 = i; ind2 = j;
      if(i == 0 && j == 2 ) { first = p2->bl; second = p2->tr;}
      else if(i==1 && j== 3)  {first = p2->tl; second = p2->br;}
      else
	{ /* need to choose between first and first1 */
	  needfix = 1;
	  if( !(i==0 && j == 3) )
	    {
	      switch (i) 
		{
		case 0: 
		  first = p2->b; first1 = p2->bl; 
		  second = p2->t;second1=p2->tl; 
		  break;
		case 1: 
		  first = p2->l; first1 = p2->tl;
		  second = p2->r;second1=p2->tr;
		  break;
		case 2: 
		  first = p2->t; first1 = p2->tr;
		  second = p2->b; second1=p2->br;
		  break;
		default: break;
		}
	    }
	  else /* i==0; j == 3 */
	    {
	      first = p2->l; first1 = p2->bl;
	      second = p2->r; second1 = p2->br;
	    }
	} /* we still need to choose first and second if useful*/
      v1 = (p2->f)[i];
      intxs1[i][0] =v1[0]; intxs1[i][1] =v1[1]; intxs1[i][2] =v1[2];
      v1 = (p2->f)[j];
      intxs1[j][0] =v1[0]; intxs1[j][1] =v1[1]; intxs1[j][2] =v1[2];
      okok = 1;
      break;
    case 3:   case 4:   default:  
      /* donot set p1->done since p1 may intersect others */
      popstck(); return;
    break;
    }
  /********************************************************************
   * compute the intersection of p2 with the plane spanned by p1
   * intxs1[ind1] and intxs1[ind2]  are the endpts of the intersection
   ********************************************************************/
  if(!okok) {
    for(i = 0; i < 4; i++)
      {
	if(flag1[i] * flag1[i+1] < 0)
	  {
	    j = i+1; if(j == 4) j = 0;
	    v1 = (p2->f)[i]; v2 =(p2->f)[j];
	    ftmp[0]=(v2[0]-v1[0]);ftmp[1]=(v2[1]-v1[1]);ftmp[2]=(v2[2]-v1[2]);
	    num = d1 - (v1[0]*n1[0]+v1[1]*n1[1]+v1[2]*n1[2]);
	    den = ftmp[0]*n1[0]+ftmp[1]*n1[1]+ftmp[2]*n1[2];
	    lambda = num/den; /* cannot be 0 */
	    intxs1[i][0] =v1[0]+ftmp[0]*lambda;
	    intxs1[i][1] =v1[1]+ftmp[1]*lambda;
	    intxs1[i][2] =v1[2]+ftmp[2]*lambda;
	    if(ind1 == -1) ind1 = i; else if(ind2 == -1) ind2 = i;
	  }
      }
  } 
  uu = &(intxs1[ind1][0]); vv = &(intxs1[ind2][0]);
  /*******************************************************************
   *
   *  Now intersect uu-vv with each of the 4 sides of p1
   *
   *******************************************************************/
  {
    float *nn,*w,*x,*ret;
    nn = (p1->f)[0]+3; 
    l = 0;
    for(i = 0; i < 4; i++)
      {
	flag[i] = 0; flag1[i] = 0;
	j = i+1; if(j == 4) j = 0;
	w = (p1->f)[i]; x = (p1->f)[j]; ret = &(intxs2[i][0]);
	k = intersectline(uu,vv,w,x,nn,ret,&m);
	if(k == -1) 
	  { /* again, donot set p1->done as it may intersect others */
	    popstck(); return;
	  }
	else if( k == 1) {flag[i] = 1; flag1[i] = m;}
	l += k;
      }
  }
  k = flag[0]+flag[1]+flag[2]+flag[3];   left = right = 1;   
  /*
   *  k is the number of times uu-vv intersects the 4 sides of p1.
   */
  switch(k)
    { /* switch k */
    case 2: { /* the simplest case intxs2 holds the intersection pts*/
      i = 0; while(flag[i] == 0) i++;
      j = i+1; while(flag[j] ==0) j++;
      extra_count++;
      abc = new_floats(9);
      abc[0]=intxs2[i][0];abc[1]=intxs2[i][1];abc[2]=intxs2[i][2];
      abc[3]=(p1->f)[0][3];abc[4]=(p1->f)[0][4];abc[5]=(p1->f)[0][5];
      abc[6]=intxs2[j][0];abc[7]=intxs2[j][1];abc[8]=intxs2[j][2];
      switch(i)
	{ /* switch i */
	case 0: 
	  if( j == 1)
	    {
	      p1->f1 = new_indices(5); p1->f2 = new_indices(3);
	      p1->nv1 = 5; p1->nv2 = 3;
	      (p1->f1)[0]=(p1->f)[0]; (p1->f1)[1]=abc;
	      (p1->f1)[2]=abc+6; (p1->f1)[3]=(p1->f)[2]; 
	      (p1->f1)[4]=(p1->f)[3]; 
	      (p1->f2)[0]=abc; (p1->f2)[1]=(p1->f)[1]; 
	      (p1->f2)[2]=abc+6;
	    }
	  else if (j == 2)
	    {
	      p1->f1 = new_indices(4); p1->f2 = new_indices(4);
	      p1->nv1 = 4; p1->nv2 = 4;
	      (p1->f1)[0]=(p1->f)[0]; (p1->f1)[1]=abc;
	      (p1->f1)[2]=abc+6; (p1->f1)[3]=(p1->f)[3]; 
	      (p1->f2)[0]=abc; (p1->f2)[1]=(p1->f)[1]; 
	      (p1->f2)[2]=(p1->f)[2]; (p1->f2)[3]=abc+6;
	    }
	  else /* j == 3 */
	    {
	      p1->f1 = new_indices(3); p1->f2 = new_indices(5);
	      p1->nv1 = 3; p1->nv2 = 5;
	      (p1->f1)[0]=(p1->f)[0]; (p1->f1)[1]=abc;
	      (p1->f1)[2]=abc+6; 
	      (p1->f2)[0]=abc; (p1->f2)[1]=(p1->f)[1]; 
	      (p1->f2)[2]=(p1->f)[2];(p1->f2)[3]=(p1->f)[3];
	      (p1->f2)[4]=abc+6;
	    }
	  break;
	case 1:
	  if( j == 2)
	    {
	      p1->f1 = new_indices(5); p1->f2 = new_indices(3);
	      p1->nv1 = 5; p1->nv2 = 3;
	      (p1->f1)[0]=(p1->f)[0]; (p1->f1)[1]=(p1->f)[1]; 
	      (p1->f1)[2]=abc; (p1->f1)[3]=abc+6;
	      (p1->f1)[4]=(p1->f)[3]; 
	      (p1->f2)[0]=abc; (p1->f2)[1]=(p1->f)[2]; 
	      (p1->f2)[2]=abc+6;
	    }
	  else /* j == 3 */
	    {
	      p1->f1 = new_indices(4); p1->f2 = new_indices(4);
	      p1->nv1 = 4; p1->nv2 = 4;
	      (p1->f1)[0]=(p1->f)[0]; (p1->f1)[1]=(p1->f)[1]; 
	      (p1->f1)[2]=abc; (p1->f1)[3]= abc+6;
	      (p1->f2)[0]=abc; (p1->f2)[1]=(p1->f)[2]; 
	      (p1->f2)[2]=(p1->f)[3]; (p1->f2)[3]=abc+6;
	    }
	  break;
	case 2:
	  p1->f1 = new_indices(5); p1->f2 = new_indices(3);
	  p1->nv1 = 5; p1->nv2 = 3;
	  (p1->f1)[0]=(p1->f)[0]; (p1->f1)[1]=(p1->f)[1]; 
	  (p1->f1)[2]=(p1->f)[2];  (p1->f1)[3]=abc;
	  (p1->f1)[4]= abc+6;
	  (p1->f2)[0]=abc; (p1->f2)[1]=(p1->f)[3]; 
	  (p1->f2)[2]=abc+6;
	  break;
	default:  break;
	}
    } /* end of case 2: */
      break;
    case 0:         /* k = 0, uu-vv is completely inside p1 */
      if( l == 8)
	{  
	  int len, ii,jj; float vts[60][3];
	  m = n =  -1;
	  fleft[0][0]=uu[0];fleft[0][1]=uu[1];fleft[0][2]=uu[2];
	  fright[0][0]=vv[0];fright[0][1]=vv[1];fright[0][2]=vv[2];
	  if(needfix)
	    { 
	      if( no_intersection(p1,first)) first = first1;
	      if( no_intersection(p1,second)) second = second1;
	    }
	  loop_count++; p1->notused = loop_count;
	  while(first != (xlst *)0)
	    {    
	      /*
	       * m or n is the side of p1 which intersect 'first'
	       * it is set only when the intersection actually
	       * occurs
	       */
	      ok1 = 0; m = -1; first->notused = loop_count;
	      first = next_quad(p1,first,&(fleft[left][0]),&m, &ok1,
				&(intxs1[ind1][0]),-1);
	      if(first != (xlst *)0) 
		{ /* this should take care of the case when 
		   * lots of polygons are co-planner with p1
		   * and are completely inside p1
		   */
		  if(first->notused == loop_count){popstck(); return;}
		}
	      if(ok1) left++;
	    }
	  if(m == -1)
	    {
	      /*
	       * no intersection found. Extend the 'left' end to
	       * the nearest bdy of p1. And of course, set m
	       * accoordingly.
	       */
	      
	      v1 = &(fleft[left-1][0]); 
	      if(left > 1) v2 = &(fleft[left-2][0]); 
	      else v2 = &(fright[0][0]);
	      if( extenduv(p1,v1,v2,&(fleft[left][0]),&m,-1)) left++;
	      else {popstck(); return;}
	    }
	  loop_count++; p1->notused = loop_count;
	  while(second != (xlst *)0)
	    {
	      ok1 = 0; n = -1; second->notused = loop_count;
	      second = next_quad(p1,second,&(fright[right][0]),&n,&ok1,
				 &(intxs1[ind2][0]),m);
	      if(second != (xlst *)0) 
		{ 
		  if(second->notused == loop_count){popstck(); return;}
		}
	      if(ok1) right++;
	    }
	  if(n == -1)
	    {  /* do the samething for the 'right' boundry */
	      v1 = &(fright[right-1][0]);
	      if( right > 1)  v2 = &(fright[right-2][0]);
	      else v2 = &(fleft[0][0]);
	      if(extenduv(p1,v1,v2,&(fright[right][0]),&n,m)) right++;
	      else {popstck(); return;}
	    }
	  /*
	   * now a complete intersection has been found, go
	   * ahead do the polygon subdivisions.
	   */
	  if(m < n) 
	    {
	      for(jj=0, ii = left-1; ii>=0; ii--,jj++)
		{
		  vts[jj][0]=fleft[ii][0];
		  vts[jj][1]=fleft[ii][1];
		  vts[jj][2]=fleft[ii][2];
		}
	      for(ii = 0; ii<right; ii++,jj++)
		{
		  vts[jj][0]=fright[ii][0];
		  vts[jj][1]=fright[ii][1];
		  vts[jj][2]=fright[ii][2];
		}
	      i = m; j = n;
	    }
	  else /* n < m */ 
	    { 
	      for(jj=0, ii = right-1; ii>=0; ii--,jj++)
		{
		  vts[jj][0]=fright[ii][0];
		  vts[jj][1]=fright[ii][1];
		  vts[jj][2]=fright[ii][2];
		}
	      for(ii = 0; ii<left; ii++,jj++)
		{
		  vts[jj][0]=fleft[ii][0];
		  vts[jj][1]=fleft[ii][1];
		  vts[jj][2]=fleft[ii][2];
		}
	      i =n; j = m;
	    }
	  extra_count++;
	  len = left+right;
	  abc = new_floats(3*len + 3);
	  abc[0]=vts[0][0]; abc[1]=vts[0][1];abc[2]=vts[0][2];
	  abc[3]=(p1->f)[0][3];abc[4]=(p1->f)[0][4];abc[5]=(p1->f)[0][5];
	  for(ii=1,jj=6;ii<len;ii++)
	    {
	      abc[jj++]=vts[ii][0]; abc[jj++]=vts[ii][1]; 
	      abc[jj++]=vts[ii][2];
	    }
	  switch(i)
	    {
	    case 0:
	      if(j == 1)
		{
		  p1->f1 = new_indices(len+3);
		  p1->f2 = new_indices(len+1);
		  p1->nv1 = len+3; p1->nv2 = len+1;
		  (p1->f1)[0]=(p1->f)[0];   (p1->f1)[1]=abc;
		  for(ii = 1; ii <len; ii++)  (p1->f1)[ii+1] = abc+3*(ii+1);
		  (p1->f1)[len+1]=(p1->f)[2]; 
		  (p1->f1)[len+2]=(p1->f)[3]; 
		  (p1->f2)[0]=abc; 
		  for(ii = 1; ii<len;ii++) (p1->f2)[ii] = abc + 3*(ii+1);
		  (p1->f2)[len]=(p1->f)[1]; 
		}
	      else if( j == 2)
		{
		  p1->f1 = new_indices(len+2);
		  p1->f2 = new_indices(len+2);
		  p1->nv1 = len+2; p1->nv2 = len+2;
		  (p1->f1)[0]=(p1->f)[0];   (p1->f1)[1]=abc;
		  for(ii = 1; ii <len; ii++)  (p1->f1)[ii+1] = abc+3*(ii+1);
		  (p1->f1)[len+1]=(p1->f)[3]; 
		  (p1->f2)[0]=abc; 
		  for(ii = 1; ii<len;ii++) (p1->f2)[ii] = abc + 3*(ii+1);
		  (p1->f2)[len]=(p1->f)[2]; (p1->f2)[len+1]=(p1->f)[1];
		}
	      else /* j==3 */
		{
		  p1->f1 = new_indices(len+1);
		  p1->f2 = new_indices(len+3);
		  p1->nv1 = len+1; p1->nv2 = len+3;
		  (p1->f1)[0]=(p1->f)[0];   (p1->f1)[1]=abc;
		  for(ii = 1; ii <len; ii++)  (p1->f1)[ii+1] = abc+3*(ii+1);
		  (p1->f2)[0]=abc; 
		  for(ii = 1; ii<len;ii++) (p1->f2)[ii] = abc + 3*(ii+1);
		  (p1->f2)[len]=(p1->f)[3]; 
		  (p1->f2)[len+1]=(p1->f)[2]; 
		  (p1->f2)[len+2]=(p1->f)[1]; 
		}
	      break;
	    case 1:
	      if(j == 2)
		{
		  p1->f1 = new_indices(len+3);
		  p1->f2 = new_indices(len+1);
		  p1->nv1 = len+3; p1->nv2 = len+1;
		  (p1->f1)[0]=(p1->f)[0];   (p1->f1)[1]=(p1->f)[1];
		  (p1->f1)[2] = abc;
		  for(ii = 1; ii <len; ii++)  (p1->f1)[ii+2] = abc+3*(ii+1);
		  (p1->f1)[len+2]=(p1->f)[3]; 
		  (p1->f2)[0]=abc; 
		  for(ii = 1; ii<len;ii++) (p1->f2)[ii] = abc + 3*(ii+1);
		  (p1->f2)[len]=(p1->f)[2]; 
		}
	      else /* j== 3 */
		{
		  p1->f1 = new_indices(len+2);
		  p1->f2 = new_indices(len+2);
		  p1->nv1 = len+2; p1->nv2 = len+2;
		  (p1->f1)[0]=(p1->f)[0];   (p1->f1)[1]=(p1->f)[1];
		  (p1->f1)[2] = abc;
		  for(ii = 1; ii <len; ii++)  (p1->f1)[ii+2] = abc+3*(ii+1);
		  (p1->f2)[0]=abc; 
		  for(ii = 1; ii<len;ii++) (p1->f2)[ii] = abc + 3*(ii+1);
		  (p1->f2)[len]=(p1->f)[3]; (p1->f2)[len+1]=(p1->f)[2];
		}
	      break;
	    case 2: /* j must be 3 */
	      p1->f1 = new_indices(len+3);
	      p1->f2 = new_indices(len+1);
	      p1->nv1 = len+3; p1->nv2 = len+1;
	      (p1->f1)[0]=(p1->f)[0];   (p1->f1)[1]=(p1->f)[1];
	      (p1->f1)[2]=(p1->f)[2]; (p1->f1)[3] = abc;
	      for(ii = 1; ii <len; ii++)  (p1->f1)[ii+3] = abc+3*(ii+1);
	      (p1->f2)[0]=abc; 
	      for(ii = 1; ii<len;ii++) (p1->f2)[ii] = abc + 3*(ii+1);
	      (p1->f2)[len]=(p1->f)[3]; 
	      break;
	    default: break;
	    }
	}    
      break;
    case 1:      /* k = 1 i.e.  uv intersects one side of p1 */
      {
	  int len, ii,jj,mm; float vts[60][3];
	  n = -1;    
	  mm = 0; while(flag[mm] == 0) mm++;  
	  m = flag1[mm];
	  /*
	   * mm is the side of p1 which intersects  uu--vv
	   * m=1 means u is inside p1, m=2 means v is inside
	   */
	  if(m == 1)                  /* uu is inside p1     */
	    {                         /* extend the left end */
	      fright[0][0]=intxs2[mm][0];
	      fright[0][1]=intxs2[mm][1];  fright[0][2]=intxs2[mm][2];
	      fleft[0][0]=uu[0];fleft[0][1]=uu[1];fleft[0][2]=uu[2];
	      if(needfix) if(no_intersection(p1,first)) first = first1;
	      loop_count++; p1->notused = loop_count;
	      while(first != (xlst *)0)
		{
		  ok1 = 0; n = -1;  first->notused = loop_count;
		  first = next_quad(p1,first,&(fleft[left][0]),&n,&ok1,
				    &(intxs1[ind1][0]),mm);
		  if(first != (xlst *)0) 
		    {
		      if(first->notused == loop_count){popstck(); return;}
		    }
		  if(ok1) left++;
		}
	      if(n == -1)
		{
		  /*
		   * no intersection found. Extend the 'left' end to
		   * the nearest bdy of p1. And of course, set n
		   * accoordingly.
		   */
		  v1 = &(fleft[left-1][0]); 
		  if(left > 1) v2 = &(fleft[left-2][0]); 
		  else v2 = &(fright[0][0]);
		  if(extenduv(p1,v1,v2,&(fleft[left][0]),&n,mm)) left++;
		  else {popstck(); return;}
		}
	      len = 1+left;
	      if(mm <n)
		{
		  vts[0][0]=fright[0][0];
		  vts[0][1]=fright[0][1];
		  vts[0][2]=fright[0][2];
		  for(jj=1, ii = 0; ii<left; ii++,jj++)
		    {
		      vts[jj][0]=fleft[ii][0];
		      vts[jj][1]=fleft[ii][1];
		      vts[jj][2]=fleft[ii][2];
		    }
		  i = mm; j = n;
		}
	      else /* n < mm */
		{
		  for(jj=0, ii = left-1; ii>=0; ii--,jj++)
		    {
		      vts[jj][0]=fleft[ii][0];
		      vts[jj][1]=fleft[ii][1];
		      vts[jj][2]=fleft[ii][2];
		    }
		  vts[jj][0]=fright[0][0];
		  vts[jj][1]=fright[0][1];
		  vts[jj][2]=fright[0][2];		  
		  i = n; j = mm;
		}
	    }
	  else                             /* m == 2 so vv is inside p1 */
	    {                              /* extend the 'right' end    */
	      fleft[0][0] = intxs2[mm][0];
	      fleft[0][1] = intxs2[mm][1]; fleft[0][2] = intxs2[mm][2];
	      fright[0][0]=vv[0];fright[0][1]=vv[1];fright[0][2]=vv[2];
	      if(needfix) if( no_intersection(p1,second)) second = second1;
	      loop_count++; p1->notused = loop_count;
	      while(second != (xlst *)0)
		{
		  ok1 = 0; n = -1; second->notused = loop_count;
		  second = next_quad(p1,second,&(fright[right][0]),&n,&ok1,
				     &(intxs1[ind2][0]),mm); 
		  if(second != (xlst *)0) 
		    { 
		      if(second->notused == loop_count){popstck(); return;}
		    }
		  if(ok1) right++;
		}
	      if(n == -1)
		{
		  /*
		   * no intersection found. Extend the 'right' end to
		   * the nearest bdy of p1. And of course, set n
		   * accoordingly.
		   */
		  v1 = &(fright[right-1][0]); 
		  if(right > 1) v2 = &(fright[right-2][0]); 
		  else v2 = &(fleft[0][0]);
		  if(extenduv(p1,v1,v2,&(fright[right][0]),&n,mm)) right++;
		  else { popstck();  return;}
		}
	      len = 1+right;
	      if(mm <n)
		{
		  vts[0][0]=fleft[0][0];
		  vts[0][1]=fleft[0][1];
		  vts[0][2]=fleft[0][2];
		  for(jj=1, ii = 0; ii<right; ii++,jj++)
		    {
		      vts[jj][0]=fright[ii][0];
		      vts[jj][1]=fright[ii][1];
		      vts[jj][2]=fright[ii][2];
		    }
		  i = mm; j = n;
		}
	      else /* n < mm */
		{
		  for(jj=0, ii = right-1; ii>=0; ii--,jj++)
		    {
		      vts[jj][0]=fright[ii][0];
		      vts[jj][1]=fright[ii][1];
		      vts[jj][2]=fright[ii][2];		  
		    }
		  vts[jj][0]=fleft[0][0];
		  vts[jj][1]=fleft[0][1];
		  vts[jj][2]=fleft[0][2];    
		  i = n; j = mm;
		}
	    }
	  extra_count++;
	  abc = new_floats(3*len + 3);
	  abc[0]=vts[0][0]; abc[1]=vts[0][1];abc[2]=vts[0][2];
	  abc[3]=(p1->f)[0][3];abc[4]=(p1->f)[0][4];abc[5]=(p1->f)[0][5];
	  for(ii=1,jj=6;ii<len;ii++)
	    {
	      abc[jj++]=vts[ii][0]; abc[jj++]=vts[ii][1]; 
	      abc[jj++]=vts[ii][2];
	    }
	  switch(i)
	    {
	    case 0:
	      if(j == 1)
		{
		  p1->f1 = new_indices(len+3);
		  p1->f2 = new_indices(len+1);
		  p1->nv1 = len+3; p1->nv2 = len+1;
		  (p1->f1)[0]=(p1->f)[0];   (p1->f1)[1]=abc;
		  for(ii = 1; ii <len; ii++)  (p1->f1)[ii+1] = abc+3*(ii+1);
		  (p1->f1)[len+1]=(p1->f)[2]; 
		  (p1->f1)[len+2]=(p1->f)[3]; 
		  (p1->f2)[0]=abc; 
		  for(ii = 1; ii<len;ii++) (p1->f2)[ii] = abc + 3*(ii+1);
		  (p1->f2)[len]=(p1->f)[1]; 
		}
	      else if( j == 2)
		{
		  p1->f1 = new_indices(len+2);
		  p1->f2 = new_indices(len+2);
		  p1->nv1 = len+2; p1->nv2 = len+2;
		  (p1->f1)[0]=(p1->f)[0];   (p1->f1)[1]=abc;
		  for(ii = 1; ii <len; ii++)  (p1->f1)[ii+1] = abc+3*(ii+1);
		  (p1->f1)[len+1]=(p1->f)[3]; 
		  (p1->f2)[0]=abc; 
		  for(ii = 1; ii<len;ii++) (p1->f2)[ii] = abc + 3*(ii+1);
		  (p1->f2)[len]=(p1->f)[2]; (p1->f2)[len+1]=(p1->f)[1];
		}
	      else /* j==3 */
		{
		  p1->f1 = new_indices(len+1);
		  p1->f2 = new_indices(len+3);
		  p1->nv1 = len+1; p1->nv2 = len+3;
		  (p1->f1)[0]=(p1->f)[0];   (p1->f1)[1]=abc;
		  for(ii = 1; ii <len; ii++)  (p1->f1)[ii+1] = abc+3*(ii+1);
		  (p1->f2)[0]=abc; 
		  for(ii = 1; ii<len;ii++) (p1->f2)[ii] = abc + 3*(ii+1);
		  (p1->f2)[len]=(p1->f)[3]; 
		  (p1->f2)[len+1]=(p1->f)[2]; 
		  (p1->f2)[len+2]=(p1->f)[1]; 
		}
	      break;
	    case 1:
	      if(j == 2)
		{
		  p1->f1 = new_indices(len+3);
		  p1->f2 = new_indices(len+1);
		  p1->nv1 = len+3; p1->nv2 = len+1;
		  (p1->f1)[0]=(p1->f)[0];   (p1->f1)[1]=(p1->f)[1];
		  (p1->f1)[2] = abc;
		  for(ii = 1; ii <len; ii++)  (p1->f1)[ii+2] = abc+3*(ii+1);
		  (p1->f1)[len+2]=(p1->f)[3]; 
		  (p1->f2)[0]=abc; 
		  for(ii = 1; ii<len;ii++) (p1->f2)[ii] = abc + 3*(ii+1);
		  (p1->f2)[len]=(p1->f)[2]; 
		}
	      else /* j== 3 */
		{
		  p1->f1 = new_indices(len+2);
		  p1->f2 = new_indices(len+2);
		  p1->nv1 = len+2; p1->nv2 = len+2;
		  (p1->f1)[0]=(p1->f)[0];   (p1->f1)[1]=(p1->f)[1];
		  (p1->f1)[2] = abc;
		  for(ii = 1; ii <len; ii++)  (p1->f1)[ii+2] = abc+3*(ii+1);
		  (p1->f2)[0]=abc; 
		  for(ii = 1; ii<len;ii++) (p1->f2)[ii] = abc + 3*(ii+1);
		  (p1->f2)[len]=(p1->f)[3]; (p1->f2)[len+1]=(p1->f)[2];
		}
	      break;
	    case 2: /* j must be 3 */
	      p1->f1 = new_indices(len+3);
	      p1->f2 = new_indices(len+1);
	      p1->nv1 = len+3; p1->nv2 = len+1;
	      (p1->f1)[0]=(p1->f)[0];   (p1->f1)[1]=(p1->f)[1];
	      (p1->f1)[2]=(p1->f)[2]; (p1->f1)[3] = abc;
	      for(ii = 1; ii <len; ii++)  (p1->f1)[ii+3] = abc+3*(ii+1);
	      (p1->f2)[0]=abc; 
	      for(ii = 1; ii<len;ii++) (p1->f2)[ii] = abc + 3*(ii+1);
	      (p1->f2)[len]=(p1->f)[3]; 
	      break;
	    default:
	      (void)fprintf(stderr,"Oops 1-1 i= %d j = %d!\n",i,j);
	      break;
	    }
	}
      break;
    case 3:    case 4:   default: break;
    }
  p1->done = 1;   popstck();    return;
}
/*************************************************************************
 *
 *  return -1 if uv are on the - side of xw
 *  return  0 if uv does not intersect xw
 *  return  2 if uv are on the + side of xw
 *   ret holds the intersection point.
 *   which tells which of u and v are on the positive side.
 *       1 ---> u is on the + side
 *       2 ---> v is on the + side
 ***********************************************************************/
static int intersectline(u,v,w,x,n,ret, which)
  float v[], u[],w[],x[],n[],*ret; int *which;
{
  int s1,s2;
  float u1[3], u2[3],u3[3],u4[3],a,b,c,d,e, lambda = -1.0;;
  
  u1[0]=x[0]-w[0]; u1[1]=x[1]-w[1]; u1[2]=x[2]-w[2];
  u2[0]=u[0]-w[0]; u2[1]=u[1]-w[1]; u2[2]=u[2]-w[2];  
  u3[0]=v[0]-w[0]; u3[1]=v[1]-w[1]; u3[2]=v[2]-w[2];  
  u4[0]=v[0]-u[0]; u4[1]=v[1]-u[1]; u4[2]=v[2]-u[2];  

  a = (u1[1]*u2[2]-u1[2]*u2[1]);
  b = (u1[2]*u2[0]-u1[0]*u2[2]);  
  c = (u1[0]*u2[1]-u1[1]*u2[0]);  
  d = a* n[0]+b* n[1]+c* n[2];
  a = (u1[1]*u3[2]-u1[2]*u3[1]);
  b = (u1[2]*u3[0]-u1[0]*u3[2]);  
  c = (u1[0]*u3[1]-u1[1]*u3[0]);  
  e = a* n[0]+b* n[1]+c* n[2];
  if(d > 0.00001) s1 = 1; else if(d < -0.00001) s1 = -1; else s1 = 0;
  if(e > 0.00001) s2 = 1; else if(e < -0.00001) s2 = -1; else s2 = 0;
  /*------------------------------------------------------------
   *  s1 = 1,0,-1 means u is in the +, on or in the - side of xw
   *-----------------------------------------------------------*/
  switch(s1)
    {
    case 1: /* u is in the + side */
      if( s2 == -1)  /* intersect, figure out the intersection */
	{
	  d = u1[0]*u4[1] - u4[0]*u1[1];
	  if( Abs(d) > 0.0001) lambda = (u4[1]*u2[0] - u4[0] *u2[1])/d;
	  else
	    {
	      d = u1[0]*u4[2] - u4[0]*u1[2];
	      if( Abs(d) > 0.0001) lambda = (u4[2]*u2[0] - u4[0] *u2[2])/d;
	      else
		{
		  d = u1[1]*u4[2] - u4[1]*u1[2];
		  if( Abs(d) > 0.0001) lambda = (u4[2]*u2[1] - u4[1] *u2[2])/d;
		}
	    }
	  if(lambda < -0.0001 || lambda > 1.0001) return(0);
	  *ret =     w[0]+lambda* u1[0];
	  *(ret+1) = w[1]+lambda* u1[1];
	  *(ret+2) = w[2]+lambda* u1[2];
	  *which = 1; /* signal that u is in the + side of xw */
	  return(1);
	}
      else if(s2 == 0)
	{
	  *which = 1;
	  *ret = v[0]; *(ret+1) = v[1]; *(ret+2) = v[2];
	  return(1);
	} 
      else  return(2); /* both u and v are on the + side of xw */
      break;
    case -1:
      if(s2 == 1)   /* intersect again , v is in the + side */
	{
	  d = u1[0]*u4[1] - u4[0]*u1[1];
	  if( Abs(d) > 0.0001) lambda = (u4[1]*u2[0] - u4[0] *u2[1])/d;
	  else
	    {
	      d = u1[0]*u4[2] - u4[0]*u1[2];
	      if( Abs(d) > 0.0001) lambda = (u4[2]*u2[0] - u4[0] *u2[2])/d;
	      else
		{
		  d = u1[1]*u4[2] - u4[1]*u1[2];
		  if( Abs(d) > 0.0001) lambda = (u4[2]*u2[1] - u4[1] *u2[2])/d;
		}
	    }
	  if(lambda < -0.0001 || lambda > 1.0001) return(0);
	  *ret =     w[0]+lambda* u1[0];
	  *(ret+1) = w[1]+lambda* u1[1];
	  *(ret+2) = w[2]+lambda* u1[2];
	  *which = 2; return(1);
	}
      else return( -1); /* uv is on the - side of xw  */
      break;
    case 0: /* just choose u */
      if(s2 == 1) 
	{
	  *ret = u[0]; *(ret+1) = u[1]; *(ret+2) = u[2];
	  *which = 2;   return(2);
	}
      else return(-1);
      break;
    default:
      break;
    }
  return(-1);
}

/**********************************************************************
 *
 *  Extend the line segment u-v across p1. Assume that u is inside p1.
 *  ret:    the endpoint of the extension.
 *  n:      marks the sides of p1 which u-v extends.
 *********************************************************************/
static int extenduv(p1,u,v,ret,flag,start)
     xlst *p1; float *u,*v,*ret; int *flag,start;
{
  int   i, j, k;  
  float *w, *x;
  float u1[3], u2[3],u4[3],d,lambda,mu;

  for(i = 0; i < 4; i++)
    { /*loop */
      k = 1; j = i+1; if(j == 4) j = 0;
      w = (p1->f)[i]; x = (p1->f)[j];
  
      u1[0]=x[0]-w[0]; u1[1]=x[1]-w[1]; u1[2]=x[2]-w[2];
      u2[0]=u[0]-w[0]; u2[1]=u[1]-w[1]; u2[2]=u[2]-w[2];  
      u4[0]=v[0]-u[0]; u4[1]=v[1]-u[1]; u4[2]=v[2]-u[2];  

      d = u1[0]*u4[1] - u4[0]*u1[1];
      if( Abs(d) > 0.0001)
	{
	  lambda = (u4[1]*u2[0] - u4[0] *u2[1])/d;
	  mu     = (u1[1]*u2[0] - u1[0] *u2[1])/d;
	}
      else
	{
	  d = u1[0]*u4[2] - u4[0]*u1[2];
	  if( Abs(d) > 0.0001) 
	    {
	      lambda = (u4[2]*u2[0] - u4[0] *u2[2])/d;
	      mu     = (u1[2]*u2[0] - u1[0] *u2[2])/d;
	    }
	  else
	    {
	      d = u1[1]*u4[2] - u4[1]*u1[2];
	      if( Abs(d) > 0.0001)
		{
		  lambda = (u4[2]*u2[1] - u4[1] *u2[2])/d;
		  mu     = (u1[2]*u2[1] - u1[1] *u2[2])/d;		  
		}
	      else  { k = 0;  lambda = 10.0; mu = 2.0;}
	    }
	}
      if(k)
	{
	  if(lambda >= -0.0001 && lambda <= 1.0001 && mu < 0.0001 &&
	     i != start)
	    {
	      *ret =     w[0]+lambda* u1[0];
	      *(ret+1) = w[1]+lambda* u1[1];
	      *(ret+2) = w[2]+lambda* u1[2];
	      *flag = i;    return 1;
	    }
	}
    } /* end loop */
  return 0 ;
}
/**********************************************************************
 * 
 *  check to see if p2 intersects p1. Return 1 if not
 *
 *********************************************************************/
static int no_intersection(p1,p2) xlst *p1,*p2;
{
  float *n1,  *ref,*v1,v,d1;
  int   i,pos,neg,zeo;

  if( p2 == (xlst *)0) return(1);
  n1 = (p1->f)[0] +3;   ref= (p1->f)[0];
  d1 = n1[0]*ref[0]+n1[1]*ref[1]+n1[2]*ref[2];  pos = neg = zeo = 0;
  for(i=0; i<4; i++)
    {
      v1 = (p2->f)[i]; 
      v = n1[0]*v1[0]+n1[1]*v1[1]+n1[2]*v1[2] - d1;
      if(v > SM)         pos++;
      else if (v < -SM)  neg++;
      else  zeo++;
    }
  if( pos !=0 && neg !=0) return(0); /* intersect */
  else if( zeo >= 2) return(0);      /* intersect */
  return(1);                         /*  not so   */
}
/**********************************************************************
 *
 *  return the next polygon that intersects p1.
 *  f1:  ---storage for the possible intersection points
 *  tag  ---the sides of p1 which intersects p2 which 
 *          will be used to determine the orientation of the 
 *          resulted polygons.
 *
 *********************************************************************/
static xlst *next_quad(p1,p2,f1,tag,ok,start,mm)
     xlst *p1,*p2; int *tag, *ok,mm; float *f1,*start;
{
  xlst  *first, *second, *middle, *first1, *second1;
  float *n1, *n2, *ref,*v1,*v2,*uu,*vv;
  float d1,d2,v, num,den, lambda;
  float intxs1[4][3],intxs2[4][3],ftmp[3];
  int   left, right, needfix = 0;
  int   i,j,k,l,m,pos,neg,zeo,flag1[5];
  int   flag[5], ind1,ind2, okok;

  n1 = (p1->f)[0] +3; n2 = (p2->f)[0] +3;  /* the normals          */
  first = second = (xlst *)0;
  first1 = second1 = (xlst *)0;
  ref= (p1->f)[0]; 
  d1 = n1[0]*ref[0]+n1[1]*ref[1]+n1[2]*ref[2];
  pos = neg = zeo = 0;
  for(i=0; i<4; i++)
    {
      v1 = (p2->f)[i]; 
      v = n1[0]*v1[0]+n1[1]*v1[1]+n1[2]*v1[2] - d1;
      if(v > SM)        { pos++; flag1[i] = 1; }
      else if (v < -SM) { neg++; flag1[i] = -1;}
      else { zeo++; flag1[i] = 0;}
    }
  flag1[4] = flag1[0]; ind1 = -1; ind2 = -1; okok = 0;
  if(pos == 4 || neg == 4 || (zeo == 1 && (pos==3 || neg == 3)) || zeo >=3)
     { return ((xlst *)0);}
  switch(zeo)
    {
    case 0: 
      i = 0; while(flag1[i]*flag1[i+1]>0 ) i++;
      j = i+1; while(flag1[j]*flag1[j+1]> 0) j++;
      switch(i)
	{
	case 0: first = p2->l; break;
	case 1: first = p2->t; break;
	case 2: first = p2->r; break;
	default: break;
	}
      switch(j)
	{
	case 1: second = p2->t; break;
	case 2: second = p2->r; break;
	case 3: second = p2->b; break;
	default: break;
	}
      break;
    case 1: 
      i = 0; while(flag1[i] != 0) i++;
      j = 0; while(flag1[j]*flag1[j+1]>= 0) j++;
      switch (i) 
	{
	case 0: first = p2->bl; break;
	case 1: first = p2->tl; break;
	case 2: first = p2->tr; break;
	case 3: first = p2->br; break;
	default: break;
	}
      switch (j) 
	{
	case 0: second = p2->l; break;
	case 1: second = p2->t; break;
	case 2: second = p2->r; break;
	case 3: second = p2->b; break;
	default: break;
	}
      if(i > j) {middle = first; first=second; second=middle; ind2 =i;}
      else ind1 = i;
      v1 = (p2->f)[i];
      intxs1[i][0] =v1[0]; intxs1[i][1] =v1[1]; intxs1[i][2] =v1[2];
      break;
    case 2:
      /*
       * two vertices are on p1. Separete two cases, diagnal 
       * and adjacent
       */
      i = 0; while(flag1[i] != 0) i++;
      j = i+1; while(flag1[j] != 0) j++;
      ind1 = i; ind2 = j;
      if(i == 0 && j == 2 ) { first = p2->bl; second = p2->tr;}
      else if(i==1 && j== 3)  {first = p2->tl; second = p2->br;}
      else
	{ /* need to choose between first and first1 */
	  needfix = 1;
	  if( !(i==0 && j == 3) )
	    {
	      switch (i) 
		{
		case 0: 
		  first = p2->b; first1 = p2->bl; 
		  second = p2->t;second1=p2->tl; 
		  break;
		case 1: 
		  first = p2->l; first1 = p2->tl;
		  second = p2->r;second1=p2->tr;
		  break;
		case 2: 
		  first = p2->t; first1 = p2->tr;
		  second = p2->b; second1=p2->br;
		  break;
		default:  break;
		}
	    }
	  else /* i==0; j == 3 */
	    {
	      first = p2->l; first1 = p2->bl;
	      second = p2->r; second1 = p2->br;
	    }
	} /* we still need to choose first and second if useful*/
      v1 = (p2->f)[i];
      intxs1[i][0] =v1[0]; intxs1[i][1] =v1[1]; intxs1[i][2] =v1[2];
      v1 = (p2->f)[j];
      intxs1[j][0] =v1[0]; intxs1[j][1] =v1[1]; intxs1[j][2] =v1[2];
      okok = 1;
      break;
    case 3:   case 4:   default:  /* co-planner polygons, bad ! */
    break;
    }
  /********************************************************************
   * compute the intersection of p2 with the plane spanned by p1
   * intxs1[ind1] and intxs1[ind2]  are the endpts of the intersection
   ********************************************************************/
  if(!okok) {
    for(i = 0; i < 4; i++)
      {
	if(flag1[i] * flag1[i+1] < 0)
	  {
	    j = i+1; if(j == 4) j = 0;
	    v1 = (p2->f)[i]; v2 =(p2->f)[j];
	    ftmp[0]=(v2[0]-v1[0]);ftmp[1]=(v2[1]-v1[1]);ftmp[2]=(v2[2]-v1[2]);
	    num = d1 - (v1[0]*n1[0]+v1[1]*n1[1]+v1[2]*n1[2]);
	    den = ftmp[0]*n1[0]+ftmp[1]*n1[1]+ftmp[2]*n1[2];
	    lambda = num/den; /* cannot be 0 */
	    intxs1[i][0] =v1[0]+ftmp[0]*lambda;
	    intxs1[i][1] =v1[1]+ftmp[1]*lambda;
	    intxs1[i][2] =v1[2]+ftmp[2]*lambda;
	    if(ind1 == -1) ind1 = i; else if(ind2 == -1) ind2 = i;
	  }
      }
  } 
  /*******************************************************************
   *
   *  Now intersect uu-vv with each of the 4 sides of p1
   *
   *******************************************************************/
  uu = &(intxs1[ind1][0]); vv = &(intxs1[ind2][0]);
  {
    float *nn,*w,*x,*ret;
    nn = (p1->f)[0]+3; 
    l = 0;
    for(i = 0; i < 4; i++)
      {
	flag[i] = 0; flag1[i] = 0;
	j = i+1; if(j == 4) j = 0;
	w = (p1->f)[i]; x = (p1->f)[j]; ret = &(intxs2[i][0]);
	k = intersectline(uu,vv,w,x,nn,ret,&m);
	if(k == -1)  return((xlst *)0);
	else if( k == 1) {flag[i] = 1; flag1[i] = m;}
	l += k;
      }
  }
  k = flag[0]+flag[1]+flag[2]+flag[3];   left = right = 1;   
  switch(k)
    {
    case 2:    /* this should never happen */  
      i = 0; while(flag[i] != 1) i++;
      j = i+1; while(flag[j] != 1) j++;
      v1 = &(intxs2[i][0]); v2 = &(intxs2[j][0]);
      if(i == mm)
	{
	  *f1    =  intxs2[j][0];
	  *(f1+1)=  intxs2[j][1];
	  *(f1+2)=  intxs2[j][2];
	  *tag = j; *ok = 1;
	}
      else if(j == mm)
	{
	  *f1    =  intxs2[i][0];
	  *(f1+1)=  intxs2[i][1];
	  *(f1+2)=  intxs2[i][2];
	  *tag = i; *ok = 1;
	}
      else
	{
	  d1 = (v1[0]-start[0])*(v1[0]-start[0]) +
	    (v1[1]-start[1])*(v1[1]-start[1]) +
	      (v1[2]-start[2])*(v1[2]-start[2]);
	  d2 = (v2[0]-start[0])*(v2[0]-start[0]) +
	    (v2[1]-start[1])*(v2[1]-start[1]) +
	      (v2[2]-start[2])*(v2[2]-start[2]);
	  if(d1 <= d2) m = j; else m = i;
	  *f1    =  intxs2[m][0];
	  *(f1+1)=  intxs2[m][1];
	  *(f1+2)=  intxs2[m][2];
	  *tag = m; *ok = 1;
	}
      return( (xlst *)0);
      break;
    case 0:
      if( l == 8 ) /* the segment is inside p1 */
	{  /* inside again */
	  v1 = uu; v2 = vv;
	  d1 = (v1[0]-start[0])*(v1[0]-start[0])+
	    (v1[1]-start[1])*(v1[1]-start[1]) +
	      (v1[2]-start[2])*(v1[2]-start[2]);
	  d2 = (v2[0]-start[0])*(v2[0]-start[0]) +
	    (v2[1]-start[1])*(v2[1]-start[1]) +
	      (v2[2]-start[2])*(v2[2]-start[2]);
	  if(d1 >= d2)
	    {
	      if(needfix) if( no_intersection(p1,first)) first = first1;
	      middle = first; 
	      m = ind1;
	    }
	  else
	    {
	      if(needfix) if( no_intersection(p1,second)) second = second1;
	      middle = second; 
	      m = ind2;
	    }
	  *f1    = *start     = intxs1[m][0];
	  *(f1+1) =*(start+1) = intxs1[m][1];
	  *(f1+2) =*(start+2) = intxs1[m][2];
	  *ok = 1; *tag = m;
	  return(middle);
	}
      else 	return( (xlst *)0);
      break;
    case 1:
      m = 0; while(flag[m] == 0) m++;
      if( m != mm)
	{
	  *f1     = intxs2[m][0];
	  *(f1+1) = intxs2[m][1];
	  *(f1+2) = intxs2[m][2];
	  *tag = m; *ok = 1;
	  return( (xlst *)0);
	}
      else
	{
	  *ok = 1; 
	  if(flag1[m] == 1)
	    {
	      *f1     = uu[0];
	      *(f1+1) = uu[1];
	      *(f1+2) = uu[2];
	      if(needfix) if( no_intersection(p1,first)) first = first1;
	      return(first);
	    }
	  else
	    {
	      *f1     = vv[0];
	      *(f1+1) = vv[1];
	      *(f1+2) = vv[2];
	      if(needfix) if( no_intersection(p1,second)) second = second1;
	      return(second);
	    }
	}
      break;
    case 3:    case 4:    default:
      return( (xlst *)0);
      break;
    }
  return( (xlst *)0 );
}
/***********************************************************************/
typedef struct stack_storage 
{
  dstck *d;
  struct stack_storage *n;
} stck_s;
static stck_s *_stackhead =(stck_s *)0;
static stck_s *stkptr = (stck_s *)0;
static dstck *entry_ptr =(dstck *)0;
static int cur_stck_entry=0;
/***********************************************************************/
static dstck *next_stck_entry()
{
  if(cur_stck_entry == 0)
    {
      stck_s *tmp = stkptr;  dstck *t;
      stkptr = (stck_s *)malloc(sizeof(stck_s));
      if(stkptr==(stck_s *)0){fprintf(stderr,"Out of memory\n");exit(12);}

      if( _stackhead == (stck_s *)0) _stackhead = stkptr;
	
      if(tmp != (stck_s *)0)  tmp->n = stkptr;
      stkptr->n = (stck_s *)0;
      stkptr->d = t = (dstck *)malloc(1024 *sizeof(dstck));
      if(t == (dstck *)0){(void)fprintf(stderr,"Out of memory\n"); exit(13);}
      cur_stck_entry = 1024;     entry_ptr = t;
    }
  cur_stck_entry--;    return(entry_ptr++);
}
      
static void clean_stack()
{
  stck_s *t, *tmp = _stackhead;
  while( tmp != (stck_s *)0)
    {
      t = tmp->n;   
      (void)free( (char *) tmp->d);    (void)free( (char *) tmp);
      tmp = t;
    }
  _stackhead =(stck_s *)0; stkptr = (stck_s *)0;  cur_stck_entry=0;
  stack_top = (dstck *)0;
}
/************************************************************************/
