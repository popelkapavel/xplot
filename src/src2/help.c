/*****************************************************************
 *
 *  Xplot, help.c
 *          ------ a very sketchy help
 *
 ****************************************************************/ 
#include <stdio.h>
#include "plot.h"
void xplot_help();
int  which_topic();
extern int c_token, num_tokens;
/**********************************************************************/

static char *topics[] = {
  "sur$face",
  "cur$ve", 
  "tube$", 
  "data$file",
  "obj$ect",
  "fun$ction",
  "var$iable",
  "dummy$_variable",
  "op$erator", 
  "plot$", 
  "show$", 
  "quit$",
  "tit$le",
  "save$",
  "load$",
  (char *)NULL,
};

static char *helptext[] = {
"\t A surface is either the graph of a 2 variable function f(x,y) or a\n\
\t parametric surface or a polygon list described by the OFF format.\n\
\t For example,\n \
\t  a = surface{[ exp( -(x^2+y^2)) ][x= -1:1][y=-1:1]};\n\
\t  b = surface{[ cos(x)*cos(y),cos(x)*sin(y),sin(x)][x=-0.5*pi,0.5*pi]\n\
\t\t [y=0:2*pi] [sample=30:40]};\n\
\t  c = surface{[\"sample_data_file\"]};\n",

"\t A space curve can be defined by a data_file or by its parametric \n\
\t coordinate functions. \n \
\t   For example,\n\
\t    d = curve{[\"sample_datafile\"][style=points]};\n\
\t    e = curve{[sin(x),cos(x),0][x=0:2*pi][sample=200][color=red]};\n\
\t There are two styles for curves. Solid (default) and points.\n",

"\t Tube defines a tube around a space curve, \n\
\t   For example,\n\
\t    torus =tube{[cos(x),sin(x),0][x=0:2*pi][radius=0.3][sample=80:20]};\n\
\t    torus1=tube{[cos(x),sin(x),0][x=0:2*pi][radius=0.15+0.1*sin(x)]};\n\
\t    f = tube{[\"sample_datafile\"][radius=0.2]};\n",

"\t Xplot accepts the following types of datafiles.\n\
\t\t M by N GRID \n\
\t where the datafile contains of MN lines, each line consists of\n\
\t three floats (the x,y,z coordinates of a point). This type of data\n\
\t can be used to define a surface, for example\n\
\t\t g = surf{[\"grid_data\"][sample=32:32]};\n\
\t A M by 1 grid can be used to define a curve or a tube, for example,\n\
\t\t h=curve{[\"A_M_by_1_grid\"]}; i=tube{[\"M_by_1_data\"][radius=1]}\n\
\t\t Polygon list (OFF format)\n\
\t This type of data can be used to define a surface. \n",

"\t An object is a group of simple graphs, for example\n\
\t\t  a = sur{[x*y][x=-1:1][y=-1:1]};\n\
\t\t  b = cur{[x,x^2,x^3][x=-1:1]};\n\
\t\t  o=object{ a, b}\n",

"\t You can define your own functions via some of the standard math\n\
\t functions and operators. For example\n\
\t\t f(x) = exp( sin(x) +0.2);\n\
\t\t g(x,y) = tan(x) + log( abs(x)) *sqrt( x^2+1);\n\
\t\t r(x)=(x<=0.0? -4*x + 0.1 : (x<=2.0? 0.1: sqrt((1.3*(x-2)))))\n\
\t Here is a list of bulid in functions\n\
\n\
\t\t  sin   cos     tan   \n\
\t\t  sinh  cosh    tanh  \n\
\t\t  asin  acos    atan \n\
\t\t  abs   sqrt    pow   \n\
\t\t  log   log10   exp   \n\
\t\t  real imag conjg arg  \n\
\t\t  besj0  besj1  besy0 besy1  \n\
\t\t  floor  ceil   int  \n\
\t\t  rand   \n",

"\t Variable are just numbers, for example,\n\
\t\t sqrt3 = sqrt(3);\n\
\t\t lambda = real( exp( 3 + 2* i));\
\t There are two internal variables, pi = 3.1415926,  i= sqrt(-1);\n",

"\t Dummy variables are names of function arguments. The defaults are\n\
\t\t  x, y, z, w\n\
\t You can overwrite the defaults by the 'set dummy' command. For example\n\
\t\t set dummy u,v;\n\
\t will set the dummy variables to  u,v,z,w. To see what are the current\n\
\t dummy variables, use the 'show dummy' command.\n",

"\t Here is a list of math operators,\n\
\t\t    +    -    *    /    %       (basic arithmetic) \n\
\t\t    ^    **                     (exponentiation) \n\
\t\t    &&   ||   !                 (logical) \n\
\t\t    ==   <=   >=  >   <   !=    (relational) \n\
\t These operators can be used in defining variables and functions.\n",

"\t To plot a defined object, use 'plot obj_name'. For example\n\
\t\t  a = sur{[sin(x)*cos(y)]};\n\
\t\t  plot a;\n",

"\t The 'show' command displays you the defined objects. for example,\n\
\t\t show func\n\
\t displays all the defined functions and the command \n\
\t\t show all\n\
\t displays all the defined objects (functions, variables, graphical objs)\n",

"\t The 'quit' command terminates Xplot. If followed by a string (meanining\
\t text in double quotes), it save all your command to the file with name\n\
\t the string. For example,\n\
\t\t quit \"session1\"\n\
\t will save all your command to 'session1'\n",

"\t The 'set title' command allows you to set the plot title. For example\n\
\t\t  set title \"This is a sample plot\"\n\
\t produces a plot title \"This is a sample plot\".\n\
\t Once a plot is done, you can move the position of the text by picking\n\
\t the text with the left mouse button and move the pointer around. The \n\
\t color of the title may be modified by pressing the middle mouse button \n\
\t on the text. You can also change the font by pressing the right button\n\
\t on the text.\n\n\
\t To disable the plot title, use the command 'set notitle'\n\n\
\t To see the current plot title, use the command 'show title'\n",

"\t The 'save' command save selected commands to a file. For example,\n\
\t\t save all \"xplot.com\"\n\
\t save all commands entered to the file \"xplot.com\"\n\
\t File saved by xplot can be loaded later via the `load` command.\n",

"\t The 'load' command reads and executes the commands from a file.\n\
\t For example,\n\
\t\t load \"xplot.com\"\n\
\t executes all commands in \"xplot.com\"\n",

NULL
};
/**********************************************************************/
void xplot_help()
{
  int i,j, k;
  if(END_OF_COMMAND)
    {
      i = 0;
      fprintf(stdout, "\tUsage: help topic. Available topics are:\n\n\t");
      while(topics[i] != NULL)
	{
	   k = strlen(topics[i]);
	  for(j = 0; j < k; j++)
	    if( *(topics[i]+j) != '$') fprintf(stdout,"%c", *(topics[i]+j));
	   fprintf(stdout,",   ");
	  if( (++i)%5 == 0) fprintf(stdout,"\n\t");
	}
      fprintf(stdout,"\n");
    }
  else
    {
      i = which_topic();
      if(i < 0) fprintf(stdout, "Sorry, cannot find help for this topic\n");
      else  fprintf(stdout,"\n%s\n", helptext[i]);
    }
}
/*********************************************************************/
int which_topic()
{
  int i = 0;
  while( topics[i])
    {
      if(almost_equals(c_token, topics[i])) return(i);
      i++;
    }
  return(-1);
}
/*********************************************************************/
