
/*************************************************************
 *  
 *          I R I S P L O T   ---------- new3.c
 *
 *    Copyright (C) 1989 Maorong Zou
 *
 *************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>

#include "plot.h"

#ifndef ORTHOGONAL
#define ORTHOGONAL 1
#define PERSPECTIVE 2
#endif

extern int    c_token,num_tokens;
extern struct udft_entry *dummy_func;
extern struct dummy_obj  *dummy_obj_list;
extern struct an_object  *look_for_object_name();
extern struct an_object  *look_for_object_by_index();
extern struct a_graph    *look_for_graph_by_index();
extern struct a_material *look_for_mat_by_index();
extern struct an_action  *r_actions;
extern int    center;
extern float  window_size;
extern char   temp_line[];

extern double        real();
extern struct value  *complex();
extern struct oview  pview;

char desfilename[256], datfilename[256];

int titleset2; char titlestring[256];
static int plot3d;
static short graph_table[128] ={0,0,0};  /* what a hack */


char   *malloc();
char   *strcat();
/*******************************************************************/

command1()
{
  int ii;

  if(is_if_cond(c_token))  do_if_cond();
  else if(is_for_loop(c_token)) do_for_loop();
  else if(is_while_loop(c_token)) do_while_loop();
  else if(is_do_loop(c_token)) do_do_loop();
  else if((ii = is_material_list(c_token)))
    {
      c_token += ii;
      while(!equals(c_token,"}") && !END_OF_COMMAND)
	{
	  make_material();
	  if(equals(c_token,";") || equals(c_token,",")) c_token++;
	}
      if(equals(c_token,"}")) c_token++;
    }
  else if((ii = is_graph_list(c_token)))
    {
      c_token += ii;
      while(!equals(c_token,"}") && !END_OF_COMMAND)
	{
	  make_graph();
	  if(equals(c_token,";") || equals(c_token,",")) c_token++;
	}
      if(equals(c_token,"}")) c_token++;
    }
  else if((ii = is_object_list(c_token)))
    {
      c_token += ii;
      while(!equals(c_token,"}") && !END_OF_COMMAND)
	{
	  make_object();
	  if(equals(c_token,";") || equals(c_token,",")) c_token++;
	}
      if(equals(c_token,"}")) c_token++;
    }
  else if((ii = is_function_list(c_token)))
    {
      c_token += ii;
      while(!equals(c_token,"}") && !END_OF_COMMAND)
	{
	  define();
	  if(equals(c_token,";") || equals(c_token,",")) c_token++;
	}
      if(equals(c_token,"}")) c_token++;
    }
  else if( (ii= is_set_view(c_token)))
    {
      c_token += ii;
      set_view();
    }
  else if((ii=is_object(c_token)))
    {
      make_object();
    }
  else if(is_material(c_token))
    {
      make_material();
    }
  else if(is_graph(c_token))
    {
      make_graph();
      make_dummy_obj();
    }
  else if(is_light(c_token))
    {
      make_light();
    }
  else if(is_light_model(c_token))
    {
      make_light_model();
    }
  else if(is_short_hand(c_token))
    {
      define_short_hands();
    }
  else if(is_definition(c_token))
    {
      define();
    }
  else if(almost_equals(c_token,"pl$ot"))
    {
      c_token++;
      plot3d = 1;
      make_plot();
    } 
  else if(equals(c_token,"plot2d"))
    {
      c_token++;
      plot3d = 0;
      make_plot();
    } 
  else if(almost_equals(c_token,"repl$ot"))
    {
      c_token++;
      do_plot(1);
    }  
  else
    command2();
}

/****************************************************************/

void tmpfilename(char *s,char *prefix,char *suffix) {
  char *td[3]={getenv("TEMP"),getenv("TMP"),"C:\\"};
  int i,j,l;
  unsigned int r,pid;
  struct stat st;
  FILE *f;
  pid=getpid()%100000;
  for(j=0;j<3;j++)
    if(td[j]) {
      if(stat(td[j],&st)||!S_ISDIR(st.st_mode))
         continue;
      strcpy(s,td[j]);
      l=strlen(s);
      if(s[l-1]!='/'&&s[l-1]!='\\'&&s[l-1]!=':')
        s[l++]='\\';
      r=pid;
      for(i=0;i<5;i++) {
        sprintf(s+l,"%s%05d.%s",prefix,r,suffix);
        if(stat(s,&st)&&(f=fopen(s,"w"))) {
          fclose(f);
          return;
        }
        r=(r+i)*(r+i+1)%100000;
      }
    }
  s[0]=0;
}

struct plot_history *mat_history;
struct plot_history *graph_history;
FILE   *attrfile, *datafile;
static int num_objs, num_graphs, num_mats, obj_count;


make_plot()
{
  register int a_flag, num_mat;
  register int num_printfs = 0;
  int   b_flag, need_to_draw;
  struct dummy_obj  *dummy_temp;
  struct an_object  *temp_obj;
  struct an_action  *temp_action;
  
  num_objs = 0;
  num_graphs = 0;
  num_mats = 0;
  obj_count = 0;
  b_flag = 0;
  need_to_draw = 0;
  temp_line[0] = '\0';

  (void)strcat(temp_line,"Objects%t|");
  if(desfilename[0] == '\0')
    { /* filename not set yet */
      tmpfilename(desfilename,"xpl","des");
      tmpfilename(datfilename,"xpl","dat");
    }

  if(attrfile) (void)fclose(attrfile);
  if(datafile) (void)fclose(datafile);
  if( !(attrfile = fopen(desfilename,"w")) ||
     ! (datafile = fopen(datfilename,"w")))
    os_error("Cannot open tmp data file",-1);

  /*
   * first set the plot type, i.e 2d or 3d
   */
  (void)  fprintf(attrfile,"%d  ",plot3d);
  /*
   *  This part is here only for compatibility
   */
  if(END_OF_COMMAND)
    {
      dummy_temp = dummy_obj_list;
      
      while(dummy_temp)
	{
	  temp_obj = dummy_temp->obj;
	  if(!(temp_obj->defined))
	    {
	      reset_plots();
	      int_error("Undefinde object",-1);
	    }
	  (void)strcat(temp_line,temp_obj->name);
	  (void)strcat(temp_line,"|");
	  
	  need_to_draw = 1;
	  num_mat = 0;
	  /* (void)fprintf(attrfile,"%d\n", BEGIN_AN_OBJECT);*/
	  a_flag = 1;

	  temp_action = (temp_obj->actions)->next;
	  while(temp_action)
	    {
	      if(temp_action->entry == BMATERIAL)
		{
		  temp_action = temp_action->next;
		  add_to_mat_history( a_flag, (int)(temp_action->entry));
		  num_mat++;
		  if(a_flag) a_flag = 0;
		  num_mats++;
		}
	      if(temp_action->entry == GRAPH)
		{
		  temp_action = temp_action->next;
		  if( add_to_graph_history(&b_flag,(int)(temp_action->entry)))
		    { num_graphs++;  graph_table[num_graphs] = obj_count;}
		}
	      temp_action = temp_action->next;
	    }

	  temp_action = temp_obj->actions;

	  /* (void)fprintf(attrfile,"%f ",(temp_action->entry + 1.0)); 
	  (void)fprintf(attrfile,"%f\n   ", (float)num_mat); */
	  temp_action = temp_action->next;
	  while( temp_action)
	    {
	      if(num_printfs >= 5)
		{
		  /* (void)fprintf(attrfile,"\n   ");*/
		  num_printfs = 0;
		}
	      if(  temp_action->entry == GRAPH)
		{ float xplot1;
		  /* (void)fprintf(attrfile,"%f ", temp_action->entry);*/
		  temp_action = temp_action->next;
		  xplot1 = (look_a_flag( (int) (temp_action->entry)));
		  /* (void)fprintf(attrfile, "%f ", xplot1);*/
		  num_printfs += 2;
		}
	      else
		{
		  /* (void)fprintf(attrfile,"%f ", temp_action->entry); */
		  num_printfs += 1;
		}
	      temp_action = temp_action->next;		  
	    }
	  /* (void)fprintf(attrfile,"\n\"%s\"\n ", temp_obj->menu_string);*/
	  if(num_mat)
	    num_objs++;  
	  obj_count++;
	  dummy_temp = dummy_temp->next;
	}
    }
  else
    {
      while(!END_OF_COMMAND)
	{
	  temp_obj = look_for_object_name(c_token);
	  if(!temp_obj || !(temp_obj->defined))
	    {
	      reset_plots();
	      int_error("undefined object",-1);
	    }
	  (void)strcat(temp_line,temp_obj->name);
	  (void)strcat(temp_line,"|");

	  need_to_draw = 1;
	  c_token++;
	  if(equals(c_token,",")) c_token++;
	  num_mat = 0;
	  a_flag = 1;
	  /* (void)fprintf(attrfile,"%d\n",BEGIN_AN_OBJECT);*/

	  free_actions();
	  expand( ((temp_obj->actions)->next));
	  temp_action = r_actions;

	  while(temp_action)
	    {
	      if(temp_action->entry == BMATERIAL)
		{
		  temp_action = temp_action->next;
		  add_to_mat_history( a_flag, (int)(temp_action->entry));
		  num_mat++;
		  if(a_flag) a_flag = 0;
		  num_mats++;
		}
	      if(temp_action->entry == GRAPH)
		{
		  temp_action = temp_action->next;
		  if( add_to_graph_history(&b_flag,(int)(temp_action->entry)))
		    { num_graphs++;  graph_table[num_graphs] = obj_count;}
		}
	      temp_action = temp_action->next;
	    }
	  temp_action = r_actions;
 /*
	  (void)fprintf(attrfile,"%f ",
			(temp_action->entry+( num_mat? 1.0:3.0)));
	  (void)fprintf(attrfile,"%f ", (float)num_mat);
	  if(!num_mat)
	    (void)fprintf(attrfile,"%f %f\n   ",(float)BMATERIAL,0.0);
*/
	  temp_action = temp_action->next;		  
	  while( temp_action)
	    {
	      if(num_printfs >= 5)
		{
/*		  (void)fprintf(attrfile,"\n   "); */
		  num_printfs = 0;
		}
	      if(temp_action->entry == GRAPH)
		{
/*		  (void)fprintf(attrfile,"%f ", temp_action->entry);*/
		  temp_action = temp_action->next;
/*		  (void)fprintf(attrfile, "%f ", (float) 
			  (look_a_flag( (int) (temp_action->entry))));*/
		  num_printfs += 2;
		}
	      else
		{
/*		  (void)fprintf(attrfile,"%f ", temp_action->entry); */
		  num_printfs += 1;
		}
	      temp_action = temp_action->next;		  
	    }
/*	  if(!num_mat)
	    (void)fprintf(attrfile,"\n\"%s\"\n\n", "Material%t|default");
	  else
	    (void)fprintf(attrfile,"\n\"%s\"\n\n", temp_obj->menu_string);
*/
	  if(num_mat) num_objs++;
	  obj_count++;
	}
      free_actions();
    }

  (void) fprintf(STDERRR,"\t   Working ...");
  {
    do_calculations();
    write_2_file_graphs();
    (void)fprintf(attrfile, "%d ", titleset2);
    if(titleset2) fprintf(attrfile, "%s\n",titlestring);
    (void)fflush(attrfile);   (void)fflush(datafile);
    (void)fclose(attrfile);   (void)fclose(datafile);
    do_plot(need_to_draw); 
    reset_plots();
  }
  (void) fprintf(STDERRR,"\t   Done!\n");
}
/************************************************************/

