/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __FILE_OPEN_HH
#define __FILE_OPEN_HH

class i4_parent_window_class;
class i4_event_handler_class;
class i4_graphical_style_class;

#include "string/string.hh"
#include "device/event.hh"

class i4_file_open_message_class : public i4_user_message_event_class
{
public:
  i4_str *filename;

  i4_file_open_message_class(w32 id, i4_str *filename)
    : i4_user_message_event_class(id), filename(filename) {}

  virtual i4_event  *copy() { return new i4_file_open_message_class(sub_type, 
                                                                    new i4_str(*filename)); }  

};

void i4_create_file_open_dialog(i4_graphical_style_class *style,
                                const i4_const_str &title_name,
                                const i4_const_str &start_dir,
                                const i4_const_str &file_mask,
                                const i4_const_str &mask_name,
                                i4_event_handler_class *tell_who,
                                w32 ok_id, w32 cancel_id);

#endif
