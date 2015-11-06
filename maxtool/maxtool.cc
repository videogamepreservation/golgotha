/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "app/app.hh"
#include "render.hh"
#include "obj3d.hh"
#include "load3d.hh"


#include "menu/textitem.hh"
#include "gui/text.hh"
#include "gui/button.hh"
#include "window/colorwin.hh"
#include "gui/text_input.hh"
#include "device/keys.hh"
#include "gui/browse_tree.hh"
#include "string/string.hh"
#include "gui/text_scroll.hh"
#include "error/error.hh"
#include "error/alert.hh"
#include "version.hh"
#include "id.hh"
#include "tupdate.hh"
#include "status/gui_stat.hh"
#include "tmanage.hh"
#include "g1_limits.hh"
#include "saver_id.hh"
#include "r1_win.hh"
#include "g1_tint.hh"
#include "file/file.hh"
#include "loaders/dir_load.hh"
#include "gui/deco_win.hh"
#include "m1_info.hh"
#include "lisp/lisp.hh"
#include "lisp/li_init.hh"
#include "st_edit.hh"
#include "gui/image_win.hh"
#include "gui/li_pull_menu.hh"
#include "device/key_man.hh"
#include "menu/pull.hh"
#include "font/anti_prop.hh"
#include "main/main.hh"
#include "r1_font.hh"

#include <string.h>

#ifdef _WINDOWS
#include "maxcomm.hh"
#include "main/win_main.hh"
#endif

#include <stdio.h>

r1_render_api_class *api=0;

#ifdef MAXCOMM_HH
m1_mail_slot_class slot;
#endif

i4_str *current_model_name=0;


i4_text_scroll_window_class *m1_warn_window=0;

int m1_max_mip_level=64;

int m1_warning(const char *format, va_list &ap)
{
  char st[500];
  vsprintf(st,format,ap);

  if (m1_warn_window)
  {
    m1_warn_window->output_string(st);
    m1_warn_window->output_char((i4_char)('\n'));
  }
  else
    fprintf(stderr,"Warning : %s\n",st);

  return 0;
}



int m1_alert(const i4_const_str &ret)
{
  i4_const_str::iterator s=ret.begin();
  while (s!=ret.end())
  {
    m1_warn_window->output_char(s.get());
    ++s;
  }
  m1_warn_window->output_char((i4_char)('\n'));

  return 0;
}

void m1_add_to_list(i4_array<i4_str *> &t_arr, i4_str *tname)
{
  int found=0;
  for (int j=0; !found && j<t_arr.size(); j++)
    if ( (*t_arr[j])==(*tname))
      found=1;

  if (!found)
    t_arr.add(tname);
  else
    delete tname;  
}


class m1_utility_app_class : public i4_application_class
{
protected:
  m1_poly_object_class *obj;
  i4_float theta,phi,dist;
  m1_utility_window_class *util_win;
  i4_text_input_class *tname_edit;
  r1_render_window_class *rwin;
public:
//   i4_bool get_display_name(char *name, int max_len)
//   {
//     strcpy(name, "Windowed GDI");
//     return i4_T;
//   }


  void quit() { i4_application_class::quit(); }

  char *name() { return "m1_utility_app_class"; }

  void update_fly(const i4_const_str &name, i4_bool check)
  {
    util_win->set_object(name);
  }


  i4_window_class *create_st_edit()
  {    
    tname_edit=new i4_text_input_class(get_style(),
                                       "",
                                       256,
                                       200, this,
                                       wm->get_style()->font_hint->small_font);
                                                             

    m1_st_edit=new m1_st_edit_window_class(256,256, tname_edit);

    i4_deco_window_class *st_deco=new i4_deco_window_class(m1_st_edit->width(),
                                                           m1_st_edit->height(),
                                                           i4_F,
                                                           get_style());

    st_deco->add_child(st_deco->get_x1(), st_deco->get_y1(), m1_st_edit.get());
                                                           

    tname_edit->resize(st_deco->width(), tname_edit->height());

   
    i4_deco_window_class *tname_deco=new i4_deco_window_class(st_deco->width(),
                                                              st_deco->height() +
                                                              tname_edit->height(),
                                                              i4_T,
                                                              get_style());

    tname_deco->add_child(tname_deco->get_x1(), tname_deco->get_y1(), tname_edit);
    tname_deco->add_child(tname_deco->get_x1(), tname_deco->get_y1() + tname_edit->height(),
                          st_deco);
                                                              
    
    return tname_deco;
  }