look_a_flag(ii)
     int ii;
{
  register struct plot_history **temp = &graph_history;
  while( *temp)
    {
      if(  (*temp)-> index == ii)
	return( (*temp)->flag);
      temp = &( (*temp)->next);
    }
  return(-1);
}
/************************************************************/
extern   double xmin,xmax,ymin,ymax,zmin,zmax;

write_2_file_view_size()
{
  (void)fprintf(attrfile,"\n%d %d %d %d %d %d\n",SET_VIEW,
		pview.window_position[0], pview.window_position[1],
		pview.window_position[2], pview.window_position[3],
		pview.proj_type);
  if(pview.proj_type == ORTHOGONAL || pview.proj_type == PERSPECTIVE)
    (void)fprintf(attrfile,"\n%f %f %f %f %f %f %d\n",
		  pview.vx, pview.vy, pview.vz, pview.px, pview.py,
		  pview.pz, pview.twist);
  if(pview.proj_type == ORTHOGONAL)
    (void)fprintf(attrfile,"\n%f %f %f %f %f %f\n",
		  pview.projections.o.left,pview.projections.o.right,
		  pview.projections.o.bottom,pview.projections.o.top,
		  pview.projections.o.near,pview.projections.o.far);
  else   if(pview.proj_type == PERSPECTIVE)
    (void)fprintf(attrfile,"\n%d %f %f %f\n",
		  pview.projections.p.fovy,pview.projections.p.aspect,
		  pview.projections.p.pnear,pview.projections.p.pfar);
  if(window_size)
    (void)fprintf(attrfile,"\n%f %f %f %f %f %f\n",
		  -window_size,window_size, -window_size,window_size,
		  -window_size,window_size);
  else
    (void)fprintf(attrfile,"\n%f %f %f %f %f %f\n",
		  (float)xmin,(float)xmax,(float)ymin,(float)ymax,
		  (float)zmin,(float)zmax);
  (void)fprintf(attrfile,"\n%d\n \"%s\"\n",OBJMENU, temp_line);
  (void)fprintf(attrfile,"\n%d\n %f\n",CENTER, (float)center);
}
/************************************************************/
write_2_file_graphs()
{
  int i,j, k, cgr = 0;
  struct plot_history **temp = &graph_history;
  struct   a_graph *temp_graph;

  struct   uaxes *axes = (struct uaxes *)NULL;

/*  
  (void)fprintf(attrfile,"%d %d\n",BEGIN_GRAPHS,num_graphs);
*/
  (void)fprintf(attrfile,"%d\n",num_graphs);
  
  while( *temp)
    {
      temp_graph = look_for_graph_by_index((*temp)->index);
      if(!temp_graph|| !(temp_graph->defined)) int_error("undefined graph",-1);
      
      k = temp_graph->plot_type;  
      if( k == TUBE) k = SURFACE;
      else if( k == EQN || k == MAP) k = CURVE;
      
      (void)fprintf(attrfile,"%d %d %d ",
		    k, temp_graph->data_type,graph_table[++cgr]);

      if(temp_graph->plot_type == SURFACE)
	{
	  if((temp_graph->picture).surface.which_first == 1)
	    (void)fprintf(attrfile," %d  %d  %d  %d ", graph_table[cgr],
			  ((temp_graph->picture).surface.sample),
			  ((temp_graph->picture).surface.sample1),0);
	  else
	    (void)fprintf(attrfile," %d  %d  %d %d ",graph_table[cgr],
			  ((temp_graph->picture).surface.sample1),
			  ((temp_graph->picture).surface.sample),0);

/*
	  (void)fprintf(attrfile,"  %f %f\n",
			(float) ((temp_graph->picture).surface.is_wire),
			(float) ((temp_graph->picture).surface.oren));

	  axes = ((temp_graph->picture).surface.uaxes);
*/
	}
      else if(temp_graph->plot_type == CURVE)
	{
	  (void)fprintf(attrfile,"  %d ",((temp_graph->picture).curve.color));
	  (void)fprintf(attrfile," %d  %d ",
			((temp_graph->picture).curve.sample),
			((temp_graph->picture).curve.sample1));
	  (void)fprintf(attrfile,"  %d ",((temp_graph->picture).curve.style));
/*
	  (void)fprintf(attrfile,"  %f  %f ",
			(float) ((temp_graph->picture).curve.color),
			(float) ((temp_graph->picture).curve.style));
	  axes = ((temp_graph->picture).curve.uaxes);
*/
	}
      else if(temp_graph->plot_type == TUBE)
	{
	  (void)fprintf(attrfile," %d  %d  %d  %d ",graph_table[cgr],
			((temp_graph->picture).tube.sample),
			((temp_graph->picture).tube.sample1),0);	  
	}
      else if(temp_graph->plot_type == MAP)
	{ 
	  (void)fprintf(attrfile," %d  %d %d %d\n",
			(temp_graph->picture).map.color,
			(temp_graph->picture).map.sample, 1, 
			(temp_graph->picture).map.style);
	  /* axes = ((temp_graph->picture).map.uaxes); */
	}
      else if(temp_graph->plot_type == EQN)
	{
	  (void)fprintf(attrfile," %d  %d  %d  %d\n",
			((temp_graph->picture).eqn.color),
			((temp_graph->picture).eqn.sample),1,0);
	  
	  /* axes = ((temp_graph->picture).eqn.uaxes); */
	}
      else if(temp_graph->plot_type == TMESH)
	{
	  (void)fprintf(attrfile,"%f %f %f\n",
			(float) ((temp_graph->picture).tmesh.sample),
			(float) ((temp_graph->picture).tmesh.sample1),
			(float) ((temp_graph->picture).tmesh.oren));
	  axes = ((temp_graph->picture).tmesh.uaxes);
	}
      else
	(void)fprintf(STDERRR,"\t unknown graph type in %s\n",
		      temp_graph->name);
  /*    if( axes != (struct uaxes*)NULL)
	{
	  (void)fprintf(attrfile," %f \n", 1.0);
	  (void)fprintf(attrfile,"    %f %f %f\n",
			axes->ox,axes->oy,axes->oz);
	  (void)fprintf(attrfile,"    %f %f %f\n",
			axes->xx, axes->xy,axes->xz);
	  (void)fprintf(attrfile,"    %f %f %f\n",
			axes->yx,axes->yy,axes->yz);
	  (void)fprintf(attrfile,"    %f %f %f\n",
			axes->zx,axes->zy,axes->zz);
	  (void)fprintf(attrfile,"    %d\n",axes->ticmarks);
	}
      else (void)fprintf(attrfile," %f \n", 0.0);
*/	
      if(temp_graph->is_data)
	(void)fprintf(attrfile," %s", temp_graph->data_file);
      (void)fprintf(attrfile, "\n");
      temp = &( (*temp)->next);
    }
}
/************************************************************/
write_2_file_material()
{
  register int ii,jj,kk;
  register struct plot_history **temp = &mat_history;
  struct  a_material *temp_mat;

  jj = find_out_lights_and_lmodel(0);
  kk = find_out_lights_and_lmodel(1);
  ii = num_objs + num_mats + num_mats * 20 + 4 + jj + kk;
  (void)fprintf(attrfile,"\n%d  %d\n",BEGIN_MATERIAL_DEFINITIONS,ii);
  (void)fprintf(attrfile, "%f\n",(float)BGNDEFINITION);
  if(jj)
    {
      register int i;
      (void)fprintf(attrfile, "% f\n",(float)BGNLIGHT);      
      for(i = 0; i < 8; i++)
	get_light_properties(i);
      (void)fprintf(attrfile, "% f\n",(float)ENDLIGHT);
    }
  if(kk)
    {
      (void)fprintf(attrfile, "% f\n",(float)BGNLMODEL);  
      get_lmodel_properties();
      (void)fprintf(attrfile, "% f\n",(float)ENDLMODEL);  
    }
  (void)fprintf(attrfile, "% f\n",(float)BGNMATERIAL);
  
  while(*temp)
    {
      if((*temp)->flag)
	(void)fprintf(attrfile," %f ",(float) NEWOBJECT);
      temp_mat = look_for_mat_by_index( (*temp)->index);
      if( !temp_mat || !(temp_mat->defined))
	int_error("undefined material",-1);
      (void)fprintf(attrfile," %f\n ",(float) NEWMATERIAL);
      (void)fprintf(attrfile,"\t%f ",(float) EMISSION);
      (void)fprintf(attrfile," %f %f %f\n",(temp_mat->token[0]),
	      *(temp_mat->token +1),*(temp_mat->token+2));
      (void)fprintf(attrfile,"\t%f ", (float)AMBIENT);
      (void)fprintf(attrfile,"%f %f %f\n",*(temp_mat->token+3),
	      *(temp_mat->token+4),*(temp_mat->token+5));
      (void)fprintf(attrfile,"\t%f ", (float)DIFUSE);
      (void)fprintf(attrfile,"%f %f %f\n",(temp_mat->token[6]),
	      (temp_mat->token[7]),(temp_mat->token[8]));
      (void)fprintf(attrfile,"\t%f ", (float)SPECULAR); 
      (void)fprintf(attrfile,"%f %f %f\n",(temp_mat->token[9]),
	      (temp_mat->token[10]),(temp_mat->token[11]));
      (void)fprintf(attrfile,"\t%f ",(float) SHINNESS);
      (void)fprintf(attrfile,"%f\n ",(temp_mat->token[12]));
      (void)fprintf(attrfile,"\t%f ", (float)ALPHA);
      (void)fprintf(attrfile,"%f\n ",(temp_mat->token[13]));
      temp = &( (*temp)->next);
    }
  (void)fprintf(attrfile,
		" %f\n%f\n\n",(float)ENDMATERIAL,(float)ENDDEFINITION);
}
/************************************************************/
add_to_graph_history(kk,ii)
     int *kk,ii;
{
  register struct plot_history **temp = &graph_history;
  
  while( *temp)
    {
      if( (*temp)->index == ii)
	return (0) ;
      else
	temp = &( (*temp)->next);
    }
  (*temp) = (struct plot_history *)malloc((unsigned int)
					  sizeof(struct plot_history));
  (*temp)->next = (struct  plot_history *)NULL;
  (*temp)->index = ii;
  (*temp)->flag = *kk;
  (*kk) += 1;
  return( 1);
}

