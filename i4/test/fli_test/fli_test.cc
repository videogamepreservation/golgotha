/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "main/main.hh"
#include "app/app.hh"
#include "loaders/fli_load.hh"
#include "window/window.hh"
#include "image/image.hh"
#include "file/file.hh"


class anim_window : public i4_window_class
{
  w32 frame_on,
    tframes;
  i4_image_class **im; 
public:
  I4_EVENT_HANDLER_NAME("anim_window");

  anim_window(i4_image_class **im,
              w32 tframes)
    : i4_window_class(400,400),
      tframes(tframes),
      im(im)
  {
    frame_on=0;
  }

  void draw(i4_draw_context_class &context)
  {
    im[frame_on]->put_image(local_image,0,0,context);
    frame_on++;
    if (frame_on>=tframes)
      frame_on=0;
    request_redraw();
  }

  virtual void show_self(w32 indent) { ; }
};

class test_app : public i4_application_class
{
public:
  I4_EVENT_HANDLER_NAME("anim_window");

  virtual w32 max_memory() const { return 6000*1024; }

  void init()
  {
    i4_application_class::init();

    i4_file_class *fp=i4_file_man.open(i4gets("fli_file"));
    if (fp)
    {
      w32 tframes;
      i4_image_class **im=i4_load_fli(fp,tframes);
      if (!im)
        i4_error("didn't load");

      anim_window *a=new anim_window(im,tframes);
      i4_parent_window_class *w=wm->add_mp_window(wm,50,50,
                                                  im[0]->width(),
                                                  im[0]->height(),i4gets("window_title"));
      w->add_child(0,0,a);
      delete fp;
    }    
  }

};

void i4_main(w32 argc, i4_const_str *argv)
{
  test_app app;
  app.run();
}
