/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "editor/e_state.hh"
#include "editor/mode/e_tile.hh"
#include "editor/e_state.hh"
#include "editor/contedit.hh"
#include "tile.hh"
#include "editor/editor.hh"
#include "height_info.hh"
#include "gui/butbox.hh"
#include "editor/editor.hh"
#include "editor/e_res.hh"
#include "map_vert.hh"

g1_tile_params g1_e_tile;

void g1_tile_mode::show_focus()
{
  if (active() && !focus_visible)
  {
    switch (g1_e_tile.get_minor_mode())
    {
      case g1_tile_params::FILL :
      case g1_tile_params::PLACE :
        c->save_and_put_cell(); 
        break;      
    }

    focus_visible=i4_T;
  }
}


void g1_tile_mode::hide_focus()
{
  if (active() && focus_visible)
  {
    switch (g1_e_tile.get_minor_mode())
    {
      case g1_tile_params::FILL :
      case g1_tile_params::PLACE :
        c->restore_cell(); 
        break;      
    }

    focus_visible=i4_F;
  }
}
void g1_tile_mode::mouse_down()
{
  if (g1_e_tile.get_minor_mode()==g1_tile_params::FILL)
  {
    g1_get_map()->mark_for_recalc(G1_RECALC_WATER_VERTS);
    c->fill_map();
  }
  else if (g1_e_tile.get_minor_mode()==g1_tile_params::PLACE)
  {
    g1_get_map()->mark_for_recalc(G1_RECALC_WATER_VERTS);
    c->change_map_cell();
  }

  g1_mode_handler::mouse_down();

}

void g1_tile_mode::mouse_move(sw32 mx, sw32 my)
{
  
  switch (g1_e_tile.get_minor_mode())
  {
    case g1_tile_params::FILL :
    case g1_tile_params::PLACE :
    {
      i4_float gx,gy, dx,dy;
      if (!c->view_to_game(mx,my, gx,gy, dx,dy))
        return;

      if (gx<0 || gy<0 ||  gx>=c->get_map()->width() || gy>=c->get_map()->height())
        return;

      if ((w32)gx!=c->cell_x ||  (w32)gy!=c->cell_y)
      {
        hide_focus();
        c->cell_x=(w32)gx;
        c->cell_y=(w32)gy;     
        show_focus();

        // if the mouse button is down, replaced the saved cell
        if (mouse_down_flag)
          c->change_map_cell();
      }     
    } break;        
  }

  g1_mode_handler::mouse_move(mx,my);

}


g1_mode_handler::state g1_tile_mode::current_state()
{
  w8 remap[]={ ROTATE, ZOOM, OTHER, OTHER, DRAG_SELECT };
  I4_ASSERT(g1_e_tile.minor_mode<=sizeof(remap), "state too big");
  return (g1_mode_handler::state)remap[g1_e_tile.get_minor_mode()];
}

void g1_tile_mode::move_selected(i4_float xc, i4_float yc, i4_float zc,
                                 sw32 mouse_x, sw32 mouse_y)
{
  vert_exact_z-=mouse_y/50.0;
  int z_int=(sw32)(vert_exact_z/0.05);
  c->move_selected_heights(z_int-move_pivot->height);

}

i4_bool g1_tile_mode::select_object(sw32 mx, sw32 my, 
                                    i4_float &ox, i4_float &oy, i4_float &oz,
                                    select_modifier mod)
{
  

  sw32 x,y;
  g1_map_vertex_class *v=c->find_map_vertex(mx, my, x, y);

  if (mod!=FOR_CURSOR_HINT)
  {
    g1_editor_instance.unmark_all_selected_verts_for_undo_save();
    if (v)
      v->set_need_undo(i4_T);

    g1_editor_instance.mark_selected_verts_for_undo_save();
    g1_editor_instance.add_undo(G1_MAP_SELECTED_VERTS);    
  }


  if (v)
  {
    ox=x; oy=y;
    oz=c->get_map()->vertex(x,y)->get_height();

    if (!v->is_selected() && mod==CLEAR_OLD_IF_NO_SELECTION)
    {
      c->clear_selected_verts();

      c->changed();
      c->refresh();
    }

    if (mod==CLEAR_OLD_IF_NO_SELECTION || mod==ADD_TO_OLD)
    {
      v->set_is_selected(i4_T);
      c->changed();
      c->refresh();

      move_pivot=v;
      vert_exact_z=c->get_map()->vertex(x,y)->get_height();
    }     


    return i4_T;
  }
  else
  {
    if (mod==CLEAR_OLD_IF_NO_SELECTION)
      c->clear_selected_verts();

    return i4_F;
  }
}

void g1_tile_mode::select_objects_in_area(sw32 x1, sw32 y1, sw32 x2, sw32 y2, 
                                          select_modifier mod)
{
  c->select_verts_in_area(x1,y1,x2,y2, mod);
}


