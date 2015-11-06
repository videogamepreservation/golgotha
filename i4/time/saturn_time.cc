/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifdef _SATURN
extern "C"
{

#include "sgl.h"

};
#endif

#include "time/time.hh"


i4_time_class::i4_time_class()
{
  w32 *sec_usec=(w32 *)private_data;

  sec_usec[0]=Smpc_Status->rtc.second +
    Smpc_Status->rtc.minute*60 +
    Smpc_Status->rtc.hour*60*60;
  sec_usec[1]=0;
}



bool i4_time_class::operator <(const i4_time_class &other) const 
{ 
  w32 *me_t=(w32 *)private_data;
  w32 *other_t=(w32 *)other.private_data;
  return (other_t[0]<me_t[0] || (other_t[0]==me_t[0] &&  other_t[1]<me_t[1]));
}



bool i4_time_class::operator >(const i4_time_class &other) const 
{ 
  w32 *me_t=(w32 *)private_data;
  w32 *other_t=(w32 *)other.private_data;
  return (other_t[0]>me_t[0] || (other_t[0]==me_t[0] &&  other_t[1]>me_t[1]));
}


sw32 i4_time_class::milli_diff(const i4_time_class &past_time) const
{
  w32 *me_t=(w32 *)private_data;
  w32 *past_t=(w32 *)past_time.private_data;

  return ((sw32)me_t[1]-(sw32)past_t[1])/1000+(me_t[0]-past_t[0])*1000;
}

