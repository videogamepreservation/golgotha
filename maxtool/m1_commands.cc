/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "m1_info.hh"
#include "st_edit.hh"
#include "render.hh"
#include "lisp/lisp.hh"
#include "lisp/li_init.hh"
#include "max_object.hh"
#include "file/file.hh"


li_object *m1_wireframe_toggle(li_object *o, li_environment *env)
{
  m1_info.set_flags(M1_WIREFRAME, ~m1_info.get_flags(M1_WIREFRAME));
  m1_render_window->request_redraw();
  return 0;
}

li_object *m1_select_none(li_object *o, li_environment *env)
{
  // deselect all the old polys
  for (int i=0; i<m1_info.obj->num_quad; i++)
    m1_info.obj->quad[i].set_flags(g1_quad_class::SELECTED,0);

  m1_render_window->request_redraw();
  m1_st_edit->edit_poly_changed();

  return 0;
}

li_object *m1_select_similar(li_object *o, li_environment *env)
{
  i4_str *texture=0;
  int i,j;

  for (i=0; i<m1_info.obj->num_quad; i++)
    if (m1_info.obj->quad[i].get_flags(g1_quad_class::SELECTED))
    {
      texture = m1_info.obj->texture_names[i];
      
      // non-optimal search (lots of reselection)
      for (j=0; j<m1_info.obj->num_quad; j++)
        if (*m1_info.obj->texture_names[j] == *texture)
          m1_info.obj->quad[j].set_flags(g1_quad_class::SELECTED);
    }
  
  if (!texture)
    return 0;
  
  m1_render_window->request_redraw();
  m1_st_edit->edit_poly_changed();
    
  return 0;
}

li_object *m1_rotate_texture_selected(li_object *o, li_environment *env)
{
  for (int i=0; i<m1_info.obj->num_quad; i++)
  {
    g1_quad_class *q = &m1_info.obj->quad[i];
    if (q->get_flags(g1_quad_class::SELECTED))
    {
      i4_float u=q->u[0],v=q->v[0], nu,nv;
      for (int j=q->num_verts()-1; j>=0; j--)
      {
        nu = q->u[j]; nv = q->v[j];
        q->u[j] = u; q->v[j] = v;
        u = nu; v = nv;
      }
    }
  }
  m1_info.obj->calc_texture_scales();
  m1_render_window->request_redraw();


  return 0;
}

li_object *m1_reverse_texture_selected(li_object *o, li_environment *env)
{
  for (int i=0; i<m1_info.obj->num_quad; i++)
  {
    g1_quad_class *q = &m1_info.obj->quad[i];
    if (q->get_flags(g1_quad_class::SELECTED))
    {
      int j = q->num_verts()-1;
      i4_float t;

      // swap uv coords
      t = q->u[1];
      q->u[1] = q->u[j];
      q->u[j] = t;
      t = q->v[1];
      q->v[1] = q->v[j];
      q->v[j] = t;
    }
  }
  m1_render_window->request_redraw();
  m1_info.obj->calc_texture_scales();

  return 0;
}

li_object *m1_flip_normal_selected(li_object *o, li_environment *env)
{
  for (int i=0; i<m1_info.obj->num_quad; i++)
  {
    g1_quad_class *q = &m1_info.obj->quad[i];
    if (q->get_flags(g1_quad_class::SELECTED))
    {
      int j = q->num_verts()-1;
      i4_float t;
      w16 ref;

      // swap references
      ref = q->vertex_ref[1];
      q->vertex_ref[1] = q->vertex_ref[j];
      q->vertex_ref[j] = ref;

      // swap uv coords
      t = q->u[1];
      q->u[1] = q->u[j];
      q->u[j] = t;
      t = q->v[1];
      q->v[1] = q->v[j];
      q->v[j] = t;
    }
  }
  m1_info.obj->calc_texture_scales();
  m1_render_window->request_redraw();

  return 0;
}

li_object *m1_default_coordinates(li_object *o, li_environment *env)
{
  for (int i=0; i<m1_info.obj->num_quad; i++)
  {
    g1_quad_class *q = &m1_info.obj->quad[i];
    if (q->get_flags(g1_quad_class::SELECTED))
    {
      q->u[0] = 0;
      q->v[0] = 0;
      q->u[1] = 1;
      q->v[1] = 0;
      q->u[2] = 1;
      q->v[2] = 1;
      q->u[3] = 0;
      q->v[3] = 1;
    }
  }
  m1_info.obj->calc_texture_scales();
  m1_render_window->request_redraw();

  return 0;
}

li_object *m1_tint_selected(li_object *o, li_environment *env)
{
  for (int i=0; i<m1_info.obj->num_quad; i++)
  {
    if (m1_info.obj->quad[i].get_flags(g1_quad_class::SELECTED))
      m1_info.obj->quad[i].set_flags(g1_quad_class::TINT);
  }
  m1_render_window->request_redraw();

  return 0;
}

