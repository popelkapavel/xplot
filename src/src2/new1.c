/*************************************************************
 *  
 *          I R I S P L O T   ---------- new1.c
 *
 *    Copyright (C) 1989 Maorong Zou
 *
 *************************************************************/

#include <stdio.h>
#include <math.h>
#include "plot.h"

extern int    c_token,num_tokens;
extern struct value  *const_express();
extern double real();
extern struct udft_entry *dummy_func;
extern struct udft_entry     *add_udf();
extern struct udvt_entry     *add_udv();
extern struct at_type   *perm_at();
extern char c_dummy_var[],c_dummy_var1[];
extern char c_dummy_var2[],c_dummy_var3[];

struct a_material *add_material();
struct an_object  *add_object();
struct a_graph    *add_graph();
struct an_object  *look_for_object_name();
struct an_object  *look_for_object_by_index();
struct an_action  *search_for_object(),*add_r_action();
struct a_material *look_for_mat_by_index();
struct a_graph    *look_for_graph_by_index();
struct a_material *material_list = (struct a_material *)NULL;
struct a_graph    *graph_list = (struct a_graph *)NULL;
struct an_object  *object_list = (struct an_object *)NULL;
struct light      *light_list = (struct light *)NULL;
struct lmodel     *lmodel_list = (struct lmodel *)NULL;
struct dummy_obj  *dummy_obj_list = (struct dummy_obj *)NULL;
struct a_material *c_material = (struct a_material *) NULL;
struct a_graph    *c_graph = (struct a_graph *)NULL;
struct an_object  *c_object = (struct an_object *)NULL;
int               global_index = 1000;
char              global_name[8];
char              *generate_name();
int               generate_index();

char *malloc();
char *strcpy();
char *strcat();
int   strlen();


/************************************************************/
struct lmodel
  *add_lmodel()
{
  register struct lmodel **temp = &(lmodel_list);
  
  while(*temp)
    {
      if(equals(c_token, (*temp)->name))
	{
	  if((*temp)->name) (void)free( (char *) (*temp)->name);
	  (*temp)->name = (char *) NULL;
	  return( *temp);
	}
      temp = &( (*temp)->next);
    }
  *temp = (struct lmodel *)
    malloc( (unsigned int) sizeof(struct lmodel));

  (*temp)->ambient[0] = 0.2;
  (*temp)->ambient[1] = 0.2;
  (*temp)->ambient[2] = 0.2;
  (*temp)->atten[0] = 1.0;
  (*temp)->atten[1] = 0.0;
  (*temp)->atten2 = 0.0;
  (*temp)->twoside = 0.0;
  (*temp)->local = 0.0;

  (*temp)->name = (char *)NULL;
  (*temp)->index = generate_index();
  (*temp)->next = (struct lmodel *) NULL;
  return( *temp);
}

/*****************************************************************/

struct light
  *add_light()
{
  register struct light **temp = &(light_list);
  
  while(*temp)
    {
      if(equals(c_token, (*temp)->name))
	{
	  if((*temp)->name) (void)free( (char *) (*temp)->name);
	  (*temp)->name = (char *) NULL;
	  return( *temp);
	}
      temp = &( (*temp)->next);
    }
  *temp = (struct light *)
    malloc( (unsigned int) sizeof(struct light));

  (*temp)->lambient[0] = 0.0;
  (*temp)->lambient[1] = 0.0;
  (*temp)->lambient[2] = 0.0;
  (*temp)->lcolor[0] = 1.0;
  (*temp)->lcolor[1] = 1.0;
  (*temp)->lcolor[2] = 1.0;
  (*temp)->lposition[0] = 10.0;
  (*temp)->lposition[1] = 10.0;
  (*temp)->lposition[2] = 2.0;
  (*temp)->lposition[3] = 0.0;

  (*temp)->lspotdir[0] = 0.0;
  (*temp)->lspotdir[1] = 0.0;
  (*temp)->lspotdir[2] = 1.0;

  (*temp)->lspotlight[0] = 0.0;
  (*temp)->lspotlight[1] = 180.0;

  (*temp)->name = (char *)NULL;
  (*temp)->index = generate_index();
  (*temp)->next = (struct light *) NULL;
  return( *temp);
}

/*****************************************************************/

struct a_material
  *add_material()
{
  register struct a_material **temp = &(material_list);
  
  while(*temp)
    {
      if(equals(c_token, (*temp)->name))
	{
	  if((*temp)->name) (void)free( (char *) (*temp)->name);
	  (*temp)->name = (char *) NULL;
	  (*temp)->defined = 0;
	  return( *temp);
	}
      temp = &( (*temp)->next);
    }
  *temp = (struct a_material *)
    malloc( (unsigned int) sizeof(struct a_material));

  (*temp)->token[0] = 0.0;
  (*temp)->token[1] = 0.0;
  (*temp)->token[2] = 0.0;
  (*temp)->token[3] = 0.2;
  (*temp)->token[4] = 0.2;
  (*temp)->token[5] = 0.2;
  (*temp)->token[6] = 0.6;
  (*temp)->token[7] = 0.6;
  (*temp)->token[8] = 0.6;
  (*temp)->token[9] = 1.0;
  (*temp)->token[10] = 1.0;
  (*temp)->token[11] = 1.0;
  (*temp)->token[12] = 14.0;
  (*temp)->token[13] = 0.8;
  (*temp)->name = (char *)NULL;
  (*temp)->defined = 0;
  (*temp)->index = generate_index();
  (*temp)->next = (struct a_material *) NULL;

  return( *temp);
}

