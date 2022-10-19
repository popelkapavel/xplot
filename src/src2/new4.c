
/************************************************************************
 * 
 *     I R I S P L O T                  --------new4.c
 *
 *    Copyright 1989 Zou Maorong
 *
 ************************************************************************/

#include <stdio.h>
#include <math.h>
#include "plot.h"

#define HELP "xplot_help"
#ifndef ORTHOGONAL
#define ORTHOGONAL 1
#define PERSPECTIVE 2
#endif

extern int  titleset;
extern char titlestring[];
extern char dummy_var[],dummy_var1[];
extern char dummy_var2[],dummy_var3[];
extern struct value *const_express();
extern char  input_line[];
extern int   num_tokens, c_token;
extern struct a_material *look_for_mat_by_index();
extern struct a_graph    *look_for_graph_by_index();
extern struct an_object  *look_for_object_by_index();
extern double real();
char   help[128];
char  *getenv(),*strncpy(),*strcpy(),*malloc();
int    center, strlen();
char   setupfile[128];

/***************************************************************/

command2()
{
  if(almost_equals(c_token,"setup$file")) 
    {
      c_token++;
      if((END_OF_COMMAND)) setupfile[0]='\0';
      else if(isstring(c_token))  quote_str(setupfile,c_token++);
      else { capture(setupfile,c_token,c_token);c_token++;}
    }
  else if (equals(c_token,"help") || equals(c_token,"?"))
    {
      register int len;
      c_token++;
      xplot_help();
      while (!(END_OF_COMMAND)) c_token++;
    }
  else if(equals(c_token,"apply") || equals(c_token,"use") || 
	  equals(c_token,"using") )
    {
      c_token++;
      apply_things();
    }
  else if(almost_equals(c_token,"pr$int")) 
    {
      struct value a;
      c_token++;
      (void) putcc('\t',STDERRR);
      while(!END_OF_COMMAND)
	{
	  (void) const_express(&a);
	  disp_value(STDERRR,&a);
	  (void) putcc(' ',STDERRR);
	  if(equals(c_token,",")) c_token++;
	}
      (void) putcc('\n' ,STDERRR);
    }
  else if(almost_equals(c_token,"reset"))
    {
      reset_plots();
      c_token++;
    }
  else if(equals(c_token,"!"))
    {
      c_token++;
      do_system1();
    }
  else if(equals(c_token,"#"))
    {
      while(!END_OF_COMMAND) c_token++;
    }
  else
    command3();
}
/***************************************************************/

struct oview pview;
float window_size;

set_window()
{
  struct value a;
  if(almost_equals(c_token,"size")) c_token++;
  if(equals(c_token,":")) c_token++;
  window_size = (float) real( const_express(&a));
  /*  if(equals(c_token,",")) c_token++;*/
}


set_window_posotion()
{
  register int i = 0;
  struct value a;
  if(almost_equals(c_token,"posi$tion")) c_token++;
  if(equals(c_token,":")) c_token++;
  while( i < 4)
    {
      if(END_OF_COMMAND)
	int_error("Usage: window position x1 x2 y1 y2",c_token);
      pview.window_position[i] = (int) real( const_express(&a));
      if(equals(c_token,",")) c_token++;
      i++;
    }
}  
reset_window_position()
{
  pview.proj_type = 0;
  pview.window_position[0] = 5;
  pview.window_position[1] = 850;
  pview.window_position[2] = 155;
  pview.window_position[3] = 1000;
}
/**********************************************************/

