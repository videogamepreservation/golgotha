/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_PMENU_HH
#define G1_PMENU_HH

#include "menu/key_item.hh"
#include "palette/pal.hh"
#include "string/string.hh"

class i4_parent_window_class;
class i4_graphical_style_class;
class i4_pull_menu_class;
class g1_editor_key_item;


i4_pull_menu_class *g1_create_editor_menu(i4_graphical_style_class *style,
                                          const i4_pal_handle_class &pal,
                                          i4_parent_window_class *root_window,
                                          i4_string_manager_class &st,
                                          g1_editor_key_item **command_key_list);
enum
{
  G1_EDITOR_NEW,
  G1_EDITOR_SAVE,
  G1_EDITOR_SAVEAS,
  G1_EDITOR_LOAD,
  G1_EDITOR_MERGE_TERRAIN,
  G1_EDITOR_EXIT,

  G1_EDITOR_UNDO,
  G1_EDITOR_REDO,
  G1_EDITOR_CUT,
  G1_EDITOR_COPY,
  G1_EDITOR_PASTE,
  G1_EDITOR_TOGGLE,
  G1_EDITOR_NO_SNAP,
  G1_EDITOR_SNAP_CENTER,
  G1_EDITOR_SNAP_ORIGIN,

  G1_EDITOR_WINDOW_OBJECTS,
  G1_EDITOR_WINDOW_SCENES,
  G1_EDITOR_WINDOW_RADAR,
  G1_EDITOR_1_VIEW,
  G1_EDITOR_4_VIEWS,
  G1_EDITOR_TILE_PICKER,

  G1_EDITOR_RECALC_LIGHT,
  G1_EDITOR_SELECT_ALL_VERTS,
  G1_EDITOR_RESIZE_MAP,
  G1_EDITOR_SET_SKY,

  G1_EDITOR_FOG_ALL,
  G1_EDITOR_FOG_NONE,

  G1_EDITOR_TICK_MAP,
  G1_EDITOR_PROFILE,
  G1_EDITOR_DEBUG,
  G1_EDITOR_AI_WINDOW,
  
  G1_EDITOR_LOAD_HEIGHT_MAP,
  G1_EDITOR_SAVE_HEIGHT_MAP,

  G1_EDITOR_FLATTEN_TERRAIN,
  G1_EDITOR_SMOOTH_TERRAIN,
  G1_EDITOR_NOISE_TERRAIN,

  G1_EDITOR_OPEN_DLL,

  G1_EDITOR_LAST,
  G1_EDITOR_SKIP
};

class g1_editor_key_item : public i4_key_item_class
{
public:
  w16 command_id;
  g1_editor_key_item(
                    const i4_const_str &_text,
                    i4_color_hint_class *color_hint,
                    i4_font_hint_class *font_hint,
                    i4_graphical_style_class *style,
                    w16 key,
                    w16 modifiers,
                    w16 command_id,
                    w16 pad_left_right,
                    w16 pad_up_down
                    );
  virtual void action();

};

void validate_command(w16 command_id);
void invalidate_command(w16 command_id);

#endif