/************************************************/
struct a_graph
  *add_graph()
{
  register struct  a_graph **temp = &(graph_list);

  while( *temp)
    {
      if(equals(c_token, (*temp)->name))
	{ 
	  if((*temp)->name) (void)free( (char *) (*temp)->name);
	  if((*temp)->definition) (void)free( (char *) (*temp)->definition);
	  (*temp)->definition = (char *)NULL;
	  (*temp)->name  = (char *)NULL;
	  (*temp)->defined = 0;
	  (*temp)->is_data = 0;
	  (*temp)->data_check = 0;  /* don't check data type */
	  (*temp)->data_type = GRID_DATA;
	  if( ( (*temp)->plot_type) == SURFACE)
	    {
	      do_free( ((*temp)->picture).surface.fx);
	      do_free( ((*temp)->picture).surface.fy);
	      do_free( ((*temp)->picture).surface.fz);

	      check_free( ((*temp)->picture).surface.uaxes);
	    }
	  else if( ( (*temp)->plot_type) == CURVE)
	    {
	      do_free( ((*temp)->picture).curve.fx);
	      do_free( ((*temp)->picture).curve.fy);	      
	      do_free( ((*temp)->picture).curve.fz);

	      check_free( ((*temp)->picture).curve.uaxes);
	    }
	  else if( ( (*temp)->plot_type) == MAP)
	    {
	      do_free( ((*temp)->picture).map.fx);
	      do_free( ((*temp)->picture).map.fy);
	      do_free( ((*temp)->picture).map.fz);
	      do_free( ((*temp)->picture).map.fw);

	      check_free( ((*temp)->picture).map.uaxes);
	    }
	  else if( ( (*temp)->plot_type) == EQN)
	    {
	      do_free( ((*temp)->picture).eqn.fx);
	      do_free( ((*temp)->picture).eqn.fy);
	      do_free( ((*temp)->picture).eqn.fz);
	      do_free( ((*temp)->picture).eqn.fw);

	      check_free( ((*temp)->picture).eqn.uaxes);
	    }
	  /*
	   * BUG, what about TMESH ?
	   */
	  else if( ( (*temp)->plot_type) == TMESH)	  
	    {
	      check_free( ((*temp)->picture).tmesh.uaxes);	      
	    }
	  return( *temp);
	}
      temp = &( (*temp)->next);
    }
  *temp = (struct a_graph *)
    malloc( (unsigned int) sizeof(struct a_graph));

  (*temp)->index = generate_index();
  (*temp)->name = (char *)NULL;
  (*temp)->defined = 0;
  (*temp)->definition = (char *)NULL;
  (*temp)->plot_type = 0;
  (*temp)->is_data = 0;
  (*temp)->data_type = GRID_DATA;
  (*temp)->data_check = 0;
  (*temp)->next = (struct a_graph *) NULL;
  
  return( *temp);
}
/***************************************************/
check_free( axis)
     struct uaxes *axis;
{
  if( axis != (struct uaxes *)NULL)
    (void) free((char *) axis);
}
/***************************************************/
do_free(fx)
     struct udft_entry *fx;
{
  if(fx)
    {
      if( fx->at)
	(void)free ( (char *) (fx->at));
      (void)free ( (char *) fx);
    }
}
/***************************************************/
extern  char temp_line[];
extern  int strcmp();

struct an_object  *add_object(ccc)
     char *ccc;
{
  register struct an_object **temp = &(object_list);

  while( *temp)
    {
      if(equals(c_token, ((*temp)->name)) || 
	 (ccc && !strcmp(ccc, ((*temp)->name))))
	{
	  if(( (*temp)->name)) (void)free((char *) ( (*temp)->name));
	  (*temp)->name = (char *)NULL;
	  (*temp)->defined = 0;
	  if( (*temp)->actions)
	    {
	      struct an_action *temp_action;
	      while( (*temp)->actions)
		{
		  temp_action= (*temp)->actions->next;
		  (void)free( (char *) ( (*temp)->actions));
		  (*temp)->actions = temp_action;
		}
	      (*temp)->actions = (struct an_action *)NULL;
	      if( (*temp)->menu_string )
		(void) free ( (*temp)->menu_string );
	      (*temp)->menu_string = (char *)NULL;
	      (void)strcpy(temp_line,"Material%t|");
	    }
	  return( *temp);
	}
      temp = &( (*temp)->next);
    }
  *temp = (struct an_object *)
    malloc( (unsigned int) sizeof(struct an_object));

  (*temp)->name = (char *)NULL;
  (*temp)->defined = 0;
  (*temp)->menu_string = (char *)NULL;
  (void)strcpy(temp_line,"Material%t|");
  (*temp)->index = generate_index();
  (*temp)->actions = (struct an_action *)NULL;
  (*temp)->next = (struct an_object  *) NULL;
  
  return( *temp);
}
/***************************************************/
m_copy(dest,src)
     char **dest,*src;
{
  register int i;

  i = strlen(src);
  (*dest) = (char *)malloc( (unsigned int) (i+1) * sizeof(char));
  (void)strcpy((*dest),src);
  *((*dest)+i) = '\0';
}
/***************************************************/
char 
  *generate_name()
{
  (void) sprintf(global_name, "No%d",global_index++);
  global_name[7] = '\0';
  return(global_name);
}
/***************************************************/
int
  generate_index()
{
  return(++global_index);
}
/***************************************************/
int
  is_object_list(t_num)
int t_num;
{
  if( almost_equals(t_num,"obj$ect") && 
     (almost_equals(t_num+1,"li$st") ||
      almost_equals(t_num+1,"tab$le")||
      almost_equals(t_num+1,"definition$s")) &&
     almost_equals(t_num+2,"{"))
    return(3);
  else if (almost_equals(t_num,"def$ine") &&
	   almost_equals(t_num+1,"object$s") &&
	   almost_equals(t_num+2,"{"))
    return(3);
  else
    return(0);
}
/*************************************************/
int
  is_material_list(t_num)
int t_num;
{
  if( almost_equals(t_num,"mat$erial") && 
     (almost_equals(t_num+1,"li$st") ||
      almost_equals(t_num+1,"tab$le")||
      almost_equals(t_num+1,"definition$s")) &&
     almost_equals(t_num+2,"{"))
    return(3);
  else if (almost_equals(t_num,"def$ine") &&
	   almost_equals(t_num+1,"material$s") &&
	   almost_equals(t_num+2,"{"))
    return(3);
  else
    return(0);
}
/*************************************************/
int
  is_material(t_num)
