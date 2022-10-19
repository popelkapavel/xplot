/************************************************************************
 * 
 *  Code is modified from the code for Gnuplot by Zou Maorong
 */
/*
 *  G N U P L O T  --  misc.c
 *  Copyright (C) 1986, 1987  Thomas Williams, Colin Kelley
 *  You may use this code as you wish if credit is given and this message
 *  is retained.
 */

/****************************************************************************/

#include <stdio.h>
#include "plot.h"

extern char       dummy_var[],dummy_var1[],dummy_var2[],dummy_var3[];
extern int        samples,samples1;
extern double     big_size;
extern int        c_token;
extern struct     at_type at;
extern struct     ft_entry ft[];
extern struct     udft_entry *first_udf;
extern struct     udvt_entry *first_udv;
extern int        built_in_func;
extern int        built_in_vari;

int               strcmp(),strncmp();
char              *malloc(),*strcat(),*strcpy();
struct at_type    *temp_at();


save_functions(fp)
     FILE *fp;
{

  register struct udft_entry *udf = first_udf;
  register int i;

  i = 0;
  while(i < built_in_func)
    {
      udf = udf->next_udf;
      i++;
    }
	
  if (fp) 
    {
      while (udf) 
	{
	  if (udf->definition)
	    (void)fprintf(fp,"\t%s;\n",udf->definition);
	      udf = udf->next_udf;
	}
    }
}


save_variables(fp)
     FILE *fp;
{
  extern int built_in_vari;
  register struct udvt_entry *udv = first_udv;
  register int i;

  i = 0;
  while(i<built_in_vari)
    {
      udv = udv->next_udv;
      i++;
    }

  if (fp) 
    {
      while (udv) 
	{
	  if (!udv->udv_undef) 
	    {
	      (void)fprintf(fp,"\t%s = ",udv->udv_name);
	      disp_value(fp,&(udv->udv_value));
	      (void) putcc(';',fp);
	      (void) putcc('\n',fp);
	    }
	  udv = udv->next_udv;
	}
    }
}

/***************************************************************/

#define YES_IT_IS  123
#define NO_IT_IS_NOT  124

