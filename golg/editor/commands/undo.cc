/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "editor/editor.hh"
#include "saver_id.hh"
#include "editor/e_state.hh"
#include "editor/e_res.hh"
#include "lisp/li_init.hh"
#include "lisp/lisp.hh"
#include "saver.hh"

void g1_editor_class::show_undo_state()
{
  int t=undo.tail, t_undos=0, t_redos=0, rt=undo.redo_tail;
  while (t!=undo.head)
  {
    t_undos++;
    t=(t+1)%undo.max;
  }

  while (rt!=undo.head)
  {
    t_redos++;
    rt=(rt+1)%undo.max;
  }   
}


void g1_editor_class::save_undo_info(w32 sections, i4_const_str &fname)
{
  li_call("redraw");

  changed();  // note that map is about to be changed so the user can save if they want

  i4_file_class *fp=i4_open(fname, I4_WRITE);
  if (!fp)
  {
    i4_mkdir(g1_ges("undo_dir"));
    fp=i4_open(fname, I4_WRITE);
  }
    
  if (fp)
  {
    g1_saver_class *sfp=new g1_saver_class(fp, i4_T);

    sfp->mark_section(G1_SECTION_MAP_SECTIONS_V1);
    sfp->write_32(sections);            
    get_map()->save(sfp, sections);
    sfp->begin_data_write();

    sfp->mark_section(G1_SECTION_MAP_SECTIONS_V1);
    sfp->write_32(sections);
    get_map()->save(sfp, sections);      
    delete sfp;
  }
}


void g1_editor_class::add_undo(w32 sections)
{
  if (g1_map_is_loaded() && undo.allow)
  {
    sections&=(~G1_MAP_VIEW_POSITIONS);   // don't save changes to the camera positions

    if (sections & G1_MAP_OBJECTS)      // if saving objects, save player info because it has 
      sections |= G1_MAP_PLAYERS;       // object references
    
    
    if (sections & (G1_MAP_CELLS 
                    | G1_MAP_VERTS 
                    | G1_MAP_OBJECTS 
                    | G1_MAP_LIGHTS 
                    | G1_MAP_SELECTED_VERTS))
      get_map()->recalc |= G1_RECALC_RADAR_VIEW;

    if (sections & (G1_MAP_CELLS | G1_MAP_VERTS | G1_MAP_SELECTED_VERTS))
      get_map()->recalc |= G1_RECALC_WATER_VERTS;


    if (!sections) return;

    g1_edit_state.hide_focus();

    if (((undo.head+1) % undo.max) == undo.tail)
      undo.tail = (undo.tail+1)%undo.max;

    int cur_undo=undo.head;

    i4_str *undo_file=g1_ges("undo_file").sprintf(100,cur_undo);

    save_undo_info(sections, *undo_file);


    delete undo_file;

    can_undo=i4_T;
    can_redo=i4_F;

    undo.head=(undo.head+1) % undo.max;
    undo.redo_tail=undo.head;

    g1_edit_state.show_focus();
  }

  show_undo_state();
}


li_object *g1_add_undo(li_object *o, li_environment *env)
{
  g1_editor_instance.add_undo(li_get_int(li_eval(li_car(o,env),env),env));
  return 0;
}

li_automatic_add_function(g1_add_undo, "add_undo");


void g1_editor_class::do_undo()
{


  if (g1_map_is_loaded() && undo.redo_tail!=undo.tail)
  {
    i4_str *old_name=new i4_str(g1_get_map()->get_filename());

    g1_edit_state.hide_focus();

    undo.redo_tail=(undo.redo_tail + undo.max - 1) % undo.max;

    i4_str *undo_file=g1_ges("undo_file").sprintf(100,undo.redo_tail);
    i4_file_class *fp=i4_open(*undo_file);
    if (fp)
    {
      g1_loader_class *lfp=g1_open_save_file(fp, i4_T);
      if (lfp->goto_section(G1_SECTION_MAP_SECTIONS_V1))
      {
        w32 sections=lfp->read_32();
     
        i4_str *redo_file=g1_ges("redo_file").sprintf(100,undo.redo_tail);        
        save_undo_info(sections, *redo_file);

        delete redo_file;

        undo.allow=0;

        if ((sections|G1_MAP_VIEW_POSITIONS)==G1_MAP_ALL)
          g1_load_level(*undo_file, 1, 0);   // need to reload textures for this one
        else
          get_map()->load(lfp, sections);

        undo.allow=1;
      }

      delete lfp;
    }
    delete undo_file;

    g1_get_map()->set_filename(*old_name);
    delete old_name;

    if (undo.redo_tail==undo.tail)
      can_undo=i4_F;

    can_redo=i4_T;
    li_call("redraw");

    g1_edit_state.show_focus();
  }

  show_undo_state();
}

void g1_editor_class::do_redo()
{
  if (g1_map_is_loaded() && undo.redo_tail!=undo.head)
  {
    i4_str *old_name=new i4_str(g1_get_map()->get_filename());

    g1_edit_state.hide_focus();

    i4_str *redo_file=g1_ges("redo_file").sprintf(100,undo.redo_tail);
    i4_file_class *fp=i4_open(*redo_file);
    if (fp)
    {    
      g1_loader_class *lfp=g1_open_save_file(fp, i4_T);
      if (lfp)
      {
        if (lfp->goto_section(G1_SECTION_MAP_SECTIONS_V1))
        {
          w32 sections=lfp->read_32();
          undo.allow=0;

          if (sections==G1_MAP_ALL)
            g1_load_level(*redo_file, 1, 0);   // need to reload textures for this one
          else
            get_map()->load(lfp, sections);

          undo.allow=1;
        }
        delete lfp;
      }
    }
    delete redo_file;

    g1_get_map()->set_filename(*old_name);
    delete old_name;


    undo.redo_tail = (undo.redo_tail + 1) % undo.max;
    
    if (undo.redo_tail==undo.head)
    {
      can_redo=i4_F;
    }

    li_call("redraw");

    g1_edit_state.show_focus();
  }

  show_undo_state();
}