li_object *m1_untint_selected(li_object *o, li_environment *env)
{
  for (int i=0; i<m1_info.obj->num_quad; i++)
  {
    if (m1_info.obj->quad[i].get_flags(g1_quad_class::SELECTED))
      m1_info.obj->quad[i].set_flags(g1_quad_class::TINT,0);
  }
  m1_render_window->request_redraw();

  return 0;
}

li_object *m1_delete_selected(li_object *o, li_environment *env)
{
  m1_poly_object_class *obj=m1_info.obj;
  if (obj)
  {
    int shift=0;

    for (int i=0; i<obj->num_quad; i++)
    {
      if (obj->quad[i].get_flags(g1_quad_class::SELECTED))
        shift++;
      else
        if (shift>0)
        {
          obj->quad[i-shift]=obj->quad[i];
          obj->texture_names[i-shift]=obj->texture_names[i];
        }
    }
    obj->num_quad-=shift;

    m1_render_window->request_redraw();
  }
  return 0;
}

li_object *m1_join_coords(li_object *o, li_environment *env)
{
  m1_poly_object_class *obj=m1_info.obj;

  i4_float u,v;
  int set=0;

  if (obj)
  {
    for (int j=0; j<obj->num_quad; j++)
    {
      g1_quad_class *q=obj->quad+j;
      if (q->get_flags(g1_quad_class::SELECTED))
        for (int i=0; i<q->num_verts(); i++)
          if (obj->get_poly_vert_flag(j, 1<<i))
            if (set)
            {
              q->u[i] = u;
              q->v[i] = v;
            }
            else
            {
              set = 1;
              u = q->u[i];
              v = q->v[i];
            }
    }

    m1_render_window->request_redraw();
  }
  return 0;
}


li_object *m1_team1(li_object *o, li_environment *env)
{
  m1_info.current_team=1;
  m1_render_window->request_redraw();
  return 0;
}


li_object *m1_team2(li_object *o, li_environment *env)
{
  m1_info.current_team=2;
  m1_render_window->request_redraw();
  return 0;
}



li_object *m1_team3(li_object *o, li_environment *env)
{
  m1_info.current_team=3;
  m1_render_window->request_redraw();
  return 0;
}



li_object *m1_team4(li_object *o, li_environment *env)
{
  m1_info.current_team=4;
  m1_render_window->request_redraw();
  return 0;
}


li_object *m1_team_default(li_object *o, li_environment *env)
{
  m1_info.current_team=0;
  m1_render_window->request_redraw();
  return 0;
}

li_object *m1_back_black(li_object *o, li_environment *env)
{
  m1_info.bg_color=0;
  m1_render_window->request_redraw();
  return 0;
}

li_object *m1_back_red(li_object *o, li_environment *env)
{
  m1_info.bg_color=0xff0000;
  m1_render_window->request_redraw();
  return 0;
}

li_object *m1_back_white(li_object *o, li_environment *env)
{
  m1_info.bg_color=0xffffff;
  m1_render_window->request_redraw();
  return 0;
}

li_object *m1_back_blue(li_object *o, li_environment *env)
{
  m1_info.bg_color=0xff;
  m1_render_window->request_redraw();
  return 0;
}

li_object *m1_back_darkblue(li_object *o, li_environment *env)
{
  m1_info.bg_color=0x2f;
  m1_render_window->request_redraw();
  return 0;
}

li_object *m1_back_green(li_object *o, li_environment *env)
{
  m1_info.bg_color=0xff00;
  m1_render_window->request_redraw();
  return 0;
}


li_object *m1_recenter(li_object *o, li_environment *env)
{
  m1_render_window->recenter();
  return 0;
}

li_object *m1_axis_toggle(li_object *o, li_environment *env)
{
  m1_info.set_flags(M1_SHOW_AXIS, ~m1_info.get_flags(M1_SHOW_AXIS));
  m1_render_window->request_redraw();
  return 0;
}


li_object *m1_toggle_shading(li_object *o, li_environment *env)
{
  m1_info.set_flags(M1_SHADING, ~m1_info.get_flags(M1_SHADING));
  m1_render_window->request_redraw();
  return 0;
}

li_object *m1_toggle_names(li_object *o, li_environment *env)
{
  m1_info.set_flags(M1_SHOW_FACE_NAMES, ~m1_info.get_flags(M1_SHOW_FACE_NAMES));
  m1_render_window->request_redraw();
  return 0;
}



li_object *m1_toggle_numbers(li_object *o, li_environment *env)
{
  m1_info.set_flags(M1_SHOW_FACE_NUMBERS, ~m1_info.get_flags(M1_SHOW_FACE_NUMBERS));
  m1_render_window->request_redraw();
  return 0;
}


