
/************************************************************************
 *
 *       I R I S P L O T
 *                      --------------- new0.c
 *
 *     Copyright 1989 Zou Maorong
 *
 ***********************************************************************/

#include <stdio.h>
#include "plot.h"

/******************************************************************/
char	dummy_var[MAX_ID_LEN+1]         = "x";
char	dummy_var1[MAX_ID_LEN+1]         = "y";
char	dummy_var2[MAX_ID_LEN+1]         = "z";
char	dummy_var3[MAX_ID_LEN+1]         = "w";
char    c_dummy_var[MAX_ID_LEN+1] = "x";
char    c_dummy_var1[MAX_ID_LEN+1] = "y";   
char    c_dummy_var2[MAX_ID_LEN+1] = "z";   
char    c_dummy_var3[MAX_ID_LEN+1] = "w";   
double	xmin =  100000.0, xmax = -100000.0,
        ymin =  100000.0, ymax = -100000.0,
        zmin =  100000.0, zmax = -100000.0;
BOOLEAN undefined;
struct lexical_unit   token[MAX_TOKENS];
char input_line[MAX_LINE_LEN+1];
int  num_tokens, c_token;
struct udft_entry     *dummy_func;
/*************************************************************/
char *strcpy();
/*************************************************************/

command()      
{
  (void)strcpy(c_dummy_var,dummy_var);
  (void)strcpy(c_dummy_var1,dummy_var1); 
  (void)strcpy(c_dummy_var2,dummy_var2); 
  (void)strcpy(c_dummy_var3,dummy_var3); 

  command1();
}
/*************************************************************/
extern char desfilename[], datfilename[], setupfile[];

do_plot(ii)
     int ii;
{
  char *arv[3]; int retv;
  if(!ii) (void) fprintf(STDERRR,"\t Nothing to plot\n");
  else 
    {
      arv[0] = desfilename; arv[1] = datfilename; arv[2] = setupfile;
      if(setupfile[0] !='\0') retv=xplot_show_main(3,arv);
      else retv = xplot_show_main(2,arv);
    }
}

do_demo()
{
  if(system("xplot_demo"))
    os_error("Can't find 'xplot_demo'",-1);
}

do_help()
{
  extern char help[];

  if (system(help))
    os_error("can't spawn help",-1);
}
clear_screen()
{
  if(system("clear"))
    os_error("Can't find 'clear'",-1);
}
/*************************************************************/

  
