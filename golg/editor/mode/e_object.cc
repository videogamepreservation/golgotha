/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "editor/mode/e_object.hh"
#include "editor/e_state.hh"
#include "error/error.hh"
#include "g1_object.hh"
#include "editor/contedit.hh"
#include "device/keys.hh"
#include "image/color.hh"
#include "image/image.hh"
#include "g1_limits.hh"
#include "editor/dialogs/d_light.hh"
#include "math/pi.hh"
#include "m_flow.hh"
#include "g1_render.hh"
#include "r1_clip.hh"
#include "objs/light_o.hh"
#include "editor/editor.hh"
#include "gui/butbox.hh"
#include "player.hh"
#include "objs/def_object.hh"
#include "lisp/li_class.hh"
#include "editor/e_res.hh"
#include "editor/dialogs/object_picker.hh"
#include "lisp/li_dialog.hh"
#include "li_objref.hh"
#include "lisp/li_init.hh"
#include "remove_man.hh"
#include "objs/path_object.hh"
#include "objs/bases.hh"
#include "tick_count.hh"

static g1_typed_reference_class<g1_object_class> preselected_object;
static li_symbol *g1_last_link_var=0;
static g1_typed_reference_class<g1_object_class> editing_object;
static li_symbol_ref path_object_type("path_object");

g1_object_params g1_e_object;


li_object *g1_object_changed(li_object *o, li_environment *env)
{
  if (editing_object.get())
  {
    if (o)
    {
      editing_object->vars=li_class::get(li_car(o,env),env);

               
      g1_object_class *list[G1_MAX_OBJECTS];
      int t=g1_get_map()->make_object_list(list, G1_MAX_OBJECTS); 
      li_class *old=li_class::get(li_second(o,env),env);
      for (int i=0; i<t; i++)
        list[i]->object_changed_by_editor(editing_object.get(), old);

    }

    g1_e_object.close_edit_window();
  }

  return 0;
}

li_automatic_add_function(g1_object_changed, "object_changed");



g1_mode_handler::state g1_object_mode::current_state()
{
  w8 remap[]={ ROTATE,
               ZOOM,
               OTHER,
               DRAG_SELECT,
               DRAG_SELECT,
               DRAG_SELECT };

  I4_ASSERT(g1_e_object.minor_mode<=sizeof(remap), "state too big");
  return (g1_mode_handler::state)remap[g1_e_object.get_minor_mode()];
}


void g1_object_mode::hide_focus()
{
  if (add.get())
  {
    add->unoccupy_location();
    add->request_remove();
    g1_remove_man.process_requests();
    add=0;
  }
}


      