set_view()
{
  register int view_defined = 0, proj_defined = 0;
  struct value a;
  while( !END_OF_COMMAND && !equals(c_token,"}"))
    {
      if(equals(c_token,"lookat"))
	{
	  c_token++;
	  if(equals(c_token,"(") || equals(c_token,":") || equals(c_token,","))
	    c_token++;
	  pview.vx = (float) real(const_express(&a));
	  if(equals(c_token,",")) c_token++;	  
	  pview.vy = (float) real(const_express(&a));
	  if(equals(c_token,",")) c_token++;	  
	  pview.vz = (float) real(const_express(&a));
	  if(equals(c_token,",")) c_token++;	  
	  pview.px = (float) real(const_express(&a));
	  if(equals(c_token,",")) c_token++;	  
	  pview.py = (float) real(const_express(&a));
	  if(equals(c_token,",")) c_token++;	  
	  pview.pz = (float) real(const_express(&a));
	  if(equals(c_token,",")) c_token++;	  
	  pview.twist = (short) real(const_express(&a));
  	  if(equals(c_token,")")) c_token++;	  
	  if(equals(c_token,",")||equals(c_token,";")) c_token++; 
	  view_defined = 1;
	}
      else if(almost_equals(c_token,"pers$pective")
	      || almost_equals(c_token,"PERS$PECTIVE"))
	{
	  pview.proj_type = PERSPECTIVE;
	  c_token++;
	  if(equals(c_token,"(") || equals(c_token,":") || equals(c_token,","))
	    c_token++;
	  pview.projections.p.fovy = (short)real(const_express(&a));
  	  if(equals(c_token,",")) c_token++; 
	  pview.projections.p.aspect = (float)real(const_express(&a));
  	  if(equals(c_token,",")) c_token++; 
	  pview.projections.p.pnear = (float)real(const_express(&a));
  	  if(equals(c_token,",")) c_token++; 
	  pview.projections.p.pfar = (float)real(const_express(&a));	  
  	  if(equals(c_token,")")) c_token++; 
  	  if(equals(c_token,",")|| equals(c_token,";")) c_token++; 
	  proj_defined = 1;
	}
      else if(almost_equals(c_token,"ortho$gonal")
	      || almost_equals(c_token,"ORTHO$GONAL"))
	{
	  pview.proj_type = ORTHOGONAL;
	  c_token++;
	  if(equals(c_token,"(") || equals(c_token,":") || equals(c_token,","))
	    c_token++; 
	  pview.projections.o.left = (float)real(const_express(&a));
  	  if(equals(c_token,",")) c_token++; 
	  pview.projections.o.right = (float)real(const_express(&a));
  	  if(equals(c_token,",")) c_token++; 
	  pview.projections.o.bottom = (float)real(const_express(&a));
  	  if(equals(c_token,",")) c_token++; 
	  pview.projections.o.top = (float)real(const_express(&a));
  	  if(equals(c_token,",")) c_token++; 
	  pview.projections.o.near = (float)real(const_express(&a));
  	  if(equals(c_token,",")) c_token++; 
	  pview.projections.o.far = (float)real(const_express(&a));	  
  	  if(equals(c_token,")")) c_token++; 
	  if(equals(c_token,",")||equals(c_token,";")) c_token++; 
	  proj_defined = 1;
	}
      else 
	int_error(" Usage:  set view {\n\t\t    lookat vx,vy,vz,px,py,pz,twist\
\n\t\t    perspective fovy,aspect,near,far \n\t\t  | ortho left,right,bottom,\
top near,far",c_token);
    }
  if( !view_defined || !proj_defined)
    int_error("Viewing and Projection must be set at the same time.", -1);
  if(equals(c_token,"}")) c_token++;
}


/*****************************************************************/

set_dummy()
{
  if(equals(c_token,":")) c_token++;
  if( !END_OF_COMMAND)      
    copy_str(dummy_var,c_token++);
  if(equals(c_token,",")) c_token++;
  if( !END_OF_COMMAND)
    copy_str(dummy_var1,c_token++);    
  if(equals(c_token,",")) c_token++;
  if( !END_OF_COMMAND)
    copy_str(dummy_var2,c_token++);    
  if(equals(c_token,",")) c_token++;
  if( !END_OF_COMMAND)
    copy_str(dummy_var3,c_token++);    
}

/*******************************************************************/

