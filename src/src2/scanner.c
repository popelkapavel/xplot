/************************************************************************
 * 
 *  Code is modified from the code for Gnuplot by Zou Maorong
 */
/*
 *  G N U P L O T  --  scanner.c
 *  Copyright (C) 1986, 1987  Thomas Williams, Colin Kelley
 *  You may use this code as you wish if credit is given and this message
 *  is retained.
 */
/****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include "plot.h"

#define MIN(x,y) ( (x) < (y) ? (x) : (y) )
#define isident(c) (isalnum(c) || (c) == '_')

#ifndef STDOUT
#define STDOUT 1
#endif

#define APPEND_TOKEN      {token[t_num].length++; current++;}
#define SCAN_IDENTIFIER   while (isident(expression[current + 1]))\
  APPEND_TOKEN

/***************************************************************************/

extern struct a_string  *short_hands;
extern int            num_tokens,c_token;
extern char           input_line[MAX_LINE_LEN+1];
extern struct lexical_unit  token[MAX_TOKENS];

struct a_string  *look_for_a_short_hand();
struct a_string  *try_to_find_an_udv();
struct a_record  *records=NULL;
struct a_record  *current_record=NULL;
struct a_record  *add_record();

char             temp_line[MAX_LINE_LEN+1];
int              number_of_inputs=1;
char             *malloc(), *strcat(), *strcpy(), *strncpy();
int              strlen();

static int       t_num;


/***************************************************************************/

scanner(expression)
     char expression[];
{
  register int current;
  register int quote;

  for (current = t_num = 0;
       t_num < MAX_TOKENS && expression[current] != '\0'; current++) 
    {
    again:
      if (isspace(expression[current]))
	continue;	                     /* skip the whitespace */
      token[t_num].start_index = current;
      token[t_num].length = 1;
      token[t_num].is_token = TRUE;	/* to start with...*/
      
      if (expression[current] == '`') 
	{
	  substitute(&expression[current],MAX_LINE_LEN - current);
	  goto again;
	}
      if (isalpha(expression[current])) 
	{
	  SCAN_IDENTIFIER;
	} else if (isdigit(expression[current]) || expression[current] == '.')
	  {
	    token[t_num].is_token = FALSE;
	    token[t_num].length = get_num(&expression[current]);
	    current += (token[t_num].length - 1);
	    /*
	     * The code for complex checking is taken off
	     */
	  }
	else if (expression[current] == '\'' || expression[current] == '\"')
	  {
	    token[t_num].length++;
	    quote = expression[current];
	    while (expression[++current] != quote) 
	      {
		if (!expression[current]) 
		  {
		    expression[current] = quote;
		    expression[current+1] = '\0';
		    break;
		  }
		else
		  token[t_num].length++;
	      }
	  }
	else switch (expression[current]) 
	  {
	  case '+':
	  case '-':
	  case '^':
	  case '/':
	  case '%':
	  case '~':
	  case '{':
	  case '}':
	  case '(':
	  case ')':
	  case '[':
	  case ']':
	  case ';':
	  case ':':
	  case '?':
	  case ',':
          case '#':
	  case '$':
	    break;
	    
	  case '&':
	  case '|':
	  case '=':
	  case '*':
	    if (expression[current] == expression[current + 1])
	      APPEND_TOKEN;
	    break;
	  case '!':
	  case '<':
	  case '>':
	    if (expression[current + 1] == '=')
	      APPEND_TOKEN;
	    break;
	  default:
	    int_error("invalid character",t_num);
	  }
      ++t_num;	/* next token if not white space */
    }
  
  /*
   * Now kludge an extra token which points to '\0' at end of expression[].
   * This is useful so printerror() looks nice even if we've fallen off the
   * line.
   */
  token[t_num].start_index = current;
  token[t_num].length = 0;
  return(t_num);
}

/***************************************************************************/

