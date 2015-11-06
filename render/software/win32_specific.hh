/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef _WIN32_SPECIFIC_HH_
#define _WIN32_SPECIFIC_HH_

#include "software/r1_software.hh"
#include <ddraw.h>

class r1_software_render_window_class : public r1_render_window_class
{
  //i (trey), unfortunately dont really know how this works.
public:
  IDirectDrawSurface3 *surface;  

  r1_software_render_window_class(w16 w, w16 h,
                               r1_expand_type expand_type,
                               r1_render_api_class *api);
 
  ~r1_software_render_window_class();

  void draw(i4_draw_context_class &context);
  void receive_event(i4_event *ev);
  
  char *name() { return "software render window"; }
};

#endif