/************************************************************/
add_to_mat_history(ii,jj)
     int ii,jj;
{
  register struct plot_history **temp = &mat_history;
  
  while( *temp)
    temp = &( (*temp)->next);
  (*temp) = (struct plot_history *)malloc( (unsigned int)
					  sizeof(struct plot_history));
  (*temp)->next = (struct plot_history *)NULL;
  (*temp)->flag = ii;
  (*temp)->index = jj;
}
/************************************************************/

reset_plots()      
{
  register struct plot_history *temp_history;
  register struct dummy_obj    *temp_dummy;

  while(mat_history)
    {
      temp_history = mat_history->next;
      (void)free( (char *)mat_history);
      mat_history = temp_history;
    }
  while(graph_history)
    {
      temp_history = graph_history->next;
      (void)free( (char *)graph_history);
      graph_history = temp_history;		 
    }
  while(dummy_obj_list)
    {
      temp_dummy = dummy_obj_list->next;
      (void)free( (char *) dummy_obj_list);
      dummy_obj_list = temp_dummy;
    }
  free_actions();
  dummy_obj_list = (struct dummy_obj *) NULL;
  graph_history  = (struct plot_history *)NULL;
  mat_history  = (struct plot_history *)NULL;  
  center = 0;
  plot3d = 1;
  xmin = ymin = zmin = 1000000.0;
  xmax = ymax = zmax = -1000000.0;
  window_size = 0.0;
  reset_window_position();
  reset_light_and_lmodel();
  reset_final_structure();
  if(attrfile)
    (void)fclose(attrfile);
  if(datafile)
    (void)fclose(datafile);
}
/**********************************************************/

do_calculations()
{
  struct a_graph    *temp_graph;
  struct plot_history *temp_history;
  
  temp_history = graph_history;
  while(temp_history)
    {
      temp_graph = look_for_graph_by_index(temp_history->index);
      calculate(temp_graph);
      
/*      set_axes(temp_graph); */

      temp_history = temp_history->next;
    }
}
/**************************************************************/
static struct value ax,ay,az,aw;

calculate(temp_plot)
     struct a_graph *temp_plot;
{
  register int    i, j;
  register double xdiff,ydiff,x,y,xx,yy,zz;

