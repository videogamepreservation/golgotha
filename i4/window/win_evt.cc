/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "window/win_evt.hh"
#include "string/string.hh"
#include "memory/malloc.hh"

void i4_drag_info_struct::copy_to(i4_drag_info_struct &drag_struct)
{
  drag_struct.t_filenames=t_filenames;
  if (t_filenames)
  {
    drag_struct.filenames=(i4_str **)i4_malloc(sizeof(i4_str *) * t_filenames, "");
    for (int i=0; i<t_filenames; i++)
      drag_struct.filenames[i]=new i4_str(*filenames[i]);
  } else drag_struct.filenames=0;

}

i4_drag_info_struct::~i4_drag_info_struct()
{
  if (t_filenames)
  {
    for (int i=0; i<t_filenames; i++)
      delete filenames[i];
    i4_free(filenames);
  }
}
