/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "editor/mode/e_camera.hh"
#include "editor/contedit.hh"
#include "m_flow.hh"
#include "image/color.hh"
#include "device/keys.hh"
#include "editor/dialogs/d_time.hh"
#include "g1_render.hh"
#include "editor/editor.hh"
#include "gui/butbox.hh"
#include "editor/e_res.hh"

g1_camera_params g1_e_camera;

void g1_camera_mode::edit_selected()
{
  i4_spline_class *s[g1_controller_edit_class::MAX_SPLINE_EDIT];
  int t=c->get_current_splines(s,g1_controller_edit_class::MAX_SPLINE_EDIT);

  w32 cur_frame=0;

  for (int i=0; i<t; i++)
  {
    i4_spline_class::point *sp=s[i]->begin();
    for (;sp;sp=sp->next)
    {
      if (sp->selected)
        cur_frame=sp->frame;
    }
  }

  i4_parent_window_class *p=g1_create_time_edit_window(c->style, cur_frame);
  g1_e_camera.set_edit_window(c->parent->root_window(), lx() + c->x(), ly() + c->y());
}

g1_mode_handler::state g1_camera_mode::current_state()
{
  w8 remap[]={ ROTATE, ZOOM, OTHER, OTHER, DRAG_SELECT, DRAG_SELECT };

  I4_ASSERT(g1_e_camera.get_minor_mode()<=sizeof(remap), "state too big");
  return (g1_mode_handler::state)remap[g1_e_camera.get_minor_mode()];
}

void g1_camera_mode::mouse_down()
{
  if (g1_e_camera.get_minor_mode()==g1_camera_params::ADD_CAMERA)
    c->add_movie_control_point(0);
  else if (g1_e_camera.get_minor_mode()==g1_camera_params::ADD_TARGET)
    c->add_movie_control_point(1);
  else if (g1_e_camera.get_minor_mode()==g1_camera_params::ADD_OBJECT)
    c->add_movie_control_point(2);

  g1_mode_handler::mouse_down();
}


i4_bool g1_camera_mode::select_object(sw32 mx, sw32 my, 
                                      i4_float &ox, i4_float &oy, i4_float &oz,
                                      select_modifier mod)
{
  i4_spline_class::point *sp=c->find_spline_point(mx, my, 0);

  if (sp)
  {
    
    if ((sp->selected && mod==SUB_FROM_OLD) ||
        (!sp->selected && (mod==ADD_TO_OLD || mod==CLEAR_OLD_IF_NO_SELECTION)))
    {
      g1_editor_instance.add_undo(G1_MAP_MOVIE);

      if (mod == CLEAR_OLD_IF_NO_SELECTION)
        c->clear_selected_points();
      sp->selected = !sp->selected;

      c->changed();
      c->refresh();
    }
         
    ox=sp->x;    oy=sp->y;    oz=sp->z;
    return i4_T;
  }
  else if (mod == CLEAR_OLD_IF_NO_SELECTION)
  {
    g1_editor_instance.add_undo(G1_MAP_MOVIE);
    
    c->clear_selected_points();
  }

  return i4_F;
}

void g1_camera_mode::move_selected(i4_float xc, i4_float yc, i4_float zc,
                                   sw32 mouse_x, sw32 mouse_y)
{
  if (g1_e_camera.get_minor_mode()==g1_camera_params::MOVE)
    c->move_selected_points(xc,yc,zc);
}

void g1_camera_mode::post_draw(i4_draw_context_class &context)
{
  i4_spline_class *s[g1_controller_edit_class::MAX_SPLINE_EDIT];
  int t=c->get_current_splines(s,g1_controller_edit_class::MAX_SPLINE_EDIT);

  i4_color pas_colors[3]={0x7f0000, 0x007f00, 0x00007f };
  i4_color act_colors[3]={0xff0000, 0x00ff00, 0x0000ff };
                          

  for (int i=0; i<t; i++)
  {
    w32 cf=c->get_map()->get_current_movie()->get_frame();
    
    c->draw_spline(c->local_image, &c->g1_context, 0xffffff,
                   pas_colors[i], act_colors[i], s[i], cf);
  }

  g1_mode_handler::post_draw(context);
}

void g1_camera_mode::select_objects_in_area(sw32 x1, sw32 y1, sw32 x2, sw32 y2, 
                                            select_modifier add_modifier)
{
  if (!c->get_map()->current_movie)
    return ;

  g1_editor_instance.add_undo(G1_MAP_MOVIE);

  if (!add_modifier)
    c->clear_selected_points();

  g1_movie_flow_class *flow=c->get_map()->current_movie;

  i4_bool change=i4_F;

  r1_vert rv;
  i4_spline_class::point *first=0, *last=0;
  
  i4_spline_class *s[g1_controller_edit_class::MAX_SPLINE_EDIT];
  int t=c->get_current_splines(s,g1_controller_edit_class::MAX_SPLINE_EDIT);


  for (w32 i=0; i<t; i++)
  {
    i4_spline_class::point *sp=s[i]->begin();

    for (;sp;sp=sp->next)
    {
      if (g1_render.project_point(i4_3d_point_class(sp->x, sp->y, sp->z),
                           rv,
                           c->g1_context.transform))
      {
        if (rv.px>=x1 && rv.py>=y1 && rv.px<=x2 && rv.py<=y2)
        {
          if (add_modifier==CLEAR_OLD_IF_NO_SELECTION || add_modifier==ADD_TO_OLD)
          {
            if (!sp->selected)
            {
              sp->selected=i4_T;
              change=i4_T;
            } 
          } else if (add_modifier==SUB_FROM_OLD)
          {
            sp->selected=i4_F;
            change=i4_T;
          }         
        }
      }     
    }
  }
 
  if (change)
  {
    c->refresh();  
    c->changed();
  }
}

void g1_camera_mode::delete_selected()
{
  c->delete_selected_points();
}



g1_camera_params::g1_camera_params()
{ 
  edit_win=0;
  minor_mode=MOVE;
}



void g1_camera_params::create_buttons(i4_parent_window_class *container)
{
  i4_button_box_class *box=new i4_button_box_class(&g1_edit_state);
  char *rn[]={"cROTATE", "cZOOM", 
              "cADD_CAMERA", "cADD_TARGET", "cADD_OBJECT",
              "cMODIFY", "cSELECT", 0};
  w32 i=ROTATE;
  for (char **a=rn; *a; a++, i++)
    g1_edit_state.add_but(box, *a, 0, (i4_bool) i==minor_mode,
                          new g1_set_minor_mode_event("CAMERA",i));

  box->arrange_right_down();
  container->add_child(0,0, box);
}



void g1_camera_params::cleanup()
{
  if (edit_win.get())
  {
    get_style()->close_mp_window(edit_win.get());
    edit_win=0;
  }
}

void g1_camera_params::set_edit_window(i4_parent_window_class *p, sw32 x, sw32 y)
{
  cleanup(); 
  edit_win=get_style()->create_mp_window(x,y,
                                   p->width(), p->height(), 
                                   g1_ges("edit_time"), 0);
  edit_win->add_child(0,0,p);
}

