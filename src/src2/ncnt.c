/*************************************************************
 * Find contour
 *************************************************************/

#include <stdio.h>
#include <math.h>
#include "plot.h"
#include "../src1/object.h"

/******************************************************************/
extern int    c_token,num_tokens;
extern struct value  *const_express();
extern double real();
extern struct udft_entry *dummy_func;
extern struct udft_entry     *add_udf();
extern struct udvt_entry     *add_udv();
extern struct at_type   *perm_at(), *temp_at();
extern char c_dummy_var[],c_dummy_var1[];
extern char c_dummy_var2[],c_dummy_var3[];
extern char input_line[MAX_LINE_LEN+1];
/******************************************************************/
extern float xminn,yminn,zminn,xmaxn,ymaxn,zmaxn;
extern float centerx,centery,centerz;
/******************************************************************/
extern int scanner(),icmp();
extern float **new_indices(),*new_floats();
extern void qsort();
extern double real();
static int cnt_free = 0,max_nelts = 0;
static elt **elt_left;
extern int redraw;
int cnt_used;
/******************************************************************/
static void output_vert(),figure_out_spaces(), add_cnt_elt();
static void reset_dsp_list();
static elt *new_elt();
/******************************************************************/
static int  current_elt = 1024;
typedef struct elt_mems {
  elt *f;
  struct elt_mems *next;
} elt_mem;
elt_mem *eltstorage = (elt_mem *)0, *current_elt_ptr;
elt_mem *entry_elt_storage; int entry_elt_position;
/*****************************************************************/

void cpt_cnt(str1,num) char *str1; int num;
{
  double cvmax = -1000000.0,cvmin=1000000.0, tf,tv;
  double cbox[9][3];
  float *f1,*f2,tmp1[3],tmp2[3];
  int i,j,k,l,m,n,nv,signs[32],pos,neg,zeo,color,count;
  struct value cv;
  struct at_type   *act;
  elt **el,*elptr;


  (void)strcpy(c_dummy_var,"x"); (void)strcpy(c_dummy_var1,"y");
  (void)strcpy(c_dummy_var2,"z"); 
  (void)strcpy(input_line,"cnt_fun_(x,y,z)= "); (void)strcat(input_line,str1); 
  num_tokens = scanner(input_line);  c_token = 0;  define(); 

  /* function and #of contours are set */
  /* now computer the contour value at the 6 corners */
  if(num == 0) {cvmax = 0.0; cvmin=0.0; tf = 0.0; count = 2;}
  else {
    cbox[0][0]=(double)xminn;cbox[0][1]=(double)yminn;cbox[0][2]=(double)zminn;
    cbox[1][0]=(double)xmaxn;cbox[1][1]=(double)yminn;cbox[1][2]=(double)zminn;
    cbox[2][0]=(double)xmaxn;cbox[2][1]=(double)ymaxn;cbox[2][2]=(double)zminn;
    cbox[3][0]=(double)xminn;cbox[3][1]=(double)ymaxn;cbox[3][2]=(double)zminn;
    cbox[4][0]=(double)xminn;cbox[4][1]=(double)yminn;cbox[4][2]=(double)zmaxn;
    cbox[5][0]=(double)xmaxn;cbox[5][1]=(double)yminn;cbox[5][2]=(double)zmaxn;
    cbox[6][0]=(double)xmaxn;cbox[6][1]=(double)ymaxn;cbox[6][2]=(double)zmaxn;
    cbox[7][0]=(double)xminn;cbox[7][1]=(double)ymaxn;cbox[7][2]=(double)zmaxn;
    cbox[8][0]=0.0; cbox[8][1]=0.0; cbox[8][2]=0.0;
    for(i = 0; i < 9; i++)
      {
	complex(&(dummy_func->dummy_value),  cbox[i][0]+centerx, 0.0);
	complex(&(dummy_func->dummy_value1), cbox[i][1]+centery, 0.0);
	complex(&(dummy_func->dummy_value2), cbox[i][2]+centerz, 0.0);
	evaluate_at(dummy_func->at,&cv); tf = real(&cv);
	if(cvmax< tf) cvmax=tf; if(cvmin > tf) cvmin=tf;
      }
    if(cvmax == cvmin) return;
    count = num + 1;
    tf = (cvmax - cvmin)/(double)count;
  }
  message(1,"    Working on contours ...");

  if(max_nelts == 0) max_nelts = nelts;
  cnt_free = 0;
  /*--------------------------------------------------------------------*/
  if(cnt_used) 
    {
      figure_out_spaces(); 
      entry_elt_storage = current_elt_ptr; entry_elt_position=current_elt;
    }
  else
    {
      entry_elt_storage = (elt_mem *)0; entry_elt_position=0;
    }
  /*--------------------------------------------------------------------*/
  cnt_used = 0; /*cnt_used count the new additions to the dpy list */

  for(i=1; i< count; i++)
    {
      el = elts; color = 8 + i%8;
      cvmax = cvmin + (double)i * tf;
      for(j = 0; j < nelts; j++)
	{
	  elptr = *el;
	  if(elptr->type <= POLYGON)
	    {
	      nv = elptr->nv; pos=neg=zeo= 0;
	      for(k = 0; k < nv; k++)
		{
		  f1 = (elptr->f)[k]; 
		  complex(&(dummy_func->dummy_value), (double)(f1[0]+centerx), 0.0);
		  complex(&(dummy_func->dummy_value1),(double)(f1[1]+centery), 0.0);
		  complex(&(dummy_func->dummy_value2),(double)(f1[2]+centerz), 0.0);
		  evaluate_at(dummy_func->at,&cv); tv = real(&cv)-cvmax;
		  signs[k] = (tv>0.0?1:(tv==0.0? 0 : -1));
		  if(signs[k]==1)pos++; else if(signs[k]==0)zeo++; else neg++;
		}
	      signs[nv]=signs[0];
	      switch(zeo)
		{
		case 0:  /* no vertex is at this contour value */
		  if(pos * neg > 0) 
		    {             /* find contour in this polygon */
		      m = 0; while(signs[m]*signs[m+1] >0) m++;
		      n=m+1; while(signs[n]*signs[n+1] >0) n++;
		      f1=(elptr->f)[m]; f2=(elptr->f)[m+1];
		      output_vert(f1,f2,signs[m],signs[m+1],cvmax,act,tmp1);
		      f1=(elptr->f)[n];
		      if(n == nv-1) f2=(elptr->f)[0];
		      else f2=(elptr->f)[n+1];
		      output_vert(f1,f2,signs[n],signs[n+1],cvmax,act,tmp2);
		      add_cnt_elt(tmp1,tmp2,color);
		    }
		  break;
		case 1:  /* one vertex is on this contour */
		  if(pos * neg > 0) 
		    {    /* find the other pt */
		      m=0; while(signs[m] != 0) m++;
		      f1 = (elptr->f)[m];
		      tmp1[0]=f1[0];tmp1[1]=f1[1];tmp1[2]=f1[2];
		      m = 0; while(signs[m]*signs[m+1]>=0) m++;
		      f1=(elptr->f)[m];
		      if(m == nv-1) f2=(elptr->f)[0];
		      else f2=(elptr->f)[m+1];
		      output_vert(f1,f2,signs[m],signs[m+1],cvmax,act,tmp2);
		      add_cnt_elt(tmp1,tmp2,color);
		    }
		  break;
		case 2: /* two vertices are on the contour */
		  m = 0; while(signs[m] != 0) m++;
		  f1 = (elptr->f)[m];
		  m++; while(signs[m] != 0) m++;
		  f2 =  (elptr->f)[m];
		  add_cnt_elt(f1,f2,color);	
		  break;
		default: /* more than two vertices are on the contour */
		         /* this is bad so do nothing */
		  break;
		}
	    }
	  el++;
	}
    }
  reset_dsp_list();
  cnt_used = 1; redraw = 1;
}
/************************************************************************/

