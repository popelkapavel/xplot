#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "vogl.h"
#include "pvm.h"
#include "object.h"

/*---------------------------------------------------------------
 * save materials, lighting setup and rotation matrices to file.
 *---------------------------------------------------------------*/
extern light lig;
extern Matrix rotmatrix;
extern mat_entry cmat[2];
extern int current_mat,nmats,is_rainbow[2],do_not_sort;
extern int plot3d,boxon,depthcue,fixedlight,lightedline,twosidedobj,redraw;
extern int not_zbuffered_wireframe, wireframe_over_polygon, polymodeflag;
extern float sizze;
extern short RGBtable[256][3];
/*****************************************************************/
extern char *getenv();
extern int my_fgets();
extern void initialize_screen_axis(),make_materialn();
extern void  setup_plotmodes(),  save_plotmodes(),rainbow_color();
int do_save_setup(fname) char *fname;
{
  FILE *fp, *fopen();
  char *ptr = fname;  char real_file_name[256];
  mat_entry *mat;

  while( *ptr == ' ' || *ptr == '\t') ptr++;
  if(*ptr == '~')
    {
      ptr++;
      (void) strcpy( &(real_file_name[0]),getenv("HOME"));
      if(real_file_name[strlen(real_file_name)-1] == '/')
	real_file_name[strlen(real_file_name)-1] = '\0';
      (void) strcat( &(real_file_name[0]), ptr); 
    }
  else strcpy(&(real_file_name[0]),ptr);
  fp = fopen(real_file_name,"w");
  if( fp == (FILE *)0) return(0);
  /*----------------------------------
   * Viewing and projection
   *---------------------------------*/
  (void)fprintf(fp,"View:\n");
  (void)fprintf(fp," %f %f %f\n",
		gview.vx/sizze,gview.vy/sizze,gview.vz/sizze);
  (void)fprintf(fp," %f %f %f %f\n" ,
		    gview.left /sizze, gview.right /sizze,
		    gview.top /sizze, gview.bottom /sizze);
  (void)fprintf(fp," %f %f %f %f\n",
		gview.near_ /sizze, gview.far_ /sizze,
		gview.pnear /sizze, gview.pfar /sizze);
  (void)fprintf(fp," %f %f\n",
		gview.aspect, gview.fovy);
  (void)fprintf(fp," %f %f %f\n",
		gview.xscals,gview.yscals,gview.zscals);
  (void)fprintf(fp," %f %f %f\n",
		gview.xscale,gview.yscale,gview.zscale);
  (void)fprintf(fp," %f %f %f\n",
		gview.xtranp /sizze,gview.ytranp/sizze,gview.ztranp/sizze);
  (void)fprintf(fp," %f %f %f\n",
		gview.xtrans /sizze,gview.ytrans/sizze,gview.ztrans/sizze);
  (void)fprintf(fp," %d\n", gview.proj_type);
  /*----------------------------------
   * Rotation matrix
   *---------------------------------*/  
  (void)fprintf(fp,"Rotation Matrix\n");
  (void)fprintf(fp," %f %f %f %f\n",  
	rotmatrix[0][0],rotmatrix[0][1],rotmatrix[0][2],rotmatrix[0][3]);
  (void)fprintf(fp," %f %f %f %f\n",  
	rotmatrix[1][0],rotmatrix[1][1],rotmatrix[1][2],rotmatrix[1][3]);
  (void)fprintf(fp," %f %f %f %f\n",  
	rotmatrix[2][0],rotmatrix[2][1],rotmatrix[2][2],rotmatrix[2][3]);
  (void)fprintf(fp," %f %f %f %f\n",  
	rotmatrix[3][0],rotmatrix[3][1],rotmatrix[3][2],rotmatrix[3][3]);
  /*----------------------------------
   * Light position and color
   *---------------------------------*/  
  (void)fprintf(fp,"Lighting\n");
  (void)fprintf(fp," %f %f %f %f %f\n",  
		lig.azim,lig.incl, (lig.x)[0],(lig.x)[1],(lig.x)[2]);
  (void)fprintf(fp," %f %f %f\n",lig.red,lig.grn,lig.blu);
  /*----------------------------------
   * Material
   *---------------------------------*/  
  (void)fprintf(fp,"Material:\n 2\n");
  mat = &(cmat[0]);
  (void)fprintf(fp," %f %f %f\n",    
		(mat->ambient)[0],(mat->ambient)[1],(mat->ambient)[2]);
  (void)fprintf(fp," %f %f %f\n",    
		(mat->diffuse)[0],(mat->diffuse)[1],(mat->diffuse)[2]);
  (void)fprintf(fp," %f %f %f\n",    
		(mat->specular)[0],(mat->specular)[1],(mat->specular)[2]);
  (void)fprintf(fp," %f \n %d\n", mat->shininess,is_rainbow[0]);
  mat = &(cmat[1]);
  (void)fprintf(fp," %f %f %f\n",    
		(mat->ambient)[0],(mat->ambient)[1],(mat->ambient)[2]);
  (void)fprintf(fp," %f %f %f\n",    
		(mat->diffuse)[0],(mat->diffuse)[1],(mat->diffuse)[2]);
  (void)fprintf(fp," %f %f %f\n",    
		(mat->specular)[0],(mat->specular)[1],(mat->specular)[2]);
  (void)fprintf(fp," %f \n %d\n", mat->shininess,is_rainbow[1]);

  (void)fprintf(fp," %d %d\n", current_mat,nmats);

  /*----------------------------------
   *  Background color
   *---------------------------------*/    
  (void)fprintf(fp,"Background color:\n");
  (void)fprintf(fp, " %d %d %d \n", (int)RGBtable[0][0],
		(int)RGBtable[0][1],(int)RGBtable[0][2]);
  /*----------------------------------
   *  Ploting modes
   *---------------------------------*/    
  (void)fprintf(fp,"Plotting Modes:\n");
  (void)fprintf(fp, " %d %d %d %d %d %d %d %d %d %d\n",
		plot3d,boxon,depthcue,lightedline,twosidedobj,
		not_zbuffered_wireframe, wireframe_over_polygon, 
		polymodeflag,fixedlight,do_not_sort);
  save_plotmodes(fp);
  /*----------------------------------
   *  Anything more ?
   *---------------------------------*/    
  (void)fclose(fp);
  return (1);
}
/**********************************************************************/
int do_load_setup(fname) char *fname;
{
  FILE *fp, *fopen();
  char *ptr = fname;  char real_file_name[256];
  float tf[6]; int ti,tj,tk;
  mat_entry *mat;

  while( *ptr == ' ' || *ptr == '\t') ptr++;
  if(*ptr == '~')
    {
      ptr++;
      (void) strcpy( &(real_file_name[0]),getenv("HOME"));
      if(real_file_name[strlen(real_file_name)-1] == '/')
	real_file_name[strlen(real_file_name)-1] = '\0';
      (void) strcat( &(real_file_name[0]), ptr); 
    }
  else strcpy(&(real_file_name[0]),ptr);
  fp = fopen(real_file_name,"r");
  if( fp == (FILE *)0) return(0);
  /*----------------------------------
   * The actual work. Read from fp
   *---------------------------------*/
#define LINEE real_file_name
  while(my_fgets(LINEE,255,fp))
    {
      if( !strncmp(LINEE,"View",3))
	{
	  if(plot3d) /* use the view setup only for plot3d */
	    {
	      my_fgets(LINEE,255,fp);
	      (void)sscanf(LINEE,"%f %f %f",tf,tf+1,tf+2);
	      gview.vx = tf[0] * sizze;
	      gview.vy = tf[1] * sizze;
	      gview.vz = tf[2] * sizze;
	      my_fgets(LINEE,255,fp);
	      (void)sscanf(LINEE,"%f %f %f %f",tf,tf+1,tf+2,tf+3);
	      gview.left = tf[0] * sizze;
	      gview.right = tf[1] * sizze;
	      gview.top = tf[2] * sizze;
	      gview.bottom = tf[3] * sizze;
	      my_fgets(LINEE,255,fp);
	      (void)sscanf(LINEE,"%f %f %f %f",tf,tf+1,tf+2,tf+3);
	      gview.near_ = tf[0] * sizze;
	      gview.far_ = tf[1] * sizze;
	      gview.pnear = tf[2] * sizze;
	      gview.pfar = tf[3] * sizze;
	      my_fgets(LINEE,255,fp);
	      (void)sscanf(LINEE,"%f %f",tf,tf+1);
	      gview.aspect = tf[0]; gview.fovy = tf[1];
	      my_fgets(LINEE,255,fp);
	      (void)sscanf(LINEE,"%f %f %f",tf,tf+1,tf+2);
	      gview.xscals = tf[0];gview.yscals = tf[1];gview.zscals = tf[2];
	      my_fgets(LINEE,255,fp);
	      (void)sscanf(LINEE,"%f %f %f",tf,tf+1,tf+2);
	      gview.xscale = tf[0];gview.yscale = tf[1];gview.zscale = tf[2];
	      my_fgets(LINEE,255,fp);
	      (void)sscanf(LINEE,"%f %f %f",tf,tf+1,tf+2);
	      gview.xtranp = tf[0] *sizze;
	      gview.ytranp = tf[1] *sizze;
	      gview.ztranp = tf[2] *sizze;
	      my_fgets(LINEE,255,fp);
	      (void)sscanf(LINEE,"%f %f %f",tf,tf+1,tf+2);
	      gview.xtrans = tf[0] *sizze;
	      gview.ytrans = tf[1] *sizze;
	      gview.ztrans = tf[2] *sizze;
	      my_fgets(LINEE,255,fp);
	      (void)sscanf(LINEE,"%d",&(ti));
	      gview.proj_type = (short )ti;  gview.proj_change = 1;
	      initialize_screen_axis();
	    }
	  else
	    {for(ti=0;ti<9;ti++) my_fgets(LINEE,255,fp);}
	}
      else  if( !strncmp(LINEE,"Rota",3))
	{
	  if(plot3d)
	    {
	      my_fgets(LINEE,255,fp);
	      (void)sscanf(LINEE,"%f %f %f %f",tf,tf+1,tf+2, tf+3);	  
	      rotmatrix[0][0]=tf[0];rotmatrix[0][1]=tf[1];
	      rotmatrix[0][2]=tf[2];rotmatrix[0][3]=tf[3];
	      my_fgets(LINEE,255,fp);
	      (void)sscanf(LINEE,"%f %f %f %f",tf,tf+1,tf+2, tf+3);	  
	      rotmatrix[1][0]=tf[0];rotmatrix[1][1]=tf[1];
	      rotmatrix[1][2]=tf[2];rotmatrix[1][3]=tf[3];
	      my_fgets(LINEE,255,fp);
	      (void)sscanf(LINEE,"%f %f %f %f",tf,tf+1,tf+2, tf+3);	  
	      rotmatrix[2][0]=tf[0];rotmatrix[2][1]=tf[1];
	      rotmatrix[2][2]=tf[2];rotmatrix[2][3]=tf[3];
	      my_fgets(LINEE,255,fp);
	      (void)sscanf(LINEE,"%f %f %f %f",tf,tf+1,tf+2, tf+3);	  
	      rotmatrix[3][0]=tf[0];rotmatrix[3][1]=tf[1];
	      rotmatrix[3][2]=tf[2];rotmatrix[3][3]=tf[3];
	    }
	  else
	    {for(ti = 0; ti < 4; ti++) my_fgets(LINEE,255,fp);}
	}
      else  if( !strncmp(LINEE,"Ligh",3))
	{
	  my_fgets(LINEE,255,fp);
	  (void)sscanf(LINEE,"%f %f %f %f %f",tf,tf+1,tf+2, tf+3,tf+4);	  
	  lig.azim=tf[0]; lig.incl=tf[1];
	  (lig.x)[0]=tf[2]; (lig.x)[1]=tf[3];(lig.x)[2]=tf[4];
	  my_fgets(LINEE,255,fp);
	  (void)sscanf(LINEE,"%f %f %f",tf,tf+1,tf+2);	  
	  lig.red=tf[0]; lig.grn=tf[1];lig.blu=tf[2];
	  mapcolor(7, (int) (255.0* lig.red), (int) (255.0* lig.grn),
	    (int) (255.0* lig.blu));
	  ti = current_mat;
	  current_mat = 0; make_materialn();
	  current_mat = 1; make_materialn();
	  current_mat =ti;
	}
      else  if( !strncmp(LINEE,"Mate",3)) /* only take the first 2 */
	{
	  my_fgets(LINEE,255,fp);  (void)sscanf(LINEE,"%d",&tk);
	  for(tj = 0; tj < tk; tj++)
	    {
	      if(tj < 2) 
		{
		  mat = &(cmat[tj]);
		  my_fgets(LINEE,255,fp);
		  (void)sscanf(LINEE,"%f %f %f",tf,tf+1,tf+2);
		  (mat->ambient)[0] = tf[0];
		  (mat->ambient)[1] = tf[1];
		  (mat->ambient)[2] = tf[2]; 
		  my_fgets(LINEE,255,fp);
		  (void)sscanf(LINEE,"%f %f %f",tf,tf+1,tf+2); 
		  (mat->diffuse)[0] = tf[0];
		  (mat->diffuse)[1] = tf[1];
		  (mat->diffuse)[2] = tf[2]; 
		  my_fgets(LINEE,255,fp);
		  (void)sscanf(LINEE,"%f %f %f",tf,tf+1,tf+2);	  	  
		  (mat->specular)[0] = tf[0];
		  (mat->specular)[1] = tf[1];
		  (mat->specular)[2] = tf[2]; 
		  my_fgets(LINEE,255,fp);
		  (void)sscanf(LINEE,"%f",tf);  mat->shininess = tf[0];
		  current_mat = tj; make_materialn();
		  my_fgets(LINEE,255,fp);
		  (void)sscanf(LINEE,"%d",&(is_rainbow[tj]));
		  if(is_rainbow[tj]) rainbow_color();
		}
	      else
		{for(ti = 0; ti< 5; ti++) my_fgets(LINEE,255,fp);}
	    }
	  my_fgets(LINEE,255,fp);
	  (void)sscanf(LINEE,"%d %d",&current_mat,&nmats);
	} 
      else  if( !strncmp(LINEE,"Back",3))
	{
	  my_fgets(LINEE,255,fp);
	  (void)sscanf(LINEE,"%d %d %d",&ti,&tj,&tk);
	  RGBtable[0][0]=ti;RGBtable[0][1]=tj; RGBtable[0][2]=tk;
	  mapcolor(0,(int)RGBtable[0][0],(int)RGBtable[0][1],
		   (int) RGBtable[0][2]);
	}
      else  if( !strncmp(LINEE,"Plot",3))
	{
	  my_fgets(LINEE,255,fp);
	  (void)sscanf(LINEE,"%d %d %d %d %d %d %d %d %d %d",
		       &ti,&boxon,&depthcue,&lightedline,&twosidedobj,
		       &not_zbuffered_wireframe,&wireframe_over_polygon, 
		       &polymodeflag, &fixedlight,&do_not_sort);
	  my_fgets(LINEE,255,fp);
	  setup_plotmodes(LINEE);
	}
      else
	{
	  (void)fprintf(stderr,"Unknown attributes %s in %s\n",LINEE,fname);
	}
      redraw=1;
    }
  (void)fclose(fp);
  return(1);
}
/********************************************************************/