void g1_object_mode::show_focus()
{
  if (!add.get() && g1_e_object.get_minor_mode()==g1_object_params::ADD)
  {
    w32 type=g1_e_object.get_object_type();
    if (g1_object_type_array[type])
    {
      add=g1_create_object(type);


      if (add.get())
      {

        add->player_num=g1_edit_state.current_team;

        i4_float gx,gy, dx,dy;
        if (!c->view_to_game(lx(),ly(), gx,gy, dx,dy))
        { gx=2; gy=2; }
    
        if (gx<0 || gy<0 ||  gx>=c->get_map()->width() || gy>=c->get_map()->height())
        { gx=2; gy=2; }

        i4_3d_point_class pos(gx,gy,c->get_map()->terrain_height(gx,gy));
        g1_edit_state.snap_point(pos);

        add->x=pos.x;
        add->y=pos.y;
        add->h=pos.z;
        add->grab_old();

        if (add->id==g1_get_object_type("lightbulb"))
        {
          add->h+=2;
          g1_light_object_class *l=g1_light_object_class::cast(add.get());
          l->setup(add->x, add->y, add->h, 1,1,1,1);
        }

        add->player_num=g1_edit_state.current_team;

        add->grab_old();

        if (!add->occupy_location())
        {
          g1_remove_man.process_requests();        
          add=0;
        }
        else
        {

          g1_object_class *list[G1_MAX_OBJECTS];
          int t=g1_get_map()->make_object_list(list, G1_MAX_OBJECTS); 

          // if we are creating a path-object, and a path object was previously selected, form a
          // link between the two
          g1_path_object_class *po=g1_path_object_class::cast(add.get()), *p2;
          if (po)
          {

            for (int i=0; i<t; i++)
            {
              if (list[i]->get_flag(g1_object_class::SELECTED))
              {
                p2=g1_path_object_class::cast(list[i]);
                if (p2)
                {
                  p2->add_link(G1_ALLY, po);
                  po->add_link(G1_ENEMY, p2);
                }
                else 
                {
                  po->add_controlled_object(list[i]);
                  for (g1_factory_class *f=g1_factory_list.first(); f; f=f->next)
                  {
                    if (f==list[i])
                      f->set_start(po);
                  }
                }
              }
            }
          } else
          {
            // see if a path object was previously selected, if so add ourself to it's
            // controlled-object list

            for (int i=0; i<t; i++)
            {
              if (list[i]->get_flag(g1_object_class::SELECTED))
              {
                po=g1_path_object_class::cast(list[i]);
                if (po)
                {
                  po->add_controlled_object(add.get());
                  for (g1_factory_class *f=g1_factory_list.first(); f; f=f->next)
                  {
                    if (f==add.get())
                      f->set_start(po);
                  }
                }
              }
            }
          }

          add->grab_old();          
        }

        c->refresh();
      }
    }
  }
}

void g1_object_mode::mouse_down()
{
  if (add.get())
  {
    hide_focus();
    g1_editor_instance.add_undo(G1_MAP_OBJECTS);
    show_focus();

    g1_object_class *list[G1_MAX_OBJECTS];
    int t=g1_get_map()->make_object_list(list, G1_MAX_OBJECTS);           
    for (int i=0; i<t; i++)
      list[i]->set_flag(g1_object_class::SELECTED, 0);    
    add->set_flag(g1_object_class::SELECTED, 1);




    add->request_think();
    g1_player_man.get(add->player_num)->add_object(add->global_id);
    add=0;
    show_focus();
  }

  g1_mode_handler::mouse_down();
}



void g1_object_mode::mouse_move(sw32 mx, sw32 my)
{
  if (!add.get())
    show_focus();

  if (add.get())
  {
    i4_float gx,gy, dx,dy;
    if (c->view_to_game(mx,my, gx,gy, dx,dy) &&
        (!(gx<0 || gy<0 ||  gx>=c->get_map()->width() || gy>=c->get_map()->height())))
    {      

      i4_3d_point_class pos(gx,gy, c->get_map()->terrain_height(gx,gy));
      g1_edit_state.snap_point(pos);

      if (pos.x!=add->x || pos.y!=add->y || pos.z!=add->h)
      {
        add->unoccupy_location();
        add->player_num=g1_edit_state.current_team;
    
        add->x=pos.x;
        add->y=pos.y;
        add->h=pos.z;
        add->grab_old();
      
        if (!add->occupy_location())
        {
          g1_remove_man.process_requests();        
          add=0;
        }
      
        c->refresh();
      }
    }
  }

  g1_mode_handler::mouse_move(mx,my);
}

static li_symbol_ref s_add_link("add_link"), s_remove_link("remove_link");

li_object *g1_add_link(li_object *o, li_environment *env)
{
  if (preselected_object.get())
  {
    li_call("redraw");
    li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));

    g1_object_class *olist[G1_MAX_OBJECTS];
    int t = g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS);    
    li_g1_ref *who=new li_g1_ref(preselected_object.get()->global_id);    
    for (int i=0; i<t; i++)
      if (olist[i]->selected())
        olist[i]->message(s_add_link.get(), who, 0);
  }
  
  return 0;
}
li_automatic_add_function(g1_add_link, "add_link");