int t_num;
{
  if(almost_equals(t_num,"mat$erial") &&
     almost_equals(t_num+1,"{"))
    return(2);
  else  if( isletter(t_num) && equals(t_num+1,"=") &&
	   almost_equals(t_num+2,"mat$erial") &&
	   almost_equals(t_num+3,"{"))
    return(4);
  else 
    return(0);
}
/*****************************************************/
int  is_graph_list(t_num)
     int t_num;
{
  if( almost_equals(t_num,"gra$ph") && 
     (almost_equals(t_num+1,"li$st") ||
      almost_equals(t_num+1,"tab$le")||
      almost_equals(t_num+1,"definition$s")) &&
     equals(t_num+2,"{"))
    return(3);
  else if (almost_equals(t_num,"def$ine") &&
	   almost_equals(t_num+1,"graph$s") &&
	   equals(t_num+2,"{"))
    return(3);
  else
    return(0);
}  
/****************************************************/
int
  is_graph_keyword(t_num)
{
  if(almost_equals(t_num,"sur$face"))
    return(SURFACE);
  else if(almost_equals(t_num,"cur$ve"))
    return(CURVE);

  else if(equals(t_num,"map"))
    return(MAP);
  else if(equals(t_num,"eqn") || equals(t_num,"ode"))
    return(EQN);
  else if(equals(t_num,"tmesh"))
    return(TMESH);
  else if(equals(t_num,"tube"))
    return(TUBE);
  else
    return(0);
}
/***************************************************/
int
  is_graph(t_num)
int t_num;
{
  int i = 0;

  if((i = is_graph_keyword(t_num)))
    return(i);
  else  if( isletter(t_num) && equals(t_num+1,"=") &&
	   (i = is_graph_keyword(t_num+2)))
    return( i + 100);
  else
    return(0);
}
/**********************************************************/
int
  is_function_list(t_num)
int t_num;
{
  if( almost_equals(t_num,"func$tion") && 
     (almost_equals(t_num+1,"li$st") ||
      almost_equals(t_num+1,"tab$le")||
      almost_equals(t_num+1,"definition$s")) &&
     equals(t_num+2,"{"))
    return(3);
  else if (almost_equals(t_num,"def$ine") &&
	   almost_equals(t_num+1,"function$s") &&
	   equals(t_num+2,"{"))
    return(3);
  else
    return(0);
}  
/****************************************************/
int 
  is_object(t_num)
int t_num;
{
  if(almost_equals(t_num,"obj$ect") && equals(t_num+1,"{"))
    return(2);
  else if( isletter(t_num) && equals(t_num+1,"=") &&
	  almost_equals(t_num+2,"obj$ect") &&
	  equals(t_num+3,"{"))
    return(4);
  else
    return(0);
}
/****************************************************/
int 
  is_light(t_num)
int t_num;
{
  if( isletter(t_num) && equals(t_num+1,"=") &&
     equals(t_num+2,"light") && equals(t_num+3,"{"))
    return(4);
  return(0);
}
/****************************************************/
int 
  is_light_model(t_num)
int t_num;
{
  if( isletter(t_num) && equals(t_num+1,"=") &&
     equals(t_num+2,"lmodel") && equals(t_num+3,"{"))
    return(4);
  return(0);
}
/***************************************************************/
int
  is_set_view(t_num)
int t_num;
{
  if( equals(t_num,"view") && equals(t_num+1,"=") &&
     equals(t_num+2,"{"))
    return (3);
  else if( equals(t_num,"set") && 
     equals(t_num+1,"view") )
    {
      if(equals(t_num+2,"{"))
	return(3);
      else if(equals(t_num+2,"=") && equals(t_num+3,"{"))
	return(4);
    }
  else if (equals(t_num,"set_view") && equals(t_num+1,"{"))
    return(2);
  return(0);
}
/*****************************************************/

make_light_model()
{
  struct value a;
  struct lmodel *c_light_model;

  c_light_model = add_lmodel();
  m_capture(&(c_light_model->name),c_token, c_token);
  c_token +=  4;

  while( !equals(c_token,"}") && !END_OF_COMMAND)
    {
      if(almost_equals(c_token,"amb$ient"))
	{
	  c_token++;
	  if(equals(c_token,":") || equals(c_token,",")) c_token++;
	  c_light_model->ambient[0] = (float)real(const_express(&a));
	  if(equals(c_token,",")) c_token++;
	  c_light_model->ambient[1] = (float)real(const_express(&a));
	  if(equals(c_token,",")) c_token++;
	  c_light_model->ambient[2] = (float)real(const_express(&a));
	  if(equals(c_token,",") || equals(c_token,";")) c_token++;
	}
      else if(almost_equals(c_token,"att$enuation"))
	{
	  c_token++;
	  if(equals(c_token,":") || equals(c_token,",")) c_token++;
	  c_light_model->atten[0] = (float)real(const_express(&a));
	  if(equals(c_token,",")) c_token++;
	  c_light_model->atten[1] = (float)real(const_express(&a));
	  if(equals(c_token,",") || equals(c_token,";")) c_token++;
	}
      else if(equals(c_token,"att2") ||
	      almost_equals(c_token,"att$enuation2"))
	{
	  c_token++;
	  if(equals(c_token,":") || equals(c_token,",")) c_token++;
	  c_light_model->atten2 = (float)real(const_express(&a));
	  if(equals(c_token,",") || equals(c_token,";")) c_token++;
	}	  
      else if(almost_equals(c_token,"twoside$s"))
	{
	  c_token++;
	  if(equals(c_token,":") || equals(c_token,",")) c_token++;
	  c_light_model->twoside = (float)real(const_express(&a));
	  if(equals(c_token,",") || equals(c_token,";")) c_token++;
	}	  
      else if(almost_equals(c_token,"local$viewer"))
	{
	  c_token++;
	  if(equals(c_token,":") || equals(c_token,",")) c_token++;
	  c_light_model->local = (float)real(const_express(&a));
	  if(equals(c_token,",") || equals(c_token,";")) c_token++;
	}
      else 
	int_error("Unknown attributes for Light definition",c_token);
    }
  if(equals(c_token,"}")) c_token++;
}
/******************************************************/