show_stuff()
{
  if( equals(c_token,"At") ) 
    {
      c_token++;
      show_at();
    }
  else if (almost_equals(c_token,"d$ummy")) 
    {
      (void)fprintf(STDERRR,"\n\tDummy variables:  %s  %s  %s  %s\n",dummy_var,
		    dummy_var1,dummy_var2,dummy_var3);
      c_token++;
    }
  else if (almost_equals(c_token,"t$itle")) 
    {
      c_token++;
      if(titleset) (void)fprintf(STDERRR, "\n\tTitle: %s\n", titlestring);
      else (void)fprintf(STDERRR, "\n\tTitle not set or disabled\n");
    }
  else if (almost_equals(c_token,"f$unctions")) 
    {
      c_token++;
      show_function_list();
    }
  else if (almost_equals(c_token,"g$raphs")) 
    {
      c_token++;
      show_graphs((FILE *)STDERRR);
    }
  else if (almost_equals(c_token,"o$bjects")) 
    {
      c_token++;
      show_objects((FILE *)STDERRR);
    }
  else if (almost_equals(c_token,"m$aterials"))
    {
      c_token++;
      show_materials((FILE *)STDERRR);
    }
  else if (almost_equals(c_token,"v$ersion")) 
    {
      show_version();
      c_token++;
    }
  else if(almost_equals(c_token,"l$ights")||almost_equals(c_token,"l$model"))
    {
      show_light_and_lmodel((FILE *)STDERRR);
      c_token++;
    }
  else if (almost_equals(c_token,"a$ll")) 
    {
      c_token++;
      show_materials((FILE *)STDERRR);
      (void)fprintf(STDERRR,"\tdummy variable:  %s  %s  %s  %s\n",dummy_var,
		    dummy_var1,dummy_var2,dummy_var3);
      show_function_list();
      show_graphs((FILE *)STDERRR);
      show_objects((FILE *)STDERRR);
      show_light_and_lmodel((FILE *)STDERRR);
      c_token++;
    }
  else
    int_error("valid options: 'all', 'dummy', 'function', 'graphs','object',\
'title', 'version'", c_token);
	      
              
  (void) putcc('\n',STDERRR);
}

/*******************************************************/
do_system()
{
  register int i;

  i = 0;
  while(input_line[i] == ' ' || input_line[i] == '\t')
    i++;
  if (system(input_line + i+1))
    os_error("system() failed",NO_CARET);
}

extern char temp_line[];

do_system1()
{
  register int i;

  i = c_token;
  while(!equals(c_token,";") && !equals(c_token,",") && c_token < num_tokens)
    c_token++;

  capture(temp_line,i,c_token-1);

  if (system(temp_line))
    os_error("system() failed",NO_CARET);
}

/*******************************************************/

#define EXEC "exec "