li_object *g1_remove_link(li_object *o, li_environment *env)
{
  if (preselected_object.get())
  {
    li_call("redraw");
    li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));
      
    g1_object_class *olist[G1_MAX_OBJECTS];
    int t = g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS);    
    li_g1_ref *who=new li_g1_ref(preselected_object.get()->global_id);    
    for (int i=0; i<t; i++)
      if (olist[i]->selected())
        olist[i]->message(s_remove_link.get(), who, 0);
  }
  
  return 0;
}
li_automatic_add_function(g1_remove_link, "remove_link");


li_object *g1_fix_forward_link(li_object *o, li_environment *env)
{
  if (preselected_object.get())
  {
    li_call("redraw");
    li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));

    g1_object_class *olist[G1_MAX_OBJECTS];
    int t = g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS);    
    for (int i=0; i<t; i++)
      if (olist[i]->selected())
      {
        g1_map_piece_class *mp = g1_map_piece_class::cast(olist[i]);
        if (mp)
          mp->fix_forward_link(preselected_object.get());
      }
  }
  
  return 0;
}
li_automatic_add_function(g1_fix_forward_link, "fix_forward_link");


li_object *g1_fix_previous_link(li_object *o, li_environment *env)
{
  if (preselected_object.get())
  {
    li_call("redraw");
    li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));

    g1_object_class *olist[G1_MAX_OBJECTS];
    int t = g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS);    
    for (int i=0; i<t; i++)
      if (olist[i]->selected())
      {
        g1_map_piece_class *mp = g1_map_piece_class::cast(olist[i]);
        if (mp)
          mp->fix_previous_link(preselected_object.get());
      }
  }
  
  return 0;
}
li_automatic_add_function(g1_fix_previous_link, "fix_previous_link");


li_object *g1_fix_path_link(li_object *o, li_environment *env)
{
  g1_path_object_class *p2 = g1_path_object_class::cast(preselected_object.get());
  g1_map_piece_class *mp = 0;
  g1_path_object_class *p1 = 0;

  if (p2)
  {
    g1_object_class *olist[G1_MAX_OBJECTS];
    int t = g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS);

    for (int i=0; i<t; i++)
      if (olist[i]->selected())
      {
        g1_map_piece_class *_mp = g1_map_piece_class::cast(olist[i]);
        g1_path_object_class *_p1 = g1_path_object_class::cast(olist[i]);
        if (_mp)
          mp = _mp;
        if (_p1)
          p1 = _p1;
      }

    if (p1 && mp)
    {
      int index = p1->get_path_index(p2);

      if (index>=0)
        p1->link[index].object = mp;
    }
  }
  
  return 0;
}
li_automatic_add_function(g1_fix_path_link, "fix_path_link");


li_object *g1_fix_tick_count(li_object *o, li_environment *env)
{
  if (o)
  { 
    w32 new_tick = li_get_int(li_eval(li_car(o,env), env),env);
    g1_tick_counter = new_tick;
  }
  
  return 0;
}
li_automatic_add_function(g1_fix_tick_count, "fix_tick_count");


void g1_object_mode::key_press(i4_key_press_event_class *kev)
{
  int i,t;
  g1_object_class *olist[G1_MAX_OBJECTS];
  
  switch (kev->key)
  {
    case 'A' :
    {
      hide_focus();
      t = c->get_map()->make_object_list(olist, G1_MAX_OBJECTS);
      for (i=0; i<t; i++)  
        olist[i]->flags |= g1_object_class::SELECTED;      
      show_focus();
      c->refresh();
      break;
    }

    case '[' :
    {
      if (g1_e_object.get_object_type()>0)
      {
        int find=-1;
        for (int j=g1_e_object.get_object_type()-1; 
             find==-1 && j>0;
             j--)
        {
          if (g1_object_type_array[j] && g1_object_type_array[j]->editor_selectable())
            find=j;
        }

        if (find)
        {
          hide_focus();        
          g1_e_object.set_object_type(find);
          show_focus();
        }

      } break;
    } break;

    case ']' :
    {      
      if (g1_e_object.get_object_type()<=g1_last_object_type)
      {
        int find=-1;
        for (int j=g1_e_object.get_object_type()+1; 
             find==-1 && j<=g1_last_object_type;
             j++)
        {
          if (g1_object_type_array[j] && g1_object_type_array[j]->editor_selectable())
            find=j;
        }
          
        
        if (find)
        {
          hide_focus();
          g1_e_object.set_object_type(find);
          show_focus();
        }

      } break;
    } break;


  
  }
  g1_mode_handler::key_press(kev);
}

