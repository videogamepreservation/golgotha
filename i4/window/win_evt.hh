/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __WIN_EVT_HH
#define __WIN_EVT_HH

#include "device/event.hh"
#include "arch.hh"

class i4_cursor_class;
class i4_window_class;

class i4_window_message_class : public i4_event
{
  public :
  enum win_sub_type { 
    GOT_MOUSE_FOCUS,           //  \/ \/ \/ sent by parent to children events \/ \/ \/
        
    LOST_MOUSE_FOCUS,
    GOT_KEYBOARD_FOCUS,
    LOST_KEYBOARD_FOCUS,

    DRAG_DROP_MOVE,           // when mouse moves in drag drop mode

    GOT_DROP_FOCUS,
    LOST_DROP_FOCUS,
    GOT_DROP,

    REQUEST_KEY_GRAB,  //  \/ \/ \/ sent by children to parent events  \/ \/ \/
    REQUEST_NEXT_KEY_FOCUS,

    REQUEST_LEFT_KEY_FOCUS,
    REQUEST_RIGHT_KEY_FOCUS,
    REQUEST_UP_KEY_FOCUS,
    REQUEST_DOWN_KEY_FOCUS,

    REQUEST_MOUSE_GRAB,
    REQUEST_MOUSE_UNGRAB,

    REQUEST_DRAG_DROP_START,
    REQUEST_DRAG_DROP_END,

    REQUEST_DRAG_START,
    REQUEST_DRAG_END,
    
    REQUEST_DELETE,

    NOTIFY_MOVE,
    NOTIFY_RESIZE,

    CHANGE_CURSOR
  } ;
  

  
  win_sub_type sub_type;
  i4_window_class *from_window;

  i4_window_class *from() { return from_window; }
  i4_window_message_class(win_sub_type win_message, i4_window_class *From)
    { 
    from_window=From;
    sub_type=win_message;
  }
  
  virtual dispatch_time when() { return NOW; }  
  virtual event_type type() { return WINDOW_MESSAGE; }
  virtual i4_event  *copy() { return new i4_window_message_class(sub_type,from_window); }
  char *name() { return "window_message"; }
} ;

class i4_window_got_mouse_focus_class : public i4_window_message_class
{
public:
  // location of mouse when you got focus, because got_focus may come before mouse_move
  int x, y;
        
  virtual void move(int xoff, int yoff) { x+=xoff; y+=yoff; }

  i4_window_got_mouse_focus_class(i4_window_class *from, int x, int y)
    : i4_window_message_class(GOT_MOUSE_FOCUS, from), x(x), y(y) {}
  char *name() { return "got_mouse_focus"; }
  virtual dispatch_time when() { return NOW; }   
  virtual i4_event  *copy() { return new i4_window_got_mouse_focus_class(from_window,x,y); }
};


class i4_window_lost_mouse_focus_class : public i4_window_message_class
{
public:
  i4_window_class *lost_to;

  i4_window_lost_mouse_focus_class(i4_window_class *from,
                                   i4_window_class *lost_to)
    : i4_window_message_class(LOST_MOUSE_FOCUS, from), lost_to(lost_to) {}
  char *name() { return "lost_mouse_focus"; }
  virtual dispatch_time when() { return NOW; }   
  virtual i4_event  *copy() { return new i4_window_lost_mouse_focus_class(from_window, lost_to); }
};

class i4_window_request_drag_drop_end_class : public i4_window_message_class
{
public:
  i4_window_request_drag_drop_end_class(i4_window_class *from)    
    : i4_window_message_class(REQUEST_DRAG_DROP_END, from)
  {}
  char *name() { return "request_drag_drop_end"; }
};


class i4_window_drag_drop_move_class : public i4_window_message_class
{
public:
  i4_coord x,y;
  w32 reference_id;
  void *further_info;

  virtual void move(int xoff, int yoff) { x+=xoff; y+=yoff; }

  i4_window_drag_drop_move_class(i4_window_class *from,
                                 i4_coord x,
                                 i4_coord y,
                                 w32 reference_id,
                                 void *further_info)
    : i4_window_message_class(DRAG_DROP_MOVE, from),
      x(x), y(y), reference_id(reference_id), further_info(further_info)
  {}

