/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __STYLE_HPP_
#define __STYLE_HPP_

#include "init/init.hh"
#include "window/window.hh"
#include "string/string.hh"
#include "video/display.hh"
#include "menu/menu.hh"
#include "font/font.hh"

class i4_font_hint_class
{
public:
  i4_font_hint_class();

  i4_font_class *normal_font;
  i4_font_class *small_font;

  ~i4_font_hint_class();
};


class i4_color_hint_class
{
public:
  i4_color black,white;

  i4_color text_foreground, 
           text_background;

  i4_color selected_text_foreground, 
           selected_text_background;

  struct bevel 
  { 
    i4_color bright, medium, dark;  
  };
  
  struct bevel_actor  { bevel active,passive; };
  
  bevel_actor window, button;

  i4_color_hint_class();

  i4_color neutral() { return window.passive.medium; }
};

class i4_cursor_hint_class
{
  i4_cursor_class *normal,*text;
public:
  i4_cursor_class *normal_cursor() const { return normal; }
  i4_cursor_class *text_cursor() const { return text; }

  i4_cursor_hint_class();
  ~i4_cursor_hint_class() 
  {
    delete normal;
    delete text;
  }
};


class i4_icon_hint_class
{
public:
  i4_image_class *close_icon;
  i4_image_class *up_icon;
  i4_image_class *down_icon;
  i4_image_class *left_icon;
  i4_image_class *right_icon;
  i4_image_class *plus_icon;
  i4_image_class *minus_icon;
  i4_image_class *ok_icon;
  i4_image_class *cancel_icon;

  i4_image_class *background_bitmap;

  i4_icon_hint_class();
  ~i4_icon_hint_class();
} ;

class i4_time_hint_class
{
public:
  w32 button_repeat,     // these are in milli seconds
      button_delay,
      double_click;

  i4_time_hint_class();
};

class i4_graphical_style_class : public i4_init_class
{
  virtual void init();
  void cleanup();

public:
  virtual void uninit();

  virtual i4_bool available_for_display(i4_display_class *whom) = 0;
  virtual char *name() = 0;

  // window will automatically be added to parent at location x,y
  // on_delete will be deleted when the window closes
  // x=-1 and y=-1 then center window in parent
  virtual i4_parent_window_class *create_mp_window(i4_coord x, i4_coord y, w16 w, w16 h, 
                                                   const i4_const_str &title,
                                                   i4_event_reaction_class *on_delete=0
                                                   ) = 0;

  virtual i4_bool close_mp_window(i4_parent_window_class *created_window) = 0;

  virtual i4_menu_class *create_menu(i4_bool hide_on_pick) = 0;

  virtual void prepare_for_mode(const i4_pal *pal, i4_display_class::mode *mode);


  virtual void get_in_deco_size(w32 &left, w32 &top, w32 &right, w32 &bottom) = 0;
  virtual void get_out_deco_size(w32 &left, w32 &top, w32 &right, w32 &bottom) = 0;

  virtual void deco_neutral_fill(i4_image_class *screen,
                                 sw32 x1, sw32 y1, sw32 x2, sw32 y2, 
                                 i4_draw_context_class &context) = 0;

  // draw a decoration around an area that looks like it's pressed into the screen
  virtual void draw_in_deco(i4_image_class *screen, 
                            i4_coord x1, i4_coord y1,
                            i4_coord x2, i4_coord y2,
                            i4_bool active,
                            i4_draw_context_class &context) = 0;

  // draw a decoration around an area that looks like it sticks out the screen
  virtual void draw_out_deco(i4_image_class *screen, 
                             i4_coord x1, i4_coord y1,
                             i4_coord x2, i4_coord y2,
                             i4_bool active,
                             i4_draw_context_class &context) = 0;

  // this will create a temporary (quick) context help window at the mouse cursor
  // you are responsible for deleting the window
  virtual i4_window_class *create_quick_context_help(int mouse_x, int mouse_y,
                                                     const i4_const_str &str) = 0;

  i4_color_hint_class  *color_hint;
  i4_font_hint_class   *font_hint;
  i4_cursor_hint_class *cursor_hint;
  i4_icon_hint_class   *icon_hint;
  i4_time_hint_class   *time_hint;

  i4_graphical_style_class *next;

  i4_graphical_style_class()
  {
    color_hint=0;
    font_hint=0;
    cursor_hint=0;
    icon_hint=0;
    time_hint=0;
  }
};


class i4_style_manager_class
{
  i4_graphical_style_class *list;
public:
  void add_style(i4_graphical_style_class *which);
  i4_graphical_style_class *find_style(char *name);
  i4_graphical_style_class *first_style();
  i4_graphical_style_class *next_style(i4_graphical_style_class *last);
};


i4_color i4_read_color_from_resource(char *name);
extern i4_style_manager_class i4_style_man;

#endif

