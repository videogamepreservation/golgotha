/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_TAKEOVER_PAD_HH
#define G1_TAKEOVER_PAD_HH


#include "g1_object.hh"
#include "objs/model_draw.hh"

class i4_stream_wav_player;

extern g1_object_type g1_sfx_obj_type;

class g1_sfx_obj_class : public g1_object_class
{ 
  enum { DATA_VERSION=1 };
public:
  static g1_sfx_obj_class *cast(g1_object_class *obj)
  {
    if (!obj || obj->id != g1_sfx_obj_type)
    { 
#ifdef I4_CAST_WARN
      i4_warning("bad cast to sfx_obj!\n"); 
#endif
      return 0; 
    }
    
    return (g1_sfx_obj_class *)obj;
  }

  i4_stream_wav_player *stream;
  i4_str *filename;
  w16 max_volume;
  w32 max_hearable_distance;
  w32 restart_delay;
  w32 random_restart_delay;   
  w32 current_delay;         // time to count down until next sound is played
  float dist_from_camera_sqrd;    // calculated by g1_sound_man

  g1_sfx_obj_class *next_sfx;
  
  g1_sfx_obj_class(g1_object_type id, g1_loader_class *fp);
  ~g1_sfx_obj_class();
  virtual void save(g1_saver_class *fp);

  i4_str *get_context_string();  

  void set_filename(const i4_const_str &fname);

  // adds self to sound_manager
  i4_bool occupy_location();

  // remove self from sound_manager
  void unoccupy_location();
  virtual void draw(g1_draw_context_class *context);
  virtual void think();
};


#endif
