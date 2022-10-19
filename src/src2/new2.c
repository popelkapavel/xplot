/*************************************************************
 *  
 *          I R I S P L O T   ---------- new2.c
 *
 *    Copyright (C) 1989 Maorong Zou
 *
 *************************************************************/

#include <stdio.h>
#include "plot.h"

extern int    c_token,num_tokens;
extern struct at_type *perm_at();
extern struct udft_entry *dummy_func;
extern char   c_dummy_var[], c_dummy_var1[];
extern char   c_dummy_var2[],c_dummy_var3[];
extern struct value  *const_express();
extern double real();
extern struct a_graph    *c_graph;

char    *malloc();
static struct uaxes *parse_for_axies();
static curve_color = 0;
/************************************************************/

#define locate_an_udft_entry(fx)  if(! (fx = (struct udft_entry *)\
  malloc( (unsigned int) sizeof( struct udft_entry)))) \
  int_error("out of memory",-1)

#define locate_an_at_entry(att)  if( !((att) =  perm_at())) \
  int_error("out of memory",-1)			       

#define  IS_DUMMY if(equals(c_token,c_dummy_var ) || \
		     equals(c_token,c_dummy_var1) || \
		     equals(c_token,c_dummy_var2) || \
		     equals(c_token,c_dummy_var3))
/************************************************************/