int no_echo = 0;    
extern int number_of_inputs,save_current_line;
extern struct a_record  *current_record;
load_file(fp)
     FILE *fp;
{
  extern   char    input_line[];
  extern   char    temp_line[];
  int              s_b,c_b,r_b, kk;
  register int     len,c_index,from_saved;
  register int     jj,len1,ii,last_one;
  char             *temp;

  if (!fp)
    os_error("Open file failed",c_token);

  from_saved = 0;
  c_b = 0; s_b = 0; r_b = 0;

  while (fgets(input_line,MAX_LINE_LEN,fp)) 
    {
      /*
       * get the line, check if it is a comment.
       */
      if(!from_saved)
	if(!strncmp(input_line,"#!echo",6))
	  {
	    no_echo = 1;
	    save_current_line = 0;
	    if(current_record)
	      current_record->erase = 0;
	    continue;
	  }
      if (is_comment(input_line))
	continue;
      else
	{
	  len = strlen(input_line) + 1;
	  input_line[len-2] = ' ';      
	  input_line[len-1] = '\0';      
	}
      
      /*
       * This is the place that was causing trouble,if from_saved ==1
       * the file being load is a Irisplot Record file, In this case,
       * we have to strip out the In[#] in front of each line, Also,
       * input will have no continuation lines, so don't have to check
       * if the following line should be part of the current input.
       * If from_saved == -1, it means the loaded file is not a Record
       * file. If from_saved == 0; if was not checked yet, so check it.
       * Notice also, user might edit the Record file, i.e. add some 
       * comments, add a \t before some line ...
       */

      if(!from_saved) 
	{
	  ii = 0;
	  while(((input_line[ii] == ' ') || (input_line[ii] == '\t')) &&
		ii < len)
	    ii++;
	  /*
	   * This is to say, after strip out the ' ' and \t, if the first
	   * valid input consists of "In[", the program will assumw that
	   * the file being loaded is a Irisplot Record file. The user
	   * is advised to edit the record file in certain format.
	   */
	  if( !strncmp( (input_line+ii),"In[",3) )
	    from_saved = YES_IT_IS;
	  else
	    from_saved = NO_IT_IS_NOT;
	}
      
      if(from_saved == YES_IT_IS)
	{
	  c_index = 0;
	  while((c_index < len) && (input_line[c_index] != ':' ))
	    input_line[c_index++] = ' ';
	  input_line[c_index++] = ' ';	  
	}
      else if( from_saved == NO_IT_IS_NOT)
	{
	  /*  
	   * This is an expensive check on delims. If paren, bracket,
	   * are not matched, it is assumed that there is a continuation
	   * line. If a '\\' is found, There is definitely a continuation
	   * line. If a # is found and the paren... are not matched, assume
	   * there is a continuation line. if paren.. are matched, assume
	   * this is the end of the line.
	   */
	  ii = 0;
	  while(((input_line[ii]==' ')||(input_line[ii]=='\t')) && ii < len)
	    ii++;
	  for(; ii < len; ii++)
	    {
	      kk = 0;
	      check_input( input_line[ii], &c_b, &s_b, &r_b,&kk);
	      if(kk || ( (input_line[ii] == '#') && ( c_b || s_b ||r_b)))
		{
		  kk = 1;
		  input_line[ii++] = ' '; 
		  input_line[ii] = '\0'; 
		  break;
		}
	      else if((input_line[ii] == '#') && ( !c_b && !s_b && !r_b))
		{
		  input_line[ii++] = ';'; 
		  input_line[ii] = '\0'; 	  
		  kk = 0; jj = 1;
		  break;
		}
	    }
	  
	  /*
	   * jj = 0 means there is a continuation line, happens when
	   * the above situation is satisfied.
	   */
	  jj = 1;
	  if(kk || c_b || s_b || r_b) jj = 0;
	  kk =0;
	  while( !jj)
	    {
	      kk = 0; /* assume there is no continuation line followed */
	      if(! fgets(temp_line,MAX_LINE_LEN,fp))
		{
		  int_error("Unexpected EOF in the file being loaded",-1);
		  break;
		}
	      if (is_comment(temp_line))      /* skip comment line */
		continue;
	      len1 = strlen(temp_line)+1 ;
	      temp_line[len1-2] = ' ';
	      temp_line[len1-1] = '\0';
	      ii = 0;
	      while(((temp_line[ii]==' ')||(temp_line[ii]=='\t')) && ii<len1)
		ii++;
	      for(; ii < len1; ii++)
		{
		  kk = 0;
		  check_input( temp_line[ii], &c_b, &s_b, &r_b,&kk);
		  if(kk || ((temp_line[ii] == '#') && (c_b || s_b || r_b)))
		    {
		      kk = 1;
		      temp_line[ii++] = ' ';
		      temp_line[ii] = '\0';
		      break;
		    }
		  else if((temp_line[ii] == '#') && (!c_b && !s_b && !r_b))
		    {
		      temp_line[ii++] = ';';
		      temp_line[ii] = '\0';
		      jj= 1; kk = 0;
		      break;
		    }
		}
	      if(kk || s_b || c_b || r_b)
		{ 
		  jj = 0;   /* must be another continuation line */
		  kk = 0;
		}
	      else  
		jj = 1;
	      last_one = 0;
	      while(temp_line[last_one] == ' '|| temp_line[last_one] == '\t')
		last_one++;
	      (void) strcat(input_line,temp_line+last_one);
	    }
	}   /* match if(frome_saved == NO_IT_IS_NOT) */
      /*
       * Now, input_line are set, just process it. 
       */
      {
	/* first strip out the white space in front of the input */
	len = strlen(input_line);
	c_index = 0;
	while( (c_index < len) &&  (input_line[c_index] == ' '
				    || input_line[c_index] == '\t'))
	  c_index++;
	if(c_index >= len)
	  input_line[0] = '\0';
	else
	  {
	    temp = input_line + c_index;
	    (void)strcpy(temp_line,temp);
	    (void)strcpy(input_line,temp_line);
	  }
	temp = input_line; ii = 0;
	if(!no_echo)
	  {
	    (void)fprintf(STDERRR,"In[%d] : ",number_of_inputs);
	    while((ii < 50) && (*temp))
	      {
		(void) putcc((*temp),STDERRR);
		ii++; temp++;
	      }
	    (void) fprintf(STDERRR," ... ... \n");
	  }
	do_line();
      }
    }
  (void) fclose(fp);
  no_echo = 0;                /* turn on command echo and record saving */
  save_current_line = 1;
}
/*******************************************************/
check_input(c,lc,ls,lr,k)
     char c;
     int *ls,*lc,*lr,*k;
{
  if( c == '{')     (*lc)++;
  else if(c == '[') (*ls)++;
  else if(c == '(') (*lr)++;
  else if(c == '}') (*lc)--;
  else if(c == ']') (*ls)--;
  else if(c == ')') (*lr)--;
  else if(c == '\\') (*k) = 1;
}
/**************************************************/
show_variables()
{
  register struct udvt_entry *udv = first_udv;
  register int i;
  
  i = 0;
  while(i < built_in_vari)
    {
      udv = udv->next_udv;
      i++;
    }
  if(udv)
    (void) putcc('\n',STDERRR);
  while(udv) 
    {
      (void)fprintf(STDERRR,"\t%s ",udv->udv_name);
      if (udv->udv_undef)
	(void) fputs("is undefined\n",STDERRR);
      else {
	(void) fputs("= ",STDERRR);
	disp_value(STDERRR,&(udv->udv_value));
	(void) putcc(';',STDERRR);
	(void) putcc('\n',STDERRR);
      }
      udv = udv->next_udv;
    }
}