do_shell()
{
  static char    exec[100] = EXEC;
  register char *shell;
  if (!(shell = getenv("SHELL")))
    shell = SHELL;

  if (system(strncpy(&exec[sizeof(EXEC)-1],shell,
		     sizeof(exec)-sizeof(EXEC)-1)))
    os_error("system() failed",NO_CARET);

  (void) putcc('\n',STDERRR);
}
/***************************************************************/
extern struct a_material *material_list;
extern struct an_object  *object_list;
extern struct a_graph    *graph_list;
/***************************************************************/
show_graphs(fff)
     FILE *fff;
{
  struct a_graph **temp = &(graph_list);
  register int i;
  register char *c;

  if( !(*temp)) return;
  (void)fprintf(fff,"\n");
  while( *temp)
    {
      if((*temp)->defined)
	{
	  (void)fprintf(fff,"\t%s = ",(*temp)->name);
	  c = (*temp)->definition;
	  i = 0;
	  while( *c)
	    {
	      (void)putcc( (*c),fff); i++; c++;
	      if(i>=45)
		{
		  while( (*c) && ((*c) != '=') && ((*c) != '[') &&
			((*c) != ':') && ((*c) != ',') && ((*c) != ';'))
		    {
		      (void)putcc( (*c),fff); c++;
		    }
		  (void)fprintf(fff,"\n\t\t\t");
		  i = 0;
		}
	    }
	  (void)putcc(';',fff);
	  (void)putcc('\n',fff);
	}
      temp = &((*temp)->next);
    }
  (void)fprintf(fff,"\n");
}
/*********************************************************/
show_materials(fff)
     FILE *fff;
{
  struct a_material **temp = &(material_list->next);

  if(!(*temp)) return;
  (void)fprintf(fff,"\n");
  while(*temp)
    {
      if((*temp)->defined) {
      (void)fprintf(fff,"\t%s = material {\n", (*temp)->name);
      (void)fprintf(fff,"\t \t emission: %f %f %f\n",
	      *((*temp)->token),*((*temp)->token+1),*((*temp)->token+2));
      (void)fprintf(fff,"\t \t ambient: %f %f %f\n",
	      *((*temp)->token+3),*((*temp)->token+4),*((*temp)->token+5));
      (void)fprintf(fff,"\t \t diffuse: %f %f %f\n",
	      *((*temp)->token+6),*((*temp)->token+7),*((*temp)->token+8));
      (void)fprintf(fff,"\t \t specular: %f %f %f\n",
	      *((*temp)->token+9),*((*temp)->token+10),*((*temp)->token+11));
      (void)fprintf(fff,"\t \t shininess: %f\n",
	      *((*temp)->token+12));
      (void)fprintf(fff,"\t \t alpha: %f \n\t\t  };\n",
	      *((*temp)->token+13)); }
      temp = &((*temp)->next);
    }
  (void)fprintf(fff,"\n");
}
/*********************************************************/

