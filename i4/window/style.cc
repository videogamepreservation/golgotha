/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "window/style.hh"
#include "palette/pal.hh"
#include "arch.hh"
#include "loaders/load.hh"
#include "font/plain.hh"
#include "font/anti_prop.hh"

i4_style_manager_class i4_style_man;

i4_icon_hint_class::i4_icon_hint_class()
{
  close_icon=i4_load_image(i4gets("close_icon"));
  up_icon=i4_load_image(i4gets("up_icon"));
  down_icon=i4_load_image(i4gets("down_icon"));
  left_icon=i4_load_image(i4gets("left_icon"));
  right_icon=i4_load_image(i4gets("right_icon"));
  plus_icon=i4_load_image(i4gets("plus_icon"));
  minus_icon=i4_load_image(i4gets("minus_icon"));
  background_bitmap=i4_load_image(i4gets("background_bitmap"));
  ok_icon=i4_load_image(i4gets("ok_icon"));
  cancel_icon=i4_load_image(i4gets("cancel_icon"));
}

i4_time_hint_class::i4_time_hint_class()
{
  i4_const_str::iterator d=i4gets("button_delay").begin();
  i4_const_str::iterator r=i4gets("button_repeat").begin();
  i4_const_str::iterator dc=i4gets("double_click").begin();
  button_delay=d.read_number();
  button_repeat=r.read_number();
  double_click=dc.read_number();
}

void i4_graphical_style_class::cleanup()
{
  if (color_hint)
  { 
    delete color_hint;
    color_hint=0;
  }

  if (font_hint)
  {
    delete font_hint;
    font_hint=0;
  }

  if (cursor_hint)
  {
    delete cursor_hint;
    cursor_hint=0;
  }

  if (icon_hint)
  {
    delete icon_hint;
    icon_hint=0;
  }

  if (time_hint)
  {
    delete time_hint;
    time_hint=0;
  }
}

void i4_graphical_style_class::uninit()
{
  cleanup();
}


void i4_graphical_style_class::init()
{
  i4_style_man.add_style(this);
}

void i4_style_manager_class::add_style(i4_graphical_style_class *which)
{
  which->next=list;
  list=which;
}

i4_graphical_style_class *i4_style_manager_class::find_style(char *name)
{
  i4_graphical_style_class *f=list;
  for (;f;f=f->next)
     if (!strcmp(f->name(),name))
       return f;
  return 0;
}

i4_graphical_style_class *i4_style_manager_class::first_style()
{ return list; }

i4_graphical_style_class *i4_style_manager_class::next_style(i4_graphical_style_class *last)
{  return last->next; }


i4_color i4_read_color_from_resource(char *name)
{
  i4_const_str::iterator i=i4gets(name).begin();

  i4_color red=i.read_number();
  i4_color grn=i.read_number();
  i4_color blue=i.read_number();

  return (red<<16)|(grn<<8)|(blue);
}



i4_color_hint_class::i4_color_hint_class()
{
  window.active.bright=i4_read_color_from_resource("window_active_bright");
  window.active.medium=i4_read_color_from_resource("window_active_medium");
  window.active.dark=i4_read_color_from_resource("window_active_dark");

  window.passive.bright=i4_read_color_from_resource("window_passive_bright");
  window.passive.medium=i4_read_color_from_resource("window_passive_medium");
  window.passive.dark=i4_read_color_from_resource("window_passive_dark");

  button=window;

  text_foreground=i4_read_color_from_resource("text_foreground");
  text_background=i4_read_color_from_resource("text_background");

  selected_text_foreground=i4_read_color_from_resource("selected_text_foreground");
  selected_text_background=i4_read_color_from_resource("selected_text_background");

  black = 0;
  white = 0xffffff;
}

i4_font_hint_class::i4_font_hint_class()
{
  i4_image_class *im=i4_load_image(i4gets("small_font"));
  normal_font=small_font=new i4_anti_proportional_font_class(im, '!');
  delete im;
}


i4_cursor_hint_class::i4_cursor_hint_class()
{
  normal=i4_load_cursor("normal_cursor", &i4_string_man);
  text=i4_load_cursor("text_cursor", &i4_string_man);
}


void i4_graphical_style_class::prepare_for_mode(const i4_pal *pal, 
                                                i4_display_class::mode *mode)
{
  cleanup();

  time_hint=new i4_time_hint_class();
  color_hint=new i4_color_hint_class();
  font_hint=new i4_font_hint_class();
  cursor_hint=new i4_cursor_hint_class();
  icon_hint=new i4_icon_hint_class();

}

i4_font_hint_class::~i4_font_hint_class() 
{  
  delete normal_font;  
  if (small_font && small_font!=normal_font)
    delete small_font;
}

i4_icon_hint_class::~i4_icon_hint_class()
{
  delete close_icon;
  delete up_icon;
  delete down_icon;
  delete left_icon;
  delete right_icon;
  delete plus_icon;
  delete minus_icon;
  delete background_bitmap;
  delete ok_icon;
  delete cancel_icon;
}