void g1_tile_mode::key_press(i4_key_press_event_class *kev)
{
  switch (kev->key)
  {
    case '1' :
    {
      hide_focus();
      int lsize=g1_get_map()->width()*g1_get_map()->height();
      g1_map_cell_class *c=g1_get_map()->cell(0,0);

      for (int i=0; i<lsize; i++, c++)
      {
        int rot=c->get_rotation();
        int remap[4]={1,0,3,2};    
        rot=remap[rot];
        c->set_rotation((g1_rotation_type)rot);
      }
      show_focus();
    } break;

    case '`' :
    {
      hide_focus();

      g1_map_cell_class *cell=c->get_map()->cell(c->cell_x, c->cell_y);

      g1_e_tile.set_cell_type(cell->type);
      g1_e_tile.set_cell_rotation(cell->get_rotation());
      g1_e_tile.set_mirrored(cell->mirrored());
      
      if (g1_e_tile.get_minor_mode()!=g1_tile_params::PLACE &&
          g1_e_tile.get_minor_mode()!=g1_tile_params::FILL)
        g1_e_tile.set_minor_mode(g1_tile_params::PLACE);

      i4_warning("rotation = %d, mirrored = %d", cell->get_rotation(), cell->mirrored());
      
      show_focus();
    } break;


    case '\'':
    {
      hide_focus();
      g1_e_tile.set_mirrored((i4_bool)!g1_e_tile.get_mirrored());

      show_focus();
    } break;

    case '[' :
    {
      if (g1_e_tile.get_minor_mode()==g1_tile_params::PLACE)
      {
        if (g1_e_tile.get_cell_type()>0)
        {
          hide_focus();
          g1_e_tile.set_cell_type(g1_e_tile.get_cell_type()-1);
          show_focus();
        } 
      }

      if (g1_e_tile.get_minor_mode()==g1_tile_params::HEIGHT)
        c->move_selected_heights(-1);

    } break;

    case ']' :
    {
      if (g1_e_tile.get_minor_mode()==g1_tile_params::PLACE)
      {
        if (g1_e_tile.get_cell_type()+1<g1_tile_man.total())
        {
          hide_focus();
          g1_e_tile.set_cell_type(g1_e_tile.get_cell_type()+1);
          show_focus();
        } 
      }
      if (g1_e_tile.get_minor_mode()==g1_tile_params::HEIGHT)
        c->move_selected_heights(1);

    } break;

    
    case '\\' :    
    {
      hide_focus();
      if (g1_e_tile.get_cell_rotation()==G1_ROTATE_270)
        g1_e_tile.set_cell_rotation(G1_ROTATE_0);
      else      
        g1_e_tile.set_cell_rotation((g1_rotation_type)(g1_e_tile.get_cell_rotation()+1));
      show_focus();


    } break;

  }
}

g1_tile_mode::g1_tile_mode(g1_controller_edit_class *c)
  : g1_mode_handler(c)
{
  c->cell_x=0;
  c->cell_y=0;
  focus_visible=i4_F;
}


g1_tile_params::g1_tile_params() 
{ 
  vert_move_snap=1;
  picker_mp_window=0;
  picker=0;

  minor_mode=ROTATE;
  current_cell_type=0;
  current_cell_rotation=G1_ROTATE_0;   
  mirrored=i4_F;
}


void g1_tile_params::refresh_picker()
{
  if (picker.get())
    picker->refresh();
}


void g1_tile_params::create_buttons(i4_parent_window_class *container)
{
  i4_button_box_class *box=new i4_button_box_class(&g1_edit_state);
  char *rn[]={"tROTATE", "tZOOM", "tPLACE", "tFILL", "tHEIGHT", 0 };
  w32 i=ROTATE;
  for (char **a=rn; *a; a++, i++)
    g1_edit_state.add_but(box, *a, 0, (i4_bool) i==minor_mode,
                          new g1_set_minor_mode_event("TILE",i));

  box->arrange_right_down();
  container->add_child(0,0, box);
}


void g1_tile_params::cleanup()
{
  if (mp_window.get())
  {
    get_style()->close_mp_window(mp_window.get());
    picker=0;
  }
}

void g1_tile_params::open_picker()
{
  cleanup();

  if (g1_e_tile.allow_picker_creation())
  {
    picker=new g1_tile_picker_class(get_style(), &picker_info, 0,0);
    picker->create_windows();

    i4_parent_window_class *mpw;
    mpw=get_style()->create_mp_window(picker_info.win_x, picker_info.win_y,
                                      picker->width(), picker->height(),
                                      g1_ges("tile_pick_title"));

    mpw->add_child(0,0, picker.get());
    mp_window=mpw;

  }
}
