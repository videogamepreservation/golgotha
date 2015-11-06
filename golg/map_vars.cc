/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "map_vars.hh"
#include "lisp/li_load.hh"
#include "saver.hh"
#include "map.hh"
#include "lisp/lisp.hh"
#include "lisp/li_init.hh"
#include "lisp/li_dialog.hh"

void g1_map_vars_class::load(g1_loader_class *fp, w32 sections)
{
  if ((sections & G1_MAP_VARS) && fp)
    if (fp->goto_section("level vars"))
      var_ptr=li_load_typed_object("level_vars", fp, fp->li_remap,0);  
    else var_ptr=li_new("level_vars");

}


void g1_map_vars_class::save(g1_saver_class *fp, w32 sections)
{
  if (sections & G1_MAP_VARS)
  {
    fp->mark_section("level vars");
    li_save_object(fp, var_ptr.get(), 0);
  }

}

g1_map_vars_class g1_map_vars;

li_object *g1_set_level_vars(li_object *o, li_environment *env)
{
  g1_map_vars.var_ptr=li_car(o, env);
  return 0;
}



li_object *g1_edit_level_vars(li_object *o, li_environment *env)
{
  li_create_dialog("Level Vars", g1_map_vars.var_ptr.get(), 0, g1_set_level_vars);
  return 0;
}

li_automatic_add_function(g1_edit_level_vars, "edit_level_vars");



