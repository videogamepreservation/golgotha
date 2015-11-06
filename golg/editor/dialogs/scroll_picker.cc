/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "editor/e_state.hh"
#include "gui/scroll_bar.hh"
#include "editor/dialogs/scroll_picker.hh"
#include "window/style.hh"
#include "gui/button.hh"
#include "tmanage.hh"
#include "r1_api.hh"
#include "tile.hh"
#include "g1_render.hh"



void g1_scroll_picker_class::create_windows()
{
  for (int i=0; i<windows.size(); i++)
    delete render_windows[i];
    
  windows.clear();
  render_windows.clear();

  int y=start_y;
  int obj_size=info->object_size;

  int on=info->scroll_offset;

  int tb=0;

  int objs_per_line=info->max_object_size/obj_size;
  int done=0;

  while (y+obj_size<=height() && !done)
  {
    int x=0;
    while (x+obj_size<=info->max_object_size && !done)
    {

      i4_window_class *w=create_window(obj_size, obj_size, on);
      if (!w)
        done=1;
      else
      {
        r1_render_window_class *rwin;
        rwin=g1_render.r_api->create_render_window(obj_size, obj_size, R1_COPY_1x1);

        windows.add(w);
        render_windows.add(rwin);

        rwin->add_child(0,0, w);

        add_child(x, y, rwin);
        on++;
        tb++;
        x+=obj_size;      
      }
    }

    y+=obj_size;
  }

  if (scroll_bar)
    scroll_bar->set_new_total((total_objects()+objs_per_line-1)/objs_per_line);  
}

g1_scroll_picker_class::g1_scroll_picker_class(i4_graphical_style_class *style,
                                               w32 option_flags,
                                               g1_scroll_picker_info *info,
                                               int total_objects)

  : i4_color_window_class(0,0, style->color_hint->neutral(), style),
    info(info),
    windows(16,16),
    render_windows(16,16)
{
  if (option_flags & (1<<ROTATE))
    add_child(0,0, g1_edit_state.create_button("tp_rotate", ROTATE, i4_T, this));

  if (option_flags & (1<<MIRROR))
    add_child(0,0, g1_edit_state.create_button("tp_mirror", MIRROR, i4_T, this));

  if (option_flags & (1<<GROW))
    add_child(0,0, g1_edit_state.create_button("tp_grow",   GROW, i4_T,  this));

  if (option_flags & (1<<SHRINK))
    add_child(0,0, g1_edit_state.create_button("tp_shrink", SHRINK, i4_T, this));
  
  arrange_down_right();
  
  resize_to_fit_children();

  int scroll_area_height=info->max_object_size * info->max_objects_down, 
    scroll_area_width=info->max_object_size;

  start_y=height();
  int tb=0;

  if (option_flags & (1<<SCROLL))
  {
    scroll_bar = new i4_scroll_bar(i4_T, scroll_area_height,
                                   scroll_area_height/info->object_size,
                                   total_objects,
                                   SCROLL, this, style);

    scroll_area_width += scroll_bar->width();
  }
  else
    scroll_bar = 0;
  


  resize(scroll_area_width > width() ? scroll_area_width : width(),  
         scroll_area_height + height());

  if (scroll_bar)
    add_child(width()-scroll_bar->width(), start_y, scroll_bar);

}

void g1_scroll_picker_class::parent_draw(i4_draw_context_class &context)
{
  i4_color_window_class::parent_draw(context);

  r1_texture_manager_class *tman=g1_render.r_api->get_tmanager();

  tman->next_frame();
}

void g1_scroll_picker_class::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::USER_MESSAGE)
  {   
    CAST_PTR(sm, i4_scroll_message, ev);
    if (sm->sub_type==SCROLL)
    {
      int obj_size=info->object_size;
      int objs_per_line=info->max_object_size/obj_size;

      int off=sm->amount * objs_per_line;

      if (off>=total_objects())
        off-=objs_per_line;

      info->scroll_offset=off;

      for (int i=0; i<windows.size(); i++)
        change_window_object_num(windows[i], off+i);
                                   
    }
  }
  else if (ev->type()==i4_event::OBJECT_MESSAGE)
  {
    CAST_PTR(om, i4_object_message_event_class, ev);
    switch (om->sub_type)
    {
      case GROW :
      {
        if (info->object_size*2<=info->max_object_size)
        {
          info->scroll_offset=0;
          info->object_size*=2;
          create_windows();
        }
      } break;

      case SHRINK :
      {
        if (info->object_size/2>=info->min_object_size)
        {
          info->scroll_offset=0;
          info->object_size/=2;
          create_windows();
        }

      } break;

      case MIRROR :
      {
        mirror();
        refresh();
      } break;

      case ROTATE :
      {
        rotate();
        refresh();
      } break;
    }
   
  }
  else
    i4_parent_window_class::receive_event(ev);
}

void g1_scroll_picker_class::refresh()
{
  for (int i=0; i<windows.size(); i++)
    windows[i]->request_redraw(i4_F);
}
