/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "m_flow.hh"
#include "saver.hh"
#include "error/error.hh"
#include "g1_speed.hh"
#include "device/device.hh"
#include "device/event.hh"
#include "device/kernel.hh"
#include "music/stream.hh"


i4_event_reaction_class *g1_frame_change_notify=0,   
  *g1_scene_change_notify=0, *g1_movie_stop_notify=0;


g1_cut_scene_class *g1_movie_flow_class::add_cut_scene(const i4_const_str &name)
{
  w32 i=t_cut_scenes;
  t_cut_scenes++;
  set=(g1_cut_scene_class **)i4_realloc(set, 
                                        sizeof(g1_cut_scene_class *)*t_cut_scenes, 
                                        "cut_scene list");


  set[i]=new g1_cut_scene_class(name);


  return set[i];
}


g1_movie_flow_class::~g1_movie_flow_class()
{
  for (w32 i=0; i<t_cut_scenes; i++)
    delete set[i];
  if (set)
    i4_free(set);  
}


g1_movie_flow_class::g1_movie_flow_class()
{
  t_cut_scenes=0;
  set=0;
  frame=0;  
  scene=0;
  memset(scores, 0, sizeof(scores));
}


void g1_cut_scene_class::save(g1_saver_class *fp)
{
  fp->start_version(DATA_VERSION_2);

  fp->write_32(t_frames);

  fp->write_16(T_PATHS);

  for (int i=0; i<T_PATHS; i++)
    paths[i].save(fp);


  if (wave_file)
    fp->write_counted_str(*wave_file);
  else fp->write_16(0);

  if (name)    
    fp->write_counted_str(*name);
  else fp->write_16(0);

  fp->end_version();
}

// note : data version check has already been done by the time load is called
g1_cut_scene_class::g1_cut_scene_class(g1_loader_class *fp, w16 ver)
{
  t_frames=fp->read_32();

  if (ver==DATA_VERSION_1)
  {
    paths[0].load(fp);
    paths[1].load(fp);
  }
  else if (ver==DATA_VERSION_2)
  {
    int tp=fp->read_16();
    I4_ASSERT(tp<=T_PATHS, "bad total paths in file");

    for (int i=0; i<tp; i++)
      paths[i].load(fp);
  }


  wave_file=fp->read_counted_str();
  name=fp->read_counted_str();
}

static g1_cut_scene_class *g1_load_cut_scene(g1_loader_class *fp)
{  
  w16 ver,size;
  fp->get_version(ver,size);

  if (ver==g1_cut_scene_class::DATA_VERSION_1 ||
      ver==g1_cut_scene_class::DATA_VERSION_2)
    return new g1_cut_scene_class(fp,ver);
  else 
  {
    fp->seek(fp->tell()+size);
    return 0;
  }
}



void g1_movie_flow_class::save(g1_saver_class *fp)
{
  fp->start_version(DATA_VERSION);
  fp->write_16(t_cut_scenes);
  fp->write_32(frame);

  w32 con=0;
  
  for (w32 i=0; i<t_cut_scenes; i++)
    set[i]->save(fp);

  fp->write_16(scene);
  fp->end_version();
}

g1_movie_flow_class *g1_load_movie_flow(g1_loader_class *fp)
{
  w16 ver,size;

  fp->get_version(ver,size);

  if (ver==g1_movie_flow_class::DATA_VERSION)
  {
    i4_bool error=i4_F;

    w32 t_cut=fp->read_16();
    w32 cframe=fp->read_32();

    g1_cut_scene_class **set;
    if (t_cut)
      set=(g1_cut_scene_class **)i4_malloc(sizeof(g1_cut_scene_class *)*t_cut,
                                                                "cut_scenes");
    else
      set=0;
  
    for (w32 i=0; i<t_cut; i++)
    {      
      set[i]=g1_load_cut_scene(fp);
      if (!set[i])
        error=i4_T;
    }

    if (error)
    {
      for (w32 i=0; i<t_cut; i++)
        if (set[i])
          delete set[i];
      i4_free(set);

      return 0;
    }

    g1_movie_flow_class *ret=new g1_movie_flow_class;
    ret->t_cut_scenes=t_cut;
    ret->set=set;
    ret->set_scene(fp->read_16());
    ret->set_frame(cframe);
    return ret;

  } else 
  {
    fp->seek(fp->tell() + size);
    return 0;
  }
}

void g1_movie_flow_class::poll_scores()
{
  for (w32 i=0; i<MAX_SCORES; i++)
    if (scores[i]) 
    {
      if (!scores[i]->poll())
      {
        delete scores[i];
        scores[i]=0;
      }
    }
}

void g1_movie_flow_class::stop()
{
  i4_kernel.send(g1_movie_stop_notify);

  for (w32 i=0; i<MAX_SCORES; i++)
    if (scores[i]) 
    {
      delete scores[i];
      scores[i]=0;
    }
}

void g1_movie_flow_class::start()
{
  if (current()->wave_file)
  {
    i4_stream_wav_player **slot=0;
    for (w32 i=0; i<MAX_SCORES && !slot; i++)
      if (!scores[i])
        slot=&scores[i];

    if (slot)
    {
      i4_file_class *fp=i4_open(*current()->wave_file, I4_READ | I4_SUPPORT_ASYNC);
      if (fp)
      {
        *slot=new i4_stream_wav_player(fp,
                                       256*1024,
                                       i4_F,
                                       i4_F);
      }
    }

  }  

  frame_time.get();
}


g1_movie_flow_class::advance_status g1_movie_flow_class::advance_movie_with_time()
{
  poll_scores();

  advance_status stat=PLAYING;

  if (t_cut_scenes)
  {
    i4_time_class now;
    sw32 md;

    do
    {
      md=now.milli_diff(frame_time);
      if (md>(1000/G1_MOVIE_HZ))
      {
        frame_time.add_milli((1000/G1_MOVIE_HZ));

        if (frame+1<current()->total_frames())
          set_frame(frame+1);              
        else if (scene+1<t_cut_scenes)
        {
          do
          {
            set_scene(scene+1);
            start();
            set_frame(0);
            stat=NEXT_SCENE;

            if (scene+1==t_cut_scenes && current()->total_frames()==0)
            {
              stop();
              return DONE;
            }

          } while (current()->total_frames()==0);
        }
        else
        {
          stop();
          return DONE;
        }
      }
    } while (md>(1000/G1_MOVIE_HZ));
  }
  return stat;
}

void g1_movie_flow_class::set_frame(w32 num)
{
  frame=num;
  i4_kernel.send(g1_frame_change_notify);

}

void g1_movie_flow_class::set_scene(w32 num)
{
  scene=num;
  i4_kernel.send(g1_scene_change_notify);
}


void g1_cut_scene_class::move(i4_float x_add, i4_float y_add, i4_float z_add)
{
  for (int i=0; i<T_PATHS; i++)
    paths[i].move(x_add, y_add, z_add);
}