make_light()
{
  struct value a;
  struct light *c_light;
  int  is_spot = 0;

  c_light = add_light();
  m_capture(&(c_light->name),c_token, c_token);
  c_token +=  4;

  while( !equals(c_token,"}") && !END_OF_COMMAND)
    {
      if(almost_equals(c_token,"color$"))
	{
	  c_token++;
	  if(equals(c_token,":") || equals(c_token,",")) c_token++;
	  c_light->lcolor[0] = (float)real(const_express(&a));
	  if(equals(c_token,",")) c_token++;
	  c_light->lcolor[1] = (float)real(const_express(&a));
	  if(equals(c_token,",")) c_token++;
	  c_light->lcolor[2] = (float)real(const_express(&a));
	  if(equals(c_token,",") || equals(c_token,";")) c_token++;
	}
      else if(almost_equals(c_token,"amb$ient"))
	{
	  c_token++;
	  if(equals(c_token,":") || equals(c_token,",")) c_token++;
	  c_light->lambient[0] = (float)real(const_express(&a));
	  if(equals(c_token,",")) c_token++;
	  c_light->lambient[1] = (float)real(const_express(&a));
	  if(equals(c_token,",")) c_token++;
	  c_light->lambient[2] = (float)real(const_express(&a));
	  if(equals(c_token,",") || equals(c_token,";")) c_token++;
	}
      else if(almost_equals(c_token,"spotdir$ection"))
	{
	  float norm;
	  c_token++;
	  if(equals(c_token,":") || equals(c_token,",")) c_token++;
	  c_light->lspotdir[0] = (float)real(const_express(&a));
	  if(equals(c_token,",")) c_token++;
	  c_light->lspotdir[1] = (float)real(const_express(&a));
	  if(equals(c_token,",")) c_token++;
	  c_light->lspotdir[2] = (float)real(const_express(&a));
	  if(equals(c_token,",") || equals(c_token,";")) c_token++;
	  norm = (float)sqrt((c_light->lspotdir[0])*(c_light->lspotdir[0])+
		      (c_light->lspotdir[1])*(c_light->lspotdir[1])+
		      (c_light->lspotdir[2])*(c_light->lspotdir[2]));
	  (c_light->lspotdir[0]) /= norm;
	  (c_light->lspotdir[1]) /= norm;
	  (c_light->lspotdir[2]) /= norm;
	  is_spot = 1;
	}
      else if(almost_equals(c_token,"spotlig$ht"))
	{
	  c_token++;
	  if(equals(c_token,":") || equals(c_token,",")) c_token++;
	  c_light->lspotlight[0] = (float)real(const_express(&a));
	  if(equals(c_token,",")) c_token++;
	  c_light->lspotlight[1] = (float)real(const_express(&a));
	  if(equals(c_token,",") || equals(c_token,";")) c_token++;
	  is_spot = 1;
	}
      else if(almost_equals(c_token,"posi$tion"))
	{
	  c_token++;
	  if(equals(c_token,":") || equals(c_token,",")) c_token++;
	  c_light->lposition[0] = (float)real(const_express(&a));
	  if(equals(c_token,",")) c_token++;
	  c_light->lposition[1] = (float)real(const_express(&a));
	  if(equals(c_token,",")) c_token++;
	  c_light->lposition[2] = (float)real(const_express(&a));
	  if(equals(c_token,",")) c_token++;
	  c_light->lposition[3] = (float)real(const_express(&a));
	  if(equals(c_token,",") || equals(c_token,";")) c_token++;
	}
      else 
	int_error("Unknown attributes for Light definition",c_token);
    }
  if(equals(c_token,"}")) c_token++;
  if(is_spot != 0 && c_light->lposition[3] == 0.0)
    {
      (void)fprintf(stderr,"\t Warning: can't set nonlocal spotlight src\n");
      c_light->lspotlight[0] = 0.0;
      c_light->lspotlight[1] = 180.0;
    }
}
/******************************************************/

make_material()
{
  c_material = add_material();
  if(almost_equals(c_token,"mat$erial"))
    {
      m_copy(&(c_material->name),generate_name());
      c_token += 2;
    }
  else
    {
      m_capture(&(c_material->name),c_token, c_token);
      c_token +=  4;
    }

  while( !equals(c_token,"}") && !END_OF_COMMAND)
    {
      if(almost_equals(c_token,"emi$ssion"))
	{
	  c_token++;
	  define_material(0);
	}
      else if(almost_equals(c_token,"amb$ient"))
	{
	  c_token++;
	  define_material(3);
	}
      else if(almost_equals(c_token,"diff$use"))
	{
	  c_token++;
	  define_material(6);
	}
      else if(almost_equals(c_token,"spe$cular"))
	{
	  c_token++;
	  define_material(9);
	}
      else if(almost_equals(c_token,"shin$iness"))
	{
	  c_token++;
	  define_material(12);
	}
      else if(almost_equals(c_token,"alp$ha"))
	{
	  c_token++;
	  define_material(13);
	}
      else
	int_error("unknown material property",c_token);
    }
  if(equals(c_token,"}")) c_token++;
  c_material->defined = 1;
}
/******************************************************/

define_material(ii)
     int ii;
{
  register int i;
  struct value a;

  i = ii;
  if(i <= 10)
    {
      if(equals(c_token,":") || equals(c_token,","))  c_token++;
      *(c_material->token + i++) = (float) real(const_express( &a));
      if(equals(c_token,":") || equals(c_token,","))  c_token++;
      *(c_material->token + i++) = (float) real(const_express( &a));
      if(equals(c_token,":") || equals(c_token,","))  c_token++;
      *(c_material->token + i++) = (float) real(const_express( &a));
      if( equals(c_token,","))  c_token++;
    }
  else
    {
      if(equals(c_token,":") || equals(c_token,","))  c_token++;
      *(c_material->token + i++) = (float) real(const_express( &a));
      if(equals(c_token,","))  c_token++;
    }
}
/********************************************************************/
make_graph()
{
  register int i,start_token;

  start_token = c_token;
  c_graph = add_graph();
  if( (i = is_graph_keyword(c_token)))
    {
      m_copy(&(c_graph->name),generate_name());
      c_graph->plot_type = i;
      c_token += 2;
    }
  else
    {
      m_capture(&(c_graph->name),c_token, c_token);
      c_token +=  2;
      start_token += 2;
      c_graph->plot_type = is_graph_keyword(c_token);
      c_token += 2;
    }
  
  switch(c_graph->plot_type)
    {
    case SURFACE:
      parse_surface();
      break;
    case CURVE:
      parse_curve();
      break;
    case TMESH:
      parse_tmesh();
      break;
    case MAP:
      parse_map();
      break;
    case EQN:
      parse_eqn();
      break;
    case TUBE:
      parse_tube();
      break;
    default:
      int_error("unknown graph type",c_token);
      break;
    }
  m_capture(&(c_graph->definition),start_token,c_token-1);
  c_graph->defined = 1;
}
/******************************************************/
struct an_action *
  add_obj_action()
{
  register struct an_action **temp = &(c_object->actions);
  
  while( *temp)
    temp = &((*temp)->next);
  (*temp) = (struct an_action *) malloc( (unsigned int)
				  sizeof(struct an_action));
  if(! (*temp))
    int_error("out of memory",c_token);
  (*temp)->next = (struct an_action *) NULL;
  (*temp)->entry = 0.0;
  return(*temp);
}
/**************************************************************/

