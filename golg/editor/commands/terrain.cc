/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "editor/editor.hh"
#include "editor/e_state.hh"
#include "gui/create_dialog.hh"
#include "gui/text_input.hh"
#include "math/random.hh"
#include "status/status.hh"
#include "editor/e_res.hh"
#include "map_vert.hh"
#include "lisp/lisp.hh"

void g1_editor_class::flatten_terrain()
{
  g1_editor_instance.unmark_all_selected_verts_for_undo_save();
  g1_editor_instance.mark_selected_verts_for_undo_save();
  g1_editor_instance.add_undo(G1_MAP_SELECTED_VERTS);


  g1_edit_state.hide_focus();

  int w=get_map()->width()+1, h=get_map()->height()+1;

  w16 lowest=0xffff;

  int t=0,x,y,i;

  i4_status_class *status=i4_create_status(g1_ges("applying_flatten"));

  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 to=get_map()->make_object_list(olist, G1_MAX_OBJECTS);
  for (i=0; i<to; i++)
    olist[i]->unoccupy_location();

  for (y=0; y<h; y++)
  {
    for (x=0; x<w; x++)
    {
      g1_map_vertex_class *v=get_map()->vertex(x,y);
      if (v->is_selected())
      {
        if (v->height<lowest)
          lowest=v->height;

        t++;
      }
    }
  }


  if (t)
  {
    for (y=0; y<h; y++)
    {
      if (status)
        status->update(y/(float)h);

      for (x=0; x<w; x++)
      {
        g1_map_vertex_class *v=get_map()->vertex(x,y);
        if (v->is_selected())
          get_map()->change_vert_height(x,y, lowest);
      }
    }
  }
  
  if (status)
    delete status;

  for (i=0; i<to; i++)
    olist[i]->occupy_location();

  g1_edit_state.show_focus();


  changed();

  li_call("redraw");
}

void g1_editor_class::smooth_terrain()
{
  g1_editor_instance.unmark_all_selected_verts_for_undo_save();
  g1_editor_instance.mark_selected_verts_for_undo_save();
  g1_editor_instance.add_undo(G1_MAP_SELECTED_VERTS);

  g1_edit_state.hide_focus();

  int w=get_map()->width()+1, h=get_map()->height()+1;

  w16 lowest=0xffff;

  int t=0,x,y,i;

  i4_status_class *status=i4_create_status(g1_ges("applying_smooth"));

  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 to=get_map()->make_object_list(olist, G1_MAX_OBJECTS);
  for (i=0; i<to; i++)
    olist[i]->unoccupy_location();


  for (y=0; y<h; y++)
  {
    if (status)
      status->update(y/(float)h);
    for (x=0; x<w; x++)
    {
      g1_map_vertex_class *v=get_map()->vertex(x,y);
      if (v->is_selected())
      {
        int t=1;
        float sum=v->get_height(); 
        
        if (x>0) { t++; sum+=v[-1].get_height(); }
        if (x<w-1) { t++; sum+=v[1].get_height(); }

        if (y>0) { t++; sum+=v[-w].get_height(); }
        if (y<h-1) { t++; sum+=v[w].get_height(); }

        sw32 avg=(sw32)(sum/(t  * 0.05));
        if (avg<0) avg=0;
        if (avg>255) avg=255;

        get_map()->change_vert_height(x,y, avg);
      }
    }
  }

  if (status)
    delete status;

  for (i=0; i<to; i++)
    olist[i]->occupy_location();

  g1_edit_state.show_focus();


  changed();

  li_call("redraw");

}


void g1_editor_class::noise_terrain()
{

  create_modal(300, 150, "terrain_noise_title");

   i4_create_dialog(g1_ges("terrain_noise_dialog"), 
                    modal_window.get(),
                    style,
                    &terrain_noise_dialog.amount,
                    vert_noise_amount,
                    this, G1_EDITOR_TERRAIN_NOISE_OK,
                    this, G1_EDITOR_MODAL_BOX_CANCEL);  
}


void g1_editor_class::noise_terrain_ok()
{
  g1_editor_instance.unmark_all_selected_verts_for_undo_save();
  g1_editor_instance.mark_selected_verts_for_undo_save();
  g1_editor_instance.add_undo(G1_MAP_SELECTED_VERTS);



  g1_edit_state.hide_focus();

  i4_str::iterator i=terrain_noise_dialog.amount->get_edit_string()->begin();
  int am=i.read_number(),x,y;
  int w=get_map()->width()+1, h=get_map()->height()+1, j;

  i4_status_class *status=i4_create_status(g1_ges("applying_noise"));

  
  g1_object_class *olist[G1_MAX_OBJECTS];
  sw32 to=get_map()->make_object_list(olist, G1_MAX_OBJECTS);

  for (j=0; j<to; j++)
    olist[j]->unoccupy_location();

  
  if (am>=1 && am<=64)
  {
    for (y=0; y<h; y++)
    {
      if (status)
        status->update(y/(float)h);

      for (x=0; x<w; x++)
      {
        g1_map_vertex_class *v=get_map()->vertex(x,y);
        if (v->is_selected())
        {
          int h=(int)v->height + (i4_rand() % am) - am/2;
          if (h<0) h=0;
          if (h>255) h=255;
          get_map()->change_vert_height(x,y, h);
        }
      }
    }
  }


  for (j=0; j<to; j++)
    olist[j]->occupy_location();

  if (status)
    delete status;

  g1_edit_state.show_focus();
  changed();
  li_call("redraw");
  close_modal();
}


