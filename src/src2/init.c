/******************************************************
 * 
 *   I R I S P L O T
 *             --------- init.c
 *
 *    Copyright 1989 Zou Maorong
 *
 *****************************************************/

extern char input_line[];
extern int save_current_line;

char *strcpy();
int  strcmp();

int built_in_func = 0;
int built_in_vari = 0;

do_initialization()
{
  static char *init_args[] =
    {
      "pi = 3.14159265358979323",
      "pow(x,y) = (x)**(y)",
      "cnt_func_(x,y,z)=z",
      "rk=11;RK=11","i=sqrt(-1.0)",
      "rkqc=12;RKQC=12",
      "LOOP_COND=0", "IF_COND=0",
      "red=9", "green=10","blue=11","yellow=12",
      "magenta=13","cyan=14", "white=15",
      "solid=0","points=-1", "dots=-1",
      "NULL"
      };

  register int i;
/*
#include <libm/math.h>
#include <float.h>
  _LIB_VERSION = _IEEE_; // _IEEE_ , _POSIX_ , _SVID_,_XOPEN
//  _control87 (0x033e, 0xffff);
 */
  built_in_func = 2;
  built_in_vari = 18;
  i = 0;
  save_current_line = 0;
  while(init_args[i] && strcmp("NULL",init_args[i]))
    {
      (void) strcpy(input_line,init_args[i]);
      do_line();
      i++;
    }
  srand((long) getpid());
  save_current_line = 1;
  make_default_material();
  reset_window_position();
}
/****************************************************************************/
