/************************************************************************
 * 
 *  Code is modified from the code for Gnuplot by Zou Maorong
 */
/*
 *  G N U P L O T  --  plot.c
 *  Copyright (C) 1986, 1987  Thomas Williams, Colin Kelley
 *  You may use this code as you wish if credit is given and this message
 *  is retained.
 */

/****************************************************************************/
#include <stdio.h>
#include <setjmp.h>
#include <unistd.h>
#include <signal.h>
#include "plot.h"

#ifndef STDOUT
#define STDOUT 1
#endif
#define HOME     "HOME"
#define PLOTRC   ".irisplotrc"

/***************************************************************************/

extern char   input_line[];

extern  f_push(),f_pushc(),f_pushd0(),f_pushd1(),f_pushd2(),f_pushd3(),
  f_call(),f_lnot(),f_bnot(),f_uminus(),
  f_lor(),f_land(),f_bor(),f_xor(),f_band(),f_eq(),f_ne(),f_gt(),f_lt(),
  f_ge(),f_le(),f_plus(),f_minus(),f_mult(),f_div(),f_mod(),f_power(),
  f_factorial(),f_bool(),f_jump(),f_jumpz(),f_jumpnz(),f_jtern();

extern  f_real(),f_imag(),f_arg(),f_conjg(),f_sin(),f_cos(),f_tan(),f_asin(),
  f_acos(),f_atan(),f_sinh(),f_cosh(),f_tanh(),f_int(),f_abs(),f_sgn(),
  f_sqrt(),f_exp(),f_log10(),f_log(),f_besj0(),f_besj1(),f_besy0(),f_besy1(),
  f_gamma(),f_floor(),f_ceil(),f_rand(),f_srand();
  


struct ft_entry ft[] = {	/* built-in function table */

  /* internal functions: */
	{"push", f_push},	{"pushc", f_pushc},	{"pushd0", f_pushd0},
	{"pushd1", f_pushd1},   {"pushd2",f_pushd2},    {"pushd3",f_pushd3},
	{"call", f_call},	{"lnot", f_lnot},
	{"bnot", f_bnot},       {"uminus", f_uminus},	{"lor", f_lor},
	{"land", f_land},	{"bor", f_bor},		{"xor", f_xor},
	{"band", f_band},	{"eq", f_eq},		{"ne", f_ne},
	{"gt", f_gt},		{"lt", f_lt},		{"ge", f_ge},
	{"le", f_le},		{"plus", f_plus},	{"minus", f_minus},
	{"mult", f_mult},	{"div", f_div},		{"mod", f_mod},
	{"power", f_power},     {"factorial", f_factorial},
	{"bool", f_bool},	{"jump", f_jump},	{"jumpz", f_jumpz},
	{"jumpnz",f_jumpnz},    {"jtern", f_jtern},

  /* standard functions: */
	{"real", f_real},	{"imag", f_imag},	{"arg", f_arg},
	{"conjg", f_conjg},     {"sin", f_sin},         {"cos", f_cos},
	{"tan", f_tan},		{"asin", f_asin},	{"acos", f_acos},
	{"atan", f_atan},	{"sinh", f_sinh},	{"cosh", f_cosh},
	{"tanh", f_tanh},	{"int", f_int},		{"abs", f_abs},
 	{"sgn", f_sgn},		{"sqrt", f_sqrt},	{"exp", f_exp},
	{"log10", f_log10},	{"log", f_log},		{"besj0", f_besj0},
	{"besj1", f_besj1},	{"besy0", f_besy0},	{"besy1", f_besy1},
/* 	{"gamma", f_gamma},  	{"floor", f_floor},	{"ceil", f_ceil}, */
	{"floor", f_floor},	{"ceil", f_ceil},       {"rand",f_rand},
        {"srand", f_srand},     
	{NULL, NULL}
      };

char                         *getenv(),*strcat(),*strcpy(),*strncpy();
jmp_buf                      env;
struct value                 *integer(),*complex();
struct udvt_entry            *first_udv = NULL;
struct udft_entry            *first_udf = NULL;
FILE                         *outfile;

/***************************************************************************/

interrupt()
{
  (void) signal(SIGINT, (void *)interrupt);
  (void) signal(SIGFPE, SIG_DFL);	/* turn off FPE trapping */
  (void) putcc('\n',STDERRR);
  longjmp(env, TRUE);		/* return to prompt */
}

/***************************************************************************/
int main(int argc,char **argv)
{
  register FILE *plotrc;
  static char home[sizeof(PLOTRC)+80];
  extern char* grx_wxh;
  extern int calc_intersections;

  { int  c;
  while(EOF!=(c=getopt(argc,argv,"r:ih")))
    switch(c) {
     case 'r':grx_wxh= optarg;break;
     case 'i':calc_intersections=0;break;
     case 'h':
     default:
      fputs("xplot [-i] [-r widthxheight] [filename1] [filename2] [...]\n",stderr);
      fputs("-r widthxheight - screen resolution ( 640x480 default)\n",stderr);
      fputs("-i              - do not calc intersection before plot\n",stderr);
      fputs("-h              - this help\n",stderr);
      return c!='h';
      break;
    }
  }

  setbuf(STDERRR,(char *)NULL);
  show_version();

  do_initialization();
  
  if (!setjmp(env)) 
    {
      (void) signal(SIGINT, (void *)interrupt);/* go there on interrupt char */
      if (!(plotrc = (fopen(PLOTRC,"r")))) 
	{
          char *env_home;
          env_home=getenv(HOME);
          if(!env_home) {
            env_home=".";
            putenv("HOME=.");
            //setenv(HOME,(env_home="."),1);
          }
	  (void) strcat(strncpy(home,env_home,sizeof(home)),"/");
	  plotrc = fopen(strcat(home,PLOTRC),"r");
	}
      if (plotrc)
	load_file(plotrc);
    }
  while(optind<argc) {
    FILE *f=fopen(argv[optind++],"r");
    if(f)
      load_file(f);
    else
      fprintf(stderr,"%s: file open failed\n",argv[optind-1]);
  }
  while(1) 
    {  
      com_line();
    }
}
/****************************************************************************/