show_objects(fff)
     FILE *fff; 
{
  struct an_object **temp = &(object_list);
  struct an_action *temp_action;
  float  x,y,z;

  if( !(*temp)) return;
  (void)fprintf(fff,"\n");
  while(*temp)
    { 
      if( (*temp)->defined)
	{
	  (void)fprintf(fff,"\t%s = object{\n",(*temp)->name);
	  temp_action = (*temp)->actions;
	  while(temp_action)
	    {
	      if(temp_action->entry == BMATERIAL)
		{
		  struct a_material *sss;
		  temp_action = temp_action->next;
		  if(temp_action) {
		    sss = (look_for_mat_by_index((int)(temp_action->entry)));
		    (void)fprintf(fff,"\t    material %s\n",
				  (sss)?(sss->name):"?"); }
		}
	      else if(temp_action->entry == PUSHMATRIX)
		(void)fprintf(fff,"\t    pushmatrix\n");
	      else if(temp_action->entry == POPMATRIX)
		(void)fprintf(fff,"\t    popmatrix\n");
	      else if(temp_action->entry == TRANSPARANT)
		(void)fprintf(fff,"\t    transparent\n");
	      else if(temp_action->entry == OBLIQUE)
		(void)fprintf(fff,"\t    oblique\n");
	      else if(temp_action->entry == PRETRANSLATION)
		{
		  temp_action = temp_action->next;
		  if(temp_action) {
		    x = temp_action->entry; 
		    temp_action = temp_action->next;}
		  if(temp_action) {
		    y = temp_action->entry;
		    temp_action = temp_action->next; }
		  if(temp_action) { 
		    z = temp_action->entry;
		    (void)fprintf(fff,
				  "\t   pre_translate %f,%f, %f,\n",x,y,z); }
		}
	      else if(temp_action->entry == PRESCALING)
		{
		  temp_action = temp_action->next;
		  if(temp_action) {
		    x = temp_action->entry;
		    temp_action = temp_action->next;}
		  if(temp_action) {
		    y = temp_action->entry;
		    temp_action = temp_action->next;}
		  if(temp_action) {
		    z = temp_action->entry;
		    (void)fprintf(fff,"\t   pre_scale %f,%f,%f,\n",x,y,z); }
		}
	      else if(temp_action->entry == PREROTATION)
		{
		  temp_action = temp_action->next;
		  if(temp_action) {
		    x = temp_action->entry;
		    temp_action = temp_action->next;}
		  if(temp_action) {
		    y = temp_action->entry;
		    temp_action = temp_action->next;}
		  if(temp_action) {
		    z = temp_action->entry;
		    (void)fprintf(fff,
				  "\t   pre_rotation %f,%f,%f,\n",x,y,z); }
		}
	      else if(temp_action->entry == TRANSLATE)
		{
		  temp_action = temp_action->next;
		  if(temp_action) {
		    x = temp_action->entry; 
		    temp_action = temp_action->next;}
		  if(temp_action) {
		    y = temp_action->entry;
		    temp_action = temp_action->next; }
		  if(temp_action) { 
		    z = temp_action->entry;
		    (void)fprintf(fff,"\t     translate %f,%f, %f,\n",x,y,z); }
		}
	      else if(temp_action->entry == SCALE)
		{
		  temp_action = temp_action->next;
		  if(temp_action) {
		    x = temp_action->entry;
		    temp_action = temp_action->next;}
		  if(temp_action) {
		    y = temp_action->entry;
		    temp_action = temp_action->next;}
		  if(temp_action) {
		    z = temp_action->entry;
		    (void)fprintf(fff,"\t     scale %f,%f,%f,\n",x,y,z); }
		}
	      else if(temp_action->entry == ROTATEX)
		{
		  temp_action = temp_action->next;
		  if(temp_action) {
		    x = temp_action->entry;
		    (void)fprintf(fff,"\t     xrotate %f,\n",x);}
		}
	      else if(temp_action->entry == ROTATEY)
		{
		  temp_action = temp_action->next;
		  if(temp_action) {
		    x = temp_action->entry;
		    (void)fprintf(fff,"\t     yrotate %f,\n",x);}
		}
	      else if(temp_action->entry == ROTATEZ)
		{
		  temp_action = temp_action->next;
		  if(temp_action) {
		    x = temp_action->entry;
		    (void)fprintf(fff,"\t     zrotate %f,\n",x);}
		}
	      else if(temp_action->entry == GRAPH)
		{
		  struct a_graph *ttt;
		  temp_action = temp_action->next;
		  if(temp_action)
		    {
		      ttt=(look_for_graph_by_index( (int)temp_action->entry));
		      (void)fprintf(fff,"\t    graph %s\n",(ttt)?(ttt->name):"?");
		    }
		}	 
	      else if(temp_action->entry == COBJECT)
		{
		  struct an_object *ss;
		  temp_action = temp_action->next;
		  if(temp_action)
		    {
		      ss=(look_for_object_by_index( (int)temp_action->entry));
		      (void)fprintf(fff,"\t    object %s\n",(ss)?(ss->name):"?");
		    }
		}	 
	      if(temp_action)
		temp_action = temp_action->next;
	    }
	  (void)fprintf(fff,"\t   };\n");
	}
      temp = &( (*temp)->next);
    }
  (void)fprintf(fff,"\n");
}
/*********************************************************/

show_function_list()
{
  show_variables();
  show_functions();
}
/*********************************************************/
save_light_and_lmodel(fff)
     FILE *fff;
{
  show_light_and_lmodel(fff);
}

/*********************************************************/

save_function_list(fff)
     FILE *fff;
{
  save_variables(fff);
  save_functions(fff);
}
/*********************************************************/
save_objects(fff)
     FILE *fff;
{
  show_objects(fff);
}
save_graphs(fff)
     FILE *fff;
{
  show_graphs(fff);
}
save_materials(fff)
     FILE *fff;
{
  show_materials(fff);
}

/****************************************************************/

static char file_name[128];