get_num(str)
     char str[];
{
  double atof();
  register int count = 0;
  long atol();
  register long lval;

  token[t_num].is_token = FALSE;
  token[t_num].l_val.type = INT;           /* assume unless . or E found */
  while (isdigit(str[count]))
    count++;
  if (str[count] == '.')
    {
      token[t_num].l_val.type = CMPLX;
      while (isdigit(str[++count])) /* swallow up digits until non-digit */
	;
                                    /* now str[count] is other than a digit */
    }
  if (str[count] == 'e' || str[count] == 'E') 
    {
      token[t_num].l_val.type = CMPLX;
      if (str[++count] == '-')
	count++;
      if (!isdigit(str[count])) 
	{
	  token[t_num].start_index += count;
	  int_error("expecting exponent",t_num);
	}
      while (isdigit(str[++count]))
	;
    }
  if (token[t_num].l_val.type == INT) 
    {
      lval = atol(str);
      if ((token[t_num].l_val.v.int_val = lval) != lval)
	int_error("integer overflow; change to floating point",t_num);
    }
  else 
    {
      token[t_num].l_val.v.cmplx_val.imag = 0.0;
      token[t_num].l_val.v.cmplx_val.real = atof(str);
    }
  return(count);
}

/***************************************************************************/

substitute(str,max)			/* substitute output from ` ` */
     char *str;
     int max;
{
  register char *last;
  register int i,c;
  register FILE *f;
  FILE *popen();
  static char pgm[MAX_LINE_LEN+1],output[MAX_LINE_LEN+1];

  i = 0;
  last = str;
  while (*(++last) != '`') 
    {
      if (*last == '\0')
	int_error("unmatched `",t_num);
      pgm[i++] = *last;
    }
  pgm[i] = '\0';		/* end with null */
  max -= strlen(last);          /* max is now the max length of output sub. */
  
  if ((f = popen(pgm,"r")) == NULL)
    os_error("popen failed",NO_CARET);
  
  i = 0;
  while ((c = getc(f)) != EOF) 
    {
      output[i++] = ((c == '\n') ? ' ' : c);	/* newlines become blanks*/
      if (i == max) {
	(void) pclose(f);
	int_error("substitution overflow", t_num);
      }
    }
  (void) pclose(f);
  if (i + strlen(last) > max)
    int_error("substitution overflowed rest of line", t_num);
  (void) strncpy(output+i,last+1,MAX_LINE_LEN-i);
                                          /* tack on rest of line to output */
  (void) strcpy(str,output);	          /* now replace ` ` with output */
}
/****************************************************************************/

int look_for_dollar_sign();

replace_short_hands()
{
  register int i,j;
  struct a_string *temp;

  num_tokens = scanner(input_line);
  while ((i = look_for_dollar_sign()) != -1)
    {
      c_token = i+1; j = 2;
      if(END_OF_COMMAND)  int_error("Unexpected EOF",c_token);
      if( equals(c_token,"(")) { j = 4; c_token++;}
      temp = try_to_find_an_udv();
      if(!temp)
	{
	  if(!(temp = look_for_a_short_hand()))
	    int_error("Undefined macro",c_token);
	}
      (void) strcpy(temp_line,input_line);
      temp_line[token[i].start_index] = '\0';
      (void)strcat(temp_line, temp->contents);
      if(input_line[token[i+j].start_index-1] == ' ' ||
	 input_line[token[i+j].start_index-1] == '\t' )
	(void)strcat(temp_line, " ");
      (void)strcat(temp_line, (input_line + token[i+j].start_index));
      (void)strcpy(input_line,temp_line);
      num_tokens = scanner(input_line);
      c_token = 0; 
    }
}

/******************************************************************/

extern int save_current_line,no_echo;

reset_input_line()
{
  register int jk;
  /*
   * register int reprint;
   */

  num_tokens = scanner(input_line);
  if( !equals(0,"for") && !equals(0,"do") && !equals(0,"while")
     && !equals(0,"if"))
    {
      replace_short_hands();
      num_tokens = scanner(input_line);
    }
  jk = search_for_replace();
  /*
   * if( jk != -1) reprint = 1;
   * else reprint = 0;
   */
  (void) strcpy(temp_line,input_line);
  while( jk != -1) 
    {
      replace(jk);
      jk = search_for_replace();
    }
  
  if(strlen(input_line) && num_tokens && save_current_line) 
     {
       save_line();     /* swallow up those bad inputs */
       if(equals(0,"help") ||
	  equals(0,"?")    ||
	  almost_equals(0,"l$oad") ||
	  almost_equals(0,"sa$ve") ||
	  almost_equals(0,"ex$it") ||
	  almost_equals(0,"sh$ow") ||
	  almost_equals(0,"she$ll")||
	  almost_equals(0,"q$uit"))
	 current_record->erase = 1;
       number_of_inputs++;
     }
  /*
   *  if(reprint)
   *  (void)fprintf(STDERRR,"In[%d] : %s \n",number_of_inputs-1,input_line);
   */
}

