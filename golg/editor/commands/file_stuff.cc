/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "lisp/li_init.hh"
#include "lisp/lisp.hh"
#include "map.hh"
#include "map_man.hh"

li_object *g1_reload_map(li_object *o, li_environment *env)
{
  i4_str *fname=new i4_str(g1_get_map()->get_filename());
  
  g1_load_level(*fname, 0, G1_MAP_VIEW_POSITIONS);
  delete fname;

  li_call("redraw_all");

  return 0;
}


li_object *g1_full_reload_map(li_object *o, li_environment *env)
{
  i4_str *fname=new i4_str(g1_get_map()->get_filename());
  
  g1_load_level(*fname, 1, G1_MAP_VIEW_POSITIONS);
  delete fname;

  li_call("redraw_all");

  return 0;
}


li_automatic_add_function(g1_reload_map, "reload_level");
li_automatic_add_function(g1_full_reload_map, "full_reload_level");
