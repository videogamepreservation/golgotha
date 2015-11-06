/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "editor/contedit.hh"
#include "controller.hh"
#include "map.hh"
#include "tile.hh"
#include "window/win_evt.hh"
#include "string/string.hh"
#include "draw_context.hh"
#include "font/font.hh"
#include "window/style.hh"
#include "resources.hh"
#include "editor/dialogs/obj_win.hh"
#include "math/pi.hh"
#include "mess_id.hh"
#include "loaders/load.hh"
#include "editor/edit_id.hh"
#include "g1_object.hh"
#include "editor/commands/fill.hh"
#include "device/keys.hh"
#include "editor/e_state.hh"
#include "window/win_evt.hh"
#include "m_flow.hh"
#include "image/color.hh"
#include "input.hh"

#include "editor/mode/e_tile.hh"
#include "editor/mode/e_camera.hh"
#include "editor/mode/e_light.hh"
#include "editor/mode/e_object.hh"
// #include "editor/mode/e_ai.hh"

#include "editor/editor.hh"
#include "r1_clip.hh"
#include "g1_render.hh"
#include "r1_api.hh"
#include "app/app.hh"
#include "window/wmanager.hh"
#include "f_tables.hh"
#include "map_vert.hh"
#include "lisp/lisp.hh"


void g1_controller_edit_class::update_cursor() 
{ 
  if (!active())
  {
    if (cursor_state!=G1_X_CURSOR)
    {
      cursor_state=G1_X_CURSOR;
      set_cursor(g1_resources.big_cursors[cursor_state]);
    }
  }
  else if (mode) 
    mode->update_cursor(); 
}



void g1_controller_edit_class::setup_context()
{  
  g1_context.window_setup(0,0,width(), height(),
                          g1_render.center_x, g1_render.center_y,
                          g1_render.scale_x, g1_render.scale_y,
                          g1_render.ooscale_x, g1_render.ooscale_y);
}

void g1_controller_edit_class::setup_mode()
{
  if (mode && active())
    mode->hide_focus();

  if (mode)
  {
    delete mode;   
  }

  mode=0;


  for (g1_mode_creator *mc=g1_mode_creator::first; mc; mc=mc->next)
    if (strcmp(g1_edit_state.major_mode, mc->name())==0)
      mode=mc->create_mode_handler(this);

  if (mode && active())
    mode->show_focus();

  refresh();


}

void g1_controller_edit_class::show_focus()
{
  if (!focus_visible)
  {
    mode->show_focus();
    focus_visible=i4_T;
  }
}

void g1_controller_edit_class::hide_focus()
{
  if (focus_visible)
  {
    mode->hide_focus();
    focus_visible=i4_F;
  }
}

void g1_controller_edit_class::refresh()
{
  i4_user_message_event_class ch(G1_MAP_CHANGED);
  i4_kernel.send_event(i4_current_app, &ch);
}

g1_controller_edit_class::~g1_controller_edit_class()
{  
  g1_edit_state.context_help.hide();

  if (active())
    unfocused();

  if (mode)
  {
    delete mode;   
    mode=0;
  }
}


void g1_controller_edit_class::unfocused()
{
  hide_focus();

  update_cursor();

  if (g1_current_controller.get()==this)
    g1_current_controller=0;
  

  request_redraw();
}

void g1_controller_edit_class::focused()
{
  setup_mode();

  g1_current_controller=this;
  
  i4_window_request_key_grab_class kgrab(this);
  i4_kernel.send_event(parent, &kgrab);

  show_focus();
  request_redraw();

  cursor_state=G1_DEFAULT_CURSOR;
  set_cursor(g1_resources.big_cursors[cursor_state]);
}

void g1_replace_cell(g1_map_class *map,
                     g1_map_cell_class &original, 
                     g1_map_cell_class &new_cell)
{
  g1_map_cell_class old;

  while (original.get_obj_list())
  {
    g1_object_class *o=original.object_list->object;
    o->unoccupy_location();

    old=original;
    original=new_cell;
    o->occupy_location();
    new_cell=original;
    original=old;        
  }

  old=original;
  original=new_cell;
  new_cell=old;
  
  I4_ASSERT(new_cell.object_list==0, "new cell list should be null");

}

