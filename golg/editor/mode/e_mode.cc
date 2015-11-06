/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "editor/mode/e_mode.hh"
#include "editor/contedit.hh"
#include "window/wmanager.hh"
#include "window/win_evt.hh"
#include "device/kernel.hh"
#include "window/style.hh"
#include "g1_render.hh"
#include "g1_limits.hh"
#include "r1_api.hh"
#include "app/app.hh"
#include "resources.hh"
#include "editor/e_state.hh"
#include "editor/editor.hh"
#include "lisp/lisp.hh"
#include "lisp/li_init.hh"

sw32 g1_mode_handler::lx() { return c->last_mouse_x; }
sw32 g1_mode_handler::ly() { return c->last_mouse_y; }

sw32 g1_mode_handler::x() { return c->x(); }
sw32 g1_mode_handler::y() { return c->y(); }


g1_mode_creator *g1_mode_creator::first=0;

void g1_mode_handler::uninit()
{
}

i4_bool g1_mode_handler::active()
{
  return c->active();
}

g1_mode_creator::g1_mode_creator()
{
  minor_mode=0;
  next=first;
  first=this;
}

g1_mode_creator::~g1_mode_creator()
{
  if (first==this)
    first=first->next;
  else
  {
    g1_mode_creator *last=0;
    for (g1_mode_creator *f=first; f!=this;)
    {
      last=f;
      f=f->next;
    }
    last->next=next;
  }
} 


i4_graphical_style_class *g1_mode_creator::get_style()
{
  return i4_current_app->get_style();
}

i4_transform_class *g1_mode_handler::get_transform()
{
  return &c->transform;
}

g1_mode_handler::g1_mode_handler(g1_controller_edit_class *c) 
  : c(c) 
{
  mouse_down_flag=i4_F;
  
  drag=i4_F;
  drag_select=i4_F;
  cursor_type=G1_DEFAULT_CURSOR;
}

void g1_mode_handler::update_cursor()
{
  float x,y,z;
  w8 nc=G1_DEFAULT_CURSOR;
  
 

  if (g1_map_is_loaded() && current_state()==DRAG_SELECT) 
  {
    if (drag)
      nc=G1_MOVE_CURSOR;
    else if (select_object(lx(), ly(), x,y,z, FOR_CURSOR_HINT))
      nc=G1_SELECT_CURSOR;
  }

  if (nc!=cursor_type)
  {
    cursor_type=nc;
    c->set_cursor(g1_resources.big_cursors[cursor_type]);
  }
}


void g1_mode_handler::mouse_move(sw32 mx, sw32 my)
{
  switch (current_state())
  {
    case DRAG_SELECT :
    {
      if (drag)   // are we dragging game stuff around?
      {
        i4_transform_class *t=&c->transform;

        i4_3d_point_class control_point;   

        t->transform(pos, control_point);

        i4_3d_vector is(1,0,0), js(0,1,0), center, i,j;
        t->inverse_transform(i4_3d_vector(0,0,0), center);
        t->inverse_transform(is, i);
        i-=center;
        t->inverse_transform(js, j); 
        j-=center;

        // i & j now for the camera plane

        i4_float csx=g1_render.scale_x;
        i4_float csy=g1_render.scale_y;



        i4_float sx=(mx-lx())/(csx*g1_render.center_x) * control_point.z;
        i4_float sy=(my-ly())/(csy*g1_render.center_y) * control_point.z;
    

        i*=sx;
        j*=sy;
   
        i4_float xc=i.x+j.x, yc=i.y+j.y, zc=i.z+j.z;

        pos.x+=xc; pos.y+=yc; pos.z+=zc;
        i4_3d_point_class current_snap_pos=pos;
        g1_edit_state.snap_point(current_snap_pos);

        move_selected(current_snap_pos.x-snap_pos.x,
                      current_snap_pos.y-snap_pos.y,
                      current_snap_pos.z-snap_pos.z,
                      mx-lx(), my-ly());    

        snap_pos=current_snap_pos;
      }
      else if (mouse_down_flag && drag_select)
      {
        sx2=mx;
        sy2=my;
        c->request_redraw();
      }
    } break;
    
    case ZOOM :
      if (mouse_down_flag)
        if (i4_current_app->get_window_manager()->shift_pressed())
          c->rotate((mx-lx())*0.01, (ly()-my)*0.01);
        else
          c->zoom((ly()-my)/50.0);
      
      break;
      
    case ROTATE :
      if (mouse_down_flag)
        if (i4_current_app->get_window_manager()->shift_pressed())
          c->zoom((ly()-my)/50.0);
        else
          c->rotate((mx-lx())*0.01, (ly()-my)*0.01);
          break; 
  }
}