make_object()
{
  register int i,count,temp_token,first_one;
  register int pushed,mat_defined,transp;
  register struct an_action *temp_action = (struct an_action *)NULL;
  char *temp_c = (char *) NULL;
  struct an_action *c_action;
  struct value a;

  temp_line[0] = '\0';                   /* no harm */

  c_object = add_object( (char *)NULL);

  if( almost_equals(c_token,"obj$ect"))
    {
      m_copy(&(c_object->name),generate_name());
      c_token += 2;
    }
  else
    {
      m_capture(&(c_object->name),c_token, c_token);
      c_token +=  4;
    }

  pushed = 0;
  mat_defined = 0;
  transp = 0;
  first_one = 1;
  c_action = add_obj_action();
  count = 1;

  while( !equals(c_token,"}") && !END_OF_COMMAND)
    {
      if( first_one)
	{
	  while(first_one <= 4)
	    {
	      if(almost_equals(c_token,"pre_trans$late") )
		{
		  c_action = add_obj_action(); count++;
		  c_action->entry = PRETRANSLATION;
		  c_token++;
		  if(equals(c_token,":")||equals(c_token,",")) c_token++;
		  c_action = add_obj_action(); count++;
		  c_action->entry = (float) real(const_express(&a));
		  if(equals(c_token,":")||equals(c_token,",")) c_token++;
		  c_action = add_obj_action(); count++;
		  c_action->entry = (float) real(const_express(&a));
		  if(equals(c_token,":")||equals(c_token,",")) c_token++;
		  c_action = add_obj_action(); count++;
		  c_action->entry = (float) real(const_express(&a));
		  if(equals(c_token,";")||equals(c_token,",")) c_token++;
		}
	      else if(almost_equals(c_token,"pre_rot$ate") )
		{
		  c_action = add_obj_action(); count++;
		  c_action->entry = PREROTATION;
		  c_token++;
		  if(equals(c_token,":")||equals(c_token,",")) c_token++;
		  c_action = add_obj_action(); count++;
		  c_action->entry = (float) real(const_express(&a));
		  if(equals(c_token,":")||equals(c_token,",")) c_token++;
		  c_action = add_obj_action(); count++;
		  c_action->entry = (float) real(const_express(&a));
		  if(equals(c_token,":")||equals(c_token,",")) c_token++;
		  c_action = add_obj_action(); count++;
		  c_action->entry = (float) real(const_express(&a));
		  if(equals(c_token,";")||equals(c_token,",")) c_token++;
		}
	      else if(almost_equals(c_token,"pre_sca$le") )
		{
		  c_action = add_obj_action(); count++;
		  c_action->entry = PRESCALING;
		  c_token++;
		  if(equals(c_token,":")||equals(c_token,",")) c_token++;
		  c_action = add_obj_action(); count++;
		  c_action->entry = (float) real(const_express(&a));
		  if(equals(c_token,":")||equals(c_token,",")) c_token++;
		  c_action = add_obj_action(); count++;
		  c_action->entry = (float) real(const_express(&a));
		  if(equals(c_token,":")||equals(c_token,",")) c_token++;
		  c_action = add_obj_action(); count++;
		  c_action->entry = (float) real(const_express(&a));
		  if(equals(c_token,";")||equals(c_token,",")) c_token++;
		}	  
	      first_one++;
	    }
	  first_one = 0;
	}

      if(almost_equals(c_token,"transp$arent"))
	{
	  transp = 1;
	  c_action = add_obj_action(); count++;
	  c_action->entry = TRANSPARANT;
	  c_token++;
	  if(equals(c_token,";")||equals(c_token,",")) c_token++;
	}
      else if(almost_equals(c_token,"endtransp$arent") ||
	      almost_equals(c_token,"obli$que"))
	{
	  transp = 0;
	  c_action = add_obj_action(); count++;
	  c_action->entry = OBLIQUE;
	  c_token++;
	  if(equals(c_token,";")||equals(c_token,",")) c_token++;
	}
      else if(almost_equals(c_token,"mat$erial") ||
	      is_material(c_token))
	{
	  if(mat_defined) 
	    {
	      (void)fprintf(STDERRR, "\t(New material hide old one in %s)\n",
			    c_object->name);
		*temp_c = '\0';  /* may cause trouble */
	      {
		  register struct an_action *temp_action1;
		  temp_action1 = temp_action->next;
		  temp_action->next =
		    ((temp_action->next)->next)->next;
		  (void)free( (char *) temp_action1->next);
		  (void)free( (char *) temp_action1);
		  count -= 2;
		}
	    }
	  mat_defined = 1;
	  temp_action = c_action;
	  c_action = add_obj_action(); count++;
	  c_action->entry = BMATERIAL;
	  if( ! is_material(c_token))
	    {
	      c_token++;
	      if(equals(c_token,":")||equals(c_token,",")) c_token++;
	    }
	  c_action = add_obj_action(); count++;
	  temp_token = c_token;
	  if((i = is_material(c_token)))
	    {
	      make_material();
	      if(equals(c_token,";")||equals(c_token,",")) c_token++;
	      c_action->entry = (float) (c_material->index);
	      temp_c = &(temp_line[strlen(temp_line)]);
	      (void)strcat(temp_line,c_material->name);
	      (void)strcat(temp_line,"|");
	    }
	  else
	    {
	      i = look_for_mat_index(temp_token);
	      if(i)
		{
		  c_action->entry = (float) i;
		  temp_c = &(temp_line[strlen(temp_line)]);
		  (void)strcat(temp_line,(look_for_mat_by_index(i))->name);
		  (void)strcat(temp_line,"|");		  
		}
	      else
		int_error("undefined material",c_token);
	      c_token++;
	    }
	  if(equals(c_token,";")||equals(c_token,",")) c_token++;
	}
      else if(almost_equals(c_token,"push$matrix"))
	{
	  pushed++;
  	  c_action = add_obj_action(); count++;
	  c_action->entry = PUSHMATRIX;
	  c_token++;
	  if(equals(c_token,";")||equals(c_token,",")) c_token++;
	}
      else if(almost_equals(c_token,"pop$matrix"))
	{
	  if(!pushed) 
	    int_error("unbalanced popmatrix",c_token);
	  pushed--;	  
  	  c_action = add_obj_action(); count++;
	  c_action->entry = POPMATRIX;
	  c_token++;
	  if(equals(c_token,";")||equals(c_token,",")) c_token++;
	}
      else if(almost_equals(c_token,"trans$late"))
	{
	  if(!pushed) 
	    int_error("need a pushmatrix",c_token);
  	  c_action = add_obj_action(); count++;
	  c_action->entry = TRANSLATE;
	  c_token++;
	  if(equals(c_token,":")||equals(c_token,",")) c_token++;
  	  c_action = add_obj_action(); count++;
	  c_action->entry = (float) real(const_express(&a));
	  if(equals(c_token,":")||equals(c_token,",")) c_token++;
  	  c_action = add_obj_action(); count++;
	  c_action->entry = (float) real(const_express(&a));
	  if(equals(c_token,":")||equals(c_token,",")) c_token++;
  	  c_action = add_obj_action(); count++;
	  c_action->entry = (float) real(const_express(&a));
	  if(equals(c_token,";")||equals(c_token,",")) c_token++;
	}
      else if(almost_equals(c_token,"sca$le"))
	{
	  if(!pushed) 
	    int_error("need a pushmatrix",c_token);
  	  c_action = add_obj_action(); count++;
	  c_action->entry = SCALE;
	  c_token++;
	  if(equals(c_token,":")||equals(c_token,",")) c_token++;
  	  c_action = add_obj_action(); count++;
	  c_action->entry = (float) real(const_express(&a));
	  if(equals(c_token,":")||equals(c_token,",")) c_token++;
  	  c_action = add_obj_action(); count++;
	  c_action->entry = (float) real(const_express(&a));
	  if(equals(c_token,":")||equals(c_token,",")) c_token++;
  	  c_action = add_obj_action(); count++;
	  c_action->entry = (float) real(const_express(&a));
	  if(equals(c_token,";")||equals(c_token,",")) c_token++;
	}
      else if(equals(c_token,"rotatex") || 
	      almost_equals(c_token,"xrot$ate"))
	{
	  if(!pushed) 
	    int_error("need a pushmatrix",c_token);
  	  c_action = add_obj_action(); count++;
	  c_action->entry = ROTATEX;
	  c_token++;
	  if(equals(c_token,":")||equals(c_token,",")) c_token++;
	  c_action = add_obj_action();  count++;
	  c_action->entry = (float) real(const_express(&a));
	  if(equals(c_token,";")||equals(c_token,",")) c_token++;
	}
      else if(equals(c_token,"rotatey") || 
	      almost_equals(c_token,"yrot$ate"))
	{
	  if(!pushed) 
	    int_error("need a pushmatrix",c_token);
	  c_action = add_obj_action(); count++;
	  c_action->entry = ROTATEY;
	  c_token++;
	  if(equals(c_token,":")||equals(c_token,",")) c_token++;
	  c_action = add_obj_action(); count++;
	  c_action->entry = (float) real(const_express(&a));
	  if(equals(c_token,";")||equals(c_token,",")) c_token++;
	}
      else if(equals(c_token,"rotatez") || 
	      almost_equals(c_token,"zrot$ate"))
	{
	  if(!pushed) 
	    int_error("need a pushmatrix",c_token);
  	  c_action = add_obj_action(); count++;
	  c_action->entry = ROTATEZ;
	  c_token++;
	  if(equals(c_token,":")||equals(c_token,",")) c_token++;
	  c_action = add_obj_action(); count++;
	  c_action->entry = (float) real(const_express(&a));
	  if(equals(c_token,";")||equals(c_token,",")) c_token++;
	}
      else if(is_graph(c_token) ||
	      ( equals(c_token,"graph") && is_graph(c_token+1)))
	{
	  if(equals(c_token,"graph")) c_token++;
	  mat_defined = 0;
	  make_graph();
	  if(equals(c_token,";") || equals(c_token,",")) c_token++;

  	  c_action = add_obj_action(); count++;
	  c_action->entry = GRAPH;
  	  c_action = add_obj_action(); count++;
	  c_action->entry = (float) (c_graph->index);
	}
      else if( is_object(c_token) || ( equals(c_token,"object") &&
	      is_object(c_token+1)))
	int_error("Nested object definition not allowed",c_token+1);
      else if(isletter(c_token))
	{
	  register int is_a_graph = 0;
	  register int is_one_object = 0;
	  i = 0;

	  if(equals(c_token,"graph"))
	    {
	      c_token++;
	      is_a_graph = 1;
	      is_one_object = 0;
	      i = look_for_graph_index();
	      if(!i) int_error("Undefined graph", c_token);
	    }
	  else  if(equals(c_token,"object"))
	    {
	      c_token++;
	      is_a_graph = 0;
	      is_one_object = 1;
	      i = look_for_object_index();
	      if(!i) int_error("Undefined object", c_token);
	    }
	  else if( (i = look_for_graph_index()))
	    {
	      is_a_graph = 1;
	      is_one_object = 0;   
	    }
	  else if( (i = look_for_object_index()))
	    {
	      is_a_graph = 0;
	      is_one_object = 1;   
	    }
	  else
	    int_error("Undefined 'graph' or 'object'",c_token);
	  
	  if(is_a_graph) 
	    {
	      mat_defined = 0;
	      c_action = add_obj_action(); count++;
	      c_action->entry = GRAPH;
	      c_action = add_obj_action(); count++;
	      c_action->entry = (float) i;
	    }
	  else if(is_one_object)
	    {
	      register struct an_object *temp_object1;
	      c_action = add_obj_action(); count++;
	      c_action->entry = COBJECT;
	      c_action = add_obj_action(); count++;
	      c_action->entry = (float) i;  
	      temp_object1 = look_for_object_by_index(i);
	      if(temp_object1)
		{
		  if(mat_defined)
		    if(look_for_mat_in_obj(temp_object1))
		      { 
			(void)fprintf(STDERRR,"\t(Material was defined in %s",
				       temp_object1->name);
			(void)fprintf(STDERRR,", new one will be ignored)\n");
			*temp_c= '\0';
			{
			  register struct an_action *temp_action1;
			  temp_action1 = temp_action->next;
			  temp_action->next =
			    ((temp_action->next)->next)->next;
			  (void)free( (char *) temp_action1->next);
			  (void)free( (char *) temp_action1);
			  count -= 2;
			}
			temp_c = (char *)NULL;
			temp_action = (struct an_action *) NULL;
		      }			
		  mat_defined = 0;
		  temp_line[ (strlen(temp_line)-1)] = '\0';
		  (void)strcat(temp_line,"-BGN-");
		  (void)strcat(temp_line,temp_object1->name);
		  (void)strcat(temp_line,">");
		  (void)strcat(temp_line,((temp_object1->menu_string)+10));
		  temp_line[ (strlen(temp_line)-1)] = '\0';
		  (void)strcat(temp_line,"<");		  
		  (void)strcat(temp_line, temp_object1->name);
		  (void)strcat(temp_line,"-END-");
		  (void)strcat(temp_line,"|");
		}
	    }
	  c_token++;
	  if(equals(c_token,";")||equals(c_token,",")) c_token++;
	}
      else
	int_error("Unknown object attribute",c_token);
    }

  if(mat_defined) 
    int_error("Multi-materials for some parts, object will not be defined",
	      c_token);
  if(pushed) 
    int_error("Unbalanced push(pop)matrix,object will not be defined",
	      c_token);
  if(transp) 
    int_error("Transparancy has to be ended,object will not be defined",
	      c_token);

  m_copy(&(c_object->menu_string), temp_line);
  (c_object->actions)->entry = (float)(count);
  if(equals(c_token,"}")) c_token++;
  c_object->defined = 1;
  c_object = (struct an_object *)NULL;
}

