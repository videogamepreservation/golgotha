/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __i4TIME_HPP_
#define __i4TIME_HPP_

#include "arch.hh"
 
#include <stdlib.h>
#include <string.h>

class i4_time_class
{
  union
  {
    struct
    {
      sw32 sec, usec;
    } unix_time;

    struct 
    {
      sw32 clock;
      sw32 overflow;
    } win32_time;
  } time;
  
public:

  i4_time_class(sw32 milli_sec);


  i4_bool operator <(const i4_time_class &other) const;
  i4_bool operator >(const i4_time_class &other) const;

  // copies the time from another previously created i4_time_class
  i4_time_class(const i4_time_class &ref) 
  { 
    time=ref.time;
  }

  void get();    // gets the time from the system

  void add_milli(sw32 milli_sec);

  i4_time_class()
  { get(); }

  sw32 milli_diff(const i4_time_class &past_time) const;  
  // returns difference between our time and a past time in 1/1000th of a second
};

w64 i4_get_system_clock();
int i4_get_clocks_per_second();

void i4_sleep(int seconds);
void i4_milli_sleep(int milli_seconds);   // 1/100th of a seconds

#endif

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
