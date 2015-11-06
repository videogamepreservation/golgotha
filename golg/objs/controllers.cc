/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "object_definer.hh"
#include "lisp/li_class.hh"
#include "objs/model_draw.hh"
#include "li_objref.hh"
#include "math/random.hh"
#include "map_man.hh"
#include "map.hh"

static li_float_class_member range_when_activated("range_when_activated"),
  range_when_deactivated("range_when_deactivated"),
  range_when_deployed("range_when_deployed"); 

static li_object_class_member who_can_trigger("who_can_trigger"),
   message_to_send("message_to_send"),
   objects_to_trigger("objects_to_trigger"),
   objects_in_range("objects_in_range"),
   send_on_trigger("send_on_trigger"),
   send_on_untrigger("send_on_untrigger"),
   current_state("current_state"),
   who_to_send("who_to_send"),
   nearby_objects("nearby_objects"),
   li_deploy_to("deploy_to"),
   objects_to_send_next_tick("objects_to_send_next_tick");
 

static li_symbol_ref on("on"), off("off"), none("none"), anyone("anyone"), team("team"),
  enemy("enemy"); 

static g1_object_type stank_type;
 
class g1_trigger_class : public g1_object_class
{
public:
  enum trigger_type { ANYONE, TEAM_MATES, ENEMY, SUPERTANKS, ENEMY_SUPERTANKS } ;
  int type;
  
  // this matches the symbol name to our enum for faster use in C
  // we leave it in symbol for in the editor easy/understandable editing
  void get_trigger_type()
  {    
    li_symbol *s=li_symbol::get(vars->get(who_can_trigger()));
    char *sym[]={"anyone", "team_mates", "enemy", "supertanks", "enemy_supertanks",0 }, **a;
    for (a=sym, type=0; *a && li_get_symbol(*a)!=s; a++, type++);      
  }
  
  g1_trigger_class(g1_object_type id, g1_loader_class *fp)
    : g1_object_class(id,fp)
  {
    get_trigger_type();
    draw_params.setup("trigger");
  }

  void object_changed_by_editor(g1_object_class *who, li_class *old_vars)
  {
    if (who==this)
      get_trigger_type();
  }
  
  
  // is the object in question one we should be triggered by?
  i4_bool triggable_object(g1_object_class *o)
  {
    switch (type)
    {
      case ANYONE:  return i4_T;
      case TEAM_MATES: if (o->player_num==player_num) return i4_T;
      case ENEMY: if (o->player_num!=player_num) return i4_T;
      case SUPERTANKS: if (o->id==stank_type) return i4_T;
      case ENEMY_SUPERTANKS: if (o->id==stank_type && o->player_num!=player_num) return i4_T;
    }
  }

  // send our trigger message to all the object our list 
  void send_to_trigger_objects(li_symbol *sym)
  {
    if (sym==none.get()) return; // don't send none, it means "no message"
  
    li_g1_ref_list *l=li_g1_ref_list::get(objects_to_trigger());
    int t=l->size();
    for (int i=0; i<t; i++)
    {
      g1_object_class *o=l->value(i);
      if (o)
        o->message(sym, 0,0);
    }        
  }
  
//   void note_enter_range(g1_object_class *who, g1_fire_range_type range)
//   {
//     li_class_context context(vars);
    
//     if (range<=range_when_activated() && triggable_object(who))
//     {      
//       li_g1_ref_list *in_range=li_g1_ref_list::get(objects_in_range());
//       if (!in_range->find(who))
//       {
//         in_range->add(who);
        
//         if (current_state()!=on.get())
//         {
//           current_state()=on.get();
//           send_to_trigger_objects(li_symbol::get(send_on_trigger()));                    
//         }
//       }        
//     }
//   }

//   void note_leave_range(g1_object_class *who, g1_fire_range_type range)
//   {
//     li_class_context context(vars);

//     if (range>=range_when_deactivated() && triggable_object(who))
//     {
//       li_g1_ref_list *in_range=li_g1_ref_list::get(objects_in_range());
//       if (in_range->find(who))
//       {
//         in_range->remove(who);
        
//         if (current_state()!=off.get())
//         {
//           current_state()=off.get();
//           send_to_trigger_objects(li_symbol::get(send_on_untrigger()));                    
//         }
//       }      
//     }
//   }
   
  
  void think() {}
};