/**************************************************************/
int look_for_mat_index(t_num)
     int t_num;
{
  register struct a_material **temp = &material_list;

  while( *temp)
    {
      if( equals(t_num,(*temp)->name))
	{
	  if( (*temp)->defined)
	    return((*temp)->index);
	  else
	    return (0);
	}
      temp = &(*temp)->next;
    }
  return(0);
}
/******************************************************/
struct a_material *
  look_for_mat_by_index(ii)
int ii;
{
  register struct a_material **temp = &material_list;

  while( *temp)
    {
      if( (*temp)->index == ii)
	{
	  if( (*temp)->defined)
	    return(*temp);
	  else
	    return( (struct a_material *)NULL);
	}
      temp = &(*temp)->next;
    }
  return( (struct a_material *)NULL);
}
/************************************************************/
struct a_graph *look_for_graph_by_index(ii)
int ii;
{
  register struct a_graph **temp = &graph_list;

  while( *temp)
    {
      if( (*temp)->index == ii)
	{
	  if( (*temp)->defined)
	    return(*temp);
	  else 
	    return( (struct a_graph *)NULL);
	}
      temp = &(*temp)->next;
    }
  return( (struct a_graph *)NULL);
}
/************************************************************/
struct an_object *
  look_for_object_by_index(ii)