  virtual i4_event  *copy() 
  { return new i4_window_drag_drop_move_class(from_window, x,y, reference_id, further_info); }
  char *name() { return "drag_drop_move"; }

};

// return_result is set to i4_T if the parent was able to give you mouse grabbing
class i4_window_request_drag_drop_start_class : public i4_window_message_class
{
  public :
  i4_bool return_result;
  i4_cursor_class *drag_cursor;
  w32 reference_id;
  i4_event *drop_event;
  void *further_info;
  i4_window_request_drag_drop_start_class(i4_window_class *from,
                                          i4_cursor_class *drag_cursor,  // this will be destoryed
                                          w32 reference_id,              // a unique number
                                          void *further_info=0)

    :  i4_window_message_class(REQUEST_DRAG_DROP_START,from),
       drag_cursor(drag_cursor),
       reference_id(reference_id),
       further_info(further_info)
  { return_result=i4_F; }

  virtual i4_event  *copy() 
  { return new i4_window_request_drag_drop_start_class(from_window, 
                                                       drag_cursor, reference_id, further_info); }

  virtual dispatch_time when() { return NOW; }  // send now because a result is expected
  char *name() { return "request_drag_drop_start"; }
} ;

class i4_str;
struct i4_drag_info_struct
{
  enum { 
    FILENAMES
  } drag_object_type;
  
  i4_coord x,y; 

  
  int t_filenames;
  i4_str **filenames;
  void copy_to(i4_drag_info_struct &drag_struct);
  i4_drag_info_struct() { t_filenames=0; filenames=0; }
  ~i4_drag_info_struct();
};

class i4_window_got_drag_drop_focus_class : public i4_window_message_class
{
public:
  i4_drag_info_struct drag_info;

  virtual void move(int xoff, int yoff) { drag_info.x+=xoff; drag_info.y+=yoff; }
    
  i4_window_got_drag_drop_focus_class(i4_window_class *from)
      : i4_window_message_class(DRAG_DROP_MOVE, from) {}

  virtual i4_event *copy()
  {
    i4_window_got_drag_drop_focus_class *dg=new i4_window_got_drag_drop_focus_class(from());
    drag_info.copy_to(dg->drag_info);
    return dg;
  }

  char *name() { return "got_drag_drop_focus"; }
};

class i4_window_got_drop_class : public i4_window_message_class
{
public:
  i4_drag_info_struct drag_info;

  virtual void move(int xoff, int yoff) { drag_info.x+=xoff; drag_info.y+=yoff; }

  i4_window_got_drop_class(i4_window_class *from) 
    : i4_window_message_class(GOT_DROP, from) {}

  virtual i4_event *copy()
  {
    i4_window_got_drop_class *dg=new i4_window_got_drop_class(from());
    drag_info.copy_to(dg->drag_info);
    return dg;
  }

  char *name() { return "got_drop"; }
};


class i4_window_lost_drag_drop_focus_class : public i4_window_message_class
{
public:
  i4_window_lost_drag_drop_focus_class(i4_window_class *from)    
    : i4_window_message_class(LOST_DROP_FOCUS, from)
  {}
  char *name() { return "lost_drag_drop_focus"; }
};


// return_result is set to i4_T if the parent was able to give you mouse grabbing
class i4_window_request_key_grab_class : public i4_window_message_class
{
  public :
  i4_bool return_result;
  i4_window_request_key_grab_class(i4_window_class *from) 
    :  i4_window_message_class(REQUEST_KEY_GRAB,from)
  { return_result=i4_F; }

  virtual i4_event  *copy() { return new i4_window_request_key_grab_class(from_window); }
  virtual dispatch_time when() { return NOW; }  // send now because a result is expected
  char *name() { return "request_key_grab"; }
} ;


// return_result is set to i4_T if the parent was able to give you mouse grabbing
class i4_window_request_mouse_grab_class : public i4_window_message_class
{
  public :
  i4_bool return_result;

  i4_window_request_mouse_grab_class(i4_window_class *from)
    :  i4_window_message_class(REQUEST_MOUSE_GRAB,from)
  { return_result=i4_F; }

  virtual i4_event  *copy() 
  { 
    return new i4_window_request_mouse_grab_class(from_window); 
  }

