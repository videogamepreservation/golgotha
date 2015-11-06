/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "r1_win.hh"
#include "r1_clip.hh"

void r1_render_window_class::clip_with_z(i4_draw_context_class &context)
{
  api->set_constant_color(0);

  api->set_alpha_mode(R1_ALPHA_DISABLED);  
  api->set_shading_mode(R1_CONSTANT_SHADING);
  api->disable_texture();

  i4_rect_list_class area_to_mask;

  i4_rect_list_class::area_iter cl;
  i4_rect_list_class *clip=&context.clip;

  int dx1, dy1, dx2, dy2;
  dx1=0;  dy1=0;
  dx2=dx1 + render_area_width() -1; 
  dy2=dy1 + render_area_height() -1;


  area_to_mask.add_area(dx1, dy1, dx2, dy2);
  for (cl = clip->list.begin(); cl !=  clip->list.end(); ++cl)
    area_to_mask.remove_area(cl->x1, cl->y1, cl->x2, cl->y2);
  
  i4_float near_z = 0.0001f;
  i4_float far_z  = r1_far_clip_z;
  
  api->set_z_range(near_z,far_z);

  api->set_write_mode(R1_WRITE_W);  

  for (cl = clip->list.begin(); cl != clip->list.end(); ++cl)
    api->clear_area(cl->x1+dx1, cl->y1+dy1, cl->x2+dx1, cl->y2+dy1, 
                    api->get_constant_color(), 
                    far_z);

  for (cl = area_to_mask.list.begin(); cl!= area_to_mask.list.end(); ++cl)
    api->clear_area(cl->x1+dx1, cl->y1+dy1, cl->x2+dx1, cl->y2+dy1, 
                    api->get_constant_color(), 
                    near_z);

  api->set_write_mode(R1_WRITE_W | R1_WRITE_COLOR | R1_COMPARE_W);
}

int r1_render_window_class::render_area_width() 
{ 
  if (expand_type == R1_COPY_1x1 ||
      expand_type == R1_COPY_1x1_SCANLINE_SKIP)
    return width();
  else
    return width()/2;
}

int r1_render_window_class::render_area_height()
{
  if (expand_type == R1_COPY_1x1 || expand_type == R1_COPY_1x1_SCANLINE_SKIP)
    return height();
  else
    return height()/2;
}

void r1_render_window_class::resize(w16 new_width, w16 new_height)
{
  if (children.begin()!=children.end())
    children.begin()->resize(new_width, new_height);

  i4_parent_window_class::resize(new_width, new_height);
}