  void receive_event(i4_event *ev)
  {
    switch (ev->type())
    {      
      case i4_event::DISPLAY_CHANGE :
      {
        CAST_PTR(dev, i4_display_change_event_class, ev);
        if (dev->change==i4_display_change_event_class::SIZE_CHANGE)
        {
          int nw=i4_current_app->get_display()->width();
          int nh=i4_current_app->get_display()->height();

          
          m1_warn_window->resize(nw,m1_warn_window->height());
          
          rwin->resize(nw-rwin->x(), nh-m1_warn_window->height()-rwin->y());         
        }
        
      } break;


      case i4_event::OBJECT_MESSAGE :
      {
        CAST_PTR(tc, i4_text_change_notify_event, ev);
        if (tc->object==tname_edit && tc->new_text && m1_info.obj)
          m1_st_edit->change_current_texture(*tc->new_text);

      } break;

      case i4_event::DO_COMMAND :
      {
        CAST_PTR(dev, i4_do_command_event_class, ev);
        if (!strcmp(dev->command,"quit"))
          quit();
        else 
        {
          li_object *f=li_get_fun(li_get_symbol(dev->command), 0);
          if (f)
          {
            li_function_type fun=li_function::get(f,0)->value();
            fun(0,0);
          }
        }

      } break;

      case i4_event::USER_MESSAGE :
      {
        CAST_PTR(uev, i4_user_message_event_class, ev);
        if (uev->sub_type >= M1_SET_PLAYER_NUMBER)
        {
          util_win->set_current_player(uev->sub_type-M1_SET_PLAYER_NUMBER);
          util_win->request_redraw(i4_F);
        } else
        {
          switch (uev->sub_type)
          {


            case M1_NEXT :        
            {
              if (m1_info.models.size())
              {
                m1_info.current_model=(m1_info.current_model+1)%m1_info.models.size();
                update_fly(*m1_info.models[m1_info.current_model], i4_T);
              } 
            } break;

            case M1_LAST :
            {
              if (m1_info.models.size())
              {
                m1_info.current_model=(m1_info.current_model+m1_info.models.size()-1) %
                  m1_info.models.size();
                update_fly(*m1_info.models[m1_info.current_model], i4_T);
              } 
            } break;

            case M1_RECENTER :
            {
              util_win->recenter();
            } break;

            case M1_AXIS :
              m1_info.set_flags(M1_SHOW_AXIS, ~m1_info.get_flags(M1_SHOW_AXIS));
              util_win->request_redraw(i4_F);
              break;


            case M1_QUIT :
              quit();
              break;
          }
        }

      } break;
    }

    i4_application_class::receive_event(ev);
  }