void g1_mode_handler::mouse_up()
{
  if (mouse_down_flag)
  {
    ungrab_mouse();

    if (drag)
    {
      update_cursor();
      drag=i4_F;
    }
    else if (current_state()==DRAG_SELECT && drag_select)
    {
      sw32 dx1,dy1,dx2,dy2;
      if (sx1<sx2) { dx1=sx1; dx2=sx2; } else { dx1=sx2; dx2=sx1; }
      if (sy1<sy2) { dy1=sy1; dy2=sy2; } else { dy1=sy2; dy2=sy1; }

      select_modifier mod;
      if (i4_current_app->get_window_manager()->shift_pressed())
        mod=ADD_TO_OLD;
      else if (i4_current_app->get_window_manager()->alt_pressed())
        mod=SUB_FROM_OLD;
      else
        mod=CLEAR_OLD_IF_NO_SELECTION;

      select_objects_in_area(dx1,dy1,dx2,dy2, mod);
      c->request_redraw();
      drag_select=i4_F;
      update_cursor();
    }

  }
}

void g1_mode_handler::grab_mouse()
{
  mouse_down_flag=i4_T;
}

void g1_mode_handler::ungrab_mouse()
{
  mouse_down_flag=i4_F;
}

void g1_mode_handler::do_command(i4_do_command_event_class *cmd)
{
  if (!strcmp(cmd->command, "edit_selected"))
    edit_selected();  
}

void g1_mode_handler::end_command(i4_end_command_event_class *cmd)
{
  
}


void g1_mode_handler::mouse_down()
{ 
  grab_mouse();

  if (current_state()==DRAG_SELECT)
  {
    select_modifier mod;
    if (i4_current_app->get_window_manager()->shift_pressed())
      mod=ADD_TO_OLD;
    else if (i4_current_app->get_window_manager()->alt_pressed())
      mod=SUB_FROM_OLD;
    else
      mod=CLEAR_OLD_IF_NO_SELECTION;


    if (select_object(lx(), ly(), pos.x, pos.y, pos.z, mod))
    {
      snap_pos=pos;
      i4_time_class now;
      if (now.milli_diff(last_click_time)<c->style->time_hint->double_click)
        edit_selected();
      else
        drag=i4_T;
    }
    else
    {
      drag_select=i4_T;
      sx2=sx1=lx();
      sy2=sy1=ly();
    }

    update_cursor();
  }

  last_click_time.get();
}

void g1_mode_handler::post_draw(i4_draw_context_class &context)
{
  if (current_state()==DRAG_SELECT && mouse_down_flag && !drag)
    g1_render.draw_rectangle(sx1,sy1,sx2,sy2, 0xffffff, context);
}

i4_bool g1_mode_handler::pass_through_focus_click()
{
  switch (current_state())
  {
    case ZOOM :
    case ROTATE : return i4_T; break;
  }
  return i4_F;
}


li_object *g1_delete_selected(li_object *o, li_environment *env)
{
  g1_controller_edit_class *v=g1_editor_instance.get_current_view();
  if (v)
  {
    v->get_mode()->delete_selected();
    return li_true_sym;
  }
  
  return 0;
}

li_automatic_add_function(g1_delete_selected, "Map/Delete Selected");