li_object *m1_toggle_vnumbers(li_object *o, li_environment *env)
{
  m1_info.set_flags(M1_SHOW_VERT_NUMBERS, ~m1_info.get_flags(M1_SHOW_VERT_NUMBERS));
  m1_render_window->request_redraw();
  return 0;
}


li_object *m1_swap_polynums(li_object *o, li_environment *env)
{
  int q1=-1,q2=-1;
  for (int i=0; i<m1_info.obj->num_quad; i++)
  {
    g1_quad_class *q = &m1_info.obj->quad[i];
    if (q->get_flags(g1_quad_class::SELECTED))
      if (q1==-1)
        q1=i;
      else if (q2==-1)
        q2=i;
      else return 0;
  }
   
  if (q1!=-1 && q2!=-1)
  {
    m1_poly_object_class *obj=m1_info.obj;

    i4_str *tmp=obj->texture_names[q1]; 
    obj->texture_names[q1]=obj->texture_names[q2];
    obj->texture_names[q2]=tmp;


    g1_quad_class tq;
    tq=obj->quad_store[q1];
    obj->quad_store[q1]=obj->quad_store[q2];
    obj->quad_store[q2]=tq;

    obj->quad[q1]=obj->quad_store[q1];
    obj->quad[q2]=obj->quad_store[q2];
  }
  return 0;
}


li_object *m1_dump_polys(li_object *o, li_environment *env)
{
  i4_file_class *fp=i4_open("dump.scm", I4_WRITE);
  if (!fp) return 0;

  for (int i=0; i<m1_info.obj->num_quad; i++)
  {
    g1_quad_class *q = &m1_info.obj->quad[i];
   
    char tname[256];
    i4_os_string(*m1_info.obj->texture_names[i], tname,  256);

    int first=1, j;
    if (q->u[0]>=0.47 &&
        q->u[1]>=0.47 &&
        q->u[2]>=0.47 &&
        q->u[3]>=0.47)
      first=0;


    for (j=0; j<4; j++)
      if (first)
      {
        q->u[j]*=2;
        m1_info.obj->texture_names[i]=new i4_str("x:\\crack\\golgotha\\textures\\nuk_sky1.tga");
      }
      else
      {
        q->u[j]=q->u[j]*2-1.0;
        m1_info.obj->texture_names[i]=new i4_str("x:\\crack\\golgotha\\textures\\nuk_sky1.tga");
      }
    

    fp->printf("(setup_quad %d %s %f %f  %f %f  %f %f  %f %f)\n",
               i, tname, 
               q->u[0], q->v[0],
               q->u[1], q->v[1],
               q->u[2], q->v[2],
               q->u[3], q->v[3]);
  }
  delete fp;
  return 0;
}

li_automatic_add_function(m1_dump_polys, "dump_polys");

li_automatic_add_function(m1_axis_toggle, "axis_toggle");
li_automatic_add_function(m1_recenter, "recenter");
li_automatic_add_function(m1_select_none, "select_none");
li_automatic_add_function(m1_select_similar, "select_similar");
li_automatic_add_function(m1_wireframe_toggle, "wireframe");
li_automatic_add_function(m1_rotate_texture_selected, "rotate_texture_sel");
li_automatic_add_function(m1_reverse_texture_selected, "reverse_texture_sel");
li_automatic_add_function(m1_flip_normal_selected, "flip_normal_sel");
li_automatic_add_function(m1_default_coordinates, "default_coords");
li_automatic_add_function(m1_tint_selected, "tint_sel");
li_automatic_add_function(m1_untint_selected, "untint_sel");
li_automatic_add_function(m1_delete_selected, "delete_sel");
li_automatic_add_function(m1_join_coords, "join_coords");

li_automatic_add_function(m1_team1, "team_1");
li_automatic_add_function(m1_team2, "team_2");
li_automatic_add_function(m1_team3, "team_3");
li_automatic_add_function(m1_team4, "team_4");
li_automatic_add_function(m1_team_default, "no_tint");
li_automatic_add_function(m1_toggle_shading, "toggle_shading");

li_automatic_add_function(m1_back_black, "back_black");
li_automatic_add_function(m1_back_red, "back_red");
li_automatic_add_function(m1_back_white, "back_white");
li_automatic_add_function(m1_back_blue, "back_blue");
li_automatic_add_function(m1_back_darkblue, "back_darkblue");
li_automatic_add_function(m1_back_green, "back_green");


li_automatic_add_function(m1_toggle_names, "toggle_names");
li_automatic_add_function(m1_toggle_numbers, "toggle_numbers");
li_automatic_add_function(m1_toggle_vnumbers, "toggle_vnumbers");
li_automatic_add_function(m1_swap_polynums, "swap_polynums");