  if(temp_plot->is_data)
    {
/*
      if(temp_plot->data_check)
	if(check_data(temp_plot)) 
	  {
	    reset_plots();
	    int_error("data file too small",-1);
	  }
      get_data(temp_plot);
*/

      if(temp_plot->plot_type == TUBE || temp_plot->plot_type == CURVE)
	count_number_of_points(temp_plot);

      if(temp_plot->plot_type == TUBE)
	construct_a_tube_from_file(temp_plot);
    }
  else
    {
      if(temp_plot->plot_type == SURFACE)
	{
	  register int range_set = 0;

	  if(!((temp_plot->picture).surface.fr1))  range_set = 1;
	  /*
	   * which_first matters because the range of the other
	   * variable is a function of the first one. This is the added 
	   * stuff, so the code is somehow redudant. Feb. 15 1990
	   */
	  if( (temp_plot->picture).surface.which_first == 1)
	    {
	      xdiff = ((temp_plot->picture).surface.xmax -
		       (temp_plot->picture).surface.xmin) /
			 ( (temp_plot->picture).surface.sample - 1.0);
	      if(range_set)
		ydiff = ((temp_plot->picture).surface.ymax -
			 (temp_plot->picture).surface.ymin) /
			   ( (temp_plot->picture).surface.sample1 - 1.0);

	      dummy_func = (temp_plot->picture).surface.fx;

	      for (i = 0; i < (temp_plot->picture).surface.sample;i++)
		{
		  x = (temp_plot->picture).surface.xmin + i* xdiff;
		  (void) complex(&(dummy_func->dummy_value), x, 0.0);
		  if(!range_set)
		    {
		      evaluate_at(((temp_plot->picture).surface.fr1)->at,&ax);
		      evaluate_at(((temp_plot->picture).surface.fr2)->at,&ay);
		      (temp_plot->picture).surface.ymin = real(&ax);
		      (temp_plot->picture).surface.ymax = real(&ay);
		      ydiff = ((temp_plot->picture).surface.ymax -
			       (temp_plot->picture).surface.ymin) /
				 ( (temp_plot->picture).surface.sample1 - 1.0);
		    }
		  for(j = 0; j < (temp_plot->picture).surface.sample1;j++) 
		    {
		      y = (temp_plot->picture).surface.ymin + j*ydiff;
		      (void) complex(&(dummy_func->dummy_value1), y, 0.0);
		      evaluate_at((((temp_plot->picture).surface.fx)->at),&ax);
		      if(! ((temp_plot->picture).surface.is_function))
			{
			  evaluate_at(((temp_plot->picture).surface.fy)->at,
				      &ay);
			  evaluate_at(((temp_plot->picture).surface.fz)->at,
				      &az);
			}
		      if(!( (temp_plot->picture).surface.is_function) )
			{
			  xx = real(&ax); yy = real(&ay);  zz = real(&az);
			}
		      else
			{
			  xx = x; yy = y; zz = real(&ax);
			}
		      (void)fprintf(datafile, "%f %f %f\n",(float)xx,
				    (float)yy,(float)zz);
		      check_range(xx,yy,zz);
		    }
		}
	    }  /*  which_first == 1 */ 
	  else
	    {
	      if(range_set)
		xdiff = ((temp_plot->picture).surface.xmax -
			 (temp_plot->picture).surface.xmin) /
			   ((temp_plot->picture).surface.sample - 1.0);

	      ydiff = ((temp_plot->picture).surface.ymax -
		       (temp_plot->picture).surface.ymin) /
			 ( (temp_plot->picture).surface.sample1 - 1.0);
	      
	      dummy_func = (temp_plot->picture).surface.fx;
	      
	      for (i = 0; i < (temp_plot->picture).surface.sample1;i++)
		{
		  y = (temp_plot->picture).surface.ymin + i* ydiff;
		  (void) complex(&(dummy_func->dummy_value1), y, 0.0);
		  if(!range_set)
		    {
		      evaluate_at(((temp_plot->picture).surface.fr1)->at,&ax);
		      evaluate_at(((temp_plot->picture).surface.fr2)->at,&ay);
		      (temp_plot->picture).surface.xmin = real(&ax);
		      (temp_plot->picture).surface.xmax = real(&ay);
		      xdiff = ((temp_plot->picture).surface.xmax -
			       (temp_plot->picture).surface.xmin) /
				 ( (temp_plot->picture).surface.sample - 1.0);
		    }
		  for(j = 0; j < (temp_plot->picture).surface.sample;j++) 
		    {
		      x = (temp_plot->picture).surface.xmin + j*xdiff;
		      (void) complex(&(dummy_func->dummy_value), x, 0.0);
		      evaluate_at((((temp_plot->picture).surface.fx)->at),&ax);
		      if(! ((temp_plot->picture).surface.is_function))
			{
			  evaluate_at(((temp_plot->picture).surface.fy)->at,
				      &ay);
			  evaluate_at(((temp_plot->picture).surface.fz)->at,
				      &az);
			}
		      if(!( (temp_plot->picture).surface.is_function) )
			{
			  xx = real(&ax); yy = real(&ay);  zz = real(&az);
			}
		      else
			{
			  xx = x; yy = y; zz = real(&ax);
			}
		      (void)fprintf(datafile, "%f %f %f\n",(float)xx,
				    (float)yy,(float)zz);
		      check_range(xx,yy,zz);
		    }
		}
	    }  /* which_first == 2 */
	}
      else if( (temp_plot->plot_type == CURVE))
	{
	  dummy_func = (temp_plot->picture).curve.fx;

	  xdiff = ((temp_plot->picture).curve.xmax -
		   (temp_plot->picture).curve.xmin) /
		     ( ((temp_plot->picture).curve.sample) - 1.0);

	  for (i = 0; i < (temp_plot->picture).curve.sample; i++)
	    {
	      x = (temp_plot->picture).curve.xmin + i*xdiff;
	      (void) complex(&(dummy_func->dummy_value), x, 0.0);
	      evaluate_at( ((temp_plot->picture).curve.fx)->at,&ax);
	      evaluate_at( ((temp_plot->picture).curve.fy)->at,&ay);
	      evaluate_at( ((temp_plot->picture).curve.fz)->at,&az);
	      xx = real(&ax); yy= real(&ay); zz = real(&az);
	      (void)fprintf(datafile, "%f %f %f\n",(float)xx,
			    (float)yy,(float)zz);
	      check_range(xx,yy,zz);
	    }
	}
      else if( (temp_plot->plot_type == TUBE))
	{
	  float *temp_float, *ftptr; int radf = 0;

	  if((temp_plot->picture).tube.radf) radf = 1;
	  temp_float=(float *)malloc((radf+3)*(temp_plot->picture).tube.sample
				     * sizeof(float));
	  if( temp_float == (float *)NULL) int_error("out of memory",-1) ;
	  ftptr = temp_float;

	  dummy_func = (temp_plot->picture).tube.fx;

	  xdiff = ((temp_plot->picture).tube.xmax -
		   (temp_plot->picture).tube.xmin) /
		 ( ((temp_plot->picture).tube.sample) - 1.0);

	  for (i = 0; i < (temp_plot->picture).tube.sample; i++)
	    {
	      x = (temp_plot->picture).tube.xmin + i*xdiff;
	      (void) complex(&(dummy_func->dummy_value), x, 0.0);
	      evaluate_at( ((temp_plot->picture).tube.fx)->at,&ax);
	      evaluate_at( ((temp_plot->picture).tube.fy)->at,&ay);
	      evaluate_at( ((temp_plot->picture).tube.fz)->at,&az);
	      if(radf)evaluate_at( ((temp_plot->picture).tube.radf)->at,&aw);
	      *temp_float++ = (float) real(&ax);
	      *temp_float++ = (float) real(&ay);
	      *temp_float++ = (float) real(&az);
	      if(radf) *temp_float++ = (float) real(&aw);
	      /*
	      (void)fprintf(datafile, "%f %f %f\n",(float)xx,
			    (float)yy,(float)zz);
	      check_range(xx,yy,zz);
	      */
	    }
	  construct_tube_from_data(temp_plot,ftptr, radf);
	  (void)free( (char *)ftptr);
	}
      else if((temp_plot->plot_type == MAP))
	{
	  dummy_func = (temp_plot->picture).map.fx;
	  make_map_traj(temp_plot);
	}
      else if((temp_plot->plot_type == EQN))
	{
	  dummy_func = (temp_plot->picture).eqn.fx;
	  make_eqn_traj(temp_plot);
	}
    }
}
/**********************************************************************/

make_map_traj(tt_plot)
     struct a_graph *tt_plot;
{
  register int    i;
  register double x,y,z,w;

