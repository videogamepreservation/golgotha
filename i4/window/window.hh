/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __I4_WINDOW_HPP_
#define __I4_WINDOW_HPP_

class i4_image_class;
class i4_cursor_class;

#include "area/rectlist.hh"
#include "device/device.hh"
#include "area/rectlist.hh"
#include "device/event.hh"
#include "isllist.hh"
#include "image/context.hh"

//  Brief summary :
//
//    2 Main classes are defined here :
//    i4_window_class : 
//    base object for most everything graphical.  
//    An i4_window_class should only draw (using local_image) during draw().
//    During draw(), proper clipping is setup.  
//    local_image will transform local coordinates to global space for you.
//    because a window is derived from i4_event_handler_class, 
//    it can receive events through receive_event (see device/device.hh 
//    for more info on this.  A the base window class does not process any events.
//
//    i4_parent_window_class : 
//    parent windows have a list of children window.  
//    The draw order for children is painter's algorithm.  So the last
//    child in the list gets drawn on top.  parent windows also 
//    maintain a dirty_area list so that redraw can be done
//    more efficiently than redrawing themselves & their children completely.
//    parent windows have a default behavior for responding to a 
//    number of events.  parent windows keep a mouse & keyboard
//    focus.   The mouse focus is set automically through mouse_move events.  
//    The key focus is not implemented yet.
//

class i4_parent_window_class;

class i4_window_class : public i4_event_handler_class
{
protected:
  
  // Data members you need not worry about
  i4_image_class     *global_image;  // pointer to the image associated with the display we draw on
  
  i4_coord global_x,global_y,   // returned by x() & y()
           mouse_x, mouse_y;
  w16 w,h;
  i4_bool redraw_flag;

  
  i4_image_class *local_image;        /* this will transform all drawing coordinates 
                                   from local-space to glabal space
                                   plus when drawing is done from draw() 
                                   proper clipping for you window is setup beforehand */
  i4_parent_window_class *parent;     // this may be 0
  i4_cursor_class *cursor;

public:
  
  // next pointer is used by i4_parent_window_class's isl_list
  i4_window_class *next;
  
  virtual void receive_event(i4_event *ev);

  // initially when a window is created it has no parent
  i4_window_class(w16 w, w16 h);  

  i4_coord x() { return global_x; }
  i4_coord y() { return global_y; } 
  w16 width() { return w; }
  w16 height() { return h; }

  i4_coord last_mouse_x() { return mouse_x; }  // last position in the window mouse was at
  i4_coord last_mouse_y() { return mouse_y; }


  // forget redraw is called by a parent if the child had ask for a redraw but
  // when the parent got around to doing the redraw, the child was clipped away
  virtual void forget_redraw() { redraw_flag=i4_F; }

  // request draw should be called in the calculating part of a windows processing
  // the windows draw() function will then be called later with the correct clip list
  // 2 versions of request_redraw exist, one redraws 
  // everything, and one tells of a specific area to redraw
  // a window may call request_redraw() within it's draw() to display continual animations

  virtual void request_redraw(i4_bool for_a_child=i4_F);

  // x1, y1, x2, y2 are in local-space coordinates
  // this used to be called request_redraw, but it got confused with regular request_redraw
  // this is called whan a window's area becomes visible when it wasn't before
  virtual void note_undrawn(i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2, 
                            i4_bool propogate_to_children=i4_T);

  // a parent uses this to determine that you need to have your draw() function called
  virtual i4_bool need_redraw() { return redraw_flag; }

  // draw is called by parent sometime after request_draw() is called
  // this draw may not occur if window is clipped away, in which case forget_redraw() is called
  virtual void draw(i4_draw_context_class &context);

  // move can be used to a change window's position 
  // within a window, to give a window a draw_area and parent or both
  // move will also send a notify_move event to it's parent 
  // (if it has one), whereas private_move will not
  // if draw_under is false the parent will not draw under 
  //the dirty area the window's movement created
  virtual void move(i4_coord x_offset, i4_coord y_offset, i4_bool draw_under=i4_T);

  // private move will not notify it's parent, and should 
  // probably only be called by the parent.
  virtual void private_move(i4_coord new_x, i4_coord new_y);

  // private resize will not notify it's parent, and
  // should probably only be called by your parent
  virtual void private_resize(w16 new_width, w16 new_height);
  
  virtual void reparent(i4_image_class *draw_area, i4_parent_window_class *parent);

  // resize will change the width and height of our window and notify our parent of the change
  virtual void resize(w16 new_width, w16 new_height);


  // this sets the mouse image to displayed when the cursor 
  // enters this window.  The actual act of changing the cursor
  // is done by i4_window_class::receive_event() on GOT & LOST mouse 
  // focus events.  So a derived window must remember to call
  // the shadowed receive_event or the cursor will not change. 
  // If cursor is 0, then the cursor will not change
  // shape when it enters this window.
    virtual void set_cursor(i4_cursor_class *cursor);


  virtual ~i4_window_class();
  void show_context(i4_draw_context_class &context);

