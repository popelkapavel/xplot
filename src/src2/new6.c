
/************************************************************************
 * 
 *     I R I S P L O T                  --------new6.c
 *
 *    Copyright 1989 Zou Maorong
 *
 ************************************************************************/

#include <stdio.h>
#include "plot.h"
#include "flow.h"

extern char input_line[];
extern struct lexical_unit  token[MAX_TOKENS];
extern int num_tokens,c_token,number_of_inputs,no_echo;
extern struct     udvt_entry *first_udv;

int save_current_line = 1;

char *strcpy(), *strcat();
int  strlen(),strcmp();

/************************************************************/

int is_if_cond(t_num)
     int t_num;
{
  if( equals(t_num,"if") && equals(t_num+1,"("))
    return (2);
  return(0);
}

int is_for_loop(t_num)
     int t_num;
{
  if( equals(t_num,"for") && equals(t_num+1,"("))
    return (2);
  return(0);
}

int is_while_loop(t_num)
     int t_num;
{
  if( equals(t_num,"while") && equals(t_num+1,"("))
    return (2);
  return(0);
}

int is_do_loop(t_num)
     int t_num;
{
  if( equals(t_num,"do") && equals(t_num+1,"{"))
    return (2);
  return(0);
}
/************************************************************/
do_if_cond()
{
  create_a_if_cond();
  if(!no_echo)  
    save_current_line = 0;
  execute_if_cond();
  if(!no_echo)  
    save_current_line = 1;
}

do_while_loop()
{
  create_a_while_loop();
  if(!no_echo)  
    save_current_line = 0;
  execute_while_loop();
  if(!no_echo)  
    save_current_line = 1;
}

do_do_loop()
{
  create_a_do_loop();
  if(!no_echo)  
    save_current_line = 0;
  execute_while_loop();
  if(!no_echo)  
    save_current_line = 1;
}

do_for_loop()
{
  create_a_for_loop();
  if(!no_echo)  
    save_current_line = 0;
  execute_for_loop();
  if(!no_echo)  
    save_current_line = 1;
}
/************************************************************/

create_a_if_cond()
{
  register int temp_token, num_pa;

  c_token += 2; temp_token = c_token;
  num_pa = 1;
  while(num_pa && c_token < num_tokens)
    {
      if(equals(c_token,"(")) num_pa++;
      if(equals(c_token,")")) num_pa--;      
      c_token++;
    }
  if(c_token > num_tokens)
    int_error(" Missing ')'",c_token);
  m_capture(&(ifcond.first), temp_token,c_token-2);
  if(!equals(c_token,"{"))
    int_error(" '{' missing",c_token);
  temp_token = ++c_token;
  num_pa = 1;
  while(num_pa && c_token < num_tokens)
    {
      if(equals(c_token,"{")) num_pa++;
      if(equals(c_token,"}")) num_pa--;      
      c_token++;
    }
  if(c_token > num_tokens)
    int_error(" Missing '}'",c_token);
  m_capture(&(ifcond.body), temp_token,c_token-2);    
}

execute_if_cond()
{
  struct value *aa;
  struct udvt_entry *t_udv = first_udv;
  
  while(t_udv && strcmp(t_udv->udv_name,"IF_COND"))
    t_udv = t_udv->next_udv;
  if(!t_udv)
    int_error(" Sorry, internal error for if stmts",-1);
  aa = &(t_udv->udv_value);
  
  (void)strcpy(input_line,"IF_COND=(");
  (void)strcat(input_line,ifcond.first);
  (void)strcat(input_line," )");
  do_line();
  
  if(( aa->type == INT)? (aa->v).int_val : (aa->v).cmplx_val.real)
    {
      (void) strcpy(input_line,ifcond.body); 
      do_line();
    }
}

/***************************************************************/

create_a_do_loop()
{
  register int temp_token, num_pa;

  c_token += 2;  temp_token = c_token;
  num_pa = 1;
  while(num_pa && c_token < num_tokens)
    {
      if(equals(c_token,"{")) num_pa++;
      if(equals(c_token,"}")) num_pa--;      
      c_token++;
    }
  if(c_token > num_tokens)
    int_error(" Missing '}'",c_token);
  m_capture(&(whileloop.body), temp_token,c_token-2);    
  if(!equals(c_token++,"while"))
    int_error("Missing keyword 'while'",c_token);
  if(!equals(c_token++,"("))  
    int_error("Missing '('",c_token);
  temp_token = c_token;
  num_pa = 1;
  while(num_pa && c_token < num_tokens)
    {
      if(equals(c_token,"(")) num_pa++;
      if(equals(c_token,")")) num_pa--;
      c_token++;
    }
  if(c_token > num_tokens)
    int_error(" Missing ')'",c_token);
  m_capture(&(whileloop.first), temp_token,c_token-2);
}