save_stuff()
{
  FILE *fff;
  
  if(isstring(c_token))
    {
      quote_str(file_name,c_token);
      fff = fopen(file_name,"w");
      if(fff == NULL)
	os_error("cannot open !! ",-1);
      save_all(fff);
      c_token++;
    }
  else if(isletter(c_token) && isstring(c_token+1))
    {
      quote_str(file_name,c_token+1);
      fff = fopen(file_name,"w");
      if(fff == NULL)
	os_error("cannot open !! ",-1);
      if(almost_equals(c_token,"f$unctions"))
	save_function_list(fff);
      else if(almost_equals(c_token,"m$aterials"))
	save_materials(fff);
      else if(almost_equals(c_token,"g$raphs"))
	save_graphs(fff);
      else if(almost_equals(c_token,"o$bjects"))
	save_objects(fff);
      else if(almost_equals(c_token,"l$ights")
	      || almost_equals(c_token,"l$model"))
	save_light_and_lmodel(fff);
      else if(almost_equals(c_token,"a$ll"))
	save_all(fff);
      else
	int_error("valid save option: 'f', 'g', 'o', 'a'",c_token);
      c_token += 2;
    }
  else 
    int_error("Usage: save [afglo] 'file'",c_token);
  if(fff)
    {
      (void) fflush(fff);
      (void)fclose(fff);
    }
}
/************************************************************/
save_all(fff)
     FILE *fff;
{
  save_function_list(fff);
  save_materials(fff);
  save_graphs(fff);
  save_objects(fff);
  save_light_and_lmodel(fff);
}

/**********************************************************************/

com_line()
{
  read_line();
  if (is_comment(input_line))   return;
  do_line();
}
/***************************************************************/

do_line()	 
{
  c_token = 0;
  reset_input_line();  
  do_line1();
}
do_line1()
{
  c_token = 0;
  while(c_token < num_tokens) 
    {
      command();
      if (c_token < num_tokens) 
	if (equals(c_token,";") || equals(c_token,","))
	  c_token++;
	else
	  int_error("';' expected",c_token);
    }
}

/************************************************************/
extern int   number_of_inputs;
read_line()
{
  register int  c;
  register int c_b,r_b,s_b;
  int   i,j;

  j = 0;   i = 0;
  c_b = 0; r_b=0; s_b = 0;

  (void) fprintf(stdout,"In[%d] : ",number_of_inputs);
  while(!j)
    {
    again:
      c=getchar();

      if( c == '\\')
	{
	  while( (c =getchar()) != '\n');
	  input_line[i++] = ' ';
	  c = ' ';
	  (void)fprintf(STDERRR,">\t   ");
	}
      else if (c == '#')
	{
	  while( (c = getchar()) != '\n');
	  input_line[i++] = ' '; 
	  if( !s_b && !c_b && !r_b)
	    j = 1;
	  else 
	    {
	      c = ' ';
	      (void)fprintf(STDERRR,">\t   ");
	    }
	}
      else if(c == '{') c_b++;
      else if( c == '[') s_b++;
      else if( c == '(') r_b++;
      else if( c == '}') c_b--;
      else if( c == ']') s_b--;
      else if( c == ')') r_b--;

      if(c == '\n') 
	{
	  if( c_b || s_b || r_b)
	    {
	      (void)fprintf(STDERRR,">\t   ");
	      goto again;
	    }
	  else
	    {
	      input_line[i++] = '\0';
	      j = 1;
	    }
	}

      else if(c == EOF)
	done(1);
      else
	input_line[i++] = c;
    }
  input_line[i] = '\0';
}
  
/*******************************************************/

extern int save_current_line;

