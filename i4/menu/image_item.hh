/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "menu/menu.hh"

class i4_str;
class i4_image_class;

class i4_image_item_class : public i4_menu_item_class
{
  i4_image_class *im, *act;
  i4_bool del_im;
  i4_bool del_act;

public:  
  i4_image_item_class(const i4_const_str *context_help,
                      i4_image_class *normal_image,                      
                      i4_graphical_style_class *style,
                      i4_image_class *active_image=0,     // if 0, then image will brighten
                      i4_bool delete_images_on_death=i4_T,
                      i4_event_reaction_class *press=0,
                      i4_event_reaction_class *depress=0,
                      i4_event_reaction_class *activate=0,
                      i4_event_reaction_class *deactivate=0);

  ~i4_image_item_class();
  virtual void parent_draw(i4_draw_context_class &context);
  virtual void receive_event(i4_event *ev);

  virtual i4_menu_item_class *copy();
  char *name() { return "image_item"; }
};
