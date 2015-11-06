/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "window/window.hh"
#include "app/app.hh"
#include "main/main.hh"
#include "window/style.hh"
#include "file/file.hh"
#include "loaders/load.hh"
#include "window/wmanager.hh"
#include "math/transform.hh"
#include "gui/text.hh"
#include "gui/button.hh"
#include "loaders/load.hh"


// merge test
// merge test 2.5

float interp(float c1, float c2, float r)
{
  return (c1-c2)*r+c2;
}


class test_win_class : public i4_window_class
{  
  i4_image_class *cloud;
  float cx,cy;
public:

  test_win_class(w16 w, w16 h)
    : i4_window_class(w,h)  
  { 
    cloud=i4_load_image("cloud.tga");
    cx=cy=5;
  }

  void draw(i4_draw_context_class &context)
  { 
    local_image->clear(0,context);


    float rx=(cx-(int)cx), ry=(cy-(int)cy);

    w32 *d=(w32 *)cloud->data;
    int w=cloud->width(), h=cloud->height();

    int dx=(int)cx, dy=(int)cy;

    for (int y=0; y<h; y++)
      for (int x=0; x<w; x++)
      {
        float x1=interp(d[0]&0xff, d[1]&0xff, rx);
        float x2=interp(d[w]&0xff, d[w+1]&0xff, rx);
        int yc=(int)interp(x1,x2, ry);
       
        local_image->put_pixel(dx+x, dy+y,  yc|(yc<<8)|(yc<<16));                
        d++;
      }

    

    cx+=0.1;
    cy+=0.2;

    request_redraw();
  }
  
  char *name() { return "test_win"; }
};

class test_app : public i4_application_class
{
public:
  enum { QUIT };

  void init()
  {
    i4_application_class::init();
    get_window_manager()->add_child(0,0, new test_win_class(300,300));

  }  


  void receive_event(i4_event *ev)
  {
    if (ev->type()==i4_event::USER_MESSAGE && ((i4_user_message_event_class *)ev)->sub_type==QUIT)
      quit();
    else
      i4_application_class::receive_event(ev);
  }



  char *name() { return "test_app"; }
};

void i4_main(w32 argc, i4_const_str *argv)
{
  test_app test;
  test.run();
}