  if((tt_plot->picture).map.dimension == 1)
    {
      x = (tt_plot->picture).map.x0;
      for(i=0; i < (tt_plot->picture).map.sample;i++)
	{
	  (void) complex(&(dummy_func->dummy_value), x, 0.0);
	  evaluate_at( (tt_plot->picture).map.fx->at,&az);
	  y = real(&az);
	  check_range(y,y,0.0);
	  (void)fprintf(datafile,"%f %f %f\n",(float)x,(float)y,0.0);
	  x = y;
	}
    }
  if((tt_plot->picture).map.dimension == 2)
    {
      x = (tt_plot->picture).map.x0;
      y = (tt_plot->picture).map.y0;
      for(i=0; i < (tt_plot->picture).map.sample;i++)
	{
	  (void) complex(&(dummy_func->dummy_value), x, 0.0);
	  (void) complex(&(dummy_func->dummy_value1), y, 0.0);
	  evaluate_at( (tt_plot->picture).map.fx->at,&ax);
	  evaluate_at( (tt_plot->picture).map.fy->at,&ay);
	  check_range(x,y,0.0);
	  (void)fprintf(datafile,"%f %f %f\n",(float)x,(float)y,0.0);
	  x = real(&ax);
	  y = real(&ay);
	}
    }
  if((tt_plot->picture).map.dimension == 3)
    {
      x = (tt_plot->picture).map.x0;
      y = (tt_plot->picture).map.y0;
      z = (tt_plot->picture).map.z0;      
      for(i=0; i < (tt_plot->picture).map.sample;i++)
	{
	  (void) complex(&(dummy_func->dummy_value), x, 0.0);
	  (void) complex(&(dummy_func->dummy_value1), y, 0.0);
	  (void) complex(&(dummy_func->dummy_value2), z, 0.0);
	  evaluate_at( (tt_plot->picture).map.fx->at,&ax);
	  evaluate_at( (tt_plot->picture).map.fy->at,&ay);
	  evaluate_at( (tt_plot->picture).map.fz->at,&az);
	  check_range(x,y,z);
	  (void)fprintf(datafile,"%f %f %f\n",(float)x,(float)y,(float)z);
	  x = real(&ax);
	  y = real(&ay);
	  z = real(&az);
	}
    }
  if((tt_plot->picture).map.dimension == 4)
    {
      x = (tt_plot->picture).map.x0;
      y = (tt_plot->picture).map.y0;
      z = (tt_plot->picture).map.z0;      
      w = (tt_plot->picture).map.w0;      
      for(i=0; i < (tt_plot->picture).map.sample;i++)
	{
	  (void) complex(&(dummy_func->dummy_value), x, 0.0);
	  (void) complex(&(dummy_func->dummy_value1), y, 0.0);
	  (void) complex(&(dummy_func->dummy_value2), z, 0.0);
	  (void) complex(&(dummy_func->dummy_value3), w, 0.0);
	  evaluate_at( (tt_plot->picture).map.fx->at,&ax);
	  evaluate_at( (tt_plot->picture).map.fy->at,&ay);
	  evaluate_at( (tt_plot->picture).map.fz->at,&az);
	  evaluate_at( (tt_plot->picture).map.fw->at,&aw);
	  check_range(x,y,z);
	  (void)fprintf(datafile,"%f %f %f\n",(float)x,(float)y,(float)z);
	  x = real(&ax);
	  y = real(&ay);
	  z = real(&az);
	  w = real(&aw);
	}
    }
}
/************************************************************************/

make_eqn_traj(t_plot)
     struct a_graph *t_plot;
{
  double time,zy1,y2,y3,y4,yy1,yy2,yy3,yy4;
  double dy1,dy2,dy3,dy4;
  double htry,hdid,hnext;
  int    count,flag,skip_count,fix_count;

  count = 0; 
  skip_count = 0;  
  zy1  = (t_plot->picture).eqn.x0;
  y2  = (t_plot->picture).eqn.y0;
  y3  = (t_plot->picture).eqn.z0;
  y4  = (t_plot->picture).eqn.w0;

  flag = 0;
  htry  = (t_plot->picture).eqn.step;
  fix_count = (t_plot->picture).eqn.skip;

  time =  (t_plot->picture).eqn.start_time;
  while( (time <=  (t_plot->picture).eqn.end_time) && !flag)
    {
      if(! (skip_count % fix_count))
	{
	  skip_count = 0;
	  count++;
	  if((t_plot->picture).eqn.dimension == 1)
	    {
	      check_range(zy1,time,0.0);
	      (void)fprintf(datafile,"%f %f %f\n",(float)zy1,(float)time,0.0);
	    }

	  if((t_plot->picture).eqn.dimension == 2)
	    {
	      check_range(zy1,y2,0.0);
	      (void)fprintf(datafile,"%f %f %f\n",(float)zy1,(float)y2,0.0);
	    }

	  if((t_plot->picture).eqn.dimension == 3)
	    {
	      check_range(zy1,y2,y3);
	      (void)fprintf(datafile,"%f %f %f\n",
			    (float)zy1,(float)y2,(float)y3);
	    }
	  if((t_plot->picture).eqn.dimension == 4)
	    {
	      check_range(zy1,y2,y3);
	      (void)fprintf(datafile,"%f %f %f\n",
			    (float)zy1,(float)y2,(float)y3);
	    }
	}
      compute_deri(t_plot,&zy1,&y2,&y3,&y4,&dy1,&dy2,&dy3,&dy4);
      if( (t_plot->picture).eqn.method == RK)
	RK4_solver(zy1,y2,y3,y4,
		   dy1,dy2,dy3,dy4,&yy1,&yy2,&yy3,&yy4,t_plot,htry);
      else if( (t_plot->picture).eqn.method == RKQC)
	RKQC_solver(zy1,y2,y3,y4,dy1,dy2,dy3,dy4,
		    &yy1,&yy2,&yy3,&yy4,t_plot,&htry,&hdid,&hnext,&flag);
      if(flag) 
	(void)fprintf(STDERRR,"\tRKQC abort at time %f,step = 0.0\n",
		      (float)time);
      zy1 = yy1;
      if((t_plot->picture).eqn.dimension >= 2) y2 = yy2; 
      if((t_plot->picture).eqn.dimension >= 3) y3 = yy3;
      if((t_plot->picture).eqn.dimension >= 4) y4 = yy4;
      if((t_plot->picture).eqn.method == RKQC )  
	{
	  htry = hnext;;
	  time += hdid;
	}
      else
	time += (t_plot->picture).eqn.step;
      skip_count++;
    }
  (t_plot->picture).eqn.sample = count;
}
/*********************************************************************/

#define one_over_6 0.166666666666666667

RK4_solver(zy1,y2,y3,y4,dy1,dy2,dy3,dy4,yy1,yy2,yy3,yy4,t_plot,h)
     double zy1,y2,y3,y4,dy1,dy2,dy3,dy4,*yy1,*yy2,*yy3,*yy4,h;
     struct a_graph *t_plot;
{
  double dyt1,dyt2,dyt3,dyt4,dym1,dym2,dym3,dym4;
  double yt1,yt2,yt3,yt4;
  double hh;
  
  hh =  0.50 * h;
  yt1 = zy1 + dy1 * hh;
  if((t_plot->picture).eqn.dimension >= 2)   yt2 = y2 + dy2 * hh;
  if((t_plot->picture).eqn.dimension >= 3)   yt3 = y3 + dy3 * hh;
  if((t_plot->picture).eqn.dimension >= 4)   yt4 = y4 + dy4 * hh;
  compute_deri(t_plot,&yt1,&yt2,&yt3,&yt4,&dyt1,&dyt2,&dyt3,&dyt4);  
  yt1 = zy1 + dyt1 * hh;
  if((t_plot->picture).eqn.dimension >= 2)   yt2 = y2 + dyt2 * hh;
  if((t_plot->picture).eqn.dimension >= 3)   yt3 = y3 + dyt3 * hh;
  if((t_plot->picture).eqn.dimension >= 4)   yt4 = y4 + dyt4 * hh;
  compute_deri(t_plot,&yt1,&yt2,&yt3,&yt4,&dym1,&dym2,&dym3,&dym4);  
  yt1 = zy1 + dym1 * h;
  if((t_plot->picture).eqn.dimension >= 2)   yt2 = y2 + dym2 * h;
  if((t_plot->picture).eqn.dimension >= 3)   yt3 = y3 + dym3 * h;
  if((t_plot->picture).eqn.dimension >= 4)   yt4 = y4 + dym4 * h;
  dym1 += dyt1;
  if((t_plot->picture).eqn.dimension >= 2)     dym2 += dyt2;
  if((t_plot->picture).eqn.dimension >= 3)     dym3 += dyt3;
  if((t_plot->picture).eqn.dimension >= 4)     dym4 += dyt4;
  compute_deri(t_plot,&yt1,&yt2,&yt3,&yt4,&dyt1,&dyt2,&dyt3,&dyt4);  
  *yy1 = zy1 + h * one_over_6 * ( dy1 + dyt1 + 2.0 * dym1 );
  if((t_plot->picture).eqn.dimension >= 2)   
    *yy2 = y2 + h * one_over_6 * ( dy2 + dyt2 + 2.0 * dym2 );
  if((t_plot->picture).eqn.dimension >= 3)   
    *yy3 = y3 + h * one_over_6 * ( dy3 + dyt3 + 2.0 * dym3 );
  if((t_plot->picture).eqn.dimension >= 4)   
    *yy4 = y4 + h * one_over_6 * ( dy4 + dyt4 + 2.0 * dym4 );
}
/*********************************************************************/