int ii;
{
  register struct an_object **temp = &object_list;

  while( *temp)
    {
      if( (*temp)->index == ii)
	{
	  if( (*temp)->defined)
	    return(*temp);
	  else 
	    return( (struct an_object *)NULL);
	}
      temp = &(*temp)->next;
    }
  return( (struct an_object *)NULL);
}
/************************************************************/

look_for_graph_index()
{
  register struct a_graph **temp = &graph_list;
  
  while(*temp)
    {
      if(equals(c_token,(*temp)->name))
	{
	  if( (*temp)->defined)
	    return((*temp)->index);
	  else
	    return(0);
	}
      temp = &(*temp)->next;
    }
  return(0);
}
/**********************************************************/
look_for_object_index()
{
  register struct an_object **temp = &object_list;
  
  while(*temp)
    {
      if(equals(c_token,(*temp)->name))
	{
	  if( (*temp)->defined)
	    return((*temp)->index);
	  else
	    return(0);
	}
      temp = &(*temp)->next;
    }
  return(0);
}
/**********************************************************/
struct an_object *
  look_for_object_name(t_num)
int t_num;
{
  register struct an_object **temp = &(object_list);
  
  while(*temp)
    {
      if( equals(t_num, (*temp)->name))
	{
	  if( (*temp)->defined)
	    return( (*temp));
	  else
	    return( (struct an_object *) NULL);
	}
      temp = &( (*temp)->next);
    }
  return( (struct an_object *) NULL);
}

/**********************************************************/