parse_eqn()
{
  register int ii;
  struct value a;

  (c_graph->picture).eqn.fx= (struct udft_entry *)NULL;
  (c_graph->picture).eqn.fy= (struct udft_entry *)NULL;
  (c_graph->picture).eqn.fz= (struct udft_entry *)NULL;
  (c_graph->picture).eqn.fw= (struct udft_entry *)NULL;
  (c_graph->picture).eqn.sample = EQNSAMPLE;
  (c_graph->picture).eqn.small  = EQNSMALL;
  (c_graph->picture).eqn.step =   EQNSTEP;
  (c_graph->picture).eqn.method = RK;
  (c_graph->picture).eqn.color = 9 + curve_color%7; curve_color++;
  (c_graph->picture).eqn.skip = 5;
  (c_graph->picture).eqn.dimension = 0;
  (c_graph->picture).eqn.start_time = 0.0;
  (c_graph->picture).eqn.end_time = 10.0;
  (c_graph->picture).eqn.x0 =  0.0;
  (c_graph->picture).eqn.y0 =  0.0;
  (c_graph->picture).eqn.z0 =  0.0;
  (c_graph->picture).eqn.w0 =  0.0;
  (c_graph->picture).eqn.x1 =  1.0;
  (c_graph->picture).eqn.y1 =  1.0;
  (c_graph->picture).eqn.z1 =  1.0;
  (c_graph->picture).eqn.w1 =  1.0;

  /*
   * added, for axes
   */
  (c_graph->picture).eqn.uaxes = (struct uaxes *)NULL;


  if(!equals(c_token,"["))
    int_error("need a '['",c_token);
  c_token++;
  locate_an_udft_entry( (c_graph->picture).eqn.fx );
  dummy_func = (c_graph->picture).eqn.fx;
  locate_an_at_entry( ( (c_graph->picture).eqn.fx)->at);
  if(equals(c_token,",") || equals(c_token,":")) c_token++;      
  if(equals(c_token, "]"))
    (c_graph->picture).eqn.dimension = 1;
  else
    {
      locate_an_udft_entry( (c_graph->picture).eqn.fy);
      locate_an_at_entry( ( (c_graph->picture).eqn.fy)->at);
      if(equals(c_token,",") || equals(c_token,":")) c_token++;      
      if(equals(c_token, "]"))
	(c_graph->picture).eqn.dimension = 2;
      else
	{
	  locate_an_udft_entry( (c_graph->picture).eqn.fz);
	  locate_an_at_entry( ( (c_graph->picture).eqn.fz)->at);
	  if(equals(c_token,",") || equals(c_token,":")) c_token++;      
	  if(equals(c_token, "]"))
	    (c_graph->picture).eqn.dimension = 3;
	  else
	    { 
	      locate_an_udft_entry( (c_graph->picture).eqn.fw);
	      locate_an_at_entry( ( (c_graph->picture).eqn.fw)->at);
	      (c_graph->picture).eqn.dimension = 4;	      
	      if(!equals(c_token,"]"))
		int_error("need a ']'",c_token);
	    }
	}
    }
  c_token++;

  if(equals(c_token,","))  c_token++;

  while(!equals(c_token,"}") && !END_OF_COMMAND)
    {
      if(!equals(c_token,"["))  
	int_error("need a '['",c_token);
      c_token++;
      if(almost_equals(c_token,"num$ber_of_steps"))
	{
	  c_token++;
	  if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	  (c_graph->picture).eqn.sample = 
	    (int) real(const_express(&a));
	  if( (c_graph->picture).eqn.sample < 10)
	    int_error("too few steps",c_token);
	}
      else if(almost_equals(c_token,"col$or"))
	{
	  c_token++;
	  if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	  (c_graph->picture).eqn.color = (int) real(const_express(&a));	  
	}
      else if(almost_equals(c_token,"meth$od"))
	{
	  c_token++;
	  if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	  (c_graph->picture).eqn.method =(int) real(const_express(&a));	  
	  if( (c_graph->picture).eqn.method != RK &&
	     (c_graph->picture).eqn.method != RKQC )
	    int_error("avaible methods: RK and RKQC",c_token);
	}
      else if(equals(c_token,"skip"))
	{
	  c_token++;
	  if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	  (c_graph->picture).eqn.skip =(int) real(const_express(&a));	  
	  if( ((c_graph->picture).eqn.skip) < 0)
	    int_error("need a positive integer",c_token);
	}
      else if(almost_equals(c_token,"ti$me"))
	{
	  c_token++;
	  if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	  (c_graph->picture).eqn.start_time = real(const_express(&a));	  
	  if(equals(c_token,",") || equals(c_token,":")) c_token++;
	  (c_graph->picture).eqn.end_time = real(const_express(&a));	  
	}
      else if(almost_equals(c_token,"sma$ll"))
	{
	  c_token++;
	  if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	  (c_graph->picture).eqn.small = real(const_express(&a));	  
	}
      else if(equals(c_token,"step"))
	{
	  c_token++;
	  if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	  (c_graph->picture).eqn.step = real(const_express(&a));	  
	}
      else if(almost_equals(c_token,"ini$tials"))
	{
	  ii = 0;
	  c_token++;
	  if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	  while(ii <  (c_graph->picture).eqn.dimension)
	    {
	      if(ii==0)
		(c_graph->picture).eqn.x0 =  real(const_express(&a));	 
	      else if(ii==1)
		(c_graph->picture).eqn.y0 =  real(const_express(&a));	 
	      else if(ii==2)
		(c_graph->picture).eqn.z0 =  real(const_express(&a));	 
	      else if(ii==3)
		(c_graph->picture).eqn.w0 =  real(const_express(&a));	 

	      if(equals(c_token,",") || equals(c_token,":")) c_token++;	
	      ii++;
	    }
	}
      else if(almost_equals(c_token,"sca$le"))
	{
	  ii = 0;
	  c_token++;
	  if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	  while(ii <  (c_graph->picture).eqn.dimension)
	    {
	      if(ii==0)
		(c_graph->picture).eqn.x1 =  real(const_express(&a));	 
	      else if(ii==1)
		(c_graph->picture).eqn.y1 =  real(const_express(&a));	 
	      else if(ii==2)
		(c_graph->picture).eqn.z1 =  real(const_express(&a));	 
	      else if(ii==3)
		(c_graph->picture).eqn.w1 =  real(const_express(&a));	 

	      if(equals(c_token,",") || equals(c_token,":")) c_token++;	
	      ii++;
	    }
	}
      /*
       * added stuff, for axes
       */ 
      else if(equals(c_token,"axes") || equals(c_token,"axis"))
	{
	  c_token++;
	  (c_graph->picture).eqn.uaxes = parse_for_axies();
	}
      if(!equals(c_token,"]")) 
	int_error("need a ']'",c_token);
      c_token++;
      if(equals(c_token,",") ) c_token++;
    }
  if(equals(c_token,"}")) c_token++;
}
/*******************************************************/

