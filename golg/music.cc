/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "music.hh"
#include "music/stream.hh"
#include "string/string.hh"
#include "file/file.hh"
#include "memory/malloc.hh"

g1_music_manager_class g1_music_man;

void g1_music_manager_class::next_song()
{
  if (no_songs) return;
  
  song_on++;
  
  if (song_on>=total_songs)
    song_on=0;

  if (stream)
  {
    delete stream;
    stream=0;
  }

  i4_const_str *slist=i4_string_man.get_array("songs");

  if (!slist[song_on].null())
  {
    i4_file_class *fp=i4_open(slist[song_on], I4_SUPPORT_ASYNC | I4_READ);
    if (fp)
    {      
      stream = new i4_stream_wav_player(fp, 512*1024, 
                                        i4_F,            // don't loop the song
                                        i4_F);           // first load is not async
      if (stream)
      {
        stream->set_volume(I4_SOUND_VOLUME_LEVELS*2/4);

        if (!stream->poll())  // music probably doesn't work if we can't poll() once
        {
          delete stream;
          stream=0;

          total_missing++;
          if (total_missing==total_songs) no_songs = i4_T;
        }   
      }
    }
    else
    {
      total_missing++;
      if (total_missing==total_songs) no_songs = i4_T;
    }
  }  

  i4_free(slist);   
}

void g1_music_manager_class::init()
{
  playing=i4_F;
  no_songs=i4_F;
  stream=0;
  song_on=-1;

  i4_const_str *slist=i4_string_man.get_array("songs");

  total_missing = 0;

  total_songs = 0;
  for (sw32 i=0; !slist[i].null(); i++) total_songs++;

  if (total_songs==0) no_songs = i4_T;

  i4_free(slist);
}

void g1_music_manager_class::poll()
{
  if (stream)
  {
    if (//playing && 
        !stream->poll())
      next_song();
  }
  else
    next_song();
}

void g1_music_manager_class::uninit()
{
  stop();
}

void g1_music_manager_class::start()
{
  if (!stream)
  {    
    next_song();
    if (stream)
      playing=i4_T;
  }
}

void g1_music_manager_class::stop()
{
  if (stream)
  {
    delete stream;
    stream=0;
  }
  playing=i4_F;
}

