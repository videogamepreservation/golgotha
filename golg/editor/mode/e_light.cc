/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "editor/mode/e_light.hh"
#include "editor/e_state.hh"
#include "error/error.hh"
#include "gui/butbox.hh"
#include "editor/contedit.hh"
#include "light.hh"
#include "map_light.hh"
#include "cwin_man.hh"
#include "editor/editor.hh"
#include "lisp/lisp.hh"

g1_light_params g1_e_light;

g1_mode_handler::state g1_light_mode::current_state()
{
  w8 remap[]={ ROTATE, ZOOM, DRAG_SELECT, DRAG_SELECT, OTHER, OTHER };
  I4_ASSERT(g1_e_light.minor_mode<=sizeof(remap), "state too big");
  return (g1_mode_handler::state)remap[g1_e_light.get_minor_mode()];
}

void g1_light_params::create_buttons(i4_parent_window_class *container)
{
  i4_button_box_class *box=new i4_button_box_class(&g1_edit_state);
  char *rn[]={"lROTATE", "lZOOM", 0 }, **a;
  w32 i=ROTATE;

  for (a=rn; *a; i++, a++)
    g1_edit_state.add_but(box, *a, 0, (i4_bool) i==minor_mode,
                          new g1_set_minor_mode_event("LIGHT",i));

  box->arrange_right_down();
  box->resize_to_fit_children();


  container->add_child(0,0,box);


  char *indiv_buts[]={"lGDARKEN", "lGBRIGHTEN", "lDDARKEN", "lDBRIGHTEN", "lAMBIENT", 0};
  int but_ids[]={ GDARKEN, GBRIGHTEN, DDARKEN, DBRIGHTEN, AMBIENT };
  int rep_ids[]={ GDARKEN_REP, GBRIGHTEN_REP, DDARKEN_REP, DBRIGHTEN_REP, 0 };


  int y=box->height()+5;

  
  for (i=0, a=indiv_buts; *a; i++, a++)
  {
    i4_button_class *b=g1_edit_state.create_button(*a, but_ids[i], i4_T, this);
    if (rep_ids[i])
      b->set_repeat_down(i4_T, new i4_event_reaction_class(this, rep_ids[i]));

    container->add_child(0, y, b);
    y+=b->height();
  }
}


void g1_light_params::receive_event(i4_event *ev)
{
  int sub_type=-1;
  if (ev->type()==i4_event::OBJECT_MESSAGE)
  {
    CAST_PTR(oev,i4_object_message_event_class,ev);
    if (oev->object==this)
      sub_type=oev->sub_type;
  }
  else if (ev->type()==i4_event::USER_MESSAGE)
    sub_type=((i4_user_message_event_class *)ev)->sub_type;

  if (sub_type!=-1)
  {
    int recalc=0;
    float increment=1/32.0;

    switch (sub_type)
    {
      case GDARKEN :
        li_call("add_undo", li_make_list(new li_int(G1_MAP_VERTS | G1_MAP_LIGHTS), 0));
        
      case GDARKEN_REP :
      {
        if (g1_lights.ambient_intensity - increment>=0)
        {
          recalc=1;
          g1_lights.set_ambient_intensity(g1_lights.ambient_intensity-increment);
        }
      } break;

      case GBRIGHTEN :
        li_call("add_undo", li_make_list(new li_int(G1_MAP_VERTS | G1_MAP_LIGHTS), 0));

      case GBRIGHTEN_REP :
      {
        if (g1_lights.ambient_intensity+increment <= 1.0)
        {
          recalc=1;
          g1_lights.set_ambient_intensity(g1_lights.ambient_intensity+increment);
        }
      } break;

      case DDARKEN :
        li_call("add_undo", li_make_list(new li_int(G1_MAP_VERTS | G1_MAP_LIGHTS), 0));

      case DDARKEN_REP :
      {
        if (g1_lights.directional_intensity - increment>=0)
        {
          recalc=1;
          g1_lights.set_directional_intensity(g1_lights.directional_intensity-increment);
        }
      } break;

      case DBRIGHTEN :
        li_call("add_undo", li_make_list(new li_int(G1_MAP_VERTS | G1_MAP_LIGHTS), 0));
      case DBRIGHTEN_REP :
      {
        if (g1_lights.directional_intensity+increment <= 1.0)
        {
          recalc=1;
          g1_lights.set_directional_intensity(g1_lights.directional_intensity+increment);
        }
      } break;

      
      case AMBIENT :
      {
        if (g1_current_controller.get())
        {
          recalc=1;
          li_call("add_undo", li_make_list(new li_int(G1_MAP_VERTS | G1_MAP_LIGHTS), 0));

          i4_transform_class *t=&g1_current_controller->transform;
          i4_3d_vector cam1, cam2; 
          t->inverse_transform(i4_3d_vector(0,0,0), cam1);
          t->inverse_transform(i4_3d_vector(0,0,1), cam2);
          cam2-=cam1;

          g1_lights.direction=cam2;
        }
      } break;

    }
    
    if (recalc)
    {
      g1_calc_static_lighting();
      g1_editor_instance.changed();

      li_call("redraw");
    }
  }
}

