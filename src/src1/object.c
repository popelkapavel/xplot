/*******************************************************************/
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "vogl.h"
#include "pvm.h"
#include "object.h"
/*******************************************************************/
extern int     plot3d,is_rainbow[];
extern char    *desfile, *datfile;
/*******************************************************************/
object *new_object(), *add_an_object();
object which_object();
float  **new_indices(); /* malloc new_indices                  */
object *obj_list;       /* list of all objects                 */
elt    **elts;          /* all elements are here               */
elt    *elt1;           /* storage for disp list               */
float  *vts;            /* vertices in R3                      */
/******************************************************************/
xlst **dlst;
xlst *dlst1;
/******************************************************************/
int nelts;           /* length of the display list    */
int nvts;            /* number of vertices            */
int nobjs;           /* number of objects             */
int nmats = 1;       /* number of materials requested */
/*******************************************************************/
float xmaxn = -10000.0 ,xminn = 10000.0,ymaxn = -10000.0;
float yminn = 10000.0,zmaxn = -10000.0,zminn = 10000.0;
float centerx,centery,centerz;
/*******************************************************************/
extern char *malloc();
extern void  use_new_material();
extern void multvecor();
int  skip_thisline();
void check_rangen();
void initialize_colors();
int my_fgets(), surf_offdata(), curv_griddata();
void recover_line_single_color();
void transform(), do_transform();
char *strip_data();
/*******************************************************************/
void read_data()  
{
  FILE *fp, *fp1, *fopen(); 
  int  i, j, k, l,m,n,row,col, has_datafile = 0, count = 0;
  int tf, offtf;
  object *tmp_obj; 
  xlst *telt, *el, *offelt;
  char line[256],*cptr;
  float *x,b,c,d,e,f,aa,bb,cc,a;
  float **ind, *vert1, *vert2;

  i = 0;
  while( i < 10 && ((fp = fopen(desfile,"r")) == (FILE *)NULL) ) 
    {sleep(1); i++;}

  if( fp == (FILE *)NULL)
    {
      (void)fprintf(stderr, "Cannot open description file\n");
      exit(1);
    }
  (void)fscanf(fp, "%d", &plot3d);
  (void)fscanf(fp, "%d", &nobjs);
  for(i = 0; i< nobjs; i++)
    {
      tmp_obj = add_an_object();
      (void)fscanf(fp, "%d", &(tmp_obj->type));
      (void)fscanf(fp, "%d", &(tmp_obj->datatype));
      (void)fscanf(fp, "%d", &(tmp_obj->serial));
      (void)fscanf(fp, "%d", &(tmp_obj->color));
      (void)fscanf(fp, "%d", &(tmp_obj->n1));
      (void)fscanf(fp, "%d", &(tmp_obj->n2));
      (void)fscanf(fp, "%d", &(tmp_obj->whatever));

      if(tmp_obj->datatype == UNKNOWN_DAT)
	{
	  (void)fscanf(fp,"%s",line);
	  tmp_obj->filename=malloc((strlen(line)+4)*sizeof(char));
	  (void)strcpy(tmp_obj->filename,line);
	  if(tmp_obj->type==SURFACE)
	    {
	      if(tmp_obj->n1 == 0)
		{
		  tmp_obj->datatype=surf_offdata(line,&m,&n);
		  tmp_obj->n1 = m; tmp_obj->n2 = n;
		}
	      else tmp_obj->datatype = GRID_DAT;
	    }
	  else if(tmp_obj->type==CURVE)
	    {
	      if(tmp_obj->n1 == 0)
		{
		  tmp_obj->datatype = curv_griddata(line,&m,&n);	    
		  tmp_obj->n1 = m; tmp_obj->n2 = n;
		}
	      else 
		tmp_obj->datatype = GRID_DAT;
	    }
	  has_datafile = 1;
	}
      if( tmp_obj->type == CURVE && tmp_obj->whatever != 0)
	tmp_obj->datatype = POIN_DAT;
      if(tmp_obj->type == SURFACE) nmats += tmp_obj->color;
    }
  /* see if the plot title is set */
  (void) fscanf(fp, "%d", &i);
  if( i ) 
    {
      titleset = 1;
      (void) fgets(titlestring,255,fp); 
      i = strlen(titlestring);
      if( i ) titlestring[ i-1] = '\0';
    }
  else titleset = 0;
  (void) fclose(fp);
  /******************************************************************
   * Now we have figured out all the object info.
   *****************************************************************/
  nelts = 116;   nvts = AXESDATA;  /* reserved for coor axes */
  tmp_obj = obj_list;
  for(i = 0; i < nobjs; i++)
    {
      switch(tmp_obj->type)
	{
	case SURFACE:
	  switch(tmp_obj->datatype)
	    {
	    case GRID_DAT:
	      tmp_obj->nv = (tmp_obj->n1)*(tmp_obj->n2);
	      nelts += (tmp_obj->n1 -1)*(tmp_obj->n2 -1);
	      nvts +=  (tmp_obj->nv);
	      break;
	    case NOFF_DAT:
	      tmp_obj->nv = (tmp_obj->n1);
	      nvts +=  (tmp_obj->n1);	  	  
	      nelts += (tmp_obj->n2);
	      break;
	    default:
	      break;
	    }
	  break;
	case CURVE:
	  switch(tmp_obj->datatype)
	    {
	    case GRID_DAT:
	      tmp_obj->nv = (tmp_obj->n1);
	      nelts += (tmp_obj->n1 -1);
	      nvts +=  (tmp_obj->n1);
	      break;
	    case POIN_DAT:
	      tmp_obj->nv = (tmp_obj->n1);
	      nelts += (tmp_obj->n1);
	      nvts +=  (tmp_obj->n1);	  
	      break;
	    default:
	      break;
	    }
	  break;
	default:
	  break;
	}
      tmp_obj = tmp_obj->next;
    }
  /*******************************************************************
   * Now we know the size all objects, allocate space for everything
   * Note: waste some space if there are curves.
   ******************************************************************/
  vts = (float *)malloc((unsigned) (6 * nvts * sizeof(float)));

  dlst= (xlst **)malloc((unsigned) (nelts * sizeof(xlst *)));
  dlst1= (xlst *)malloc((unsigned) (nelts * sizeof(xlst)));
  if( vts == (float *)NULL || dlst == (xlst **)NULL || dlst1 == (xlst *)0)
    {  (void) fprintf(stderr, "Out of memory A\n");  exit(3);}
  {
    xlst *t;
    for(i = 0; i< nelts; i++)
      {
	t =  dlst1 +i;       dlst[i] = t;
	t->done = 0; t->nv1 = 0; t->notused=0; t->type=UNKNOWNTYPE;
      }
  }
  /******************************************************************/
  if( (fp = fopen(datfile, "r")) == (FILE *)NULL)
    if( !has_datafile)
      { (void)fprintf(stderr, "Cannot open data file\n");  exit(4);}
  /******************************************************************
   * object descriptions are in, spaces are allocated,
   * read in data and build the display list.
   ******************************************************************/

  tf = 0;       telt = dlst1;      tmp_obj = obj_list;
  for(i = 0; i < nobjs; i++)
    {
      x = vts+ 6*tf;        el= telt;
      switch(tmp_obj->type)
	{
	case SURFACE:
	  switch(tmp_obj->datatype)
	    {
	    case GRID_DAT:
	      fp1 = fp;
	      if(tmp_obj->filename != (char *)NULL) /* from a data file */
		if( (fp1 = fopen(tmp_obj->filename,"r")) == (FILE *)NULL)
		  {
		    (void)fprintf(stderr,"Cannot open %s\n",tmp_obj->filename);
		    exit(4);
		  }
	      m = (tmp_obj->n1)*(tmp_obj->n2);
	      for(n = 0; n < m; n++)
		{
		  if(! my_fgets(line,255,fp1)) 
		    {
		      (void)fprintf(stderr,"Error while reading data\n");
		      exit(5);
		    }
		  (void)sscanf(line,"%f %f %f",(x),(x+1),(x+2)); 
		  check_rangen(x); x += 6;
		}
	      if(tmp_obj->filename != (char *)NULL) (void)fclose(fp1);

	      if(tmp_obj->color ==0) m = 16;   else  m = 136;
	      row = tmp_obj->n1; col = tmp_obj->n2; 
	      n= tf;  /* tf is the position of the current vertex */
	      for(j = 0; j < row - 1; j++)
		{ 
		  for(k = 0; k < col -1; k++)
		    {
		      el->type = QUADPOLY;  el->mat = m; el->nv = 4;
		      el->f = ind =  new_indices(4);
		      l = 6 * (n+k);
		      ind[0]= vts + l;    ind[1]= vts +l+6*col; 
		      ind[2]= ind[1]+6;   ind[3]= vts + l+6;
		      if(k < col -2) el->r = el+1; else el->r = (xlst *)0;
		      if(j < row -2) el->t = el+col-1; else el->t= (xlst *)0;
		      if(k > 0) el->l = el -1; else el->l = (xlst *)0;
		      if(j > 0) el->b = el+1-col; else el->b=(xlst *)0;
		      if(j < row -2)
			{
			  if(k < col -2) el->tr=el->t+1; else el->tr=(xlst *)0;
			  if(k > 0) el->tl=el->t -1; else el->tl =(xlst *)0;
			}
		      if( j > 0)
			{
			  if(k < col -2) el->br=el->b+1; else el->br=(xlst *)0;
			  if(k > 0) el->bl=el->b -1; else el->bl =(xlst *)0;
			}
		      el++;
		    }
		  n += col;
		} /* finsh constructing display list for SUR_GRID */
	      tf +=  row * col; /* update the vts pointer */	  
	      el = telt;
	      for(j = 0; j < (row-1)*(col-1); j++) /* compute normal */
		{
		  vert1 = (el->f)[0]; vert2 = (el->f)[2];
		  a = vert2[0] - vert1[0];
		  b = vert2[1] - vert1[1];
		  c = vert2[2] - vert1[2];
		  vert1 = (el->f)[1]; vert2 = (el->f)[3];
		  d = vert2[0] - vert1[0];
		  e = vert2[1] - vert1[1];
		  f = vert2[2] - vert1[2];
		  aa = (b * f - c * e);
		  bb = (c*d - a* f);
		  cc = (a*e - b* d);
		  a = sqrt(aa*aa + bb*bb + cc*cc); 
		  if (a == 0.0) { a = 1.0; cc = 1.0;}
		  vert2 = (el->f)[0];
		  vert2[3] = aa/a; vert2[4] = bb/a; vert2[5] = cc/a;
		  el++;
		}
	      telt += (row-1)*(col-1);
	      break;
	    case NOFF_DAT:
	      if( (fp1 = fopen(tmp_obj->filename,"r")) == (FILE*)NULL)
		{
		  (void)fprintf(stderr, "Cannot open %s\n",tmp_obj->filename);
		  exit(1);
		}
	      count = 0; offelt = telt; offtf = tf; 
	      while(count < tmp_obj->n1)
		{
		  x = vts+ 6*offtf; 
		  (void) my_fgets(line,255,fp1);
		  if(skip_thisline(line)) my_fgets(line,255,fp1);
		  /* now the first line or two are skiped */
		  cptr = strip_data(line, &m);
		  cptr = strip_data(cptr, &n);
		  for(j = 0; j < m; j++)
		    {
		      (void) my_fgets(line,255,fp1);		
		      (void)sscanf(line,"%f %f %f",(x),(x+1),(x+2));
		      check_rangen(x); x += 6;
		    }    /* data are in */
		  row = n;
		  el = offelt;
		  if(tmp_obj->color ==0) col = 16; else  col = 136;
		  for(j = 0; j < row; j++)
		    {
		      (void) my_fgets(line,255,fp1); cptr = line;
		      cptr = strip_data(cptr,&n); 
		      el->type = POLYGON; el->nv = n; el->mat = col;
		      el->f =ind= new_indices(n);
		      for(k=0; k < n; k++)
			{
			  cptr = strip_data(cptr,&l);
			  ind[k] = vts + 6*(l + offtf);
			}
		      el++;
		    }
		  offtf += m; offelt += row; /* update vts ptr and elt ptr */
		  count += m;
		}
	      (void) fclose(fp1);
	      tf += tmp_obj->n1;   /* update vts ptr */
	      el = telt; row = tmp_obj->n2;
/*              for(j=0;j<row;j++)
                telt[j]->f[j][3]=telt->f[j][4]=telt->f[j][5]=0.57735; */
	      for(j = 0; j < row; j++)
		{
		  vert1 = (el->f)[0]; vert2 = (el->f)[1];
		  a = vert2[0] - vert1[0];
		  b = vert2[1] - vert1[1];
		  c = vert2[2] - vert1[2];
		  vert2 = (el->f)[2];		  
		  d = vert2[0] - vert1[0];
		  e = vert2[1] - vert1[1];
		  f = vert2[2] - vert1[2];
		  aa = (b * f - c * e);
		  bb = (c*d - a* f);
		  cc = (a*e - b* d);
		  a = sqrt(aa*aa + bb*bb + cc*cc); 
		  if (a == 0.0) a = 1.0;
		  vert2 = (el->f)[0]; 
		  vert2[3] = aa/a; vert2[4] = bb/a;  vert2[5] = cc/a;
		  el++;
		}
	      telt += row; /* update elts pointer */
	      break;
	    default:
	      break;
	    }
	  break;
	case CURVE:  /* curve */
	  switch(tmp_obj->datatype)
	    {
	    case GRID_DAT:
	      fp1 = fp;
	      if(tmp_obj->filename != (char *)NULL)
		{
		  if( (fp1 = fopen(tmp_obj->filename,"r"))==(FILE *)NULL)
		    {
		      (void)fprintf(stderr, "Cannot open %s\n",
				    tmp_obj->filename);
		      exit(5);
		    }
		}
	      row = tmp_obj->n1;
	      for(n = 0; n < row; n++) 
		{
		  (void)my_fgets(line,255,fp1);
		  (void)sscanf(line,"%f %f %f",(x),(x+1),(x+2)); 
		  check_rangen(x); x += 6;
		}
	      if(tmp_obj->filename != (char *)NULL) (void)fclose(fp1);
	      m = tmp_obj->color;  
	      n= tf; 
	      for(j = 0; j < row - 1; j++)
		{  
		  el->type = POLYLINE;  el->color = m; 
		  el->mat = m;  el->nv = 2;
		  el->f = ind =  new_indices(2);
		  l = 6 * (n+j);
		  ind[0]=vts + l; ind[1]=vts+l+6;
		  el++;
		}
	      tf +=  row; 
	      el = telt;
	      for(j = 0; j < row-1; j++) 
		{
		  vert1 = (el->f)[0]; vert2 = (el->f)[1];
		  a = vert2[0] -vert1[0];
		  b = vert2[1] -vert1[1];
		  c = vert2[2] -vert1[2];
		  d = b; e = -a; f = 0.0;
		  aa = (b * f - c * e);
		  bb = (c*d - a* f);
		  cc = (a*e - b* d);
		  a = sqrt(aa*aa + bb*bb + cc*cc); 
		  if (a == 0.0) a = 1.0;
		  vert2 = (el->f)[0];
		  vert2[3] = aa/a; vert2[4] = bb/a; vert2[5] = cc/a;
		  el++;
		}
	      telt += (row-1); 
	      break;
	    case POIN_DAT:
	      fp1 = fp;
	      if(tmp_obj->filename != (char *)NULL)
		{
		  if( (fp1 = fopen(tmp_obj->filename,"r"))==(FILE *)NULL)
		    {
		      (void)fprintf(stderr, 
				    "Cannot open %s\n",tmp_obj->filename);
		      exit(5);
		    }
		}
	      row = (tmp_obj->n1);
	      for(n = 0; n < row; n++)  
		{
		  (void)my_fgets(line,255,fp1);
		  (void)sscanf(line,"%f %f %f",(x),(x+1),(x+2)); 
		  check_rangen(x); x += 6;
		}
	      if(tmp_obj->filename != (char *)NULL) (void)fclose(fp1);
	      m = tmp_obj->color;
	      n = tf;
	      for(j = 0; j < row ; j++)
		{
		  el->type = LINEPOINT;  el->color = m; 
		  el->mat = m; el->nv = 1;
		  el->f = ind =  new_indices(1); 
		  ind[0]= vts + 6 * (n+j);
		  el++;
		}
	      tf +=  row;  
	      el = telt;
	      for(j = 0; j < row; j++) 
		{
		  vert2 = (el->f)[0]; 
		  vert2[3] = 1.0; vert2[4] = 0.0; vert2[5] = 0.0;
		  el++;
		}
	      telt +=  row; 

	      break;
	    default:
	      break;
	    }
	default:
	  break;
	}
      tmp_obj = tmp_obj->next;
    }
  if(fp != (FILE*)NULL) (void)fclose(fp); /* done with fp */
  
  /**********************************************************************
   *
   *   Translate data to the center  of all objects 
   *
   *********************************************************************/
  {  
    float cx,cy,cz;
    cx = 0.5*(xmaxn+xminn); cy = 0.5*(ymaxn+yminn); cz = 0.5*(zmaxn+zminn);
    centerx = cx; centery=cy;centerz=cz;
    xmaxn -= cx; xminn -= cx; ymaxn -= cy; yminn -= cy; zmaxn -= cz; zminn -= cz;
    x = vts;
    for(i = 0; i < nvts-AXESDATA; i++)
      {
        *x -= cx; *(x+1) -= cy; *(x+2) -= cz;
        x += 6;
      }

  }
}
/*************************************************************/
void add_axes()
{             /* initialize the coordinate box */
  float dt;
  float **ind, *vert2;
  int i,j,k,l;

  object *tmp_obj; 
  elt  *el;


  j = 6*(nvts - AXESDATA);
  dt = (xmaxn -xminn)/9.0;
  for(i = 0;i<10;i++) 
    {vts[j++]=xminn+i*dt; vts[j++]=yminn;vts[j++]=zminn; j += 3;}
  dt = (ymaxn -yminn)/9.0;
  for(i = 0;i<10;i++)
    {vts[j++]=xmaxn; vts[j++]=yminn+i*dt;vts[j++]=zminn; j += 3;}
  dt = (zmaxn -zminn)/9.0;
  for(i = 0;i<10;i++)
    {vts[j++]=xmaxn; vts[j++]=ymaxn;vts[j++]=zminn+i*dt; j += 3;}
  dt = (yminn -ymaxn)/9.0;
  for(i = 0;i<10;i++)
    {vts[j++]=xmaxn; vts[j++]=ymaxn+i*dt;vts[j++]=zmaxn; j += 3;}
  dt = (xminn -xmaxn)/9.0;
  for(i = 0;i<10;i++)
    {vts[j++]=xmaxn+i*dt; vts[j++]=yminn;vts[j++]=zmaxn;j += 3;}    
  dt = (ymaxn -yminn)/9.0;
  for(i = 0;i<10;i++)
    {vts[j++]=xminn; vts[j++]=yminn+i*dt;vts[j++]=zmaxn; j += 3;}
  dt = (zminn -zmaxn)/9.0;
  for(i = 0;i<10;i++)
    {vts[j++]=xminn; vts[j++]=ymaxn;vts[j++]=zmaxn+i*dt; j += 3;}
  dt = (yminn -ymaxn)/9.0;
  for(i = 0;i<10;i++)
    {vts[j++]=xminn; vts[j++]=ymaxn+i*dt;vts[j++]=zminn; j += 3;}
  dt = (zmaxn -zminn)/9.0;
  for(i = 0;i<10;i++)
    {vts[j++]=xminn; vts[j++]=yminn;vts[j++]=zminn+i*dt; j += 3;}
  dt = (xmaxn -xminn)/9.0;
  for(i = 0;i<10;i++)
    {vts[j++]=xminn+i*dt; vts[j++]=ymaxn;vts[j++]=zminn; j += 3;}
  dt = (zmaxn -zminn)/9.0;
  for(i = 0;i<10;i++)
    {vts[j++]=xmaxn; vts[j++]=yminn; vts[j++]=zminn+i*dt; j += 3;}
  dt = (xmaxn -xminn)/9.0;
  for(i = 0;i<10;i++)
    {vts[j++]=xminn+i*dt; vts[j++]=ymaxn;vts[j++]=zmaxn; j += 3;}

  tmp_obj = add_an_object();  tmp_obj->type = AXESBOX; tmp_obj->color = 0;
  tmp_obj->n1 = 90;  tmp_obj->n2 = 1;
  tmp_obj = add_an_object();tmp_obj->type = AXESBOX; tmp_obj->color = 0;
  tmp_obj->n1 = 10;  tmp_obj->n2 = 1;
  tmp_obj = add_an_object(); tmp_obj->type = AXESBOX; tmp_obj->color = 0;
  tmp_obj->n1 = 10;  tmp_obj->n2 = 1;
  tmp_obj = add_an_object(); tmp_obj->type = AXESBOX; tmp_obj->color = 0;
  tmp_obj->n1 = 10;  tmp_obj->n2 = 1;
  nobjs += 4;
  j =  (nvts - AXESDATA);    /* where the data starts      */
  el = *(elts +(nelts - 116)); /* where the disp list starts */
  for(i = 0; i <89; i++)
    {
      el->type = AXESBOX; el->nv = 2;
      el->f = ind = new_indices(2);
      l = 6*(j+i); 
      vert2 = ind[0] = vts+l; ind[1] = vts+l+6;
      vert2[3] = 1.0; vert2[4] = 0.0; vert2[5] = 0.0; 
      el++;
    }
  j += 90;
  for(k = 0; k < 3; k++)
    {
      for(i = 0; i <9; i++)
	{
	  el->type = AXESBOX; el->nv = 2; el->f = ind = new_indices(2);
	  l = 6*(j+i); 
	  ind[0] = vert2 = vts+l; ind[1] = vts+ l+6;
	  vert2[3] = 1.0; vert2[4] = 0.0; vert2[5] = 0.0; 
	  el++;
	}
      j += 10;
    }
}
/*************************************************************/
int skip_thisline(line) char *line;
{
  while(*line == ' ' || *line == '\t' || *line == '\n') line++;
  if(isalpha(*line) || isalpha(*(line+1))) return(1);
  else if(isdigit(*line)) return(0);
  return(1);
}
/*************************************************************/
static int available_indices = 0, current_ind_point = 0;
typedef struct mymem{
  float **i;
  struct mymem *next;
} indices;
indices *indstorage, *current_ind;
float **new_indices(n)
     int n;
{
  float **ans;
  if(available_indices - n < 0)
    {
      indices **t = &indstorage;
      while( (*t) != (indices *)NULL) t = &((*t)->next);
      *t = (indices *)malloc((unsigned)sizeof(indices));
      (*t)->i = (float **)malloc(2048 * sizeof(float *));
      if( (*t) == (indices *)NULL || (*t)->i == (float **)NULL)
	{
	  (void)fprintf(stderr, "Out of memory I\n");
	  exit(1);
	}
      (*t)->next = (indices *)NULL;
      available_indices = 2048; current_ind_point = 0;
      current_ind = *t;
    }
  ans = current_ind->i + current_ind_point;
  available_indices -= n; current_ind_point += n;
  return(ans);
}
/*************************************************************/
object *add_an_object()
{
  object **t = &obj_list;
  while( *t != (object *)NULL)  t = &((*t)->next);
  *t = new_object();
  return(*t);
}
object *new_object()
{
  object *tmp;
  tmp = (object *) malloc((unsigned)sizeof( object));
  if( tmp == (object *)NULL)
    {
      (void) fprintf(stderr, "Out of memory C\n");
      exit(2);
    }
/*  identmatrix(tmp->m);*/
  tmp->filename = (char *)NULL;
  tmp->next = (object *)NULL;
  return(tmp);
}
/*************************************************************/
void check_rangen(ptr)
     float *ptr;
{
  if( *ptr > xmaxn) xmaxn = *ptr;
  if( *ptr < xminn) xminn = *ptr;
  ptr++;
  if( *ptr > ymaxn) ymaxn  = *ptr;
  if( *ptr < yminn) yminn = *ptr;  
  ptr++;
  if( *ptr > zmaxn) zmaxn  = *ptr;
  if( *ptr < zminn) zminn = *ptr;
}
/*************************************************************/
int icmp(a,b) elt **a,**b;
{
  /* if(a->depth == b->depth) return(0); */
  return( ((*a)->depth >= (*b)->depth)? -1: 1);
}
/*************************************************************/
extern int current_mat;      
void initialize_colors()
{
  use_new_material(3);
  if(nmats)
    {
      current_mat = 1;
      use_new_material(8);
      current_mat = 0;      
    }
}
/*************************************************************/
int my_fgets(str,leng,fp)
     char *str; int leng; FILE *fp;
{
  int  found = 0;
  char *c = str;
  while(!found)
    {
      if( fgets(str,leng,fp) == (char *)NULL) return(0);
      while( *c && (*c == ' ' || *c == '\t')) c++;
      if( *c == '\0' || *c == '#' || *c == '\n') found = 0;
      else found = 1;
    }
  return(1);
}
/**********************************************************************/      
char *strip_data(ptr,n)
     char *ptr; int *n;
{
  (void)sscanf(ptr, "%d", n);
  while( *ptr == ' ' || *ptr == '\t') ptr++;
  while( *ptr != ' ' && *ptr != '\t') ptr++;
  return(ptr);
}
/**********************************************************************/      
int surf_offdata(file,m,n)  /* figure out OFF data set */
     char *file; int *m,*n;
{
  FILE *fp, *fopen(); char line[256];
  int i,k,l, totalvts=0, totalpoly=0;
  char *c;

  if( (fp = fopen(file,"r")) == (FILE *)NULL)
    { (void)fprintf(stderr,"Cannot open %s\n", file); exit(4);}
  
  while(my_fgets(line,255,fp))
    {
      if(skip_thisline(line)) my_fgets(line,255,fp);
      c = line; c=strip_data(c, &k); c=strip_data(c,&l);
      totalvts += k; totalpoly +=l;
      for(i = 0; i <k+l; i++)
	{
	  if(!my_fgets(line,255,fp))
	    {
	      (void)fprintf(stderr,"%s is not in OFF format.\n", file);
	      exit(5);
	    }
	}
    }
  *m = totalvts; *n = totalpoly;
  (void)fclose(fp);
  return(NOFF_DAT);
}
/***************************************************************/
int curv_griddata(file,m,n)
     char *file; int *m,*n;
{
  FILE *fp, *fopen(); char line[256];
  int  totalvts=0;

  if( (fp = fopen(file,"r")) == (FILE *)NULL)
    { (void)fprintf(stderr,"Cannot open %s\n", file); exit(4);}
  
  while(my_fgets(line,255,fp))
    totalvts++;
  *m = totalvts; *n = 1;
  (void)fclose(fp);
  return(GRID_DAT);
}
	  
