/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_MUSIC_HH
#define G1_MUSIC_HH

#include "arch.hh"
class i4_stream_wav_player;

class g1_music_manager_class
{
  i4_bool no_songs;
  sw32 song_on;
  sw32 total_songs;
  sw32 total_missing;
  i4_stream_wav_player *stream;
  i4_bool playing;

  void next_song();

public: 
  void init();
  void uninit();

  void poll();  // called by main game once per game loop

  void start(); 
  void stop();
};

extern g1_music_manager_class g1_music_man;


#endif