void g1_trigger_init()
{
  stank_type=g1_get_object_type("stank");
}

static g1_object_definer<g1_trigger_class>
trigger_def("trigger", g1_object_definition_class::EDITOR_SELECTABLE, g1_trigger_init);



class g1_director_class : public g1_object_class
{
public:
  g1_model_draw_parameters draw_params;
  void draw(g1_draw_context_class *context)  { g1_model_draw(this, draw_params, context); }
  virtual i4_float occupancy_radius() const { return draw_params.extent(); }
  
  g1_director_class(g1_object_type id, g1_loader_class *fp)
    : g1_object_class(id,fp)
  {
    draw_params.setup("trigger");
  }
  
  void think()
  {
    li_g1_ref_list *list=li_g1_ref_list::get(objects_to_send_next_tick());
    while (list->size())
    {
      g1_object_class *o=list->value(0);
      list->remove(o->global_id);
      send_object(o);
    }    
  }

  void add_to_send_list(g1_object_class *o)
  {
    li_g1_ref_list *list=li_g1_ref_list::get(objects_to_send_next_tick());
    if (list->find(o)==-1)
      list->add(o);
    
    request_think();
  }
  
  void send_object(g1_object_class *o)
  {
    if (!o) return ;
   
    li_g1_ref_list *list=li_g1_ref_list::get(li_deploy_to());
    int list_size=list->size(), total=0;

    // count how many valid objects we point to
    for (int i=0; i<list_size; i++)
    {
      g1_object_class *to=list->value(i);
      if (to)
        total++;
    }

    if (total)
    {
      // pick one of the valid destinations
      int dest=g1_rand(56)%total;
      total=0;
      for (int j=0; j<list_size; j++)
      {
        g1_object_class *to=list->value(j);
        if (to)
        {
          if (total==dest)
          {
            o->deploy_to(to->x, to->y);
            return ;
          }
          else
            total++;
        }
      }
    }
    
    return ;
  }
  
//   void note_enter_range(g1_object_class *who, g1_fire_range_type range)
//   {
//     li_class_context context(vars);
    
//     if (range<=range_when_deployed() &&
//         (who_to_send()==anyone.get() ||
//          (who->player_num==player_num && who_to_send()==team.get()) ||
//          (who->player_num!=player_num && who_to_send()==enemy.get())))
//     {
//       li_g1_ref_list *r=li_g1_ref_list::get(nearby_objects());
//       if (r->find(who)==-1)
//       {
//         r->add(who);

//         add_to_send_list(who);
//       }
//     }
//   }

//   void note_leave_range(g1_object_class *who, g1_fire_range_type range)
//   {
//     li_class_context context(vars);
        
//     li_g1_ref_list *r=li_g1_ref_list::get(nearby_objects());
//     if (r->find(who)!=-1)
//       r->remove(who);
//   }

  li_object *message(li_symbol *message_name, li_object *message_params, li_environment *env)
  {
    li_class_context context(vars);
    
    if (message_name==on.get())
    {
      current_state()=on.get();

      // we just turned on, see if there are any nearby object we should send
      // to there destinations
      li_g1_ref_list *r=li_g1_ref_list::get(nearby_objects());
      int t=r->size();
      for (int i=0; i<t; i++)
        add_to_send_list(r->value(i));
      
    } else if (message_name==off.get())
      current_state()=off.get();
  }


  // we were put down in the editor, scan for nearby objects and send them if we are on
  i4_bool occupy_location()
  {
    li_class_context context(vars);

    g1_object_class *olist[G1_MAX_OBJECTS];    
    int t=g1_get_map()->get_objects_in_range(x,y, range_when_deployed(), olist, G1_MAX_OBJECTS,
                                       0xffffffff,  g1_object_definition_class::MOVABLE);


    li_object *o=nearby_objects();

    li_g1_ref_list *r=li_g1_ref_list::get(o);
    
    while (r->size())
      r->remove(r->get_id(0));

    for (int i=0; i<t; i++)
    {
      r->add(olist[i]);
      if (current_state()==on.get())
        add_to_send_list(olist[i]);
    }        
    
    return g1_object_class::occupy_location();
  }

  
};

static g1_object_definer<g1_director_class>
director_def("director", g1_object_definition_class::EDITOR_SELECTABLE);