command4()
{

  if (almost_equals(c_token,"set_dummy"))
    {
      c_token++;
      set_dummy();
    }
  else if(equals(c_token,"set") && almost_equals(c_token+1,"dum$my"))
    {
      c_token += 2;
      set_dummy();
    }
  else if(equals(c_token,"set") && almost_equals(c_token+1,"tit$le"))
    {
      c_token += 2;
      if((END_OF_COMMAND)) titleset = 0;
      else if(isstring(c_token)) 
	{ quote_str(titlestring, c_token++); titleset = 1;}
      else {capture(titlestring,c_token,c_token);c_token++;titleset=1;}
    }
  else if(equals(c_token,"set") && almost_equals(c_token+1,"notit$le"))
    {
      c_token += 2;
      titleset = 0;
    }
  else if (almost_equals(c_token,"sh$ow"))
    {
      c_token++;
      show_stuff();
    }
  else if (almost_equals(c_token,"she$ll")) 
    {
      c_token++;
      do_shell();
    }
  else if(almost_equals(c_token,"sa$ve")) 
    {
      c_token++;
      save_stuff();
    }
  else if (almost_equals(c_token,"l$oad")) 
    {
      c_token++;
      if(END_OF_COMMAND)
	int_error("expecting filename",c_token);
      
      if(isstring(c_token)) quote_str(file_name,c_token);
      else capture(file_name,c_token,c_token);
      load_file(fopen(file_name,"r"));
      c_token = num_tokens = 0;
    }
  else if (almost_equals(c_token,"ex$it") ||
	   almost_equals(c_token,"q$uit"))   done(IO_SUCCESS);
  else if(almost_equals(c_token,"cl$ear")) 
    {
      c_token++;
      clear_screen();
    }
  else if(equals(c_token,"%"))
    {
      int temp;
      struct value a;
      c_token++;
      temp = 1;
      while( equals(c_token,"%"))
	{
	  temp++;
	  c_token++;
	}
      if(!END_OF_COMMAND && (temp == 1))
	{
	  temp = (int) real( const_express(&a));
	  if(temp < 0)
	    temp = number_of_inputs + temp-1;	    
	}
      else
	temp = number_of_inputs - temp-1;
      if( temp > number_of_inputs - 2 || temp <= 0 || number_of_inputs <= 2)
	int_error("Record not available", NO_CARET);
      else
	{
	  extern struct a_record *current_record;
	  (void)sprintf(input_line,"In[%d]",temp);
	  save_current_line = 0;
	  c_token = 0;
	  reset_input_line();  
	  if(current_record)
	    {
	      if(current_record->definition)
		(void)free(current_record->definition);
	      current_record->definition=
		(char *)malloc((unsigned)(strlen(input_line)+3));
	      (void)strcpy(current_record->definition,input_line);
	    }
	  do_line1();
	  save_current_line = 1;
	}
    }
  else if(equals(c_token,";")) 
    c_token++;
  else
    int_error("unknown command",c_token-1);
}
/***********************************************************/

static int current_light[9];

apply_things()
{
  register int j=0,i = 0;
  if(almost_equals(c_token,"light$s"))
    {
      c_token++;
      if(equals(c_token,":")) c_token++;
      while(!END_OF_COMMAND && i < 8)
	{
	  current_light[i++] = j = look_for_light_index(c_token);
	  if(!j) int_error("Undefined Light Source",c_token);
	  if(equals(++c_token,",")) c_token++;
	}
    }
  else   if(almost_equals(c_token,"lmodel"))
    {
      c_token++;
      if(equals(c_token,":")) c_token++;
      current_light[8] = j = look_for_lmodel_index(c_token++);      
      if(!j)int_error("Undefined Light_Model",c_token-1);
    }
}
reset_light_and_lmodel()
{
  register int i = 0;
  for(i = 0; i < 9; i++)
    current_light[i] = 0;
}
/****************************************************************/

extern struct light *light_list;
extern struct lmodel *lmodel_list;
extern FILE *attrfile;

/******************************************************/