  void init()
  {
    i4_set_max_memory_used(10*1024*1024);
    memory_init();

    char *cd_name="cd_image_name = golgotha.cd";
    i4_string_man.load_buffer(cd_name, "");


    resource_init("fly_util.res", 0); 


#ifdef MAXCOMM_HH
    slot.create(i4gets("slot_name"));
#endif

    m1_info.init(i4_global_argc, i4_global_argv);
    if (m1_info.update_mode)
      li_call("update_all_textures");
    else
    {    
      display_init();

      api = r1_create_api(display);
      if (!api)
        i4_error("could not create a render api for this display");
      m1_info.r_api=api;


      g1_init_color_tints(api);


      theta=0;
      phi=0;
      dist=40;

      obj=0;

      i4_graphical_style_class *style=wm->get_style();
      style->font_hint->normal_font=style->font_hint->small_font;

      i4_image_class *im=i4_load_image("maxtool_font.tga");
      style->font_hint->normal_font=new i4_anti_proportional_font_class(im);
      delete im;


    
      i4_key_man.load("keys.res");
      i4_key_man.set_context("maxtool");
      i4_kernel.request_events(this, 
                               i4_device_class::FLAG_DO_COMMAND |
                               i4_device_class::FLAG_END_COMMAND);


      i4_pull_menu_class *menu=li_create_pull_menu("menu.scm");
      menu->show(wm,0,0);


      i4_color fore=wm->i4_read_color_from_resource("warning_window_fore");
      i4_color back=wm->i4_read_color_from_resource("warning_window_back");

    

      w32 l,t,r,b;
      style->get_in_deco_size(l,t,r,b);
      i4_parent_window_class *deco=new i4_deco_window_class(wm->width()-(l+r), 80, i4_T, style);

      m1_warn_window=new i4_text_scroll_window_class(wm->get_style(),
                                                     fore, back,
                                                     deco->width()-(l+r),
                                                     deco->height()-(t+b));   

      m1_warn_window->output_string("Flytool warning messages.....\n");
      deco->add_child(l,t, m1_warn_window);

#ifndef __linux
      //    i4_set_warning_function(m1_warning);
      i4_set_alert_function(m1_alert);
#endif


    

      i4_window_class *st_edit=create_st_edit();
      wm->add_child(0,wm->height()-deco->height()-st_edit->height(), st_edit);


    
      wm->add_child(0, menu->height(),
                    new i4_deco_window_class(st_edit->width(),
                                             wm->height()-deco->height()-st_edit->height()-menu->height(),
                                             i4_T, style));

      //     wm->add_child(0, menu->height(), 
      //                   new i4_image_window_class(i4_load_image("fly_tool_logo.jpg"), i4_T, i4_F));


      int rw=wm->width()-st_edit->width(), 
        rh=wm->height()-deco->height()-menu->height();

      rwin=api->create_render_window(rw, rh, R1_COPY_1x1);

      util_win=new m1_utility_window_class(rw, rh, api, wm, theta, phi, dist);
      m1_render_window=util_win;

      util_win->init();
    
      rwin->add_child(0,0, util_win);
      wm->add_child(st_edit->width(), menu->height(), rwin);


      /*    i4_parent_window_class *warn=
          wm->add_mp_window(wm, control_window->width(),m1_util_win->height(),
          m1_warn_window->width(),
          m1_warn_window->height(),
          i4gets("warn_title")); */

    //    warn->add_child(0,0, m1_warn_window);
      wm->add_child(0,util_win->height()+
                    menu->height(), deco);
    

      i4_kernel.request_events(this, i4_device_class::FLAG_DISPLAY_CHANGE);

      i4_init_gui_status(wm, display);
    }

  }


  
  void kill_obj()
  {
    if (obj)
    {
      r1_texture_manager_class *tman=api->get_tmanager();

      tman->reset();
      m1_info.textures_loaded=i4_F;

      delete obj;

      obj = 0;
    }    
  }
  
  void m1_load_next()
  {
#ifdef MAXCOMM_HH
    if (slot.initialized())
    {
      if (slot.read_ready())
      {
        char name[128];

        name[0] = 0;
        slot.read(name,sizeof(name));

        i4_str *new_name = i4gets("name_format").sprintf(128,name);

        m1_info.set_current_filename(*new_name);
        update_fly(*new_name, i4_T);

        BringWindowToTop(i4_win32_window_handle);


        delete new_name;
      }
    }
#endif
  }
  
  void calc_model()
  {
    r1_texture_manager_class *tman=api->get_tmanager();
    //tman->do_collection();

    m1_load_next();

#ifdef _WINDOWS
    Sleep(0);
#endif

    i4_application_class::calc_model();

    if (!m1_info.textures_loaded)
    {
      api->get_tmanager()->load_textures();


      i4_image_class *im=i4_load_image("maxtool_font.tga");
      if (m1_info.r_font)
        delete m1_info.r_font;
      m1_info.r_font=new r1_font_class(api, im);
      delete im;


      m1_info.textures_loaded=i4_T;
    }
     
  }
  
  virtual void run(int argc, i4_const_str *argv)
  {
    i4_current_app=this;
    init();
    finished=i4_F; 

    if (!m1_info.update_mode)  // bypassing graphics?
    {
      if (m1_info.models.size())
        update_fly(*m1_info.models[0], i4_T);

      do
      {
        calc_model();
        refresh();      

      
        //next frame thing      
        api->get_tmanager()->next_frame();

        do 
        {
          m1_load_next();
          get_input();
        }
        while (display->display_busy() && !finished);

      } while (!finished);
    }

    m1_info.uninit();
    uninit();
  }
  
  void uninit()
  {
    if (!m1_info.update_mode)  // bypassing graphics?
    {
      i4_uninit_gui_status();

      i4_kernel.unrequest_events(this, i4_device_class::FLAG_DISPLAY_CHANGE);
      kill_obj();

      r1_destroy_api(api);
      i4_application_class::uninit();
    }
    else
      i4_uninit();


  }
  
};

m1_utility_app_class *m1_app;

li_object *m1_quit(li_object *o, li_environment *env)
{
  if (m1_app)
    m1_app->quit();
  return 0;
}
li_automatic_add_function(m1_quit, "quit");

void i4_main(w32 argc, i4_const_str *argv)
{
  m1_utility_app_class m1_app;
  m1_app.run(argc, argv);
}