void g1_controller_edit_class::replace_cell(g1_map_class *map,
                                            g1_map_cell_class &original, 
                                            g1_map_cell_class &new_cell)
{
  g1_replace_cell(map, original, new_cell);
}

void g1_controller_edit_class::restore_cell()
{
  if (need_restore_cell)
  {
    if (g1_map_is_loaded())
    {
      replace_cell(get_map(),
                   *get_map()->cell(cell_x, cell_y),
                   saved_cell);

      need_restore_cell=i4_F;

      li_call("redraw");
    }
  }
}

void g1_controller_edit_class::save_and_put_cell()
{
  if (g1_map_is_loaded())
  {
    saved_cell=*get_map()->cell(cell_x, cell_y);
    saved_cell.object_list=0;                    // don't keep the object list

    g1_map_cell_class new_cell;
    new_cell.init(g1_e_tile.get_cell_type(), 
                  g1_e_tile.get_cell_rotation(),
                  g1_e_tile.get_mirrored());

    replace_cell(get_map(), *get_map()->cell(cell_x, cell_y), new_cell);
  
    need_restore_cell=i4_T;

    li_call("redraw");
  }
}

class g1_map_fill_class : public g1_flood_fill_class
{

public:
  g1_map_class *map;
  g1_map_class *get_map() { return map; }
  w16 type, block_type;
  g1_rotation_type rotation;
  g1_player_type owner;
  i4_bool mirror;

  virtual void get_clip(sw32 &x1, sw32 &y1, sw32 &x2, sw32 &y2)
  {
    x1=y1=0;
    x2=get_map()->width()-1;
    y2=get_map()->height()-1;
  }

  i4_bool blocking(sw32 x, sw32 y)
  {
    w16 on_type=get_map()->cell(x,y)->type;
    if (on_type==block_type)
      return i4_F;
    else return i4_T;
  }

  void fill_block(sw32 x, sw32 y, sw32 startx, sw32 starty)
  { 
    g1_map_cell_class c;
    c.init(type, rotation, mirror);

    g1_replace_cell(map, *get_map()->cell(x,y), c);
  }

  g1_map_fill_class(g1_map_class *map, w16 type, 
                    g1_rotation_type rotation, 
                    i4_bool mirror,
                    g1_player_type owner,
                    w16 block_type)
    : map(map), type(type), rotation(rotation), 
      mirror(mirror),
      block_type(block_type), owner(owner)
  {}
};

void g1_controller_edit_class::fill_map()
{
  if (g1_map_is_loaded())
  {
    g1_editor_instance.add_undo(G1_MAP_CELLS);

    if (active())
      hide_focus();    


    w32 type_on=get_map()->cell(cell_x, cell_y)->type;

    if (type_on!=g1_e_tile.get_cell_type())
    {
      g1_map_fill_class f(get_map(), 
                          g1_e_tile.get_cell_type(),
                          g1_e_tile.get_cell_rotation(),
                          g1_e_tile.get_mirrored(),
                          g1_edit_state.current_team,
                          type_on);

      f.fill(cell_x, cell_y);
    }


    if (active())
      show_focus();

    changed();
  }
}

void g1_controller_edit_class::change_map_cell()
{
  g1_editor_instance.add_undo(G1_MAP_CELLS);

  graph_changed=i4_T;
  saved_cell.init(g1_e_tile.get_cell_type(), 
                  g1_e_tile.get_cell_rotation(),
                  g1_e_tile.get_mirrored());
  changed();
}


void g1_controller_edit_class::draw_3d_line(const i4_3d_point_class &p1,
                                            const i4_3d_point_class &p2,
                                            i4_color color1,
                                            i4_color color2,
                                            i4_image_class *local_image,
                                            g1_draw_context_class *context)
{
  g1_render.render_3d_line(p1, p2, color1, color2, context->transform);
}


void g1_controller_edit_class::draw_3d_point(sw32 w,
                                             i4_color color,
                                             const i4_3d_point_class &p,
                                             i4_image_class *local_image,
                                             g1_draw_context_class *context)
{
  r1_vert v;
  if (g1_render.project_point(p, v, context->transform))
    r1_clip_clear_area((sw32)v.px-w, (sw32)v.py-w, 
                       (sw32)v.px+w, (sw32)v.py+w, color, v.v.z, *context->context,
                       g1_render.r_api);
}


