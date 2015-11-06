/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


#include <time.h>

class i4_profile_class
{
public:
  static i4_profile_class *list;

  i4_profile_class *next;
  char *name; 

	timespec start_time;
  double total_time;
  w32 recursion_level;  // current level of recursion


  i4_profile_class(char *debug_name) : name(debug_name)
  {
    recursion_level=0;
    total_time=0;
    next=list;
    list=this;
  }

  void start()
  {
    if (recursion_level==0)
		{
			clock_gettime(CLOCK_SGI_CYCLE, &start_time);
		}

    recursion_level++; 
 }

  void stop()
  {
    recursion_level--;
    if (recursion_level==0)
    {
      struct timespec end;
			clock_gettime(CLOCK_SGI_CYCLE, &end);

      double endt=(double)(end.tv_sec+(double)end.tv_nsec/(double)10000000000000.0);
      double startt=(double)(start_time.tv_sec+(double)start_time.tv_nsec/(double)10000000000000.0);

      total_time+=endt-startt;
    }
  }
} ;