void g1_object_mode::delete_selected()
{
  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 t=c->get_map()->make_object_list(olist, G1_MAX_OBJECTS);
  for (int i=0; i<t; i++)
    if (olist[i]->selected())
    {
      olist[i]->unoccupy_location();
      olist[i]->request_remove();
      g1_remove_man.process_requests();
    }

  c->refresh();
}

float g1_object_mode::get_snap_theta()
{
  return real_object_theta-fmod(real_object_theta, i4_pi()/2.0);
}

void g1_object_mode::move_selected(i4_float xc, i4_float yc, i4_float zc,
                                   sw32 mouse_x, sw32 mouse_y)
{
  if (!no_more_move_undos)
  {
    g1_editor_instance.add_undo(G1_MAP_OBJECTS);
    no_more_move_undos=i4_T;
  }



  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 t=c->get_map()->make_object_list(olist, G1_MAX_OBJECTS);


  for (int i=0; i<t; i++)
    if (olist[i]->flags&g1_object_class::SELECTED)
    {
      olist[i]->unoccupy_location();

      switch (g1_e_object.get_minor_mode())
      {
        case g1_object_params::MOVE :
        {
          g1_object_class *o=olist[i];
          o->x+=xc;
          o->y+=yc;
          o->h+=zc;


        } break;

        case g1_object_params::OBJECT_ROTATE :
        {
          real_object_theta += mouse_y/300.0 * 2*3.14;

          if (g1_edit_state.snap==g1_edit_state_class::SNAP_CENTER ||
              g1_edit_state.snap==g1_edit_state_class::SNAP_ORIGIN)
            olist[i]->theta = get_snap_theta();
          else
            olist[i]->theta = real_object_theta;

        }
        break;

      }

      olist[i]->grab_old();
      olist[i]->occupy_location();
    }
  
  c->changed();
  c->refresh();
}

i4_bool g1_object_mode::select_object(sw32 mx, sw32 my, 
                                      i4_float &ox, i4_float &oy, i4_float &oz,
                                      select_modifier mod)
{
  no_more_move_undos=i4_F;

  if (mod!=FOR_CURSOR_HINT)
    g1_editor_instance.add_undo(G1_MAP_OBJECTS);

  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 t=c->get_map()->make_object_list(olist, G1_MAX_OBJECTS);
  int i;
  i4_bool ret=i4_F;
  i4_bool change=i4_F;

  preselected_object=0;
  if (!c->g1_context.transform)
    return i4_F;
  
  int set_team=-1;

  for (i=0; i<t && !ret; i++)
  {
    r1_vert rv;
    if (g1_render.project_point(i4_3d_point_class(olist[i]->x, olist[i]->y, olist[i]->h), 
                         rv, c->g1_context.transform))
      if (abs((sw32)rv.px-mx)<3 && abs((sw32)rv.py-my)<3)
      {
        ox=olist[i]->x;
        oy=olist[i]->y;
        oz=olist[i]->h;
        real_object_theta=olist[i]->theta;

        if ((olist[i]->flags & g1_object_class::SELECTED)==0 &&
            mod==CLEAR_OLD_IF_NO_SELECTION)
        {
          for (int j=0; j<t; j++)
            olist[j]->flags &= ~g1_object_class::SELECTED;
          change=i4_T;
        }


        if (mod==CLEAR_OLD_IF_NO_SELECTION || mod==ADD_TO_OLD)
        {
          olist[i]->flags |= g1_object_class::SELECTED;
          change=i4_T;

          set_team=olist[i]->player_num;
          
        }
        else if (mod==SUB_FROM_OLD)
        {
          olist[i]->flags &= ~g1_object_class::SELECTED;       
          change=i4_T;
        }
        else if (mod==FOR_CURSOR_HINT)
          preselected_object=olist[i];

        ret=i4_T;        
      }
  }

  if (set_team!=-1)
    g1_edit_state.set_current_team(set_team);


  if (change)
  {
    c->changed();
    c->refresh();
  }


  return ret;
}