  i4_bool isa_parent(i4_window_class *who);
  virtual i4_parent_window_class *root_window();

  // if this is true, then parent will not clip you out when it draws
  virtual i4_bool transparent() { return i4_F; }

  i4_parent_window_class *get_parent() { return parent; }
#ifndef I4_RETAIL
  void debug_show();
#endif
};


class i4_parent_window_class : public i4_window_class
{
private:
  i4_bool mouse_focus_grabbed, have_mouse_focus;
  i4_bool child_rerequested_redraw;
  void send_event_to_child(i4_window_class *w, i4_event *ev);
  
protected:
  i4_bool child_need_redraw;

  virtual void private_move(i4_coord x_offset, i4_coord y_offset);

  
  typedef i4_isl_list<i4_window_class>           win_list;
  typedef i4_isl_list<i4_window_class>::iterator win_iter;

  win_list children;
  win_iter key_focus, mouse_focus, drag_drop_focus;
  

  // Note:
  // so a parent can refresh itself and children more efficiently it keeps a list of dirty area.
  // this is kept in global coordinates for several reasons.   It is important to note that
  // request_redraw(x1,y1,x2,y2) takes coordinates in local_space, but transforms them to
  // local space when adding to dirty_area
  i4_rect_list_class undrawn_area;   


  // These functions shouldn't really be needed by derived classes
  void change_key_focus(i4_window_class *new_focus);
  void change_mouse_focus(i4_window_class *new_focus);
  void redraw_area(i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2);   // in local coordinates
  win_iter find_window(i4_coord mouse_x, i4_coord mouse_y);
  
  void drag_drop_move(i4_event *ev);
  void mouse_move(i4_event *ev);

public:
  i4_bool has_mouse_focus() { return have_mouse_focus; }
  virtual i4_bool find_new_mouse_focus();  // return true if focus changed

  virtual i4_parent_window_class *root_window();

  virtual void resize(w16 new_width, w16 new_height);
  virtual void reparent(i4_image_class *draw_area, i4_parent_window_class *parent);

  // these functions are called when REQUEST_NEXT_FOCUS, etc. events are recieved
    // so that you can over-ride these event functionalities if you want
  virtual void next_key_focus();
  virtual void left_key_focus();
  virtual void right_key_focus();
  virtual void up_key_focus();
  virtual void down_key_focus();
  
  // Because a parent window needs to tells it's children 
  // to draw() as well and setup proper clipping for itself
  // a parent_draw() function is supplied which is called by draw() which is where all drawing 
  // done by a derived class of i4_window_class should go.  parent_draw() will have the correct clipping 
  // list (including the exclusion of it's children. 
    virtual void draw(i4_draw_context_class &context);

  // see comment's above draw(), this function should be used by derived class instead of draw()
  virtual void parent_draw(i4_draw_context_class &context);

  // receive_event is a general event notification routine.  
  // If you get a event and note that you cannot handle it, you should
  // pass it on down to you derived parent so that functionality still works
  // example receive_event :
  // void receive_event(i4_event *ev)
  // {
  //   if (ev->type==i4_event::MOUSE_MOVE)
  //   {
  //     do something
  //   } else i4_parent_window_class::receive_event(ev);
  // }  
  virtual void receive_event(i4_event *ev);
  
  //  forget redraw is called by a parent if the child had ask for a redraw but
  // when the parent got around to doing the redraw, the child was clipped away
  // for the parent window case, undrawn_area is discarded and all 
  // children's forget_redraw are called
  virtual void forget_redraw();

  i4_parent_window_class(w16 w, w16 h);

  // x & y are relative to parent x,y (added to end of window list)
  virtual void add_child(i4_coord x, i4_coord y, i4_window_class *child);  

  // x & y are relative to parent x,y (added to front of window list)
  virtual void add_child_front(i4_coord x, i4_coord y, i4_window_class *child);  

  // does not delete the child, simply removes and request redraw under its area
  virtual void remove_child(i4_window_class *child);                       

  // removes all children from self and addes them into other_parent
  void transfer_children(i4_parent_window_class *other_parent, 
                         i4_coord x_offset, i4_coord y_offset);

  // arranges child windows from left to right then down
  virtual void arrange_right_down();                                  

  // arranges child windows from top to bottom then right
  virtual void arrange_down_right();                                  

  // makes the window the minimum size needed to fit all children
  virtual void resize_to_fit_children();

  // adds all local area to undrawn_area
  virtual void request_redraw(i4_bool for_a_child=i4_F);                            

  // adds this area to undrawn_area (in local coordinates)
  virtual void note_undrawn(i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2,
                            i4_bool propogate_to_children=i4_T);


  virtual i4_bool need_redraw() 
  // returns true if we have any dirty area or childen report they need redrawing 
  { return (i4_bool)(!undrawn_area.empty()|
                     child_need_redraw | 
                     i4_window_class::need_redraw()); }
  
  virtual i4_window_class *get_nth_window(w32 win_num);

  i4_bool isa_child(i4_window_class *w);

  virtual ~i4_parent_window_class();
};

#endif

