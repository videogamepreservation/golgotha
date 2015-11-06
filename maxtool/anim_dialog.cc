/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "window/window.hh"
#include "window/colorwin.hh"
#include "window/style.hh"
#include "gui/text_input.hh"
#include "gui/text.hh"
#include "app/app.hh"
#include "lisp/lisp.hh"
#include "lisp/li_init.hh"

#include "m1_info.hh"
#include "max_object.hh"

class m1_animation_dialog_class : public i4_color_window_class
{
protected:
  enum 
  //{{{
  {
    SPEED,
    DU,
    DV,
    MAXX,
    MAXY,
    MAXFRAMES,
    NONE,
    ITEMS
  };
  //}}}
  i4_text_window_class *caption[ITEMS];
  i4_text_input_class *input[ITEMS];
public:
  g1_texture_animation *special;

  void add_item(int &h, int item)
  //{{{
  {
    int dh=0;

    if (caption[item]) 
    {
      add_child(10 ,h,caption[item]); 
      dh = caption[item]->height(); 
    }
    if (input[item])
    {
      add_child(100,h,input[item]);
      dh = input[item]->height();
    }
    h += dh + 4;
  }
  //}}}

  void clear_kids()
  //{{{
  {
    for (int i=0; i<ITEMS; i++)
    {
      if (isa_child(caption[i]))
        remove_child(caption[i]);
      if (isa_child(input[i]))
        remove_child(input[i]);
    }   
  }
  //}}}

  void edit_pan()
  //{{{
  {
    int h=10;
    char buf[512];

    clear_kids();

    sprintf(buf, "%f", special->speed);
    input[SPEED]->change_text(buf);

    sprintf(buf, "%f", special->du);
    input[DU]->change_text(buf);
    sprintf(buf, "%f", special->dv);
    input[DV]->change_text(buf);

    add_item(h,SPEED);
    add_item(h,DU);
    add_item(h,DV);
  }
  //}}}

  void edit_animation()
  //{{{
  {
    int h=10;
    char buf[512];

    clear_kids();

    sprintf(buf, "%f", special->speed);
    input[SPEED]->change_text(buf);

    int frames_y = int(1.0/special->dv+0.5);

    sprintf(buf, "%d", special->frames_x);
    input[MAXX]->change_text(buf);
    sprintf(buf, "%d", frames_y);
    input[MAXY]->change_text(buf);
    sprintf(buf, "%d", special->max_frames);
    input[MAXFRAMES]->change_text(buf);

    add_item(h,SPEED);
    add_item(h,MAXX);
    add_item(h,MAXY);
    add_item(h,MAXFRAMES);
  }
  //}}}

  void edit_none()
  //{{{
  {
    int h=10;

    clear_kids();

    add_item(h,NONE);
  }
  //}}}

  void find_special()
  //{{{
  {
    m1_poly_object_class *obj = m1_info.obj;
    int i,found=-1;

    special=0;

    if (!obj) return;

    for (i=0; i<obj->num_quad; i++)
      if (obj->quad[i].get_flags(g1_quad_class::SELECTED))
        if (found==-1)
          found = i;
        else
          found = -2;

    if (found<0) return;
      
    for (i=0; i<obj->num_special; i++)
      if (obj->special[i].quad_number == found)
        special = &obj->special[i];
  }
  //}}}
  
  void init()
  //{{{
  {
    find_special();
    if (!special)
      edit_none();
    else if (special->max_frames==0)
      edit_pan();
    else
      edit_animation();
  }
  //}}}

  m1_animation_dialog_class(i4_graphical_style_class *style)
    : i4_color_window_class(400,200,style->color_hint->neutral(),style)
  //{{{
  {
    caption[SPEED]     = new i4_text_window_class(      "Frames/Tick:",style);
    input  [SPEED]     = new i4_text_input_class(style, "<default>",200,40, this);
    caption[DU]        = new i4_text_window_class(      "DU::",style);
    input  [DU]        = new i4_text_input_class(style, "<default>",200,40, this);
    caption[DV]        = new i4_text_window_class(      "DV:",style);
    input  [DV]        = new i4_text_input_class(style, "<default>",200,40, this);
    caption[MAXX]      = new i4_text_window_class(      "X divisions:",style);
    input  [MAXX]      = new i4_text_input_class(style, "<default>",200,40, this);
    caption[MAXY]      = new i4_text_window_class(      "Y divisions:",style);
    input  [MAXY]      = new i4_text_input_class(style, "<default>",200,40, this);
    caption[MAXFRAMES] = new i4_text_window_class(      "Frames:",style);
    input  [MAXFRAMES] = new i4_text_input_class(style, "<default>",200,40, this);
    caption[NONE]      = new i4_text_window_class(      "No special animation",style);
    input  [NONE]      = 0;

    init();
  }
  //}}}

  virtual void receive_event(i4_event *ev)
  //{{{
  {
    switch (ev->type())
    {
      case i4_event::OBJECT_MESSAGE:
      {
        if (!special)
          break;

        CAST_PTR(tev, i4_text_change_notify_event, ev);
        int found=-1;
        
        for (int i=0; i<ITEMS; i++)
          if (input[i]==tev->object)
            found = i;

        i4_str::iterator p = tev->new_text->begin();
        switch (found)
        {
          case SPEED:     special->speed = p.read_float(); break;
          case DU:        special->du = p.read_float(); break;
          case DV:        special->dv = p.read_float(); break;
          case MAXFRAMES:
            special->max_frames = p.read_number(); 
            if (special->max_frames<1) special->max_frames=1;
            break;
          case MAXX:
            special->frames_x = p.read_number(); 
            if (special->frames_x<1) special->frames_x=1;
            special->du = 1.0/i4_float(special->frames_x);
            break;
          case MAXY:
          {
            int frames_y=p.read_number();
            if (frames_y<1) frames_y=1;
            special->dv = 1.0/i4_float(frames_y);
          } break;
        }
      } break;

      default:
        i4_color_window_class::receive_event(ev);
        break;
    }
  }
  //}}}
};

static i4_event_handler_reference_class<m1_animation_dialog_class> m1_animation_dialog;

li_object *m1_edit_special(li_object *o, li_environment *env)
//{{{
{
  if (m1_animation_dialog.get())
    m1_animation_dialog->init();
  else
  {
    i4_graphical_style_class *style = i4_current_app->get_style();
    m1_animation_dialog = new m1_animation_dialog_class(style);
    
    style->create_mp_window(-1,-1, 
                            m1_animation_dialog->width(),
                            m1_animation_dialog->height(),
                            "Edit Special")
      ->add_child(0,0,m1_animation_dialog.get());
  }
  return 0;
}
//}}}
li_automatic_add_function(m1_edit_special, "edit_special");

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