parse_surface()
{
  struct value a;
  int which_first = 0;

  (c_graph->picture).surface.fx= (struct udft_entry *)NULL;
  (c_graph->picture).surface.fy= (struct udft_entry *)NULL;
  (c_graph->picture).surface.fz= (struct udft_entry *)NULL;
  (c_graph->picture).surface.fr1= (struct udft_entry *)NULL;
  (c_graph->picture).surface.fr2= (struct udft_entry *)NULL;
  (c_graph->picture).surface.sample = SAMPLE;
  (c_graph->picture).surface.sample1 = SAMPLE;
  (c_graph->picture).surface.which_first = 1;
  (c_graph->picture).surface.is_function = 0;
  (c_graph->picture).surface.is_wire = 0;
  (c_graph->picture).surface.oren = 1;
  (c_graph->picture).surface.xmin = -4.0;
  (c_graph->picture).surface.ymin = -4.0;
  (c_graph->picture).surface.xmax =  4.0;
  (c_graph->picture).surface.ymax =  4.0;
  /*
   * added, for axes
   */
  (c_graph->picture).surface.uaxes = (struct uaxes *)NULL;

  if(!equals(c_token,"[")) 
    int_error("need a '['",c_token);
  c_token++;
  if(isstring(c_token))
    {
      c_graph->is_data = 1;

      c_graph->data_type = UNKNOWN_DATA;
      (c_graph->picture).surface.sample = 0;
      (c_graph->picture).surface.sample1 = 0;
      quote_str(c_graph->data_file,c_token);
      c_token++;
      if(!equals(c_token,"]")) 
	int_error("need a ']'",c_token);
    }
  else
    {
      locate_an_udft_entry( (c_graph->picture).surface.fx );
      dummy_func = (c_graph->picture).surface.fx;
      locate_an_at_entry( ( (c_graph->picture).surface.fx)->at);
      if(equals(c_token, "]"))
	(c_graph->picture).surface.is_function = 1;
      else
	{
	  if(equals(c_token,",") || equals(c_token,":")) c_token++; 
	  locate_an_udft_entry( (c_graph->picture).surface.fy);
	  locate_an_at_entry( ( (c_graph->picture).surface.fy)->at);
	  if(equals(c_token,",") || equals(c_token,":")) c_token++; 
	  locate_an_udft_entry( (c_graph->picture).surface.fz);
	  locate_an_at_entry( ( (c_graph->picture).surface.fz)->at);
	  if(!equals(c_token,"]"))
	    int_error("need a ']'",c_token);
	}
    }
  c_token++;

  if(equals(c_token,","))  c_token++;

  while(!equals(c_token,"}") && !END_OF_COMMAND)
    {
      if(!equals(c_token,"["))  
	int_error("need a '['",c_token);
      c_token++;
      if(almost_equals(c_token,"samp$les"))
	{
	  c_token++;
	  if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	  (c_graph->picture).surface.sample = 
	    (int) real(const_express(&a));
	  if(equals(c_token,":") || equals(c_token,",")) c_token++;
	  (c_graph->picture).surface.sample1 = 
	    (int) real(const_express(&a));
	  if(c_graph->is_data)
	    {
	      if( ((c_graph->picture).surface.sample) < 4 ||
		 ((c_graph->picture).surface.sample1) < 4)
		int_error("sample rate too small",c_token);
	    }
	  else
	    {
	      if( ((c_graph->picture).surface.sample) < 4 ||
		 ((c_graph->picture).surface.sample1) < 4)
		int_error("sample rate too small",c_token);
	    }
	}
      else if(almost_equals(c_token,"wire$frame"))
	{
	  c_token++;
	  if( equals(c_token,"=") || equals(c_token,":")) c_token++;
	  if(equals(c_token,"false")||equals(c_token,"FALSE")) c_token++;
	  if(equals(c_token,"true")||equals(c_token,"TRUE"))
	    {
	      c_token++;
	      (c_graph->picture).surface.is_wire = 1;
	    } 
	}
      else if(almost_equals(c_token,"orien$tation"))
	{
	  c_token++;
	  if( equals(c_token,"=") || equals(c_token,":")) c_token++;
	  (c_graph->picture).surface.oren = 
	    (int) real(const_express(&a));
	}
      else if(almost_equals(c_token,"data_ch$eck") ||
	      almost_equals(c_token,"check$_data") )
	{
	  c_token++;
	  if( equals(c_token,"=") || equals(c_token,":")) c_token++;
	  if(equals(c_token,"true")||equals(c_token,"TRUE")) c_token++;
	  if(equals(c_token,"false")||equals(c_token,"FALSE")) 
	    {
	      c_token++;
	      c_graph->data_check = 0;
	    }	  
	}
      /*
       * added stuff, for axes
       */ 
      else if(equals(c_token,"axes") || equals(c_token,"axis"))
	{
	  c_token++;
	  (c_graph->picture).surface.uaxes = parse_for_axies();
	}
      else if(isletter(c_token))
	{
	  if(equals(c_token,c_dummy_var))
	    {
	      c_token++;
	      if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	      if(!which_first) 
		{
		  which_first = 1;
		  (c_graph->picture).surface.xmin = 
		    real(const_express(&a));
		  if(equals(c_token,":") || equals(c_token,",")) c_token++;
		  (c_graph->picture).surface.xmax =
		    real(const_express(&a));
		}
	      else
		{
		  locate_an_udft_entry( (c_graph->picture).surface.fr1 );
		  locate_an_at_entry( ( (c_graph->picture).surface.fr1)->at);
		  if(equals(c_token,":") || equals(c_token,",")) c_token++;
		  locate_an_udft_entry( (c_graph->picture).surface.fr2 );
		  locate_an_at_entry( ( (c_graph->picture).surface.fr2)->at);
		}
	      if(equals(c_token,",")) c_token++;
	    }
	  else  if(equals(c_token,c_dummy_var1))
	    {
	      c_token++;
	      if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	      if(!which_first) 
		{
		  which_first = 1;
		  (c_graph->picture).surface.which_first = 2;
		  (c_graph->picture).surface.ymin = 
		    real(const_express(&a));
		  if(equals(c_token,":") || equals(c_token,",")) c_token++;
		  (c_graph->picture).surface.ymax =
		    real(const_express(&a));
		}
	      else
		{
		  locate_an_udft_entry( (c_graph->picture).surface.fr1 );
		  locate_an_at_entry( ( (c_graph->picture).surface.fr1)->at);
		  if(equals(c_token,":") || equals(c_token,",")) c_token++;
		  locate_an_udft_entry( (c_graph->picture).surface.fr2 );
		  locate_an_at_entry( ( (c_graph->picture).surface.fr2)->at);
		}
	      if(equals(c_token,",")) c_token++;
	    }  
	  else
	    int_error("unkown attributes",c_token);
	}
      if(!equals(c_token,"]")) 
	    int_error("need a ']'",c_token);
      c_token++;
      if(equals(c_token,",") ) c_token++;
    }
  if(equals(c_token,"}")) c_token++;
}
/*******************************************************/
parse_curve()
{

  struct value a;

  (c_graph->picture).curve.fx= (struct udft_entry *)NULL;
  (c_graph->picture).curve.fy= (struct udft_entry *)NULL;
  (c_graph->picture).curve.fz= (struct udft_entry *)NULL;
  (c_graph->picture).curve.sample = CURVESAMPLE;
  (c_graph->picture).curve.sample1 = 1;
  (c_graph->picture).curve.color = 9 + curve_color%7; curve_color++;
  (c_graph->picture).curve.style = 0;
  (c_graph->picture).curve.xmin = -4.0;
  (c_graph->picture).curve.xmax =  4.0;

  /*
   * added, for axes
   */
  (c_graph->picture).curve.uaxes = (struct uaxes *)NULL;

  if(!equals(c_token,"["))
	    int_error("need a '['",c_token);
  c_token++;  
  if(isstring(c_token))
    {
      c_graph->is_data = 1;

      c_graph->data_type = UNKNOWN_DATA;   
      (c_graph->picture).curve.sample = 0; /* need to count data unless
					    * sample will be specified 
					    */
      quote_str(c_graph->data_file,c_token);
      c_token++;
    }
  else
    {
      locate_an_udft_entry( (c_graph->picture).curve.fx );
      dummy_func = (c_graph->picture).curve.fx;
      locate_an_at_entry( ( (c_graph->picture).curve.fx)->at);
      if(equals(c_token,",") || equals(c_token,":")) c_token++;      
      locate_an_udft_entry( (c_graph->picture).curve.fy);
      locate_an_at_entry( ( (c_graph->picture).curve.fy)->at);
      if(equals(c_token,",") || equals(c_token,":")) c_token++;      
      locate_an_udft_entry( (c_graph->picture).curve.fz);
      locate_an_at_entry( ( (c_graph->picture).curve.fz)->at);
    }
  if(!equals(c_token,"]"))
    int_error("need a ']'",c_token);
  c_token++;

  if(equals(c_token,","))  c_token++;

  while(!equals(c_token,"}") && !END_OF_COMMAND)
    {
      if(!equals(c_token,"["))  
	int_error("need a '['",c_token);
      c_token++;
      if(almost_equals(c_token,"samp$les"))
	{
	  c_token++;
	  if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	  (c_graph->picture).curve.sample = 
	    (int) real(const_express(&a));
	  if( (c_graph->picture).curve.sample < 2)
	    int_error("sample rate too small",c_token);
	  if(equals(c_token,",") || equals(c_token,":")) c_token++;
	  if( (c_graph->is_data) && !equals(c_token,"]")) {
	    (c_graph->picture).curve.sample1 =
	      (c_graph->picture).curve.sample;
	    (c_graph->picture).curve.sample =   
	      (int) real(const_express(&a));
	  if( (c_graph->picture).curve.sample < 1)
	    int_error("sample rate too small",c_token); }
	}
      else if(almost_equals(c_token,"sty$le"))
	{
	  c_token++;
	  if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	  (c_graph->picture).curve.style =
	    (int) real(const_express(&a)); 
	}
      else if(almost_equals(c_token,"col$or"))
	{
	  c_token++;
	  if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	  (c_graph->picture).curve.color =
	    (int) real(const_express(&a)); 
	}
      else if(almost_equals(c_token,"data_ch$eck") ||
	      almost_equals(c_token,"check$_data") )
	{
	  c_token++;
	  if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	  if(equals(c_token,"true")||equals(c_token,"TRUE")) c_token++;
	  if(equals(c_token,"false")||equals(c_token,"FALSE")) 
	    {
	      c_token++;
	      c_graph->data_check = 0;
	    }	  
	}
      /*
       * added stuff, for axes
       */ 
      else if(equals(c_token,"axes") || equals(c_token,"axis"))
	{
	  c_token++;
	  (c_graph->picture).curve.uaxes = parse_for_axies();
	}
      else if(isletter(c_token))
	{
	  copy_str(c_dummy_var,c_token++);
	  if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	  (c_graph->picture).curve.xmin =
	    real( const_express(&a));	  
	  if(equals(c_token,",") || equals(c_token,":")) c_token++;
	  (c_graph->picture).curve.xmax =
	    real( const_express(&a));	  
	}
      if(!equals(c_token,"]"))
	int_error("need a ']'",c_token);
      c_token++;
      if(equals(c_token,",") ) c_token++;
    }
  if(equals(c_token,"}")) c_token++;
}
/*******************************************************/
parse_tube()
{
  struct value a;

  (c_graph->picture).tube.fx= (struct udft_entry *)NULL;
  (c_graph->picture).tube.fy= (struct udft_entry *)NULL;
  (c_graph->picture).tube.fz= (struct udft_entry *)NULL;
  (c_graph->picture).tube.radf= (struct udft_entry *)NULL;
  (c_graph->picture).tube.sample = TUBESAMPLE1;
  (c_graph->picture).tube.sample1 = TUBESAMPLE2;
  (c_graph->picture).tube.color = 0;
  (c_graph->picture).tube.xmin = -4.0;
  (c_graph->picture).tube.xmax =  4.0;

  /*
   * added, for axes
   */
  (c_graph->picture).tube.uaxes = (struct uaxes *)NULL;

  c_graph->data_type = GRID_DATA;
  if(!equals(c_token,"[")) int_error("need a '['",c_token);  c_token++;  
  if(isstring(c_token))
    {
      c_graph->is_data = 1;
      /* c_graph->data_type = UNKNOWN_DATA; */ /* has to be grid data*/
      (c_graph->picture).tube.sample = 0;  /* count entries in data_file */
      quote_str(c_graph->data_file,c_token);
      c_token++;
    }
  else
    {
      locate_an_udft_entry( (c_graph->picture).tube.fx );
      dummy_func = (c_graph->picture).tube.fx;
      locate_an_at_entry( ( (c_graph->picture).tube.fx)->at);
      if(equals(c_token,",") || equals(c_token,":")) c_token++;      
      locate_an_udft_entry( (c_graph->picture).tube.fy);
      locate_an_at_entry( ( (c_graph->picture).tube.fy)->at);
      if(equals(c_token,",") || equals(c_token,":")) c_token++;      
      locate_an_udft_entry( (c_graph->picture).tube.fz);
      locate_an_at_entry( ( (c_graph->picture).tube.fz)->at);
    }
  if(!equals(c_token,"]")) int_error("need a ']'",c_token); c_token++;
  if(equals(c_token,","))  c_token++;

  while(!equals(c_token,"}") && !END_OF_COMMAND)
    {
      if(!equals(c_token,"[")) 	int_error("need a '['",c_token); c_token++;
      if(almost_equals(c_token,"samp$les"))
	{
	  c_token++;
	  if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	  (c_graph->picture).tube.sample =  (int) real(const_express(&a));
	  if( (c_graph->picture).tube.sample < 2)
	    int_error("sample rate too small",c_token);
	  if(equals(c_token,",") || equals(c_token,":")) c_token++;
	  (c_graph->picture).tube.sample1 =  (int) real(const_express(&a));
	  if( (c_graph->picture).tube.sample1 < 3)
	    int_error("sample rate too small",c_token); 
	}
      else if(almost_equals(c_token,"rad$ius"))
	{
	  c_token++; if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	  locate_an_udft_entry( (c_graph->picture).tube.radf );
	  locate_an_at_entry( ( (c_graph->picture).tube.radf)->at);
	}
      else if(almost_equals(c_token,"col$or"))
	{
	  c_token++; if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	  (c_graph->picture).tube.color = (int) real(const_express(&a)); 
	}
      else if(almost_equals(c_token,"data_ch$eck") ||
	      almost_equals(c_token,"check$_data") )
	{
	  c_token++;
	  if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	  if(equals(c_token,"true")||equals(c_token,"TRUE")) c_token++;
	  if(equals(c_token,"false")||equals(c_token,"FALSE")) 
	    {
	      c_token++;
	      c_graph->data_check = 0;
	    }	  
	}
      /*
       * added stuff, for axes
       */ 
      else if(equals(c_token,"axes") || equals(c_token,"axis"))
	{
	  c_token++;
	  (c_graph->picture).tube.uaxes = parse_for_axies();
	}
      else if(isletter(c_token))
	{
	  copy_str(c_dummy_var,c_token++);
	  if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	  (c_graph->picture).tube.xmin = real( const_express(&a));	  
	  if(equals(c_token,",") || equals(c_token,":")) c_token++;
	  (c_graph->picture).tube.xmax =  real( const_express(&a));	  
	}
      if(!equals(c_token,"]")) int_error("need a ']'",c_token);
      c_token++;  if(equals(c_token,",") ) c_token++;
    }
  if(equals(c_token,"}")) c_token++;
}
/*******************************************************/

