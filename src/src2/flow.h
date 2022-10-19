
/****************************************************
 *
 *     IRISPLOT ---------------
 *                             flow.h
 *
 ***************************************************/ 

typedef struct a_for_loop {
  char *first, *second,*third,*body;
} FORLOOP;

typedef struct a_while_loop {
  char *first,*body;
} WHILELOOP;

typedef struct if_cond {
  char *first, *body;
} IFCOND;

FORLOOP    forloop;
WHILELOOP  whileloop;
IFCOND     ifcond;

/*****************************************************************/
  




 