void g1_object_mode::select_objects_in_area(sw32 x1, sw32 y1, sw32 x2, sw32 y2, 
                                            select_modifier mod)
{
  no_more_move_undos=i4_F;

  g1_editor_instance.add_undo(G1_MAP_OBJECTS);

  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 t=c->get_map()->make_object_list(olist, G1_MAX_OBJECTS);
 
  for (int i=0; i<t; i++)
  {
    r1_vert rv;
    if (g1_render.project_point(i4_3d_point_class(olist[i]->x, 
                                           olist[i]->y, 
                                           olist[i]->h), rv, c->g1_context.transform))
    {
      if (rv.px>=x1 && rv.px<=x2 && rv.py>=y1 && rv.py<=y2)
      {
        if (mod==SUB_FROM_OLD)
          olist[i]->flags&=~g1_object_class::SELECTED;
        else
          olist[i]->flags|=g1_object_class::SELECTED;        
      }
      else if (mod==CLEAR_OLD_IF_NO_SELECTION)
        olist[i]->flags&=~g1_object_class::SELECTED;
    }
  } 

  c->changed();
  c->refresh();
}

li_symbol_ref dbug_objs("dbug_objects");

void g1_object_mode::post_draw(i4_draw_context_class &context)
{
  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 t=c->get_map()->make_object_list(olist, G1_MAX_OBJECTS);
  int w=1;
  for (int i=0; i<t; i++)
  {
    r1_vert rv;
    i4_3d_point_class v(olist[i]->x, olist[i]->y, olist[i]->h);
    if (g1_render.project_point(v, rv, c->g1_context.transform))
    {
      w32 color=norm_color;

      li_object *dbug=li_get_value(dbug_objs.get());
      for (;dbug; dbug=li_cdr(dbug,0))
        if (li_get_int(li_car(dbug,0),0)==olist[i]->global_id)
          color=0xff0000;


      if (olist[i]->flags & g1_object_class::SELECTED)
      {
        color=sel_color;
        i4_3d_point_class floor(v.x, v.y, c->get_map()->terrain_height(v.x,v.y));
        g1_render.render_3d_line(v,floor,sel_color,norm_color,c->g1_context.transform);
      }
  
      r1_clip_clear_area((sw32)rv.px-w, (sw32)rv.py-w, (sw32)rv.px+w, (sw32)rv.py+w, 
                         color, 0.01, *c->g1_context.context, g1_render.r_api);


    }
  }

  g1_mode_handler::post_draw(context);
}

g1_object_mode::g1_object_mode(g1_controller_edit_class *c) : g1_mode_handler(c)  
{
  add=0;
  sel_color=0xffff00;
  norm_color=0x7f7f7f;
  no_more_move_undos=i4_F;
}

void g1_object_mode::edit_selected()
{
  g1_object_class *olist[G1_MAX_OBJECTS], *first=0;
  sw32 t=c->get_map()->make_object_list(olist, G1_MAX_OBJECTS), tsel=0;
  
  g1_e_object.close_edit_window();

  
  for (int i=0; i<t; i++)
  {
    if (olist[i]->selected())
    {
      editing_object=olist[i];
      tsel++;
    }        
  }

  if (tsel==1)    
  {
    i4_window_class *w=g1_object_type_array[editing_object->id]->create_edit_dialog();
    if (w)
      g1_e_object.set_edit_window(w);
  }
  else
    editing_object=0;
}