static void figure_out_spaces()
{
  elt **el, *t;
  int i,c = 0;
  el = elts;
  for(i = 0; i < max_nelts; i++)
    {
      t = *el;
      if( t->type == CNTLINE)
	{ t->depth = -123456.0; c++;}
      else t->depth = 123456.0;
      el++;
    }
  qsort(elts, max_nelts, sizeof(elt *), icmp);  
  cnt_free = c; elt_left = elts + (max_nelts-c);
}
/*****************************************************************/
static void add_cnt_elt(f1,f2,col) float *f1,*f2; int col;
{
  elt *t; float *f;

  if(cnt_free > 0)
    {
      t = *elt_left;
      f = (t->f)[0];         f[0]=f1[0];f[1]=f1[1];f[2]=f1[2];
      f = (t->f)[1];         f[0]=f2[0];f[1]=f2[1];f[2]=f2[2];
      t->color =t->mat = (short) col;
      cnt_free--;  elt_left++;
    }
  else
    {
      t = new_elt();
      t->type = CNTLINE; t->color=t->mat= (short)col; t->nv = 2; 
      t->f= new_indices(2);
      f = new_floats(9);
      f[0]=f1[0];f[1]=f1[1];f[2]=f1[2];
      f[3]=1.0;f[4]=0.0;f[5]=0.0;
      f[6]=f2[0];f[7]=f2[1];f[8]=f2[2];
      (t->f)[0] = f;   (t->f)[1] = f+6;
      cnt_used++;
    }
}

/*****************************************************************/
#define Abs(x) ((x)>=0.0? (x): -(x))

