/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "video/display.hh"
#include "device/kernel.hh"
#include "error/error.hh"

i4_display_list_struct *i4_display_list=0;


void i4_display_list_struct::add_to_list(char *_name, int _driver_id, 
                                         i4_display_class *_display,
                                         i4_display_list_struct *_next)
{
  name=_name;
  driver_id=_driver_id;
  display=_display;
  next=_next;
  i4_display_list=this;
}
