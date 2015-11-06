/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "map_man.hh"
#include "map.hh"
#include "lisp/li_init.hh"
#include "lisp/lisp.hh"
#include "objs/map_piece.hh"
#include "resources.hh"
#include "window/colorwin.hh"
#include "window/style.hh"
#include "gui/text_input.hh"
#include "gui/text.hh"
#include "app/app.hh"

li_object *g1_full_health(li_object *o, li_environment *env)
{
  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 t=g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS);

  li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));

  for (int i=0; i<t; i++)
  {
    if (olist[i]->selected())
      olist[i]->health=olist[i]->get_type()->defaults->health;
  }

  li_call("redraw");

  return 0;
}

class g1_set_health_dialog_class : public i4_color_window_class
{
protected:
  i4_text_window_class *caption;
  i4_text_input_class *input;
public:
  g1_set_health_dialog_class(i4_graphical_style_class *style)
    : i4_color_window_class(300,50,style->color_hint->neutral(),style)
  {
    sw32 health = 0;
    g1_object_class *olist[G1_MAX_OBJECTS];
    sw32 t=g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS);
    for (int i=0; i<t; i++)
      if (olist[i]->selected())
        health = olist[i]->health;

    char buf[256];
    sprintf(buf, "%d", health);
        
    caption = new i4_text_window_class(       "Health:",style);
    add_child(10,8,caption); 
    input   = new i4_text_input_class (style, buf,100,40, this);
    add_child(100,5,input);
  }

  virtual void receive_event(i4_event *ev)
  {
    switch (ev->type())
    {
      case i4_event::OBJECT_MESSAGE:
      {
        CAST_PTR(tev, i4_text_change_notify_event, ev);
        int found=-1;
        
        i4_str::iterator p = tev->new_text->begin();
        sw32 health = p.read_number();

        g1_object_class *olist[G1_MAX_OBJECTS];
        sw32 t=g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS);
        
        li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));

        for (int i=0; i<t; i++)
          if (olist[i]->selected())
            olist[i]->health=health;

        li_call("redraw");

        style->close_mp_window(parent);
      } break;

      default:
        i4_color_window_class::receive_event(ev);
        break;
    }
  }
};

static i4_event_handler_reference_class<g1_set_health_dialog_class> g1_set_health_dialog;

li_object *g1_set_health(li_object *o, li_environment *env)
{
  if (!g1_set_health_dialog.get())
  {
    i4_graphical_style_class *style = i4_current_app->get_style();
    g1_set_health_dialog = new g1_set_health_dialog_class(style);
    
    style->create_mp_window(-1,-1, 
                            g1_set_health_dialog->width(),
                            g1_set_health_dialog->height(),
                            "Set Health")
      ->add_child(0,0,g1_set_health_dialog.get());
  }
  return 0;
}
li_automatic_add_function(g1_set_health,"set_health");
