/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "editor/contedit.hh"
#include "m_flow.hh"
#include "input.hh"
#include "g1_speed.hh"
#include "objs/stank.hh"
#include "g1_render.hh"
#include "editor/editor.hh"

i4_spline_class::point *g1_controller_edit_class::find_spline_point(sw32 mx, sw32 my, 
                                                                    w32 instance)
{
  i4_spline_class *s[MAX_SPLINE_EDIT];
  int t=get_current_splines(s,MAX_SPLINE_EDIT);


  r1_vert rv;
  i4_spline_class::point *first=0, *last=0;


  for (w32 i=0; i<t; i++)
  {
    i4_spline_class::point *c=s[i]->begin();
    for (;c;c=c->next)
    {
      if (g1_render.project_point(i4_3d_point_class(c->x, c->y, c->z),
                                  rv,
                                  g1_context.transform))
      {
        if (abs((sw32)rv.px-mx)<3 && abs((sw32)rv.py-my)<3)
        {
          if (instance)
            instance--;
          else
            return c;
        }
      }          
    }        
  }
  return 0;
}


void g1_controller_edit_class::clear_selected_points()
{
  i4_bool change=i4_F;

  g1_editor_instance.add_undo(G1_MAP_MOVIE);

  i4_spline_class *s[MAX_SPLINE_EDIT];
  int t=get_current_splines(s,MAX_SPLINE_EDIT);

  i4_spline_class::point *c;

  for (int i=0; i<t; i++)
    for (c=s[i]->begin(); c; c=c->next) 
      if (c->selected)
      {
        c->selected=i4_F;
        change=i4_T;
      }
  
  if (change)
    changed();
}

void g1_controller_edit_class::select_points_in_area(sw32 x1, sw32 y1, sw32 x2, sw32 y2, 
                                                     g1_mode_handler::select_modifier)
{
  /*  clear_selected_points();

  if (!map->current_movie)
    return ;

  g1_movie_flow_class *flow=map->current_movie;

  sw32 dx1,dy1,dx2,dy2;
  drag_area(dx1,dy1,dx2,dy2);
  i4_bool change=i4_F;

  float x,y;
  i4_spline_class::point *first=0, *last=0;
  

  if (flow && flow->current())
  {
    for (w32 i=0; i<2; i++)
    {
      i4_spline_class::point *c;
      if (i==0) 
        c=flow->current()->camera.begin();
      else
        c=flow->current()->target.begin();

      for (;c;c=c->next)
      {
        if (project_point(i4_3d_point_class(c->x, c->y, c->z),
                          x,y,
                          &g1_context))
        {
          if (x>=dx1 && y>=dy1 && x<=dx2 && y<=dy2)
          {
            if (!c->selected)
            {
              c->selected=i4_T;
              change=i4_T;
            }
          }
        }
      }
    }
  }
 
  if (change)
  {
    refresh();  
    changed();
  }*/
}


void g1_controller_edit_class::add_movie_control_point(int list_number)
{
  g1_editor_instance.add_undo(G1_MAP_MOVIE);

  i4_spline_class *s[MAX_SPLINE_EDIT];
  w32 list_number_2;
  int t=get_current_splines(s,MAX_SPLINE_EDIT);
  
  i4_float h=0;
  if (list_number == g1_cut_scene_class::CAMERA)
    h=2;
  else if (list_number == g1_cut_scene_class::TARGET)
    h=0.2;

  if (list_number<t)
  {    
    i4_float gx,gy, dx,dy;
    if (!view_to_game(last_mouse_x, last_mouse_y, gx,gy, dx,dy))
      return;

    h+=get_map()->terrain_height(gx, gy);

#if 0 //movie hack that no longer works
    if (g1_input.button_1() && list_number==g1_cut_scene_class::CAMERA)
    {
      
      list_number_2 = g1_cut_scene_class::TARGET;

      g1_player_piece_class *p;
      p=(g1_player_piece_class *)get_map()->find_object_by_id(g1_supertank_type, 
                                                              g1_default_player);
      gx = p->x;
      gy = p->y;
      h  = p->h;

      i4_3d_point_class view_dir,view_adj;

      i4_transform_class tank_trans,tmp;
      tank_trans.identity();      
      
      p->cam_pitch = p->groundpitch;//*sin(p->base_angle) + p->groundroll*cos(p->base_angle);
      p->cam_roll  = p->groundroll;// *sin(p->base_angle) - p->groundpitch*cos(p->base_angle);
      
      tmp.rotate_z((p->base_angle));
      tank_trans.multiply(tmp);

      tmp.rotate_y((p->cam_pitch));
      tank_trans.multiply(tmp);

      tmp.rotate_x((p->cam_roll));
      tank_trans.multiply(tmp);

      tank_trans.transform(i4_3d_point_class(1,0,0),view_dir);
      tank_trans.transform(i4_3d_point_class(0.25,0.023,0.15),view_adj);
      
      gx += view_adj.x;
      gy += view_adj.y;
      h  += view_adj.z;

      view_dir.x += gx;
      view_dir.y += gy;
      view_dir.z += h;

      if (s[list_number_2]->total())
      {
        w32 last_time=s[list_number_2]->get_control_point(s[list_number_2]->total()-1)->frame;
        s[list_number_2]->add_control_point(view_dir.x, view_dir.y, view_dir.z, last_time + G1_MOVIE_HZ);
      }
      else
        s[list_number_2]->add_control_point(view_dir.x, view_dir.y, view_dir.z, 0);
    }
#endif

    if (s[list_number]->total())
    {
      w32 last_time=s[list_number]->get_control_point(s[list_number]->total()-1)->frame;
      s[list_number]->add_control_point(gx, gy, h, last_time + G1_MOVIE_HZ);
    }
    else
      s[list_number]->add_control_point(gx, gy, h, 0);

    refresh();
    changed();
  }
}

void g1_controller_edit_class::delete_selected_points()
{
  g1_editor_instance.add_undo(G1_MAP_MOVIE);

  i4_spline_class *s[MAX_SPLINE_EDIT];
  int t=get_current_splines(s,MAX_SPLINE_EDIT);
  for (int i=0; i<t; i++)    
    s[i]->delete_selected();

  refresh();
  changed();
}

void g1_controller_edit_class::insert_control_points()
{
  g1_editor_instance.add_undo(G1_MAP_MOVIE);

  i4_spline_class *s[MAX_SPLINE_EDIT];
  int t=get_current_splines(s,MAX_SPLINE_EDIT);

  for (int i=0; i<t; i++)    
    s[i]->insert_control_points();

  refresh();
  changed();
}

void g1_controller_edit_class::move_selected_points(i4_float xa, i4_float ya, i4_float za)
{
  g1_editor_instance.add_undo(G1_MAP_MOVIE);

  i4_spline_class *s[MAX_SPLINE_EDIT];
  int t=get_current_splines(s,MAX_SPLINE_EDIT);

  for (int i=0; i<t; i++)
  {
    i4_spline_class::point *c=s[i]->begin();

    for (;c;c=c->next)
    {
      if (c->selected)
      {
        c->x+=xa;
        c->y+=ya;
        c->z+=za;
      }
    }

    changed();
    refresh();  
  }  
}




