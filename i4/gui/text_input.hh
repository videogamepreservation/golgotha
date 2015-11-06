/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef TEXT_INPUT_HH
#define TEXT_INPUT_HH

#include "window/window.hh"
#include "window/style.hh"
#include "string/string.hh"
#include "time/timedev.hh"

class i4_text_change_notify_event : public i4_object_message_event_class
{
public:
  i4_str *new_text;

  i4_text_change_notify_event(void *object,
                              i4_str *str) 
    : i4_object_message_event_class(object),
      new_text(new i4_str(*str)) { }
               
  virtual i4_event *copy() { return new i4_text_change_notify_event(object, new_text); }
  virtual dispatch_time when()  { return NOW; } 
  ~i4_text_change_notify_event() { delete new_text; }
};


class i4_query_text_input_class : public i4_query_message_event_class
{  
public:

  i4_str *copy_of_data;

  i4_query_text_input_class() { copy_of_data=0; }

  virtual i4_event  *copy() 
  {
    i4_query_text_input_class *t=new i4_query_text_input_class;
    if (copy_of_data)
      t->copy_of_data=new i4_str(*copy_of_data,copy_of_data->length()+1);
    return t;
  }

  ~i4_query_text_input_class()
  {
    if (copy_of_data)
      delete copy_of_data;
  }

} ;

class i4_text_input_class : public i4_window_class
{
protected:
  i4_event_handler_class *change_notify;
  i4_str *st;
  i4_font_class *font;
  int max_width;

  sw32 cursor_x;       // position of the cursor
  sw32 left_x;         // how many characters to skip in string before drawing at left side

  i4_bool need_cancel_blink;             // do we have a timer event currently?
  i4_bool cursor_on;                     // this gets toggled by the timer event
  i4_time_device_class::id blink_timer;  // if we have a time ever scheduled
  i4_bool selecting;
  i4_coord last_x,last_y;
  i4_bool changed, sent_change;


  sw32 hi_x1, hi_x2;   // hilight start and end
  i4_graphical_style_class *style;
  i4_bool selected;

  // this will find the character the mouse is on, this should work for non-constantly spaced
  // character strings as well
  w32 mouse_to_cursor();
  void request_blink();
  void stop_blink();
  void sustain_cursor();
  void del_selected();
  void move_cursor_right();
  void move_cursor_left();
  void move_cursor_end();

  virtual void draw_deco(i4_draw_context_class &context);
  void become_active();
  void become_unactive();

  void note_change()
  {
    changed=i4_T;
    sent_change=i4_F;
  }

public:  
  char *name() { return "text_input"; }

  void send_to_parent(i4_event *ev)  {    parent->receive_event(ev);  }

  i4_text_input_class(i4_graphical_style_class *style,
                      const i4_const_str &default_string,
                      w32 width,                         // width in pixels of window
                      w32 max_width,
                      i4_event_handler_class *change_notify=0,
                      i4_font_class *font=0);            // uses style->normal_font by default



  virtual void draw(i4_draw_context_class &context);
  virtual void receive_event(i4_event *ev);

  void change_text(const i4_const_str &new_st);
  i4_str *get_edit_string() { return st; }          // don't delete this return string!
  sw32 get_number();                                // assuming text is a proper numbe


  ~i4_text_input_class();
} ;


i4_window_class *i4_create_text_input(i4_graphical_style_class *style,
                                      const i4_const_str &default_string,
                                      w32 width,            // width in pixels of window
                                      w32 max_width);

#endif

