/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "device/event.hh"
#include "video/display.hh"
#include "device/device.hh"


#include "image/image.hh"
#include "quantize/histogram.hh"
#include "quantize/median.hh"
#include "loaders/tga_write.hh"
#include "video/display.hh"
#include "palette/pal.hh"
#include "device/kernel.hh"
#include "loaders/tga_write.hh"
#include "app/app.hh"
#include "device/keys.hh"

class g1_screen_shot_watcher_class : public i4_event_handler_class
{  
  int num_shots;
  w16 key;

public:
  g1_screen_shot_watcher_class()
  {
    key=I4_F2;
    
    i4_kernel.request_events(this, i4_device_class::FLAG_KEY_PRESS);
    num_shots = 0;
  }

  ~g1_screen_shot_watcher_class()
  {
    i4_kernel.unrequest_events(this, i4_device_class::FLAG_KEY_PRESS);
  }

  void save_shot()
  {
    i4_display_class *display=i4_current_app->get_display();
    if (display)
    {      
      char outname[200];
      sprintf(outname, "shot%03d.tga", num_shots++);
    
      i4_file_class *fp=i4_open(outname, I4_WRITE);
      i4_image_class *screen=display->lock_frame_buffer(I4_BACK_FRAME_BUFFER, I4_FRAME_BUFFER_READ);
      if (screen)
      {
        i4_tga_write(screen, fp);
        display->unlock_frame_buffer(I4_BACK_FRAME_BUFFER);
      }
 
      delete fp;
    }
  }

  void receive_event(i4_event *ev)
  {
    if (ev->type()==i4_event::KEY_PRESS)
    {
      CAST_PTR(kev, i4_key_press_event_class, ev);
      if (kev->key == key)
        save_shot();
    }
  }

  char *name() { return "screen_shot watcher"; }
};

static g1_screen_shot_watcher_class *g1_screen_shot_instance=0;

class g1_screen_shot_adder : public i4_init_class
{
public:
  void init() { g1_screen_shot_instance=new g1_screen_shot_watcher_class(); }
  void uninit() { delete g1_screen_shot_instance; g1_screen_shot_instance=0; }
} g1_screen_shot_adder_instance;