  virtual dispatch_time when() { return NOW; }  // send now because a result is expected
  char *name() { return "request_mouse_grab"; }
} ;


// return_result is set to i4_T if the parent was able to give you mouse grabbing
class i4_window_request_mouse_ungrab_class : public i4_window_message_class
{
  public :
  i4_bool return_result;
  i4_window_request_mouse_ungrab_class(i4_window_class *from) 
    :  i4_window_message_class(REQUEST_MOUSE_UNGRAB,from)
  { return_result=i4_F; }

  virtual i4_event  *copy() { return new i4_window_request_mouse_ungrab_class(from_window); }
  virtual dispatch_time when() { return NOW; }  // send now because a result is expected
  char *name() { return "request_mouse_ungrab"; }
} ;

// return_result is set to i4_T if the parent was able to start dragging
class i4_window_request_drag_start_class : public i4_window_message_class
{
  public :
  i4_bool return_result;
  i4_window_request_drag_start_class(i4_window_class *from) :  
    i4_window_message_class(REQUEST_DRAG_START,from)
  { return_result=i4_F; }

  virtual i4_event  *copy() 
  { return new i4_window_request_drag_start_class(from_window); }

  virtual dispatch_time when() 
  { return NOW; }  // send now because a result is expected
  char *name() { return "request_drag_start"; }
} ;


// return_result is set to i4_T if the parent was able to start dragging
class i4_window_request_drag_end_class : public i4_window_message_class
{
  public :
  i4_window_request_drag_end_class(i4_window_class *from) :  
    i4_window_message_class(REQUEST_DRAG_END,from) {}

  virtual i4_event  *copy() 
  { return new i4_window_request_drag_end_class(from_window); }

  virtual dispatch_time when() 
  { return NOW; }  // send now because a result is expected

  char *name() { return "request_drag_end"; }
} ;


// this event is sent automatically by set_cursor
class i4_window_change_cursor_class : public i4_window_message_class
{
  public :
  i4_cursor_class *cursor;     // if cursor is 0, then default cursor is used
  i4_bool   only_if_active;    // if window is not active (mouse focus) then cursor will not loaded
  i4_window_change_cursor_class(i4_window_class *From, 
                                i4_cursor_class *cursor, 
                                i4_bool only_if_active=i4_F) : 
    i4_window_message_class(CHANGE_CURSOR,From),
    cursor(cursor),
    only_if_active(only_if_active) 
  {}


  virtual i4_event  *copy() 
  { return new i4_window_change_cursor_class(from_window,cursor,only_if_active); }

  // send now in case the cursor gets destoryed before delivery
  virtual dispatch_time when() { return NOW; }   
  char *name() { return "change_cursor"; }
} ;


// this event is sent to your parent and children automatically by resize()
class i4_window_notify_resize_class : public i4_window_message_class
{
  public :
  w16 new_width,new_height;
  i4_bool  draw_covered;
  i4_window_notify_resize_class(i4_window_class *from, w16 new_width, w16 new_height) :  
    i4_window_message_class(NOTIFY_RESIZE,from), 
    new_width(new_width),
    new_height(new_height)
  { draw_covered=i4_T;  }

  virtual i4_event  *copy() 
  { return new i4_window_notify_resize_class(from_window,new_width,new_height); }

  // send now so recepient doesn't draw incorrect before event is delivered
  virtual dispatch_time when() { return NOW; }  
  char *name() { return "notify_resize"; }
} ;



// the request move_notify is automatically sent to parent during move()
class i4_window_notify_move_class : public i4_window_message_class
{
  public :
  i4_coord x_offset,y_offset;
  i4_bool  draw_covered;
  i4_window_notify_move_class(i4_window_class *from, 
                              i4_coord x_offset, i4_coord y_offset, 
                              i4_bool draw_covered=i4_T) :  
    i4_window_message_class(NOTIFY_MOVE,from),
    x_offset(x_offset),
    y_offset(y_offset),
    draw_covered(draw_covered) {};

  virtual i4_event  *copy() 
  { return new i4_window_notify_move_class(from_window,x_offset,y_offset,draw_covered); }

  virtual dispatch_time when() 
  { return NOW; }  // send now so reciever doesn't draw incorrectly before it arrives
  char *name() { return "notify_move"; }
} ;



#endif