/***********************************************************************/  
void recover_line_single_color()
{
  int i;   elt   **el = elts;
  for(i = 0; i < nelts  ; i++)
    {
      if((*el)->type >= POLYLINE && (*el)->type <=  CNTLINE)
	{  (*el)->color = (*el)->mat; }
      el++;
    }
}
/***************************************************************************/
static int available_floats = 0, current_floats = 0;
typedef struct float_mem{
  float *f;
  struct float_mem *next;
} mem_floats;
mem_floats *floatstorage = (mem_floats *)0, *current_float_ptr;
float *new_floats(n) int n;   /*allocate n floats */
{
  float *ans;
  if(available_floats - n < 0)
    {
      mem_floats **t = &floatstorage;
      while( (*t) != (mem_floats *)NULL)   t = &((*t)->next);
      *t = (mem_floats *)malloc((unsigned)sizeof(mem_floats));
      (*t)->f = (float *)malloc(2048 * sizeof(float));
      if( (*t) == (mem_floats *)NULL || (*t)->f == (float *)NULL)
	{
	  (void)fprintf(stderr, "Out of memory III\n");
	  exit(1);
	}
      (*t)->next = (mem_floats *)NULL;
      available_floats = 2048; current_floats = 0;
      current_float_ptr = *t;
    }
  ans = current_float_ptr->f + current_floats;
  available_floats -= n; current_floats += n;
  return(ans);
}
/*************************************************************************/
extern void free_cnt_stuff();
void free_data()
{ 
  int i;
  object *o = obj_list,*oo;
  indices *ind = indstorage, *ind1;
  mem_floats *fs = floatstorage, *fs1;
  while( ind != (indices *)NULL)
    {
      ind1 = ind->next;
      (void) free (ind->i);  (void)free(ind);
      ind = ind1;
    }
  available_indices = 0; current_ind_point = 0;
  indstorage =(indices *)0; current_ind = (indices *)0;
  while( fs != (mem_floats *)NULL)
    {
      fs1 = fs->next;
      (void) free (fs->f);  (void)free(fs);
      fs = fs1;
    }

  floatstorage = current_float_ptr= (mem_floats *)0; 
  available_floats = 0; current_floats = 0;
  (void)free( (char *)elts);
  (void)free( (char *)elt1);  
  (void)free( (char *)vts );

  for(i=0; i<nobjs; i++)
    {
      if(o->filename != (char *)NULL) (void)free(o->filename);
      oo = o->next;
      (void)free( (char *)o);
      o = oo;
    }

  obj_list = (object *)0;
  elts = (elt **)0; elt1 = (elt *)0; vts = (float *)0; nobjs = 0;
  nmats = 1;     titleset = 0;
  xmaxn = -10000.0; xminn = 10000.0; ymaxn = -10000.0;
  yminn = 10000.0; zmaxn = -10000.0; zminn = 10000.0;
  is_rainbow[0]=is_rainbow[1]=0; current_mat = 0;
  free_cnt_stuff();
}
/**********************************************************************/
