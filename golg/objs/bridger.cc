/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "object_definer.hh"
#include "objs/map_piece.hh"
#include "lisp/li_class.hh"
#include "map_man.hh"
#include "map.hh"
#include "li_objref.hh"
#include "camera.hh"
#include "objs/path_object.hh"
#include "saver.hh"
#include "map_cell.hh"

static li_symbol_class_member bridgeable_spot("bridgeable_spot");
static li_symbol_ref bridger("bridger"), yes("yes"), already_attached("already_attached");
static li_g1_ref_class_member marker_attached_to("marker_attached_to");

class g1_bridger_object_class : public g1_map_piece_class
{
public:

  g1_typed_reference_class<g1_object_class> last_on;
  
  g1_bridger_object_class(g1_object_type id, g1_loader_class *fp)
    : g1_map_piece_class(id,fp)
  {
    allocate_mini_objects(1, "");
    mini_objects[0].defmodeltype = g1_model_list_man.find_handle("bridger_wheels");
    draw_params.setup("bridger_road", 0, "bridger_road");

    radar_type=G1_RADAR_VEHICLE;
    set_flag(BLOCKING      |
             TARGETABLE    |
             GROUND        | 
             SHADOWED      |
             DANGEROUS, 1);
  }


  i4_bool occupy_location()
  {
    li_class_context context(vars);
    if (!marker_attached_to()->value())
      return g1_map_piece_class::occupy_location();
    else
      return occupy_location_center();
  }

  
  void think()
  {
    if (!marker_attached_to()->value())
    {
      g1_map_piece_class::think();
      stagger=0;                      // I want bridges to be in a straight line
      
      if (!alive())
        return;
    

      g1_object_class *mark=0;
      int bridgers_found=0;

      
      g1_object_chain_class *chain=g1_get_map()->cell((int)x, (int)y)->get_obj_list();
      for (g1_object_chain_class *c=chain; c; c=c->next)
        if (g1_path_object_class::cast(c->object))
          mark=c->object;


      if (mark && mark->vars->get(bridgeable_spot)==yes.get())
      {
        g1_camera_event cev;
        cev.type=G1_WATCH_IDLE;
        cev.follow_object=this;
        g1_current_view_state()->suggest_camera_event(cev);
        
        unoccupy_location();

        unlink();
        set_flag(CAN_DRIVE_ON, 1);
        set_flag(DANGEROUS,0);

        x=mark->x;
        y=mark->y;
        h=mark->h;
        
        mark->vars->get(bridgeable_spot)=already_attached.get();
        vars->get(marker_attached_to)=new li_g1_ref(mark->global_id);
        set_flag(CAN_DRIVE_ON, 1);
        
        occupy_location();
              
        pitch = roll = 0;
        groundpitch = 0;
        groundroll = 0;
        grab_old();        

      }      
    }
    else if (!check_life())
    {
      unoccupy_location();
      request_remove();      
    }
  }

  void request_remove()
  {
    g1_object_class *marker=li_g1_ref::get(vars->get(marker_attached_to),0)->value();
    if (marker)
      marker->vars->get(bridgeable_spot)=yes.get();
    
    g1_map_piece_class::request_remove();
  }
 
};


g1_object_definer<g1_bridger_object_class>
g1_bridger_def("bridger", 
               g1_object_definition_class::EDITOR_SELECTABLE |
               g1_object_definition_class::TO_MAP_PIECE |
               g1_object_definition_class::MOVABLE);



