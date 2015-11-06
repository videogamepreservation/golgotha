/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_SIMPLE_DIALOG_HH
#define I4_SIMPLE_DIALOG_HH


#include "string/string.hh"
class i4_event_handler_class;
class i4_event;
class i4_parent_window_class;
class i4_graphical_style_class;

i4_parent_window_class *i4_create_yes_no_dialog(i4_parent_window_class *parent,
                                                i4_graphical_style_class *style,
                                                const i4_const_str &title,
                                                const i4_const_str &message,
                                                const i4_const_str &yes, const i4_const_str &no,
                                                i4_event_handler_class *send_to,
                                                i4_event *yes_event, i4_event *no_event);



#endif