show_functions()
{
  register struct udft_entry *udf = first_udf;
  register int i;

  i = 0;
  while(i < built_in_func)
    {
      udf = udf->next_udf;
      i++;
    }

  if(udf)
    (void) putcc('\n',STDERRR);  
  while (udf)
    {
      if (udf->definition)
	(void)fprintf(STDERRR,"\t%s;\n",udf->definition);
      else
	(void)fprintf(STDERRR,"\t   %s is undefined\n",udf->udf_name);
      udf = udf->next_udf;
    }
}


show_at()
{
  (void) putcc('\n',STDERRR);
  disp_at(temp_at(),0,(char *)0);
}


disp_at(curr_at, level,func)
     struct at_type *curr_at;
     int level;
     char *func;
{
  register int             i, j;
  register union argument  *arg;
  char                     *func_name = NULL;

  for (i = 0; i < curr_at->a_count; i++) 
    {
      (void) putcc('\t',STDERRR);
      for (j = 0; j < level; j++)
	(void) putcc(' ',STDERRR);	/* indent */

      /* print name of instruction */
      (void) fputs(ft[(int)(curr_at->actions[i].index)].f_name,STDERRR);
      arg = &(curr_at->actions[i].arg);

      /* now print optional argument */
      switch(curr_at->actions[i].index) 
	{
	case PUSH:  
	  (void)fprintf(STDERRR," %s\n", arg->udv_arg->udv_name);
	  break;
	case PUSHC:
	  (void) putcc(' ',STDERRR);
	  disp_value(STDERRR,&(arg->v_arg));
	  (void) putcc('\n',STDERRR);
	  break;
    case PUSHD:	
	  (void)fprintf(STDERRR," %s dummy0\n",
			arg->udf_arg->udf_name);
	  break;
	case PUSHD1:
	  (void)fprintf(STDERRR," %s dummy1\n",
			arg->udf_arg->udf_name);
	  break;
    case PUSHD2:
	  (void)fprintf(STDERRR," %s dummy2\n",
			arg->udf_arg->udf_name);
	  break;
    case PUSHD3:
	  (void)fprintf(STDERRR," %s dummy3\n",
			arg->udf_arg->udf_name);
	  break;
	case CALL:	
	  {
	    if(func && !strcmp(arg->udf_arg->udf_name,func))
	      {
		(void)fprintf(STDERRR," %s  %s\n", "RECURSIVE:",
			      arg->udf_arg->udf_name);
		break;
	      }
	    (void)fprintf(STDERRR," %s", arg->udf_arg->udf_name);
	    func_name = arg->udf_arg->udf_name;
	    if (arg->udf_arg->at) 
	      {
		(void) putcc('\n',STDERRR);
		disp_at(arg->udf_arg->at,level+2,func_name); /* recurse! */
	      } 
	    else
	      (void) fputs(" (undefined)\n",STDERRR);
	  }
	  break;
	case JUMP:
	case JUMPZ:
	case JUMPNZ:
	case JTERN:
	  (void)fprintf(STDERRR," +%d\n",arg->j_arg);
	  break;
	default:
	  (void) putcc('\n',STDERRR );
	}
    }
}


/****************************************************************************/