/***************************************************************************/
	
replace(n_t)
     int n_t;
{
  register int jj,ij,done;
  register struct  a_record *temp_record = records;

  
  ij = token[n_t+2].l_val.v.int_val;
  if(ij < 1 || ij > number_of_inputs-1)
    int_error("No such record",n_t+2);
  done = 0;

  while(temp_record && !done)
    {
      if( temp_record->index == ij ) done = 1;
      if(!done) temp_record = temp_record->next_record;
    }
  if(n_t && !equals(n_t-1,";"))
    jj = search_eq_sign(temp_record->definition);
  else
    jj = 0;
  
  (void)strcpy((temp_line+token[n_t].start_index),
	       (temp_record->definition+jj));
  (void)strcat(temp_line,(input_line + token[n_t+4].start_index));
  (void)strcpy(input_line,temp_line);
  num_tokens = scanner(input_line);
}

/***************************************************************************/

search_for_replace()
{
  t_num = 0;
  while(t_num < num_tokens)
    {
      if(isreinput(t_num))
	return(t_num);
      t_num++;
    }
  return (-1);
}


/***************************************************************************/
search_eq_sign(str)
     char *str;
{
  register char *st = str;
  register int i = 0;
  
  while(i < strlen(str))
    {
      if( *(st+i) == '{' ) return(0);
      else if( *(st+ i++) == '=')
	{
	  if( *(str+i-2) != '<' &&
  	      *(str+i-2) != '>' &&
	      *(str+i-2) != '!' &&
	      *(str+i-2) != '=' &&
	      *(str+i) != '=' )
	    return (i);
	}
    }
  return (0);
}
	  
/******************************************************************/

save_line()
{
  register struct  a_record *temp_record;
  register int i = 0;

  temp_record = add_record();
  temp_record->index = number_of_inputs;
  temp_record->definition =
    (char *)malloc((unsigned int)(strlen(input_line) + 3));
  temp_record->definition[0] ='\0';   
  while( input_line[i] == ' ' || input_line[i] == '\t'  ) i++;
  (void)strcpy(temp_record->definition,input_line+i);
  (temp_record->definition)[strlen(input_line)+1] = '\0';
}

/***************************************************************************/

struct a_record *add_record()
{
  register struct a_record **t_record = &records;

  while(*t_record)
    t_record = &( (*t_record)->next_record);
  if(!(*t_record =
       (struct a_record *)malloc( (unsigned int)sizeof(struct a_record))))
    int_error("not enought memory",NO_CARET);
  (*t_record)->next_record = (struct a_record *)NULL;
  (*t_record)->index = -1;
  (*t_record)->erase = 0;
  (*t_record)->definition = (char *)NULL;

  current_record = (*t_record);
  return (*t_record);
}


/*******************************************************************/
extern char desfilename[], datfilename[];
done(status)
     int status;
{
  FILE *fp,*fopen();
  char file_name[30];
  struct a_record *temp_record = records;

  c_token++;
  if(!END_OF_COMMAND)
     {
	if(isstring(c_token))
	  quote_str(file_name,c_token);
	else
          capture(file_name, c_token,c_token);
     }
  else strcpy(file_name,"xplot.sav");
  
  if((fp = fopen(file_name,"w"))!=NULL) 
    {
      (void)fprintf(fp,"\n#\t\t Xplot Record File\n# If you want \
to load this file later, do not modify it.\n#\n");
      while(temp_record) 
	{
	  if(temp_record->definition)
	    {
	      if(!temp_record->erase)
		(void) fprintf(fp,"In[%d] : %s\n",
			       temp_record->index,temp_record->definition);
	      else
		(void) fprintf(fp,"In[%d] : #---- %s\n",
			       temp_record->index,temp_record->definition);
	    }
	  temp_record = temp_record->next_record;
	}
    }
  if(datfilename[0] !='\0')
    {
      remove(desfilename);
      remove(datfilename);
      /*
      (void)sprintf(file_name,"/bin/rm %s %s\n",desfilename,datfilename);
      (void) system(file_name); */
    }
  exit(status);
}

/******************************************************************/