compute_deri(t_plot,zy1,y2,y3,y4,dy1,dy2,dy3,dy4)  
     struct a_graph *t_plot;
     double *zy1,*y2,*y3,*y4,*dy1,*dy2,*dy3,*dy4;  
{
  (void) complex(&(dummy_func->dummy_value), *zy1, 0.0);
  (void) complex(&(dummy_func->dummy_value1), *y2, 0.0);
  (void) complex(&(dummy_func->dummy_value2), *y3, 0.0);
  (void) complex(&(dummy_func->dummy_value3), *y4, 0.0);

  if((t_plot->picture).eqn.dimension == 1)
    evaluate_at( ((t_plot->picture).eqn.fx)->at,&ax);
  else if((t_plot->picture).eqn.dimension == 2)
    {
      evaluate_at( ((t_plot->picture).eqn.fx)->at,&ax);
      evaluate_at( ((t_plot->picture).eqn.fy)->at,&ay);
    }
  else if((t_plot->picture).eqn.dimension == 3)
    {
      evaluate_at( ((t_plot->picture).eqn.fx)->at,&ax);
      evaluate_at( ((t_plot->picture).eqn.fy)->at,&ay);
      evaluate_at( ((t_plot->picture).eqn.fz)->at,&az);
    }
  else if((t_plot->picture).eqn.dimension == 4)
    {
      evaluate_at( ((t_plot->picture).eqn.fx)->at,&ax);
      evaluate_at( ((t_plot->picture).eqn.fy)->at,&ay);
      evaluate_at( ((t_plot->picture).eqn.fz)->at,&az);
      evaluate_at( ((t_plot->picture).eqn.fw)->at,&aw);
    }
  *dy1 = real(&ax);
  if((t_plot->picture).eqn.dimension >= 2)    *dy2 = real(&ay);
  if((t_plot->picture).eqn.dimension >= 3)    *dy3 = real(&az);
  if((t_plot->picture).eqn.dimension >= 4)    *dy4 = real(&aw);
}

/*********************************************************************/

#ifndef MAX
#define MAX(x,y) ((x)> (y)? (x):(y))
#endif
#define SAFETY  0.9
#define PGROW -0.2
#define PSHRNK -0.25
#define FCOR 0.0666666666666667
#define ERRCON 0.0006


RKQC_solver(zy1,y2,y3,y4,dy1,dy2,dy3,dy4,
	    yy1,yy2,yy3,yy4,t_plot,htry,hdid,hnext,flag)
     double zy1,y2,y3,y4,*yy1,*yy2,*yy3,*yy4,*hdid,*hnext,*htry;
     double dy1,dy2,dy3,dy4;
     int    *flag;
     struct a_graph *t_plot;
{
  double ysa1,ysa2,ysa3,ysa4,dysa1,dysa2,dysa3,dysa4;
  double xy1,xy2,xy3,xy4;
  double yt1,yt2,yt3,yt4,dyy1,dyy2,dyy3,dyy4;
  double h,hh,errmax;

  dysa1 = dy1; ysa1 = zy1;
  if((t_plot->picture).eqn.dimension >= 2)  {  dysa2 = dy2; ysa2 = y2;}
  if((t_plot->picture).eqn.dimension >= 3)  {  dysa3 = dy3; ysa3 = y3;}
  if((t_plot->picture).eqn.dimension >= 4)  {  dysa4 = dy4; ysa4 = y4;}

  h = *htry;
 tryagain:
  hh = 0.5 * h;
  
  RK4_solver(ysa1,ysa2,ysa3,ysa4,dysa1,dysa2,dysa3,dysa4,
	     &yt1,&yt2,&yt3,&yt4,t_plot,hh);
  compute_deri(t_plot,&yt1,&yt2,&yt3,&yt4,&dyy1,&dyy2,&dyy3,&dyy4); 
  RK4_solver(yt1,yt2,yt3,yt4,dyy1,dyy2,dyy3,dyy4,
	     &xy1,&xy2,&xy3,&xy4,t_plot,hh);  
  zy1 = xy1; y2 = xy2; y3 = xy3; y4 = xy4;

  if( fabs(hh) < 0.000000001) 
    {
      *flag = 1;
      return;
    }
  RK4_solver(ysa1,ysa2,ysa3,ysa4,dysa1,dysa2,dysa3,dysa4,
	     &yt1,&yt2,&yt3,&yt4,t_plot,h);  

  errmax = 0.0;
  yt1 = zy1 - yt1;
  errmax = MAX( errmax, fabs( yt1/( (t_plot->picture).eqn.x1)));
  if((t_plot->picture).eqn.dimension>=2)
    {
      yt2 = y2 - yt2;
      errmax = MAX( errmax, fabs( yt2/( (t_plot->picture).eqn.y1)));
    }
  if((t_plot->picture).eqn.dimension>=3)
    {
      yt3 = y3 - yt3;
      errmax = MAX( errmax, fabs( yt3/( (t_plot->picture).eqn.z1)));
    }
  if((t_plot->picture).eqn.dimension>=4)
    {
      yt4 = y4 - yt4;
      errmax = MAX( errmax, fabs( yt4/( (t_plot->picture).eqn.w1)));
    }

  errmax /=  (t_plot->picture).eqn.small;
  if( errmax >= 1.0) 
    {
      h = SAFETY * h * (pow(errmax,PSHRNK));
      goto tryagain;
    }
  else
    {
      *hdid = h;
      if(errmax > ERRCON)
	*hnext = SAFETY * h * (pow(errmax, PGROW));
      else
	*hnext = 4.0 * h;
    }
  
  *yy1 = zy1 + yt1 * FCOR;
  if((t_plot->picture).eqn.dimension >= 2)   *yy2 = y2 + yt2 * FCOR;
  if((t_plot->picture).eqn.dimension >= 3)   *yy3 = y3 + yt3 * FCOR;
  if((t_plot->picture).eqn.dimension >= 4)   *yy4 = y4 + yt4 * FCOR;
}
/***********************************************************/
check_range(bx,by,bz)
     double  bx, by, bz;
{
  if(xmax <  bx) xmax =  bx;
  if(xmin >  bx) xmin =  bx;
  if(ymax <  by) ymax =  by;
  if(ymin >  by) ymin =  by;
  if(zmax <  bz) zmax =  bz;
  if(zmin >  bz) zmin =  bz;
}
/***********************************************************/
check_data(t_plot)
     struct a_graph *t_plot;
{
  register int  l_num,over_flow,nnn,done,check_out;
  register FILE *fp;
  static char   line[MAX_LINE_LEN+1];

  done = GRID_DATA;
  check_out = 0;

  if (!(fp = fopen(t_plot->data_file, "r")))
    os_error("can't open data file", 1);

  while (!check_out && fgets(line, 128, fp))  
    {
      if (is_comment(line) )
	{
	  check_out = 0;
	  continue;
	}
      else
	{
	  l_num = 0;
	  while( line[l_num] == ' ' || line[l_num] == '\t') l_num++;
	  if(isalpha(line[l_num])) 
	    {
	      done = CVN_DATA;   
	      check_out = 1;
	    }
	  else
	    {
	      over_flow = 0;
	      while( line[l_num] && ! check_out)
		{
		  if( line[l_num] == ' ' || line[l_num] == '\t')
		    {
		      register int fff = 0;
		      over_flow++;
		      while( line[l_num] &&
			    ( line[l_num] == ' ' || line[l_num] == '\t'))
			{
			  l_num++; fff++;
			  if(!line[l_num] || line[l_num] == '\n')
			    over_flow--;
			}
		      if(fff) l_num--;
		    }
		  l_num++;
		}
	      if(over_flow >= 2) 
		done = GRID_DATA;	
	      else if(over_flow == 1)
		done = FEA_POLYGON;
	      else
		done = POLYGON_DATA; 
	      check_out = 1;
	    }
	}
    }
  (void) fclose(fp);

