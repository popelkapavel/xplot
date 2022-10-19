/************************************************************************
 * 
 *     I R I S P L O T                  --------new5.c
 *
 *    Copyright 1989 Zou Maorong
 *
 ************************************************************************/

#include <stdio.h>
#include "plot.h"

extern struct lexical_unit  token[MAX_TOKENS];
extern int num_tokens,c_token;
extern FILE *attrfile;

struct a_string  *short_hands = (struct a_string *)NULL;
struct a_string  *add_a_string();
struct a_string  *look_for_a_short_hand();
static struct final final_structure;

char *malloc(),*strcpy();
/***************************************************/

write_final_structure()
{
  (void)fprintf(attrfile, "%d   %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
		FINALSTRUCTURE,final_structure.light_off,
		final_structure.zbuffer_off,final_structure.depthcue_on,
		final_structure.box_on,final_structure.slice_on,
		final_structure.double_buffer_off,
		final_structure.save_image,
		final_structure.bulb_flag,
		final_structure.coor_flag,
		final_structure.big_box,
		final_structure.base_box,
		final_structure.wire,
		final_structure.normal);
}
/***************************************************/
/*
 * define a short hand notation
 */
extern struct     udvt_entry *first_udv;

struct a_string *try_to_find_an_udv()
{
  /*
   * extern int built_in_vari;
   * register int i; 
   */
  register struct udvt_entry *udv = first_udv;

  struct a_string  *temp;
  char temp_char1[MAX_ID_LEN];

/*
 * i = 0;
 * while(i<built_in_vari)
 * {
 *    udv = udv->next_udv;
 *      i++;
 *   } 
 */

  while( udv)
    {
      if(equals(c_token,udv->udv_name))
	break;
      udv = udv->next_udv;
    }
  if(!udv)
    return( (struct a_string *)NULL);

  temp = add_a_string();
  m_capture( &(temp->name),c_token,c_token);  
  if( udv->udv_value.type == INT)
    (void)sprintf(temp_char1,"%d",udv->udv_value.v.int_val);
  else
    (void)sprintf(temp_char1,"%.16f",(udv->udv_value.v.cmplx_val.real));
  temp->contents = (char *)malloc((unsigned)(strlen(temp_char1)+2));
  (void)strcpy(temp->contents,temp_char1);
  return( temp);
}


int is_short_hand(t_num)
     int t_num;
{
  if(isletter(t_num) && equals(t_num+1,"=") && isstring(t_num+2))
    return(2);
  return(0);
}

struct a_string *add_a_string()
{
  register struct a_string  **temp = &(short_hands);  
  
  while( (*temp))
    {
      if( equals(c_token, (*temp)->name))
	{
	  (void)free((char *) (*temp)->name);
	  if( (*temp)->contents)
	    (void)free((char *) (*temp)->contents);
	  (*temp)->name = (char *)NULL;
	  (*temp)->contents = (char *)NULL;
	  return( (*temp));
	}
      temp = &( (*temp)->next);
    }
  (*temp) = (struct a_string *)malloc((unsigned) sizeof(struct a_string));
  (*temp)->next = (struct a_string *)NULL;
  (*temp)->name = (char *)NULL;
  (*temp)->contents = (char *)NULL;
  return( *temp);
}

define_short_hands()
{
  struct a_string  *temp;

  temp = add_a_string();
  m_capture( &(temp->name),c_token,c_token);
  c_token += 2;
  temp->contents = (char *)malloc((unsigned)(token[c_token].length));
  quote_str1((temp->contents),c_token);
  c_token++;
}

struct a_string *look_for_a_short_hand()
{
  register struct a_string  **temp = &(short_hands);  
  
  while( (*temp))
    {
      if( equals(c_token, (*temp)->name))
	return( (*temp));
      temp = &( (*temp)->next);
    }
  return ( (struct a_string *)NULL);
}

int look_for_dollar_sign()
{
  register int i = num_tokens - 1;
  
  while( i >= 0 )
    {
      if(equals(i,"$")) return(i);
      i--;
    }
  return (-1);
}

/******************************************************/

command3()
{
  command4();
}

/**********************************************/

off_double_buffer()
{
  final_structure.double_buffer_off = 1;
}  
off_lights()
{
  final_structure.light_off = 1;
}
off_zbuffer()
{
  final_structure.zbuffer_off = 1;
}
on_depthcue()
{
  final_structure.depthcue_on = 1;
}
on_box()
{
  final_structure.box_on = 1;
}
on_slice()
{
  final_structure.slice_on = 1;
}
image_save(i)
     int i;
{
  final_structure.save_image = i;
}
on_bulb()
{
  final_structure.bulb_flag = 1;
}
on_coor()
{
  final_structure.coor_flag = 1;
}
on_big_box()
{
  final_structure.big_box = 1;
}
on_base()
{
  final_structure.base_box = 1;
}
on_wire()
{
  final_structure.wire = 1;
}
on_normal()
{
  final_structure.normal = 1;
};

reset_final_structure()
{
  final_structure.depthcue_on = 0;  
  final_structure.box_on = 0;
  final_structure.slice_on = 0;
  final_structure.light_off = 0;
  final_structure.zbuffer_off = 0;
  final_structure.save_image = 0;
  final_structure.double_buffer_off = 0;
  final_structure.bulb_flag = 0;
  final_structure.coor_flag = 0;  
  final_structure.big_box = 0;
  final_structure.base_box = 0;
  final_structure.wire = 0;
  final_structure.normal = 0;
}
/***************************************************/







