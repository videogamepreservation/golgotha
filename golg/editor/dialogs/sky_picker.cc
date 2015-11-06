/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "editor/editor.hh"
#include "gui/create_dialog.hh"
#include "gui/text_input.hh"
#include "window/colorwin.hh"
#include "sky.hh"
#include "r1_api.hh"
#include "g1_render.hh"
#include "window/win_evt.hh"
#include "app/app.hh"
#include "r1_win.hh"
#include "gui/scroll_bar.hh"
#include "tmanage.hh"
#include "editor/editor.hh"
#include "lisp/lisp.hh"
#include "string/string.hh"

static int  sky_scroll_offset=0;

class g1_sky_view : public i4_window_class
{
  int offset;
  i4_bool active;
public:
  g1_sky_view(w16 w, w16 h, int offset)
    : i4_window_class(w,h),
      offset(offset)
  {
    active=i4_F;
   
  }

  int off() { return offset+sky_scroll_offset;}

  void setup_vert(float x, float y, r1_vert &v)
  {
    v.px=x*(width());  v.py=y*(height());  v.v.z=50;  v.w=1.0/v.v.z;
    v.s=x; v.t=y;
    v.r=v.g=v.b=1.0;
    v.a=1;                                                
  }

  void draw(i4_draw_context_class &context)
  {
    i4_bool valid=i4_F;

    if (off()>=0 && off()<g1_sky_list.size())
      valid=i4_T;

    r1_render_api_class *api=g1_render.r_api;

    r1_texture_handle han;
    if (valid)
    {
      char tname[256], tname1[256];
      i4_os_string(*g1_sky_list[off()], tname, 256);
      sprintf(tname1, "%s1", tname);
      
      i4_bool loaded;
      han=api->get_tmanager()->register_texture(tname1, "sky texture not found", &loaded);
      if (!loaded)
        han=api->get_tmanager()->register_texture(tname, "sky texture not found");
        

    }
    else
      han=0;



    api->default_state();
    api->set_shading_mode(R1_WHITE_SHADING);

    if (active)
      api->set_constant_color(0xffffff);
    else               
      api->set_constant_color(0x9f9f9f);

    if (han)      
      api->use_texture(han, width(), 0);

    r1_vert v[3];
    setup_vert(0,0, v[0]);  setup_vert(0,1, v[1]);  setup_vert(1,1, v[2]); 
    api->render_poly(3, v);

    setup_vert(0,0, v[0]);  setup_vert(1,1, v[1]);  setup_vert(1,0, v[2]); 
    api->render_poly(3, v);

  }

  void receive_event(i4_event *ev)
  {
    if (ev->type()==i4_event::WINDOW_MESSAGE)
    {
      CAST_PTR(wev, i4_window_message_class, ev);
      if (wev->sub_type==i4_window_message_class::GOT_MOUSE_FOCUS)
      {
        active=i4_T;
        request_redraw(i4_F);
      }
      else if (wev->sub_type==i4_window_message_class::LOST_MOUSE_FOCUS)
      {
        active=i4_F;
        request_redraw(i4_F);
      }
    }
    else if (ev->type()==i4_event::MOUSE_BUTTON_DOWN)
    {
      CAST_PTR(bev, i4_mouse_button_down_event_class, ev);

      if (bev->left() && off()>=0 && off()<g1_sky_list.size())
      {
        g1_editor_instance.add_undo(G1_MAP_SKY);
        
        if (g1_get_map()->sky_name)
          delete g1_get_map()->sky_name;

        g1_get_map()->sky_name=new i4_str(*g1_sky_list[off()]);

        li_call("redraw");
      }


    }
    

  }


  char *name() { return "sky_view"; }
};

class g1_sky_picker_class : public i4_color_window_class
{
  i4_graphical_style_class *style;
    i4_array<r1_render_window_class *> render_windows;


public:
  g1_sky_picker_class(i4_graphical_style_class *style)
    : i4_color_window_class(0,0, style->color_hint->neutral(), style),
      render_windows(0,32)
  {
    int t_vis=5, t_obj=g1_sky_list.size(), obj_w=64, obj_h=64, i,x=0, y=0, x1=0;
    
    for (i=0; i<t_vis; i++)
    {
      render_windows.add(g1_render.r_api->create_render_window(obj_w, obj_h, R1_COPY_1x1));
      render_windows[i]->add_child(0,0, new g1_sky_view(obj_w, obj_h, i));      
      add_child(x1,y, render_windows[i]);
      x1+=render_windows[i]->width();
    }
    y+=obj_h;
    
    i4_scroll_bar *sb=new i4_scroll_bar(i4_F, t_vis * obj_w, t_vis, t_obj, 0, this, style);
    add_child(x,y, sb);

    resize_to_fit_children();
  }

  void receive_event(i4_event *ev)
  {
    i4_color_window_class::receive_event(ev);
    if (ev->type()==i4_event::USER_MESSAGE)
    {
      CAST_PTR(sev, i4_scroll_message, ev);
      sky_scroll_offset=sev->amount;
      for (int i=0; i<render_windows.size(); i++)
        render_windows[i]->request_redraw(i4_F);
    }
  }

  char *name() { return "sky picker"; }
};



void g1_editor_class::create_sky_window()
{
  if (get_map())
  {
    sky_scroll_offset=0;
    g1_sky_picker_class *sp=new g1_sky_picker_class(i4_current_app->get_style());

    create_modal(sp->width(), sp->height(), "set_sky_title");
    modal_window.get()->add_child(0,0,sp);
  }
}

void g1_editor_class::set_sky()
{  
//   i4_os_string(*sky_dialog.name->get_edit_string(), 
//                get_map()->sky_name, 
//                sizeof(get_map()->sky_name));

//   char *name=get_map()->sky_name;
//   get_map()->sky_model=g1_model_list_man.get_model(g1_model_list_man.find_handle(name));
//   close_modal();
}