  t_plot->data_type = done;

  if(t_plot->plot_type == CONTOUR && t_plot->data_type != GRID_DATA)
    return(1);

  if(t_plot->data_type != GRID_DATA)
    return (0);
  else
    {
      if (!(fp = fopen(t_plot->data_file, "r")))
	os_error("can't open data file", 1);

      l_num = over_flow = 0;

      if(t_plot->plot_type == SURFACE)
	nnn = (t_plot->picture).surface.sample *
	  (t_plot->picture).surface.sample1;
      else if(t_plot->plot_type == CURVE)
	nnn = (t_plot->picture).curve.sample *
	  (t_plot->picture).curve.sample1;
      else if(t_plot->plot_type == TMESH)
	nnn = (t_plot->picture).tmesh.sample *
	  (t_plot->picture).tmesh.sample1;

      while (!over_flow && fgets(line, 128, fp))
	{
	  l_num++;
	  if (is_comment(line) || ! line[1]) 
	    {
	      l_num--;
	      continue;	
	    }
	  if(l_num == nnn+1)
	    over_flow = 1;
	}
      (void) fclose(fp);
      if((t_plot->plot_type == SURFACE) || (t_plot->plot_type == CURVE &&
	       (t_plot->picture).curve.sample1 != 1))
	{
	  if(l_num < nnn)
	    return(1);
	  else if(l_num > nnn)
	    {
	      (void)fprintf(STDERRR,"Warning:  too many data in %s\n",
			    t_plot->data_file);
	      return(0);
	    }
	}
      else if(t_plot->plot_type == CURVE)
	(t_plot->picture).curve.sample = l_num;
      return (0);
    }
}

/****************************************************/
get_data(t_plot)
     struct a_graph *t_plot;
{
  register FILE *fp;
  static char   line[MAX_LINE_LEN+1];
  float         xx,yy,zz;           
  int  l_num,over_flow,nnn;

  if (!(fp = fopen(t_plot->data_file, "r")))
    os_error("can't open data file", 1);

  if(t_plot->data_type == GRID_DATA)
    {
    over_flow = l_num = 0;

    if(t_plot->plot_type == SURFACE)
      nnn = (t_plot->picture).surface.sample *
	(t_plot->picture).surface.sample1;
    else if(t_plot->plot_type == CURVE)
      nnn = (t_plot->picture).curve.sample *
	(t_plot->picture).curve.sample1;
    else if(t_plot->plot_type == TMESH)
      nnn = (t_plot->picture).tmesh.sample *
	(t_plot->picture).tmesh.sample1;

    while (!over_flow && fgets(line,128, fp)) 
      {
	l_num++;
	if (is_comment(line) )
	  {
	    l_num--;
	    continue;
	  }
	if(l_num == nnn+1)
	  over_flow = 1; 
	if(!over_flow)
	  {
	    (void) sscanf(line,"%f %f %f",&xx,&yy,&zz);
	    check_range(xx,yy,zz);
	    (void) fputs(line,datafile);
	  }
      }
  }
  else if(t_plot->data_type == CVN_DATA)
    {
      l_num = 0;
      while(fgets(line,128, fp))
	{
	  if (is_comment(line) )
	    continue;
	  else
	    {
	      over_flow = 0;
	      while(line[over_flow] && 
		    (line[over_flow] == ' ' || line[over_flow] == '\t'))
		over_flow++;
	      if( line[over_flow] == 'v')
		{
		  (void) sscanf( (line+1),"%f %f %f",&xx,&yy,&zz);
		  check_range(xx,yy,zz);
		}
	      (void) fputs((line+over_flow),datafile);	  
	      l_num++;
	    }
	}
      if(t_plot->plot_type == SURFACE)
	(t_plot->picture).surface.sample = l_num;
      else if(t_plot->plot_type == CURVE)
	(t_plot->picture).curve.sample = l_num;
    }
  else if(t_plot->data_type == POLYGON_DATA)
    {
      int ii;
      l_num = 0;
      while(fgets(line,128, fp))
	{
	  if (is_comment(line))
	    continue;
	  else
	    {
	      l_num++;
	      (void) fputs(line,datafile);	  
	      (void) sscanf(line,"%d",&over_flow);
	      for(ii=0; ii < over_flow; ii++)
		{
		  if(!fgets(line,128,fp))
		    {
		      reset_plots();
		      int_error("Bad  data file",-1);
		    }
		  if (is_comment(line)) ii--;
		  else
		    {
		      (void) sscanf(line,"%f %f %f",&xx,&yy,&zz);
		      check_range(xx,yy,zz);
		      (void) fputs(line,datafile);	  
		      l_num++;
		    }
		}
	    }
	}
      if(t_plot->plot_type == SURFACE)
	(t_plot->picture).surface.sample = l_num;
      else if(t_plot->plot_type == CURVE)
	(t_plot->picture).curve.sample = l_num;
    }
  else if(t_plot->data_type == FEA_POLYGON)  
    {
      int ii,jj,mm;
      l_num = 0; over_flow = 0;
      while( fgets(line,128, fp))
	{
	  if (is_comment(line))
	    continue;
	  else
	    {
	      (void) fputs(line,datafile); 
	      (void) sscanf(line,"%d %d",&ii,&jj); 
	      l_num++;
	      for(mm = 0; mm < ii; mm++)
		{
		  if( !(fgets(line,128, fp)))
		    {
		      reset_plots();
		      int_error("Bad Data File.",-1);
		    }
		  if(is_comment(line)) mm--;
		  else
		    {
		      (void) fputs(line,datafile); 
		      (void) sscanf(line,"%f %f %f",&xx,&yy,&zz);
		      check_range(xx,yy,zz);
		    }
		}		  
	      for(mm = 0; mm < jj; mm++)
		{
		  if( !(fgets(line,128, fp)))
		    {
		      reset_plots();
		      int_error("Bad Data File.",-1);
		    }
		  if(is_comment(line)) mm--;
		  else
		    (void) fputs(line,datafile); 
		}		  
	      l_num += ii; l_num += jj;
	    }
	}
      if(t_plot->plot_type == SURFACE)
	t_plot->picture.surface.sample = l_num;
      else if(t_plot->plot_type == CURVE)
	t_plot->picture.curve.sample = l_num;
    }
  (void) fclose(fp);
}
/************************************************************/
extern double xmax,xmin,ymax,ymin,zmax,zmin;

set_axes(t_plot)
     struct a_graph *t_plot;
{
  struct uaxes *axes = (struct uaxes *)NULL;
  if(t_plot->plot_type == SURFACE)
    axes = t_plot->picture.surface.uaxes;
  else  if(t_plot->plot_type == CURVE)
    axes = t_plot->picture.curve.uaxes;
  else  if(t_plot->plot_type == CONTOUR)
    axes = t_plot->picture.contour.uaxes;
  else  if(t_plot->plot_type == MAP)
    axes = t_plot->picture.contour.uaxes;
  else  if(t_plot->plot_type == EQN)
    axes = t_plot->picture.eqn.uaxes;
  else  if(t_plot->plot_type == TMESH)
    axes = t_plot->picture.tmesh.uaxes;
  
  if( axes == (struct uaxes *)NULL) return;
  if( (axes->xx == 0.0 && axes->xy == 0.0 && axes->xz == 0.0) ||
     (axes->yx == 0.0 && axes->yy == 0.0 && axes->yz == 0.0) )
    {
      axes->ox =(float)xmin; axes->oy =(float)ymin; axes->oz =(float)zmin;
      axes->xx =(float)xmax; axes->xy =(float)ymin; axes->xz =(float)zmin;
      axes->yx =(float)xmin; axes->yy =(float)ymax; axes->yz =(float)zmin;
      axes->zx =(float)xmin; axes->zy =(float)ymin; axes->zz =(float)zmax;
    }
  if( (axes->zx == 0.0 && axes->zy == 0.0 && axes->zz == 0.0))
    {
      axes->zx = axes->ox; axes->zy = axes->oy;
      axes->zz = axes->oz + (float)(zmax-zmin);
    }
}
/************************************************************/
construct_a_tube_from_file(t_plot)
     struct a_graph *t_plot;
{
  register FILE *fp;
  static char   line[MAX_LINE_LEN+1];
  float  *tfs, *fptr;
  int  l_num,over_flow,nnn;

