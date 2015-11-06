/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef ALERT_HH_
#define ALERT_HH_


#include "string/string.hh"

typedef int (*i4_alert_function_type)(const i4_const_str &ret);

void i4_set_alert_function(i4_alert_function_type fun);

// max_length is the maximum length of the string when expanded
void i4_alert(const i4_const_str &format, w32 max_length, ...);
void i4_set_alert_function(i4_alert_function_type fun);

#endif