void g1_object_mode::idle()
{
  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 t=c->get_map()->make_object_list(olist, G1_MAX_OBJECTS);
  int closest=-1, closest_d=10000;

  for (int i=0; i<t; i++)
  {
    r1_vert rv;
    if (g1_render.project_point(i4_3d_point_class(olist[i]->x, olist[i]->y, olist[i]->h), 
                         rv, c->g1_context.transform))
      if (abs((sw32)rv.px-lx())<8 && abs((sw32)rv.py-ly())<8)
      {
        int d=abs((sw32)rv.px-lx()) + abs((sw32)rv.py-ly());
        if (d<closest_d)
        {
          closest=i;
          closest_d=d;
        }
      }
  }

  if (closest!=-1)
  {
    i4_str *help=olist[closest]->get_context_string();

    i4_str *h2=i4_const_str("gid=%d %S").sprintf(200,olist[closest]->global_id, help);
    g1_edit_state.context_help.show(*h2, x()+lx(), y()+ly());

    if (help)
      delete help;
    delete h2;

  } 
}



void g1_object_params::create_buttons(i4_parent_window_class *containter)
{
  i4_button_box_class *box=new i4_button_box_class(&g1_edit_state);
  char *rn[]={"oROTATE", "oZOOM", "oADD", "oSELECT", "oMOVE", "oOBJECT_ROTATE", 0 };
  w32 i=ROTATE;
  for (char **a=rn; *a; a++, i++)
    g1_edit_state.add_but(box, *a, 0, (i4_bool) i==minor_mode,
                          new g1_set_minor_mode_event("OBJECT",i));

  box->arrange_right_down();
  containter->add_child(0,0, box);
}


void g1_object_params::cleanup()
{
  if (edit_win.get())
  {
    get_style()->close_mp_window(edit_win.get());
    edit_win=0;
  }

  if (g1_object_picker_mp.get())
  {
    get_style()->close_mp_window(g1_object_picker_mp.get());
    g1_object_picker_mp=0;
  }

}


void g1_object_params::close_edit_window()
{
  if (edit_win.get())
  {
    get_style()->close_mp_window(edit_win.get());
    edit_win=0;
  }
}

void g1_object_params::set_edit_window(i4_window_class *p)
{
  close_edit_window();
  edit_win=get_style()->create_mp_window(0,0,
                                   p->width(), p->height(), 
                                   g1_ges("edit_object"), 0);
  edit_win->add_child(0,0,p);
}


g1_object_params::g1_object_params()
{
  minor_mode=MOVE; 

  current_object_type=0;
  edit_win=0;

}

g1_mode_handler *g1_object_params::create_mode_handler(g1_controller_edit_class *c)
{  
  return new g1_object_mode(c);
}



li_object *g1_join_objects(li_object *o, li_environment *env)
{
  li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));

  g1_object_class *olist[G1_MAX_OBJECTS];
  int t = g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS),i,j;

  for (i=0; i<t; i++)
  {
    if (olist[i]->get_flag(g1_object_class::SELECTED))
    {
      g1_path_object_class *p=g1_path_object_class::cast(olist[i]);
      if (p)
      {
        for (j=0; j<t; j++)
        {
          if (j!=i && olist[j]->get_flag(g1_object_class::SELECTED))
          {
            g1_path_object_class *p2=g1_path_object_class::cast(olist[j]);

            if (!p2)
            {
              p->add_controlled_object(olist[j]);
              
              for (g1_factory_class *f=g1_factory_list.first(); f; f=f->next)
              {
                if (f==olist[j])
                  f->set_start(p);
              }
            }
            else
            {
              // join to path objects if we can determine the direction
              if ((p->total_links(G1_ENEMY)>0 || p->total_links(G1_ALLY)>0) &&
                  p->total_links(G1_ALLY)==0 &&
                  p2->total_links(G1_ENEMY)==0 &&
                  p->get_path_index(p2)<0)
              {
                p->add_link(G1_ALLY, p2);
                p2->add_link(G1_ENEMY, p);
              }
            }
          }
        }          
      }

    }
  }

  return 0;
}


