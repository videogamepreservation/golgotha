/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "window/style.hh"
#include "g1_object.hh"
#include "gui/slider.hh"
#include "window/colorwin.hh"
#include "gui/text.hh"
#include "editor/dialogs/d_light.hh"
#include "objs/light_o.hh"
#include "editor/editor.hh"
#include "editor/e_state.hh"
#include "map_man.hh"
#include "editor/e_res.hh"
#include "lisp/lisp.hh"

static i4_parent_window_class *g1_create_slider(i4_event_handler_class *notify,
                                                w32 milli_delay,
                                                w32 width,
                                                w32 start,
                                                const i4_const_str name,
                                                i4_graphical_style_class *style)
{
  i4_slider_class *s=new i4_slider_class(width, start, notify, milli_delay, style);
  i4_text_window_class *t=new i4_text_window_class(name, style);

  i4_parent_window_class *c=new i4_color_window_class(width, s->height()+t->height(),
                                                      style->color_hint->neutral(),
                                                      style);
  c->add_child(0,0,s);
  c->add_child(0,s->height(), t);
  c->resize_to_fit_children();

  return c;

}
                        

class g1_bulb_ewin : public i4_color_window_class
{
  i4_parent_window_class *r,*g,*b, *c3;
  i4_float or,og,ob,oc3;

  w32 selected_objects[G1_MAX_OBJECTS];
  int t_sel;

public:


  char *name() { return "bulb edit"; }

  g1_bulb_ewin(i4_graphical_style_class *style,
               g1_object_class *def)

    : i4_color_window_class(0,0,style->color_hint->neutral(), style)
  {
    g1_light_object_class *d=g1_light_object_class::cast(def);
    if (d)
    {
      or=d->r;
      og=d->g;
      ob=d->b;
      oc3=d->c3;   

      r=g1_create_slider(this, 1000, 200, (sw32)(or*200), g1_ges("red"), style);
      add_child(0,0,r);

      g=g1_create_slider(this, 1000, 200, (sw32)(og*200), g1_ges("green"), style);
      add_child(0,r->height()+5,g);

      b=g1_create_slider(this, 1000, 200, (sw32)(ob*200), g1_ges("blue"), style);
      add_child(0,r->height() + g->height() + 10,b);

      c3=g1_create_slider(this, 1000, 200, (sw32)((1.0-(oc3-0.05)/(1.0-0.05))*200),
                          g1_ges("brightness"), style);
      add_child(0,r->height() + g->height() + b->height() + 20, c3);

      resize_to_fit_children();
    }

    t_sel=g1_get_map()->make_selected_objects_list(selected_objects, G1_MAX_OBJECTS);
  }

  void update_lights()
  {
    g1_map_class *m=g1_editor_instance.get_map();
    
   
    g1_editor_instance.add_undo(G1_MAP_OBJECTS | G1_MAP_CELLS);

    for (int i=0; i<t_sel; i++)
    {
      if (g1_global_id.check_id(selected_objects[i]))
      {
        g1_light_object_class *l;

        if (l=g1_light_object_class::cast(g1_global_id.get(selected_objects[i])))
          l->setup(l->x, l->y, l->y, or, og, ob, 0.01, 0.25, oc3);
      }
    }

    g1_editor_instance.changed();

    li_call("redraw");
  }

  void receive_event(i4_event *ev)
  {
    if (ev->type()==i4_event::OBJECT_MESSAGE)
    {
      CAST_PTR(sev, i4_slider_event, ev);
      if (r->isa_child((i4_window_class *)sev->object))
        or=sev->x/(i4_float)sev->divisor;
      else if (g->isa_child((i4_window_class *)sev->object))
        og=sev->x/(i4_float)sev->divisor;
      else if (b->isa_child((i4_window_class *)sev->object))
        ob=sev->x/(i4_float)sev->divisor;
      else if (c3->isa_child((i4_window_class *)sev->object))
        oc3=1.0-(sev->x/(i4_float)sev->divisor)*(1.0-0.05);
      else i4_parent_window_class::receive_event(ev);
      
      update_lights();
    }
    else i4_parent_window_class::receive_event(ev);
   
  }
};

i4_parent_window_class *g1_create_bulb_edit(i4_graphical_style_class *style,
                                            g1_object_class *def)
{
  return new g1_bulb_ewin(style, def);

}