create_a_while_loop()
{
  register int temp_token, num_pa;

  c_token += 2; temp_token = c_token;
  num_pa = 1;
  while(num_pa && c_token < num_tokens)
    {
      if(equals(c_token,"(")) num_pa++;
      if(equals(c_token,")")) num_pa--;      
      c_token++;
    }
  if(c_token > num_tokens)
    int_error(" Missing ')'",c_token);
  m_capture(&(whileloop.first), temp_token,c_token-2);
  if(!equals(c_token,"{"))
    int_error(" '{' missing",c_token);
  temp_token = ++c_token;
  num_pa = 1;
  while(num_pa && c_token < num_tokens)
    {
      if(equals(c_token,"{")) num_pa++;
      if(equals(c_token,"}")) num_pa--;      
      c_token++;
    }
  if(c_token > num_tokens)
    int_error(" Missing '}'",c_token);
  m_capture(&(whileloop.body), temp_token,c_token-2);    
}

execute_while_loop()
{
  struct value *aa;
  struct udvt_entry *t_udv = first_udv;
  
  while(t_udv && strcmp(t_udv->udv_name,"LOOP_COND"))
    t_udv = t_udv->next_udv;
  if(!t_udv)
    int_error(" Sorry, internal error for while loop",-1);
  aa = &(t_udv->udv_value);
  
  (void)strcpy(input_line,"LOOP_COND=(");
  (void)strcat(input_line,whileloop.first);
  (void)strcat(input_line," )");
  do_line();
  
  while(( aa->type == INT)? (aa->v).int_val : (aa->v).cmplx_val.real)
    {
      (void) strcpy(input_line,whileloop.body); 
      do_line();
      (void)strcpy(input_line,"LOOP_COND=(");
      (void)strcat(input_line,whileloop.first);
      (void)strcat(input_line," )");
      do_line();
    }
}

/****************************************************************/

create_a_for_loop()
{
  register int temp_token, num_pa;

  c_token += 2; temp_token = c_token;
  while(!equals(c_token,";") && c_token < num_tokens) c_token++;
  if(c_token >= num_tokens)
    int_error("incomplete for loop",c_token);
  m_capture(&(forloop.first), temp_token,c_token-1);
  temp_token = ++c_token;
  while(!equals(c_token,";") && c_token < num_tokens) c_token++;
  if(c_token >= num_tokens)
    int_error("incomplete for loop",c_token);
  m_capture(&(forloop.second), temp_token,c_token-1);
  temp_token = ++c_token;
  num_pa = 1;
  while(num_pa && c_token < num_tokens)
    {
      if(equals(c_token,"(")) num_pa++;
      if(equals(c_token,")")) num_pa--;
      c_token++;
    }
  if(c_token > num_tokens)
    int_error("incomplete for loop",c_token);
  m_capture(&(forloop.third), temp_token, c_token - 2);  
  if(!equals(c_token,"{"))
    int_error(" '{' missing",c_token);
  temp_token = ++c_token;
  num_pa = 1;
  while(num_pa && c_token < num_tokens)
    {
      if(equals(c_token,"{")) num_pa++;
      if(equals(c_token,"}")) num_pa--;      
      c_token++;
    }
  if(c_token > num_tokens)
    int_error("incomplete for loop",c_token);
  m_capture(&(forloop.body), temp_token,c_token-2);    
}

execute_for_loop()
{
  struct value *aa;
  struct udvt_entry *t_udv = first_udv;
  
  while(t_udv && strcmp(t_udv->udv_name,"LOOP_COND"))
    t_udv = t_udv->next_udv;
  if(!t_udv)
    int_error(" Sorry, internal error for for loop",-1);
  aa = &(t_udv->udv_value);
  
  (void) strcpy(input_line,forloop.first);
  do_line();
  (void)strcpy(input_line,"LOOP_COND=(");
  (void)strcat(input_line,forloop.second);
  (void)strcat(input_line," )");
  do_line();
  
  while(( aa->type == INT)? (aa->v).int_val : (aa->v).cmplx_val.real)
    {
      (void) strcpy(input_line,forloop.body); 
      do_line();
      (void)strcpy(input_line,forloop.third);
      do_line(); 
      (void)strcpy(input_line,"LOOP_COND=(");
      (void)strcat(input_line,forloop.second);
      (void)strcat(input_line," )");
      do_line();
    }
}
/****************************************************************/