void g1_controller_edit_class::draw_spline(i4_image_class *local_image,
                                           g1_draw_context_class *g1_context, 
                                           i4_color cpoint_color,
                                           i4_color line_color,
                                           i4_color spline_color,
                                           i4_spline_class *s,
                                           w32 cur_frame)
{
  sw32 i;
  i4_float lx,ly,x,y,z;
  if (!s->total()) return;

  i4_spline_class::point *p;    
  i4_3d_point_class cur_p, last_p;

  for (i=0; i<s->total(); i++)
  {
    p=s->get_control_point(i);     

    cur_p=i4_3d_point_class(p->x, p->y, p->z);

    if (i!=0)
      draw_3d_line(cur_p, last_p, line_color, line_color, local_image, g1_context);
    
    last_p=cur_p;
  }


  p=s->get_control_point(0);

  w32 frame=0;

  while (p->next)
  {
    while (frame<p->next->frame)
    {
      i4_float x1,y1,z1,x2,y2,z2;

      i4_3d_point_class q1,q2;
      s->get_point(frame, x1,y1,z1);
          

      if (g1_e_camera.should_show_frames())  
        draw_3d_point(1, 0x0000ff, i4_3d_point_class(x1,y1,z1), local_image, g1_context);

      if (frame==cur_frame)
        draw_3d_point(1, 0xff00ff, i4_3d_point_class(x1,y1,z1), local_image, g1_context);

      frame++;

      s->get_point(frame, x2,y2,z2);
      draw_3d_line(i4_3d_point_class(x1,y1,z1), 
                   i4_3d_point_class(x2,y2,z2),
                   spline_color, spline_color,
                   local_image, g1_context);


    }
    p=p->next;
  }

  for (i=0; i<s->total(); i++)
  {
    i4_3d_point_class q;

    i4_spline_class::point *p=s->get_control_point(i);

    if (p->selected)
      draw_3d_point(1, 0xffff00,
                    i4_3d_point_class(p->x,p->y,p->z), local_image, g1_context);
    else
      draw_3d_point(1, 
                    cpoint_color,
                    i4_3d_point_class(p->x,p->y,p->z), local_image, g1_context);
  }


}

void g1_controller_edit_class_tile_post_cell_draw(sw32 x, sw32 y, void *context)
{
  ((g1_controller_edit_class *)context)->tile_cell_draw(x,y);
}


void g1_controller_edit_class_object_post_cell_draw(sw32 x, sw32 y, void *context)
{
  ((g1_controller_edit_class *)context)->object_cell_draw(x,y);
}

void g1_controller_edit_class::object_cell_draw(sw32 x, sw32 y)
{
  g1_map_class *map=get_map();

   int bgrade=-1;
//   for (int j=0; j<G1_GRADE_LEVELS; j++)
//   {
//     g1_block_map_class *bmap=map->get_block_map(j);
//     if (bmap && bmap->is_blocked(x,y, G1_NORTH | G1_SOUTH | G1_WEST | G1_EAST))
//       bgrade=j;
//   }

  if ((map->cell(x,y)->flags & g1_map_cell_class::IS_GROUND)==0)
    bgrade=0;

  if (map->cell(x,y)->get_solid_list())
    bgrade=1;
  else
    bgrade=-1;

  if (bgrade!=-1)
  {
    w32 c[5]={0xff0000, 0x00ff00, 0x0000ff, 0xffff00, 0xff00ff };

    float r=0.01;
    draw_3d_line(i4_3d_point_class(x,y, map->terrain_height(x,y)+r),
                 i4_3d_point_class(x+1,y+1, map->terrain_height(x+1,y+1)+r),
                 c[bgrade], c[bgrade], local_image, &g1_context);

    draw_3d_line(i4_3d_point_class(x+1,y, map->terrain_height(x+1,y)+r),
                 i4_3d_point_class(x,y+1, map->terrain_height(x,y+1)+r),
                 c[bgrade], c[bgrade], local_image, &g1_context);
  }

}