parse_map()
{
  register int ii;
  struct value a;

  (c_graph->picture).map.fx= (struct udft_entry *)NULL;
  (c_graph->picture).map.fy= (struct udft_entry *)NULL;
  (c_graph->picture).map.fz= (struct udft_entry *)NULL;
  (c_graph->picture).map.fw= (struct udft_entry *)NULL;
  (c_graph->picture).map.sample = MAPSAMPLE;
  (c_graph->picture).map.color = 9 + curve_color%7; curve_color++;
  (c_graph->picture).map.dimension = 0;
  (c_graph->picture).map.style = -1; /* added, for points */
  (c_graph->picture).map.x0 = -0.0;
  (c_graph->picture).map.y0 = -0.0;
  (c_graph->picture).map.z0 =  0.0;
  (c_graph->picture).map.w0 =  0.0;
  /*
   * added, for axes
   */
  (c_graph->picture).map.uaxes = (struct uaxes *)NULL;

  if(!equals(c_token,"[")) 
    int_error("need a '['",c_token);
  c_token++;
  locate_an_udft_entry( (c_graph->picture).map.fx );
  dummy_func = (c_graph->picture).map.fx;
  locate_an_at_entry( ( (c_graph->picture).map.fx)->at);
  if(equals(c_token,",") || equals(c_token,":")) c_token++;      
  if(equals(c_token, "]"))
    (c_graph->picture).map.dimension = 1;
  else
    {
      locate_an_udft_entry( (c_graph->picture).map.fy);
      locate_an_at_entry( ( (c_graph->picture).map.fy)->at);
      if(equals(c_token,",") || equals(c_token,":")) c_token++;      
      if(equals(c_token, "]"))
	(c_graph->picture).map.dimension = 2;
      else
	{
	  locate_an_udft_entry( (c_graph->picture).map.fz);
	  locate_an_at_entry( ( (c_graph->picture).map.fz)->at);
	  if(equals(c_token,",") || equals(c_token,":")) c_token++;      
	  if(equals(c_token, "]"))
	    (c_graph->picture).map.dimension = 3;
	  else
	    { 
	      locate_an_udft_entry( (c_graph->picture).map.fw);
	      locate_an_at_entry( ( (c_graph->picture).map.fw)->at);
	      (c_graph->picture).map.dimension = 4;	      
	      if(!equals(c_token,"]")) 
		int_error("need a ']'",c_token);
	    }
	}
    }
  c_token++;

  if(equals(c_token,","))  c_token++;

  while(!equals(c_token,"}") && !END_OF_COMMAND)
    {
      if(!equals(c_token,"["))  
	int_error("need a '['",c_token);
      c_token++;
      if(almost_equals(c_token,"iter$ates"))
	{
	  c_token++;
	  if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	  (c_graph->picture).map.sample = 
	    (int) real(const_express(&a));
	  if( (c_graph->picture).map.sample < 10)
	    int_error("iterate must > 10",c_token);
	}
      else if(almost_equals(c_token,"col$or"))
	{
	  c_token++;
	  if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	  (c_graph->picture).map.color = (int) real(const_express(&a));	  
	}
      else if(almost_equals(c_token,"sty$le"))
	{
	  c_token++;
	  if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	  (c_graph->picture).map.style = (int) real(const_express(&a));	  
	}
      else if(almost_equals(c_token,"ini$tials"))
	{
	  ii = 0;
	  c_token++;
	  if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	  while(ii <  (c_graph->picture).map.dimension)
	    {
	      if(ii==0)
		(c_graph->picture).map.x0 =  real(const_express(&a));	 
	      else if(ii==1)
		(c_graph->picture).map.y0 =  real(const_express(&a));	 
	      else if(ii==2)
		(c_graph->picture).map.z0 =  real(const_express(&a));	 
	      else if(ii==3)
		(c_graph->picture).map.w0 =  real(const_express(&a));	 
	      if(equals(c_token,",") || equals(c_token,":")) c_token++;	
	      ii++;
	    }
	}
      /*
       * added stuff, for axes
       */ 
      else if(equals(c_token,"axes") || equals(c_token,"axis"))
	{
	  c_token++;
	  (c_graph->picture).map.uaxes = parse_for_axies();
	}
      if(!equals(c_token,"]")) 
	int_error("need a ']'",c_token);	
      c_token++;
      if(equals(c_token,",") ) c_token++;
    }
  if(equals(c_token,"}")) c_token++;
}
/*******************************************************/