li_object *g1_insert_path_object(li_object *o, li_environment *env)
{
  li_call("add_undo", li_make_list(new li_int(G1_MAP_OBJECTS)));

  g1_object_class *olist[G1_MAX_OBJECTS];
  int t = g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS),i,j;
  g1_path_object_class *p1=0, *p2=0;

  for (i=0; i<t; i++)
  {
    if (olist[i]->get_flag(g1_object_class::SELECTED))
    {
      if (!p1)
        p1=g1_path_object_class::cast(olist[i]);
      else if (!p2)
        p2=g1_path_object_class::cast(olist[i]);
      else
        return 0;
    }
  }

  if (p1 && p2)
  {
    // determine if we should swap the two
    int t=p1->total_links(G1_ENEMY), swap=0;
    for (i=0; i<t; i++)
      if (p1->get_link(G1_ENEMY,i)==p2)
        swap=1;

    if (swap)
    {
      g1_path_object_class *t=p1;
      p1=p2;
      p2=p1;
    }

    g1_path_object_class *newp;
    newp=(g1_path_object_class *)g1_create_object(g1_get_object_type(path_object_type.get()));
    if (newp)


    if (!p1->remove_link(G1_ALLY, p2) ||
        !p2->remove_link(G1_ENEMY, p1))
      return 0;
      

    p1->add_link(G1_ALLY, newp);
    p2->add_link(G1_ENEMY, newp);
    newp->add_link(G1_ENEMY, p1);
    newp->add_link(G1_ALLY, p2);

    newp->x=(p1->x + p2->x)/2.0;
    newp->y=(p1->y + p2->y)/2.0;
    newp->h=(p1->h + p2->h)/2.0;
    newp->grab_old();

    g1_player_man.get(newp->player_num)->add_object(newp->global_id);
    newp->occupy_location();
    
  }

  return 0;
}

//li_automatic_add_function(g1_unjoin_objects, "join_path_ends");
li_automatic_add_function(g1_join_objects, "join_path_ends");
li_automatic_add_function(g1_insert_path_object, "insert_path_object");


static float move_v=0;
static void move_objs()
{
  g1_object_class *olist[G1_MAX_OBJECTS];
  int t = g1_get_map()->make_object_list(olist, G1_MAX_OBJECTS);
  for (int i=0; i<t; i++)
  {
    if (olist[i]->get_flag(g1_object_class::SELECTED))
    {
      olist[i]->unoccupy_location();
      olist[i]->h+=move_v;
      olist[i]->grab_old();
      olist[i]->occupy_location();
    }
  }
  li_call("redraw");
}



li_object *g1_move_object_up_start(li_object *o, li_environment *env)
{
  move_v+=0.05;
  move_objs();
  return 0;
}

li_object *g1_move_object_up_end(li_object *o, li_environment *env)
{
  move_v=0;
  return 0;
}


li_object *g1_move_object_down_start(li_object *o, li_environment *env)
{
  move_v-=0.05;
  move_objs();
  return 0;
}

li_object *g1_move_object_down_end(li_object *o, li_environment *env)
{
  move_v=0;
  return 0;
}


li_automatic_add_function(g1_move_object_up_start, "move_selected_up");
li_automatic_add_function(g1_move_object_up_end, "-move_selected_up");
li_automatic_add_function(g1_move_object_down_start, "move_selected_down");
li_automatic_add_function(g1_move_object_down_end, "-move_selected_down");