  if(!(fp=fopen(t_plot->data_file,"r"))) os_error("can't open data file",1);
  nnn = (t_plot->picture).tube.sample;
  if( (tfs = (float *)malloc(3* nnn * sizeof(float))) == (float *)NULL)
    int_error("Out of memory.",-1);
  fptr = tfs;

  over_flow = l_num = 0;
  while (!over_flow && fgets(line,128, fp)) 
      {
	l_num++;
	if(is_comment(line)) { l_num--; continue; }
	if(l_num > nnn) over_flow = 1; 
	else
	  { (void) sscanf(line,"%f %f %f",tfs,tfs+1,tfs+2); tfs += 3; }
      }
  (void) fclose(fp);
  construct_tube_from_data(t_plot,fptr, 0);
  (void)free( (char *)fptr);
}
/*********************************************************************/
float vangle();

construct_tube_from_data(tplot,fptr, var_rad)
     struct a_graph *tplot; float *fptr; int var_rad;
{
  float dxyz[3], n1[3], n2[3],tmpf[3];
  float old1[3],old2[3],old[3],first1[3],first2[3];
  float *f1, *f2, *f3;
  int i, j, closed = 0, ptsize;
  float radius = 0.5, angle = 0.0;
  int nsides = (tplot->picture).tube.sample1;
  int nnn = (tplot->picture).tube.sample;
  float t, dt = 3.14159265358979323*2.0/(nsides-1);

  if(var_rad == 0)
    {
      dummy_func = (tplot->picture).tube.radf;
      if(dummy_func)
	{
	  (void) complex(&(dummy_func->dummy_value), 0.0, 0.0);
	  evaluate_at( ((tplot->picture).tube.radf)->at,&ax);
	  radius = real(&ax);
	}
    }
  /*
   * first check if the curve is a closed curve
   */
  ptsize = 3 + var_rad;  /* var_rad = 0 or 1, 0_for_fixed_radius */
  f1 = fptr; f2 = fptr + ptsize*(nnn-1);
  t = fabs( *f1 - *f2) + fabs( *(f1+1)- *(f2+1)) + fabs( *(f1+2)- *(f2+2));
  if( t < 0.002)  closed = 1; 
  if(closed)  {  f2 = fptr + ptsize; f1 = fptr + ptsize*(nnn-2); }
  else { f1 = fptr; f2 = fptr + ptsize; }
  dxyz[0] = *f2 - *f1;
  dxyz[1] = *(f2+1) - *(f1+1);
  dxyz[2] = *(f2+2) - *(f1+2);
  if(dxyz[0] == 0.0 && dxyz[1] == 0.0 && dxyz[2] == 0.0) dxyz[0]=1.0;
  old[0] = 0.6543654;   old[1] = 0.0765456443;   old[2] = 0.2965433;
  vcross(dxyz,old,old1);    vcross(dxyz,old1,old2);
  normalize(old1);   normalize(old2);
  vcopy(n1,old1);  vcopy(n2,old2); vcopy(old,dxyz);
  vcopy(first1,n1); vcopy(first2,n2);
  if(var_rad) radius = *(fptr+3);
  for(i = 0; i < nsides; i++) /* the first circle */
    {
      t = dt*i;
      tmpf[0] = radius*(old2[0]*cos(t) + old1[0] * sin(t)) + *(fptr);
      tmpf[1] = radius*(old2[1]*cos(t) + old1[1] * sin(t)) + *(fptr+1);
      tmpf[2] = radius*(old2[2]*cos(t) + old1[2] * sin(t)) + *(fptr+2);
      (void)fprintf(datafile,"%f %f %f\n", tmpf[0],tmpf[1],tmpf[2]);
    }
  for( j = 1; j < nnn -1 ; j++)
    {
      f1 = fptr + ptsize*j; f2 = f1 + ptsize;
      dxyz[0] = *f2 - *f1;
      dxyz[1] = *(f2+1) - *(f1+1);
      dxyz[2] = *(f2+2) - *(f1+2);
      if(dxyz[0] == 0.0 && dxyz[1] == 0.0 && dxyz[2] == 0.0) vcopy(dxyz,old);
      else  vcopy(old, dxyz);
      vcross(dxyz,n1,old1);    vcross(n2,dxyz,old2);
      normalize(old1);   normalize(old2);
      vcopy(n2,old1);  vcopy(n1,old2);
      if(var_rad) radius = *(f1+3);
      for(i = 0; i < nsides; i++)
	{
	  t = dt*i;
	  tmpf[0] = radius*(n2[0]*cos(t) + n1[0] * sin(t)) + *(f1);
	  tmpf[1] = radius*(n2[1]*cos(t) + n1[1] * sin(t)) + *(f1+1);
	  tmpf[2] = radius*(n2[2]*cos(t) + n1[2] * sin(t)) + *(f1+2);
	  (void)fprintf(datafile,"%f %f %f\n", tmpf[0],tmpf[1],tmpf[2]);
          }
    }
  if(closed) { f2 = fptr+ptsize; f1 = fptr + ptsize*(nnn-2);  f3= fptr;}
  else  {      f1 = fptr + ptsize*(nnn -2); f2 = f1 + ptsize; f3 =f2;}
  dxyz[0] = *f2 - *f1;
  dxyz[1] = *(f2+1) - *(f1+1);
  dxyz[2] = *(f2+2) - *(f1+2);
  if(dxyz[0] == 0.0 && dxyz[1] == 0.0 && dxyz[2] == 0.0) vcopy(dxyz,old);
  vcross(dxyz,n1,old1);
  vcross(n2,dxyz,old2);
  normalize(old1); normalize(old2); 
  vcopy(n2,old1);  vcopy(n1,old2);
  if(closed)
    {
      angle = vangle(first1,n1);
      vcopy(n1,first1); vcopy(n2,first2);
    }
  if(var_rad) radius = *(f3+3);
  for(i = 0; i < nsides; i++)
    {
      t = dt*i + angle;
      tmpf[0] = radius*(n2[0]*cos(t) + n1[0] * sin(t)) + *(f3);
      tmpf[1] = radius*(n2[1]*cos(t) + n1[1] * sin(t)) + *(f3+1);
      tmpf[2] = radius*(n2[2]*cos(t) + n1[2] * sin(t)) + *(f3+2);
      (void)fprintf(datafile,"%f %f %f\n", tmpf[0],tmpf[1],tmpf[2]);
    }
}
/************************************************************/
float vangle(x1,x2) 
     float *x1,*x2;
{
  float s;
  s = acos(x1[0]*x2[0] + x1[1]*x2[1]+ x1[2]*x2[2]);
  return(s);
}
/************************************************************/    

vcross(x1,x2,n)
     float *x1,*x2,*n;
{
  float norm; 
  
  n[0] = x1[1]*x2[2]- x2[1]*x1[2];
  n[1] = x1[2]*x2[0]- x2[2]*x1[0];
  n[2] = x1[0]*x2[1]- x2[0]*x1[1];
  norm = sqrt(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);
  if(norm == 0.0) norm = 1.0;
  n[0] /= norm; n[1] /= norm; n[2] /= norm;
}
vcopy(des,src)
     float *des, *src;
{
  *des= *src; *(des+1)= *(src+1); *(des+2)= *(src+2);
}

normalize(v) float *v;
{
  float t = sqrt(v[0]*v[0] + v[1]*v[1] +v[2]*v[2]);
  if (t == 0.0)
    { v[0] = 1.0; v[2] = 0.0; v[3] = 0.0; }
  else
    { v[0] /= t; v[1] /=t; v[3] /= t;}
}
/**********************************************************************/
count_number_of_points(tplot)
     struct a_graph *tplot;
{
  int n1, i,total;
  FILE *fp, *fopen();
  char line[256];

  if(tplot->plot_type == CURVE)
    n1 = (tplot->picture).curve.sample;
  else
    n1 = (tplot->picture).tube.sample;
  if(n1) return(0); /* sample hass been specified */

  if( (fp=fopen(tplot->data_file,"r")) == (FILE *)NULL)
    int_error("Cannot open data file",-1);    
  
  total = 0;
  while(fgets(line,255, fp)) 
      {
	if(is_comment(line))  continue; 
	else total++;
      }
  if(tplot->plot_type == CURVE)
    (tplot->picture).curve.sample = total;
  else
    (tplot->picture).tube.sample = total;
}  
/********************************************************/