parse_tmesh()
{
  struct value a;

  (c_graph->picture).tmesh.sample = SAMPLE;
  (c_graph->picture).tmesh.sample1 = SAMPLE;
  (c_graph->picture).tmesh.oren = 1;
  /*
   * added, for axes
   */
  (c_graph->picture).tmesh.uaxes = (struct uaxes *)NULL;


  if(!equals(c_token,"["))
    int_error("need a '['",c_token);
  c_token++;

  if(!isstring(c_token))
    int_error("file name in quotes expected",c_token);
  c_graph->is_data = 1;
  quote_str(c_graph->data_file,c_token);
  c_token++;
  if(!equals(c_token,"]"))
    int_error("need a ']'",c_token);

  c_token++;
  if(equals(c_token,","))  c_token++;
  while(!equals(c_token,"}") && !END_OF_COMMAND)
    {
      if(!equals(c_token,"["))  
	int_error("need a '['",c_token);
      c_token++;
      if(almost_equals(c_token,"samp$les"))
	{
	  c_token++;
	  if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	  (c_graph->picture).tmesh.sample = 
	    (int) real(const_express(&a));
	  if(equals(c_token,",") || equals(c_token,":")) c_token++;
	  (c_graph->picture).tmesh.sample1 = 
	    (int) real(const_express(&a));
	  if( (c_graph->picture).tmesh.sample < 1 ||
	     (c_graph->picture).tmesh.sample1 < 3)
	    int_error("need at least 3 vertices for a t-mesh",c_token);
	  if( (c_graph->picture).tmesh.sample1 > 255)
	    int_error("Too many vertices for t-mesh (should < 256)",c_token);
	}
      else if(almost_equals(c_token,"orien$"))
	{
	  c_token++;
	  if( equals(c_token,"=") || equals(c_token,":")) c_token++;
	  (c_graph->picture).tmesh.oren = 
	    (int) real(const_express(&a));
	}
      else if(almost_equals(c_token,"data_ch$eck") ||
	      almost_equals(c_token,"check$_data") )
	{
	  c_token++;
	  if(equals(c_token,"=") || equals(c_token,":")) c_token++;
	  if(equals(c_token,"true")||equals(c_token,"TRUE")) c_token++;
	  if(equals(c_token,"false")||equals(c_token,"FALSE")) 
	    {
	      c_token++;
	      c_graph->data_check = 0;
	    }	  
	}
      /*
       * added stuff, for axes
       */ 
      else if(equals(c_token,"axes") || equals(c_token,"axis"))
	{
	  c_token++;
	  (c_graph->picture).tmesh.uaxes = parse_for_axies();
	}
      if(!equals(c_token,"]")) 
	int_error("need a ']'",c_token);
      c_token++;
      if(equals(c_token,",") ) c_token++;
    }
  if(equals(c_token,"}")) c_token++;
}
/*******************************************************/
static struct uaxes *parse_for_axies()
{
  struct uaxes *tmp;
  struct value a;

