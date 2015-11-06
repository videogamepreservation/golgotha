/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "menu/boxmenu.hh"
#include "image/image.hh"
#include "window/style.hh"

void i4_box_menu_class::show(i4_parent_window_class *show_on, i4_coord _x, i4_coord _y)
{
  if (parent)
    i4_error("show called twice");

  win_iter m=children.begin();
  w16 hi=0,wi=0;
  i4_coord dy=2,dx=2;

  for (;m!=children.end();++m)
  {
    hi+=m->height();
    if (m->width()>wi)
      wi=m->width();

    m->private_move(dx-(m->x()-x()),dy-(m->y()-y()));
    dy+=m->height();
  }
    
  for (m=children.begin(); m!=children.end(); ++m)
    m->private_resize(wi, m->height());


  resize(wi+4,hi+4);

  show_on->add_child(_x,_y,this);
}

void i4_box_menu_class::hide()
{
  if (!parent)
    return ;
  
  parent->remove_child(this);
  parent=0;
}


void i4_box_menu_class::parent_draw(i4_draw_context_class &context)
{
  local_image->add_dirty(0,0,width()-1,height()-1,context);
    
  i4_color_hint_class::bevel *color;

  color=&style->color_hint->window.passive;
    
  i4_color b,d,m=color->medium;
  b=color->bright;
  d=color->dark;


  local_image->bar(0,0,width()-1,0,b,context);
  local_image->bar(0,0,0,height()-1,b,context);    
  local_image->bar(1,1,width()-2,1,m,context);
  local_image->bar(1,1,1,height()-2,m,context);

  local_image->bar(2,height()-2,width()-2,height()-2,d,context);
  local_image->bar(width()-2,2,width()-2,height()-2,d,context);
  local_image->bar(0,height()-1,width()-1,height()-1,style->color_hint->black,context);
  local_image->bar(width()-1,0,width()-1,height()-1,style->color_hint->black,context);

  local_image->bar(2,2,width()-3,height()-3,color->medium,context);  
}
