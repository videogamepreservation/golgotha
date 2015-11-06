/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "window/wmanager.hh"
#include "app/app.hh"
#include "time/profile.hh"
#include "app/registry.hh"
#include "threads/threads.hh"

i4_profile_class pf_app_calc_model("app::calc_model");
i4_profile_class pf_app_refresh("app::refresh");
i4_profile_class pf_app_get_input("app::get_input");

i4_application_class *i4_current_app=0;

i4_parent_window_class *i4_application_class::get_root_window()
{
  return wm;
}

i4_graphical_style_class *i4_application_class::get_style()
{
  if (wm)
    return wm->get_style();
  else return 0;
}

void i4_application_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::DISPLAY_CLOSE)
    finished=i4_T;
}

void i4_application_class::run()
{  
  i4_current_app=this;
  init();
  finished=i4_F; 
  int restore_priority=0;

  do
  {
    pf_app_calc_model.start();
    calc_model();    
    pf_app_calc_model.stop();

    pf_app_refresh.start();
    refresh();
    pf_app_refresh.stop();

    if (restore_priority)
    {
      i4_set_thread_priority(i4_get_thread_id(), I4_THREAD_PRIORITY_NORMAL);
      restore_priority=0;
    }

    pf_app_get_input.start();
    int repeat;
    i4_kernel.events_sent=0;
    do 
    {
      repeat=0;
      get_input();
      if (!finished)
      {
        if (display->display_busy() || 
            (idle() && i4_kernel.events_sent==0 && 
             !wm->need_redraw() &&
             display->get_context()->both_dirty->empty()))
        {
          repeat=1;
          if (!restore_priority)
          {
            restore_priority=1;
            i4_set_thread_priority(i4_get_thread_id(), I4_THREAD_PRIORITY_LOW);
          }
          i4_thread_yield();
        }
      }
    }
    while (repeat);

    pf_app_get_input.stop();


  } while (!finished);
  uninit();
  i4_current_app=0;
}


void i4_application_class::calc_model()
{}

// get_input polls the hardware for changes and reports them as events to event_handlers
void i4_application_class::get_input()
{
  i4_kernel.process_events();

}

void i4_application_class::refresh()
{
  wm->root_draw();
}


// if an exact match is not found the closest width and height are return
i4_display_class::mode *i4_application_class::find_mode(w16 &width, w16 &height, int driver_id)
{
  sw32 closest_dist=0xfffffff;
  w16 closest_width=0,
      closest_height=0;

  i4_display_class::mode *use=display->get_first_mode(driver_id);
  if (!use)
    return 0;

  do
  {

    // if we are opening a window and we can set the xres and yres, 
    //   then make them fit as best we can to the suggested width and height

    if (use->flags & i4_display_class::mode::RESOLUTION_DETERMINED_ON_OPEN)
    {
      use->xres=width;
      use->yres=height;
    }
      

    if (width==use->xres &&     // did we find a matching mode?
        height==use->yres)
      return use;


    // see how far off we are
    sw32 dist=((sw32)use->xres-(sw32)width)*((sw32)use->xres-(sw32)width)+
              ((sw32)use->yres-(sw32)height)*((sw32)use->yres-(sw32)height);
    if (dist<closest_dist)
    {
      closest_dist=dist;
      closest_width=use->xres;
      closest_height=use->yres;
    }

    use=display->get_next_mode();
  } while (use);

  width=closest_width;
  height=closest_height;

  return 0;
}

i4_application_class::~i4_application_class()
{

}

void i4_application_class::memory_init()
{
  i4_init();
}


void i4_application_class::resource_init(char *resource_file, 
                                         void *resource_buffer)
{
  i4_string_man.load("resource/i4.res");

  if (resource_buffer)
    i4_string_man.load_buffer(resource_buffer,
                              "internal_buffer");
  else
    i4_string_man.load(resource_file);
}


void i4_application_class::handle_no_displays()
{
  i4_error("Could not find a display!");
}


static char *i4_display_key="SOFTWARE\\Crack dot Com\\I4\\1.0";

i4_bool i4_application_class::get_display_name(char *name, int max_len)
{
  return i4_get_registry(I4_REGISTRY_MACHINE, i4_display_key, "display", name, max_len);
}

void i4_application_class::display_init()
{
  char name[256];

  i4_display_list_struct *d, *found=0;

  if (get_display_name(name, 256))
  {
    for (d=i4_display_list; d; d=d->next)
      if (strcmp(d->name, name)==0)
        found=d;
  }
  
  if (!found)
    found=i4_display_list;

  if (!found)
    handle_no_displays();
    

  w16 width=640, height=480;

  // first try to find mode specified in the resource file
  i4_const_str xres=i4gets("default_xres", i4_F);
  i4_const_str yres=i4gets("default_yres", i4_F);
  
  if (!xres.null() && !yres.null())
  {
    i4_const_str::iterator xres_str=xres.begin(),
      yres_str=yres.begin();

    width=xres_str.read_number();
    height=yres_str.read_number();
  }

  w16 found_width=width,
      found_height=height;

  display=found->display;

  i4_display_class::mode *use=find_mode(found_width, found_height, found->driver_id);
  if (!use)
  {
    i4_warning("Unable to find an exact match for mode %dx%d, using %dx%d instead\n",
        width,height,
        found_width,found_height);

    use=find_mode(found_width, found_height, found->driver_id);

    if (!use)
      i4_error("What? Could not find that mode either");
  }

  if (!display->initialize_mode())
    handle_no_displays();

  wm=new i4_window_manager_class();
  wm->prepare_for_mode(display, use);

  i4_kernel.request_events(this,i4_device_class::FLAG_DISPLAY_CLOSE);
}



void i4_application_class::init()
{
  memory_init();
  resource_init("resource.res",0);
  display_init();
}

void i4_application_class::uninit()
{
  display_uninit();

  i4_uninit();
}

void i4_application_class::display_uninit()
{
  i4_kernel.process_events();
  delete wm;
  wm=0;
  
  i4_kernel.unrequest_events(this, i4_device_class::FLAG_DISPLAY_CLOSE);
  display->close();
  display=0;
}
