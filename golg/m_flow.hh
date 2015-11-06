/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_MOVIE_FLOW_HH
#define G1_MOVIE_FLOW_HH

#include "math/spline.hh"
#include "string/string.hh"
#include "time/time.hh"


class g1_loader_class;
class g1_saver_class;
class i4_file_class;
class i4_event_handler_class;
class i4_event_reaction_class;
class i4_stream_wav_player;

// this class contains a single cut scene
class g1_cut_scene_class
{
public:
  enum { DATA_VERSION_1=1,
         DATA_VERSION_2
  };  // for loading/saving data

  enum { CAMERA, TARGET, OBJECT, T_PATHS };
  i4_spline_class paths[3];



  w32 t_frames;              // frames in cut scene

  i4_str *wave_file;         // sound effect to play at the begining of cut scene
  i4_str *name;              // name of cut scene to show in editor

  void move(i4_float x_add, i4_float y_add, i4_float z_add);

  g1_cut_scene_class(const i4_const_str &name) : name(new i4_str(name))
  {
    t_frames=0;
    wave_file=0;    
  }

  g1_cut_scene_class(g1_loader_class *fp, w16 ver);

  w32 total_frames()
  {
    w32 tf=paths[0].last_frame();

    for (int i=1; i<T_PATHS; i++)
    {
      w32 t=paths[i].last_frame();
      if (t>tf)
        tf=t;
    }
    
    return tf;
  }

  ~g1_cut_scene_class()
  {
    if (name)
      delete name;
    if (wave_file)
      delete wave_file;
  }

  void save(g1_saver_class *fp);
  
};

// this class manages a set of cut scenes
class g1_movie_flow_class
{
  w32 frame, scene;

  enum { MAX_SCORES=4 };  // maximum streaming sounds that can be played at the same time
  i4_stream_wav_player *scores[4];

  void poll_scores();     // called by advance_movie_with_time  

public:
  enum { DATA_VERSION=1 };  // for loading/saving data
  enum { MESSAGE_FRAME_CHANGED, MESSAGE_SCENE_CHANGED, MESSAGE_MOVIE_STOPPED };

  g1_cut_scene_class **set;

  w32 t_cut_scenes;


  i4_time_class frame_time;

  g1_cut_scene_class *add_cut_scene(const i4_const_str &name);
  g1_movie_flow_class();

  g1_cut_scene_class *current() { if (set) return set[scene]; else return 0; }

  w32 get_scene() { return scene; }
  void set_scene(w32 num);
  
  w32 get_frame() { return frame; }
  void set_frame(w32 num);

  ~g1_movie_flow_class();

  void save(g1_saver_class *fp);

  enum advance_status { DONE, NEXT_SCENE, PLAYING };
  advance_status advance_movie_with_time();

  void start();   // should be called when a movie begins playing
  void stop();    // should be called whan a movie is done playing
};

g1_movie_flow_class *g1_load_movie_flow(g1_loader_class *fp);


extern i4_event_reaction_class *g1_frame_change_notify,   
  *g1_scene_change_notify, *g1_movie_stop_notify;


#endif