make_dummy_obj()
{
  register struct dummy_obj **temp = &dummy_obj_list;
  struct   an_action *c_action;

  while(*temp)
    temp = &( (*temp)->next);

  (*temp) = (struct dummy_obj *) malloc( (unsigned int)
					sizeof(struct dummy_obj));
  (*temp)->next = (struct dummy_obj *)NULL;

  temp_line[0] = '\0';
  c_object = add_object((c_graph->name));
  (*temp)->obj = c_object;
  m_copy( &( c_object->name), (c_graph)->name);
  (void)strcat(temp_line,"default|");
  c_action = add_obj_action();
  c_action = add_obj_action();
  c_action->entry = BMATERIAL;
  c_action = add_obj_action();
  c_action->entry = DEFAULT_MATERIAL;
  c_action = add_obj_action();
  c_action->entry = GRAPH;
  c_action = add_obj_action();
  c_action->entry = (float) ( (c_graph)->index);
  ( c_object->actions)->entry = 5.0;
  c_object->defined = 1;
  m_copy(&(c_object->menu_string),temp_line);
}
/********************************************************************/
make_default_material()
{
  c_token = 0;
  c_material = add_material();
  c_material->index = DEFAULT_MATERIAL;
  c_material->defined = 1;
  m_copy( &(c_material->name),"default"); 
  *(c_material->token    ) = 0.05;
  *(c_material->token + 1) = 0.05;
  *(c_material->token + 2) = 0.05;
  *(c_material->token + 3) = 0.2;
  *(c_material->token + 4) = 0.2;
  *(c_material->token + 5) = 0.2;
  *(c_material->token + 6) = 0.7;
  *(c_material->token + 7) = 0.7;
  *(c_material->token + 8) = 0.7;
  *(c_material->token + 9) = 0.0;
  *(c_material->token + 10) = 0.0;
  *(c_material->token + 11) = 0.0;
  *(c_material->token + 12) = 3.0;
  *(c_material->token + 13) = 0.5;
}
/***************************************************************/
define()
{
  register int  start_token,temp_token,n_args;
  register struct udvt_entry *udv;
  register struct udft_entry *udf;

  if (equals(c_token+1,"("))    /* function ! */
    { 
      start_token = c_token;
      temp_token  = c_token;
      copy_str(c_dummy_var, c_token + 2);
      if(equals(temp_token+3,")"))
	{
	  c_token += 5;                /* skip (, dummy, ) and = */
	  check_func_def_error();
	  n_args = 1;
	}
      else if(equals(temp_token+5,")")) 
	{    
	  copy_str(c_dummy_var1, c_token + 4);
	  c_token += 7;                /* skip ( dummy, dummy1 ) and = */
	  check_func_def_error();
	  n_args = 2;
	}
      else if(equals(temp_token+7,")")) 
	{    
	  copy_str(c_dummy_var1, c_token + 4);
	  copy_str(c_dummy_var2, c_token + 6);
	  c_token += 9;     /* skip ( dummy, dummy1,dummy2 ) and = */
	  check_func_def_error();
	  n_args = 3;
	}
      else if(equals(temp_token+9,")")) 
	{    
	  copy_str(c_dummy_var1, c_token + 4);
	  copy_str(c_dummy_var2, c_token + 6);
	  copy_str(c_dummy_var3, c_token + 8);
	  c_token += 11;       /* skip ( dummy,dummy1,dummy2,dummy3) and = */
	  check_func_def_error();
	  n_args = 4;
	}
      udf = dummy_func = add_udf(start_token);
      if (udf->at)	        /* already a dynamic a.t. there */
	free((char *)udf->at);	/* so free it first */
      if (!(udf->at = perm_at()))
	int_error("not enough memory for function",start_token);
      udf->n_arg = n_args;
      m_capture(&(udf->definition),start_token,c_token-1);
    }
  else             /* variable ! */
    {  
      start_token = c_token;
      c_token +=2;
      udv = add_udv(start_token);
      (void) const_express(&(udv->udv_value));
      udv->udv_undef = FALSE;
    }
}

check_func_def_error()
{
  if (END_OF_COMMAND)
    int_error("function definition expected",c_token);
}
/*************************************************************/

struct an_action *r_actions = (struct an_action *)NULL;

struct an_action *add_r_action()
{
  register struct an_action **temp = &(r_actions);
  
  while(*temp)
    temp = &( (*temp)->next);
  (*temp) = (struct an_action *)malloc((unsigned) sizeof(struct an_action));
  (*temp)->entry = 0.0;
  (*temp)->next = (struct an_action *)NULL;
  return( (*temp));
}
/*************************************************************/
struct an_action *search_for_object()
{
  register struct an_action **temp = &(r_actions);
  
  while( *temp)
    {
      if( (*temp)->entry == (float) COBJECT)
	return( (*temp));
      temp = &( (*temp)->next);
    }
  return( (struct an_action *)NULL);
}
/************************************************************/
expand(ac2)
     struct an_action *ac2;
{
  register int count;
  struct an_action *c_action,*t_ac1,*g_ac,*temp1,*o_ac;
  struct an_object *temp_obj;

  (void)add_r_action(); 
  t_ac1 = ac2;

  while(t_ac1)
    {
      c_action = add_r_action();
      c_action->entry = t_ac1->entry;
      t_ac1 = t_ac1->next;
    }

  while( (g_ac = search_for_object()) )
    {
      temp1 = (g_ac->next)->next;
      temp_obj = look_for_object_by_index( (int) (( g_ac->next)->entry));
      g_ac->next = (struct an_action *)NULL;
      o_ac = ((temp_obj->actions)->next);
    
      g_ac->entry = o_ac->entry;
      o_ac = o_ac->next;
    
      {   
	/* 
	 * strip out those pre_operations, they are necessary
	 * the 1sr few tokens
	 */
	register int i;
	for (i = 0; i < 3; i++)
	  if( (o_ac->entry == (float)PRETRANSLATION) ||
	     (o_ac->entry == (float)PREROTATION) ||
	     (o_ac->entry == (float)PRESCALING))
	    {
	      o_ac = (o_ac->next->next->next->next);
	      if(!o_ac) 
		int_error( "Internal Error, Sorry",-1);
	    }
      }
      while(o_ac)
	{
	  c_action = add_r_action();
	  c_action->entry = o_ac->entry;
	  o_ac = o_ac->next;
	}
      c_action->next = temp1;
    }
  temp1 = r_actions;
  count = 0;
  while(temp1)
    {
      count++;
      temp1 = temp1->next;
    }
  r_actions->entry = (float) count;
}
/****************************************************************/
free_actions()
{
  struct an_action *temp,*temp1;
  temp = r_actions;
  while(temp)
    {
      temp1=temp->next;
      (void)free( (char *)temp);
      temp = temp1;
    }
  r_actions = (struct an_action *)NULL;
}
/*******************************************************/
look_for_mat_in_obj(temp_obj)
     struct an_object *temp_obj;
{
  struct an_action *temp;
  struct an_object *temp_obj1;
  
  temp = temp_obj->actions;
  while(temp &&(temp->entry != (float)GRAPH))
    {
      if(temp->entry == (float)BMATERIAL)
	return(1);
      else if((temp->entry == (float)COBJECT))
	{
	  temp_obj1 = 
	    look_for_object_by_index((int)( (temp->next)->entry));
	  if(look_for_mat_in_obj(temp_obj1))
	    return(1);
	  else 
	    return (0);
	}
      temp = temp->next;
    }
  return(0);
}
/*****************************************************/
int
  look_for_lmodel_index(t_num)
int t_num;
{
  register struct lmodel **temp = &(lmodel_list);

  while( (*temp))
    {
      if(equals(t_num, (*temp)->name))
	return( (*temp)->index);
      temp = &( (*temp)->next);
    }
  return(0);
}
    
int
  look_for_light_index(t_num)
int t_num;
{
  register struct light **temp = &(light_list);

  while( (*temp))
    {
      if(equals(t_num, (*temp)->name))
	return( (*temp)->index);
      temp = &( (*temp)->next);
    }
  return(0);
}
/******************************************************************/
