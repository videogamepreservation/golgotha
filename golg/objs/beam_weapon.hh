/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_BEAM_WEAPON_HH
#define G1_BEAM_WEAPON_HH

#include "g1_object.hh"
#include "objs/model_draw.hh"
#include "player_type.hh"
#include "path.hh"
#include "objs/smoke_trail.hh"
#include "sound_man.hh"

class g1_solid_class;
class g1_map_piece_class;

class g1_beam_weapon_class : public g1_object_class
{
protected:  
  g1_typed_reference_class<g1_object_class> who_fired_me;
  g1_typed_reference_class<g1_object_class> track_object;
  g1_typed_reference_class<g1_smoke_trail_class> smoke_trail;

  i4_3d_vector end_point;
  float range;
  
  //  g1_voice_class *fire_sound; sfxfix
  int idle_ticks, should_draw;
public:
  g1_beam_weapon_class(g1_object_type id, g1_loader_class *fp);

  virtual void setup(const i4_3d_vector &pos,
                     const i4_3d_vector &dir,
                     g1_object_class *this_guy_fired_me,
                     float range);
  
  virtual void think();
  void unoccupy_location();
  i4_bool occupy_location();

  virtual void draw(g1_draw_context_class *context);
};

#endif
