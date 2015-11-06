/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "init/init.hh"
#include "video/svgalib/svgal_vd.hh"
#include "device/svgalib/mouse.hh"
#include "device/svgalib/keyboard.hh"

// this class glues the svgakey board and svga mouse to the svga display
// if you want to use the mouse or keyboard seperately, don't link in this
// this file needs to after the others as far as link order

class svgalib_unit_class : public i4_init_class
{
  public :
  void init()
  {
    svgalib_display_instance.devs.add_device(&svgalib_mouse_instance);
    svgalib_display_instance.devs.add_device(&svgalib_keyboard_instance);
  }

  void uninit()
  {
    svgalib_display_instance.devs.remove_device(&svgalib_mouse_instance);
    svgalib_display_instance.devs.remove_device(&svgalib_keyboard_instance);
  }

} ;

static svgalib_unit_class svga_unit_instance;