void g1_controller_edit_class::tile_cell_draw(sw32 x, sw32 y)
{
  r1_vert rv;
  g1_map_class *map=get_map();

  g1_map_vertex_class *v[4];
  v[0]=map->vertex(x,y);
  v[1]=v[0]+1;
  v[2]=v[0]+map->width()+1;
  v[3]=v[0]+map->width()+1+1;

  int vt[8]={x,y,x+1,y,x,y+1,x+1,y+1};

  object_cell_draw(x,y);

  for (w32 i=0; i<4; i++)
  {
    if (v[i]->is_selected())
    { 
      i4_3d_point_class cp(vt[i*2],vt[i*2+1],v[i]->get_height());

      if (g1_render.project_point(cp, rv, g1_context.transform))
        r1_clip_clear_area((sw32)rv.px-1, (sw32)rv.py-1, (sw32)rv.px+1, (sw32)rv.py+1, 
                           0xffff00, rv.v.z, *g1_context.context,
                           g1_render.r_api);
    }

    v[i]->set_flag(g1_map_vertex_class::WAS_DRAWN_LAST_FRAME,1);
  }

}

void g1_controller_edit_class::editor_pre_draw(i4_draw_context_class &context)
{
  g1_render.frame_ratio=0;

  if (g1_map_is_loaded() && active())
  {
    int w=(get_map()->width()+1) * (get_map()->height()+1);
    g1_map_vertex_class *v=get_map()->vertex(0,0);

    for (int i=0; i<w; i++, v++)
      v->set_flag(g1_map_vertex_class::WAS_DRAWN_LAST_FRAME, 0);
  }

  if (strcmp(g1_edit_state.major_mode,"TILE")==0 && get_map())
    get_map()->set_post_cell_draw_function(g1_controller_edit_class_tile_post_cell_draw, this);

  if (strcmp(g1_edit_state.major_mode,"OBJECT")==0 && get_map())
    get_map()->set_post_cell_draw_function(g1_controller_edit_class_object_post_cell_draw, this);


  if (g1_resources.paused)
    g1_context.draw_editor_stuff=i4_T;

  if (view.get_view_mode()==G1_EDIT_MODE)
  {
    g1_render.r_api->set_write_mode(R1_WRITE_COLOR | R1_WRITE_W);
    r1_clip_clear_area(0,0,width()-1, height()-1, 0, r1_far_clip_z, context, g1_render.r_api);
    g1_render.r_api->set_write_mode(R1_WRITE_COLOR | R1_WRITE_W | R1_WRITE_COLOR);
  }

}

void g1_controller_edit_class::editor_post_draw(i4_draw_context_class &context)
{

  g1_context.draw_editor_stuff=i4_F;

  if (g1_map_is_loaded())
    get_map()->set_post_cell_draw_function(0,0);

  mode->post_draw(context);

  if (active())
    g1_render.draw_rectangle(0,0, width()-1, height()-1, 0xffff00, context);
  else
    g1_render.draw_rectangle(0,0, width()-1, height()-1, 0, context);
}



void g1_controller_edit_class::process_input(i4_time_class tick_time)
{
  /*  if (have_focus)
  {
    if (!get_map()) return;
   
    i4_angle a=camera.ground_rotate, move_speed=1.0, pan_speed=0.2;

    if (camera.gx>=0 && camera.gy>=0 && 
        camera.gx<get_map()->width() && camera.gy<get_map()->height())
    {
      i4_float th=get_map()->terrain_height(camera.gx, camera.gy);
      i4_float camera_dist=camera.gz-th;

      if (camera_dist<6)
      {
        pan_speed*=camera_dist/6.0;
        move_speed*=camera_dist/6.0;
      }
    }



    if (g1_editor_instance.movement.rotate_left)
      rotate(0.08, 0);

    if (g1_editor_instance.movement.rotate_right)
      rotate(-0.08, 0);

    if (g1_editor_instance.movement.rotate_up)
      rotate(0, -0.08);

    if (g1_editor_instance.movement.rotate_down)
      rotate(0, 0.08);


    if (g1_editor_instance.movement.pan_down)
      pan(0,0,-pan_speed);

    if (g1_editor_instance.movement.pan_up)
      pan(0,0,pan_speed);

    if (g1_editor_instance.movement.pan_left)
      pan(-sin(a)*move_speed, cos(a)*move_speed,0);

    if (g1_editor_instance.movement.pan_right)
      pan(sin(a)*move_speed, -cos(a)*move_speed,0);

    if (g1_editor_instance.movement.pan_forward)
      pan(cos(a)*move_speed, sin(a)*move_speed,0);

    if (g1_editor_instance.movement.pan_backward)
      pan(-cos(a)*move_speed, -sin(a)*move_speed,0);


      } */
}