  tmp = (struct uaxes *)malloc( sizeof(struct uaxes));
  tmp->ox = 0.0;   tmp->oy = 0.0;   tmp->oz = 0.0;
  tmp->xx = 0.0;   tmp->xy = 0.0;   tmp->xz = 0.0;
  tmp->yx = 0.0;   tmp->yy = 0.0;   tmp->yz = 0.0;
  tmp->zx = 0.0;   tmp->zy = 0.0;   tmp->zz = 0.0;
  tmp->ticmarks = 20;
  if(equals(c_token,":") || equals(c_token,",")) c_token++ ;
  if(equals(c_token,"on") || almost_equals(c_token,"auto$matic"))
    c_token++;
  while( !equals(c_token,"]"))
    {
      if(almost_equals(c_token,"o$rigin")) 
	{
	  c_token++;
	  if(equals(c_token,":") || equals(c_token,"="))
	    c_token++;
	  if(!equals(c_token, "(") )
	    int_error("Need a '(' ", c_token);
	  c_token++;
	  tmp->ox = real(const_express(&a));
	  if(equals(c_token,",")) c_token++;
	  tmp->oy = real(const_express(&a));
	  if(equals(c_token,",")) c_token++;
	  tmp->oz = real(const_express(&a));
	  if(!equals(c_token,")") )
	    int_error("Need a ')' ", c_token);
	  c_token++;
	}
      else if(equals(c_token,"x") || equals(c_token,"X"))
	{
	  c_token++;
	  if(equals(c_token,":") || equals(c_token,"="))
	    c_token++;
	  if(!equals(c_token, "(") )
	    int_error("Need a '(' ", c_token);
	  c_token++;
	  tmp->xx = real(const_express(&a));
	  if(equals(c_token,",")) c_token++;
	  tmp->xy = real(const_express(&a));
	  if(equals(c_token,",")) c_token++;
	  tmp->xz = real(const_express(&a));
	  if(!equals(c_token,")") )
	    int_error("Need a ')' ", c_token);
	  c_token++;
	}
      else if(equals(c_token,"y") || equals(c_token,"Y"))
	{
	  c_token++;
	  if(equals(c_token,":") || equals(c_token,"="))
	    c_token++;
	  if(!equals(c_token, "(") )
	    int_error("Need a '(' ", c_token);
	  c_token++;
	  tmp->yx = real(const_express(&a));
	  if(equals(c_token,",")) c_token++;
	  tmp->yy = real(const_express(&a));
	  if(equals(c_token,",")) c_token++;
	  tmp->yz = real(const_express(&a));
	  if(!equals(c_token,")") )
	    int_error("Need a ')' ", c_token);
	  c_token++;
	}
      else if(equals(c_token,"z") || equals(c_token,"Z"))
	{
	  c_token++;
	  if(equals(c_token,":") || equals(c_token,"="))
	    c_token++;
	  if(!equals(c_token, "(") )
	    int_error("Need a '(' ", c_token);
	  c_token++;
	  tmp->zx = real(const_express(&a));
	  if(equals(c_token,",")) c_token++;
	  tmp->zy = real(const_express(&a));
	  if(equals(c_token,",")) c_token++;
	  tmp->zz = real(const_express(&a));
	  if(!equals(c_token,")") )
	    int_error("Need a ')' ", c_token);
	  c_token++;
	}
      else if(almost_equals(c_token,"tic$marks"))
	{
	  c_token++;
	  if(equals(c_token,":") || equals(c_token,"="))
	    c_token++;
	  tmp->ticmarks = (int)real(const_express(&a));
	}
      if(equals(c_token,",")) c_token++;
    }
  return(tmp);
}
/*******************************************************/
