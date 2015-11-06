/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/def_object.hh"
#include "lisp/li_class.hh"
#include "map_man.hh"
#include "map.hh"
#include "li_objref.hh"
#include "lisp/li_init.hh"
#include "object_definer.hh"

static li_object_class_member target("takeover_objects"), me_turret("turret");
static li_symbol_ref turret_obj("turret");

class g1_takeover_pad_class : public g1_object_class
{
public:
  g1_takeover_pad_class(g1_object_type id, g1_loader_class *fp)
    : g1_object_class(id,fp) {}

  void think() {}

  virtual void change_player_num(int new_team)
  //{{{
  {
    li_class_context c(vars);

    g1_object_class *turret=li_g1_ref::get(me_turret(),0)->value();
    if (!turret)
    {
      g1_object_class::change_player_num(new_team);
      
      li_g1_ref_list *list=li_g1_ref_list::get(target(),0);
      int list_size=list->size();
      for (int i=0; i<list_size; i++)  
      {
        g1_object_class *tar=list->value(i);
        if (tar)
        {
          tar->request_think();
          tar->change_player_num(player_num);
        }
      }
      
      g1_object_class *t = g1_create_object(g1_get_object_type(turret_obj.get()));
      if (t)
      {
        t->x = x;
        t->y = y;
        t->player_num = player_num;
        t->occupy_location();
        t->request_think();
        t->grab_old();
        
        me_turret() = new li_g1_ref(t->global_id);
      }      
    }
  }
  //}}}
};

g1_object_definer<g1_takeover_pad_class>
g1_takeover_pad_def("takeover_pad", g1_object_definition_class::EDITOR_SELECTABLE);

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
