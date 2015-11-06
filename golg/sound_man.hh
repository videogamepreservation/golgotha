/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_SOUND_MAN_HH
#define G1_SOUND_MAN_HH

#include "sound/sound.hh"
#include "memory/que.hh"
#include "math/transform.hh"


class g1_sound_manager_class;
class g1_sfx_obj_class;


class g1_sound_manager_class
{
public:
  g1_sfx_obj_class *sfx_obj_list;
  i4_bool loop_current_song;

  g1_sound_manager_class();
  void add_sfx_to_list(g1_sfx_obj_class *sfx);
  void remove_sfx_from_list(g1_sfx_obj_class *sfx);

  // should be calle by main game every tick, check to see if a new narative should be played
  // and to update the position of the 3d listener
  void poll(i4_bool game_is_running);   

  void pause();
  void unpause();
  void next_song();
};

extern g1_sound_manager_class g1_sound_man;

#endif
