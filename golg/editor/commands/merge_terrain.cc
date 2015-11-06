/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "editor/editor.hh"
#include "file/get_filename.hh"
#include "saver.hh"
#include "mess_id.hh"

void g1_editor_class::merge_terrain()
{
  i4_create_file_open_dialog(style,
                             get_editor_string("merge_ter_title"),
                             get_editor_string("merge_ter_start_dir"),
                             get_editor_string("merge_ter_file_mask"),
                             get_editor_string("merge_ter_mask_name"),
                             this,
                             G1_TMERGE_FILE_OPEN_OK,
                             G1_TMERGE_FILE_OPEN_CANCEL);


}

i4_bool g1_editor_class::merge_terrain_ok(i4_user_message_event_class *ev)
{
  CAST_PTR(f, i4_file_open_message_class, ev);

 
  i4_bool ret=i4_F;
  if (get_map())
  {
    i4_file_class *in=i4_open(*f->filename);
    if (in)
    {
      g1_loader_class *l=g1_open_save_file(in);
    
      if (l)
      {
        g1_editor_instance.add_undo(G1_MAP_CELLS | G1_MAP_VERTS);

        get_map()->load(l, G1_MAP_CELLS | G1_MAP_VERTS);
        delete l;
        ret=i4_T;
      }

      delete in;
    } 
  }

  return ret;
}