show_light_and_lmodel(fff)
     FILE *fff;
{
  struct light **temp = &(light_list);
  struct lmodel **temp1 = &(lmodel_list);

  (void)fprintf(fff,"\n");
  if((*temp)) 
    {
      while(*temp)
	{
	  (void)fprintf(fff,"\t%s= light{\n",(*temp)->name);
	  (void)fprintf(fff,"\t\tambient %f, %f, %f,\n",(*temp)->lambient[0],
			(*temp)->lambient[1],(*temp)->lambient[2]);
	  (void)fprintf(fff,"\t\tcolor %f, %f, %f,\n",(*temp)->lcolor[0],
			(*temp)->lcolor[1], (*temp)->lcolor[2]);
	  (void)fprintf(fff,"\t\tposition %f,%f,%f,%f,\n",
			(*temp)->lposition[0],
			(*temp)->lposition[1], (*temp)->lposition[2],
			(*temp)->lposition[3]);
	  (void)fprintf(fff,"\t\tspotdirection %f,%f,%f,\n",
			(*temp)->lspotdir[0],
			(*temp)->lspotdir[1],(*temp)->lspotdir[2]);
	  (void)fprintf(fff,"\t\tspotlight %f,%f\n",
			(*temp)->lspotlight[0],(*temp)->lspotlight[1]);
	  (void)fprintf(fff,"\t\t };\n");
	  temp = &((*temp)->next);
	}
    }
  if(*temp1)
    {
      while(*temp1)
	{
	  (void)fprintf(fff,"\t%s= lmodel{\n",(*temp1)->name);
	  (void)fprintf(fff,"\t\tambient %f, %f, %f,\n",(*temp1)->ambient[0],
			(*temp1)->ambient[1], (*temp1)->ambient[2]);
	  (void)fprintf(fff,"\t\tattenuation %f,%f,\n",(*temp1)->atten[0],
			(*temp1)->atten[1]);
	  (void)fprintf(fff,"\t\tattenuation2 %f,\n",(*temp1)->atten2);
	  (void)fprintf(fff,"\t\tlocalviewer %f,\n",(*temp1)->local); 
	  (void)fprintf(fff,"\t\ttwoside %f\n",(*temp1)->twoside);
	  (void)fprintf(fff,"\t\t };\n");
	  temp1 = &((*temp1)->next);
	}
    }
} 
/******************************************************/

get_light_properties(i)
     int i;
{
  struct light **temp = &(light_list);
  
  if(!current_light[i]) return;
  while( (*temp) && ((*temp)->index != current_light[i]))
    temp = &( (*temp)->next);
  
  (void) fprintf(attrfile," %f %f %f %f %f %f\n %f %f %f %f \n %f %f %f %f \n %f %f %f %f\n %f %f %f %f \n",
		 (float)NEWLIGHT,(float)LCOLOR, (*temp)->lcolor[0], 
		 (*temp)->lcolor[1],
		 (*temp)->lcolor[2], (float)POSITION,(*temp)->lposition[0],
		 (*temp)->lposition[1], (*temp)->lposition[2],
		 (*temp)->lposition[3], 
		 (float)AMBIENT,(*temp)->lambient[0],
		 (*temp)->lambient[1], (*temp)->lambient[2],
		 (float)SPOTDIRECTION,(*temp)->lspotdir[0],
		 (*temp)->lspotdir[1],(*temp)->lspotdir[2],
		 (float)SPOTLIGHT,(*temp)->lspotlight[0],
		 (*temp)->lspotlight[1],
		 (float)ON);
}

get_lmodel_properties()
{
  struct lmodel **temp = &(lmodel_list);
  
  if(!current_light[8]) return;
  while( (*temp) && (*temp)->index != current_light[8])
    temp = &( (*temp)->next);
  
  (void) fprintf(attrfile," %f %f %f %f %f\n %f %f %f %f \n %f %f %f %f \n",
		 (float)AMBIENT, (*temp)->ambient[0], (*temp)->ambient[1],
		 (*temp)->ambient[2],(float)ATTENUATION,(*temp)->atten[0],
		 (*temp)->atten[1],(float)LOCALVIEWER,(*temp)->local,
		 (float)ATTENUATION2, (*temp)->atten2,
		 (float)TWOSIDE,(*temp)->twoside);
}
/***********************************************************************/
int
  find_out_lights_and_lmodel(i)
int i;
{
  register int j,k;

  k = 0;
  if(!i)
    {
      for (j = 0; j < 8; j++)
	{
	  if(current_light[j]) k += 22;
	}
    }
  else
    if(current_light[8]) k = 13;

  if(k)
    return( k+2);
  return(0);

}
/****************************************************************/
