/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_GUI_STAT_HH
#define I4_GUI_STAT_HH


class i4_window_manager_class;
class i4_display_class;

void i4_init_gui_status(i4_window_manager_class *wm,
                        i4_display_class *display);

void i4_uninit_gui_status();

#endif