g1_map_vertex_class *g1_controller_edit_class::find_map_vertex(sw32 x, sw32 y,
                                                               sw32 &vx, sw32 &vy)
{
  sw32 sx,sy,ex,ey, tx,ty;
  sx=(sw32)0;  sy=(sw32)0;
  ex=(sw32)get_map()->width();  ey=(sw32)get_map()->height();

  g1_map_vertex_class *mv=get_map()->vertex(0,0);

  if (!g1_context.transform)
    return 0;

  // transform all the points in area and test proximity to mouse x,y
  for (ty=sy; ty<=ey; ty++)
    for (tx=sx; tx<=ex; tx++)
    {
      if (mv->get_flag(g1_map_vertex_class::WAS_DRAWN_LAST_FRAME))
      {
        r1_vert v;

        if (g1_render.project_point(i4_3d_point_class(tx, ty, mv->get_height()), 
                                    v, g1_context.transform))
        {
          if (abs((sw32)v.px-x)<=3 && abs((sw32)v.py-y)<=3)
          {
            vx=tx; vy=ty;
            return mv;
          }
        }      
      }
      mv++;
    }

  return 0;
}




void g1_controller_edit_class::clear_selected_verts()
{
  if (!i4_current_app->get_window_manager()->control_pressed())
  {
    i4_bool change=i4_F;

    sw32 w=(get_map()->width()+1)*(get_map()->height()+1);
    g1_map_vertex_class *v=get_map()->vertex(0,0);


    for (w32 x=0; x<w; x++, v++)
      if (v->is_selected())
      {
        change=i4_T;
        v->set_is_selected(i4_F);
      }

    if (change)
    {
      changed();
      refresh();
    }
  }
}


void g1_controller_edit_class::select_verts_in_area(sw32 x1, sw32 y1, 
                                                    sw32 x2, sw32 y2, 
                                                    g1_mode_handler::select_modifier mod)
{
  sw32 w=(get_map()->width()+1),h=(get_map()->height()+1),x,y;
  g1_map_vertex_class *v=get_map()->vertex(0,0);
  
  i4_bool change=i4_F;

  g1_editor_instance.add_undo(G1_MAP_VERTS);


  for (y=0; y<h; y++)
    for (x=0; x<w; x++, v++)
    {
      if (v->get_flag(g1_map_vertex_class::WAS_DRAWN_LAST_FRAME))
      {
        r1_vert vt;
        if (g1_render.project_point(i4_3d_point_class(x, y, v->get_height()),
                                    vt,
                                    g1_context.transform))
        {
          if (vt.px>=x1 && vt.px<=x2 && vt.py>=y1 && vt.py<=y2)
          {
            change=i4_T;

            if (mod==g1_mode_handler::SUB_FROM_OLD)
              v->set_is_selected(i4_F);
            else
              v->set_is_selected(i4_T);


          }
          else if (mod==g1_mode_handler::CLEAR_OLD_IF_NO_SELECTION)
          {
            if (v->is_selected())
            {
              change=i4_T;
              v->set_is_selected(i4_F);
            }
          }
        } else if (mod==g1_mode_handler::CLEAR_OLD_IF_NO_SELECTION)
        {
          if (v->is_selected())
          {
            change=i4_T;
            v->set_is_selected(i4_F);
          }
        }
      }
    }

  if (change)
  {
    changed();
    refresh();
  }
}


g1_controller_edit_class::g1_controller_edit_class(w16 w, w16 h,
                                                   i4_graphical_style_class *style)
  : g1_object_controller_class(w,h, style)
{    
  focus_visible=i4_F;

  move_vert_x=move_vert_y=0;
  move_vert_z=0;
  move_verts=i4_F;

  edit_win=0;
  drag_select=i4_F;
  move_points=i4_F;

  graph_changed=i4_F;
  need_restore_cell=i4_F;     
  place_object=0;
  selected_object=0;

  place_object_on_map=i4_F;
  cell_x=cell_y=0;
    
  mode=0;
  setup_mode();

}