static void output_vert(fa,fb,sa,sb,cval,act,sp)
     float *fb,*fa,*sp,cval; struct at_type *act; int sa,sb;
{ /*  bisection method */

  int s1,s2,s3;  struct value cv;
  double mid[3],tv=1.0;
  double f1[3],f2[3];
  
  f1[0]=fa[0]+centerx; f1[1]=fa[1]+centery; f1[2]=fa[2]+centerz;
  f2[0]=fb[0]+centerx; f2[1]=fb[1]+centery; f2[2]=fb[2]+centerz;
  s1 = sa; s2 = sb;

  while( Abs(tv)>0.000001)
    {
      mid[0]=0.5*(f1[0]+f2[0]);
      mid[1]=0.5*(f1[1]+f2[1]);
      mid[2]=0.5*(f1[2]+f2[2]);
      complex(&(dummy_func->dummy_value), mid[0], 0.0);
      complex(&(dummy_func->dummy_value1),mid[1], 0.0);
      complex(&(dummy_func->dummy_value2),mid[2], 0.0);
      evaluate_at(dummy_func->at,&cv); tv = real(&cv)-cval;
      s3 = (tv>0.0?1:(tv==0.0? 0 : -1));  
      if(s1 + s3 == 0) /* s1*s2 < 0 */
	{ f2[0] =mid[0]; f2[1] =mid[1]; f2[2] =mid[2];}
      else
	{ f1[0] =mid[0]; f1[1] =mid[1]; f1[2] =mid[2];}
    }
  sp[0] =(float)mid[0]-centerx; 
  sp[1] =(float)mid[1]-centery; 
  sp[2] =(float)mid[2]-centerz;
  return;
}
/*****************************************************************/
static elt *new_elt() 
{
  elt *ans;
  if(current_elt == 1024)
    {
      elt_mem **t = &eltstorage;
      while( (*t) != (elt_mem *)NULL)   t = &((*t)->next);
      *t = (elt_mem *)malloc((unsigned)sizeof(elt_mem));
      (*t)->f = (elt *)malloc(1024 * sizeof(elt));
      if( (*t) == (elt_mem *)NULL || (*t)->f == (elt *)NULL)
	{
	  (void)fprintf(stderr, "Out of memory\n");
	  exit(1);
	}
      (*t)->next = (elt_mem *)NULL;
      current_elt = 0;
      current_elt_ptr = *t;
    }
  ans = current_elt_ptr->f + current_elt;
  current_elt++;
  return(ans);
}
/***************************************************************/
void free_cnt_stuff()
{
  elt_mem *fs = eltstorage, *fs1;
  while( fs != (elt_mem *)NULL)
    {
      fs1 = fs->next;
      (void) free((char *)fs->f);  (void)free(fs);
      fs = fs1;
    }
  current_elt = 1024;
  eltstorage =(elt_mem *)0; current_elt_ptr = (elt_mem *)0;
  cnt_used = 0; max_nelts = 0; cnt_free = 0;
  entry_elt_storage = (elt_mem *)0; entry_elt_position=0;
}
/***************************************************************/
static void reset_dsp_list()
{
  int i,j,tscount;
  elt **t,*s;
  elt_mem *ts;

  if(cnt_free > 0 )
    {
      nelts = max_nelts -cnt_free;
      return;
    }
  else if(cnt_used > 0)
    {
      t = (elt **)malloc( (max_nelts+cnt_used)*sizeof(elt *));
      if( t == (elt **)0)
	{
	  (void)fprintf(stderr,"Out of memory\n"); exit(1);
	}
      for(i = 0; i < max_nelts; i++)
	{
	  t[i] = elts[i];
	}
      (void)free( (char *)elts);
      elts = t; nelts = max_nelts + cnt_used; max_nelts = nelts;
      if( entry_elt_storage != (elt_mem *)0)
	{ ts = entry_elt_storage; j = entry_elt_position; }
      else { ts= eltstorage; j = 0;}
      s = ts->f;  tscount = cnt_used; 
      while(tscount > 0)
	{
	  if(j == 1024)
	    {
	      j = 0; ts = ts->next; s = ts->f;
	    }
	  t[i++] = (s+j); j++;
	  tscount--;
	}
    }
  else { nelts = max_nelts;}
}
/************************************************************************/
void delete_all_contours()
{
  elt **el, *t;
  int i,c = 0;
  el = elts;
  if(cnt_used)
    {
      for(i = 0; i < nelts; i++)
	{
	  t = *el;
	  if( t->type == CNTLINE)
	    { t->depth = -123456.0; c++;}
	  else t->depth = 123456.0;
	  el++;
	}
      qsort(elts, max_nelts, sizeof(elt *), icmp);  
      nelts -= c;
      redraw = 1;
    }
}
/**********************************************************************/
