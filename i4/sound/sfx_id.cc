/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "sound/sfx_id.hh"
#include "string/string.hh"
#include "file/file.hh"
#include "error/error.hh"
#include "loaders/wav_load.hh"
#include "sound/sound.hh"
#include "file/file.hh"
#include "math/transform.hh"
#include "time/profile.hh"
#include "music/stream.hh"
#include "main/main.hh"
#include "app/registry.hh"
#include <stdio.h>

i4_profile_class pf_sfx_update("sfx_update");

s1_sfx_ref *s1_sfx_ref::first=0;
s1_sound_handle *s1_sound_handle::first=0;
int show_sfx=0;


static char sfx_path[80];


static void get(char *var, char *buffer, int max_buffer, char *def)
{
  if (i4_get_registry(I4_REGISTRY_USER,
                      "SOFTWARE\\Crack dot Com\\i4\\1.0",
                      var, buffer, max_buffer))
    return ;
  
  char *c=getenv(var);
  if (c)
  {
    strncpy(buffer, c, max_buffer);
    return ;
  }
  
  strcpy(buffer, def);

}


char *s1_get_sfx_path()
{
  if (!sfx_path[0])
  {
    get("G_SFX_PATH", sfx_path, 80, "sfx");
  }

  return sfx_path;
}


static char *add_sfx_path(char *filename, char *buffer)
{
  sprintf(buffer, "%s/%s", s1_get_sfx_path(), filename);
  return buffer;
}

s1_sound_handle::~s1_sound_handle()
{
  if (playing)
    s1_end_looping(*this);
}

s1_sound_handle::s1_sound_handle()
{
  originator=0;
  next=0;
  channel_on=-1;
  playing=i4_F;
}

static i4_voice_class *load_sfx(s1_sfx_ref &ref)
{
  char buf[200];

  // don't try if no sound
  if (i4_sound_man==&i4_null_sound)
    return 0;

  i4_file_class *fp=i4_open(add_sfx_path(ref.name, buf));
  if (!fp) 
  {
    i4_warning("sfx missing %s (%s:%d)\n", ref.name, 
               ref.file_defined_in, ref.line_defined_on);
    return 0;
  }

  i4_sound_info info;
  i4_load_wav_info(fp, info);

  if (info.channels==2)
    i4_warning("%s is stereo sound (waste of memory)\n", ref.name);

  if (info.sample_rate>22*1024)
    i4_warning("%s is %dHz (waste of memory)\n", ref.name, info.sample_rate);
  else if (show_sfx)
    i4_warning("%s is %dHz\n", ref.name, info.sample_rate);


  i4_sound_manager_class::sound_parameters p;
  p.channels=info.channels;
  p.sample_size=info.sample_size;
  p.frequency=info.sample_rate;

  if (ref.flags & S1_3D)
    p.capable_3d=i4_T;
      
  i4_voice_class *v = i4_sound_man->alloc(info.size, p);
  if (v)
  {
    void *b1,*b2;
    w32   s1,s2;

    v->lock(0, info.size, b1, s1, b2, s2);
    fp->read(b1,s1);          
    v->unlock(b1, s1, b2, s2);
  }
  delete fp;

  return v;
}



s1_sfx_ref::s1_sfx_ref(char *filename, w8 flags, w8 priority, float dist, char *file, int line)
  : flags(flags),
    name(filename),
    file_defined_in(file),
    line_defined_on(line),
    priority(priority),
    hearable_distance_sqrd(dist*dist)
    
{  
  next=first;
  first=this;
  id=0;
  base_sfx=0;
}


s1_sfx_ref::~s1_sfx_ref()
{  
  if (first==this)
    first=first->next;
  else
  {
    s1_sfx_ref *last=0, *p=first;
    for (;p!=this && p;)
    {
      last=p;
      p=p->next;
    }
    if (!p) 
      i4_error("~sfx not found");
    last->next=next;
  }
}




struct channel_info
{
  i4_stream_wav_player *stream;
  i4_voice_class       *voice;
  s1_sfx_ref           *originator;
  s1_sound_handle      *loop_handler;

  void free()
  {
    if (stream)
    {
      delete stream;
      stream=0;
      voice=0;
    }
    else if (voice)
    {    
      delete voice;
      voice=0;
    }

    if (loop_handler)
    {
      loop_handler->channel_on=-1;
      loop_handler=0;    
    }

  }

};

enum { T_CHANNELS=8 };
static channel_info channels[T_CHANNELS];
static i4_3d_vector camera_pos;
static int channels_sorted=0;

void s1_get_camera_pos(i4_3d_vector &pos)
{
  pos=camera_pos;
}


void s1_load()      // load all the referenced static & dynamic sounds
{
  for (int j=1; j<i4_global_argc; j++)
    if (i4_global_argv[j]=="-sfxdbg")
      show_sfx=1;

  for (s1_sfx_ref *s=s1_sfx_ref::first; s; s=s->next)
    if ((s->flags&S1_STREAMED)==0 && !s->base_sfx)
      s->base_sfx=load_sfx(*s);

  for (int i=0; i<T_CHANNELS; i++)
  {
    channels[i].voice=0;
    channels[i].stream=0;
    channels[i].loop_handler=0;
    channels[i].originator=0;
    
  }
}


void s1_unload()
{
  for (s1_sfx_ref *s=s1_sfx_ref::first; s; s=s->next)
    if (s->base_sfx)
    {
      delete s->base_sfx;
      s->base_sfx=0;
    }
}


static float get_dist_sqrd(float x, float y, float z)
{
  return (camera_pos.x-x)*(camera_pos.x-x)+
         (camera_pos.y-y)*(camera_pos.y-y)+
         (camera_pos.z-z)*(camera_pos.z-z);
}

int channel_compare(const void *va, const void *vb)
{
  const channel_info *a=(channel_info *)va;
  const channel_info *b=(channel_info *)vb;

  if (a->originator->priority>b->originator->priority)
    return 1;
  else if (a->originator->priority<b->originator->priority)
    return -1;

  // non-looping sounds have priority over looping
  else if (a->loop_handler && !b->loop_handler)
    return -1;
  else if (!a->loop_handler && b->loop_handler)
    return 1;
  // compare the distance for 2 looping sounds
  else if (a->loop_handler && b->loop_handler)
  {
    float d1=a->loop_handler->dist_sqrd;
    float d2=b->loop_handler->dist_sqrd;
    if (d1>d2)
      return 1;
    else if (d1<d2)
      return -1;
    else return 0;
  }
  // compare the offset for non-looping sounds
  else
  {
    int o1=a->voice->get_sound_position();
    int o2=b->voice->get_sound_position();
    if (o1<o2) return -1;
    else if (o1>o2) return 1;
    else return 0;
  }
} 

int find_channel(s1_sfx_ref *ref, s1_sound_handle *h)
{
  int use=-1,i;
  for (i=0; i<T_CHANNELS; i++)
    if (!channels[i].voice)
    {
      channels_sorted=0;
      return i;
    }



  if (!channels_sorted)
  {
    qsort(channels, T_CHANNELS, sizeof(channel_info), channel_compare);
    channels_sorted=1;
  }

  // the worst channel after sorting
  channel_info *w=channels+T_CHANNELS-1;

  if (ref->priority<w->originator->priority)
    use=T_CHANNELS-1;
  else if (ref->priority==w->originator->priority) // we are not better
  {
    if (w->loop_handler && !h)
      use=T_CHANNELS-1;
    else if (w->loop_handler && h)
    {
      float d1=w->loop_handler->dist_sqrd;
      float d2=h->dist_sqrd;
      if (d1>d2)
        use=T_CHANNELS-1;
    }
    else if (!h)
      use=T_CHANNELS-1;
  }

  
  if (use!=-1)
  {
    channels_sorted=0;
    channels[use].free();

//     i4_warning("throwing out %s (priority %d) for %s (p=%d)",
//                channels[use].originator->name,
//                channels[use].originator->priority,
//                ref->name, ref->priority);
  }
//    else
//      i4_warning("no channel for %s (priority %d)",
//                 ref->name, ref->priority);


  return use;
}


void s1_set_camera_pos(i4_transform_class &new_cam_transform)
{
  int i=0;

  pf_sfx_update.start();

  i4_3d_vector last_pos=camera_pos;
  new_cam_transform.inverse_transform(i4_3d_vector(0,0,0), camera_pos);
  s1_sound_handle *s;

  
  i4_3d_point_class temp,cfv,cuv;  
  i4_3d_vector cam_velocity;
  cam_velocity = i4_3d_vector(camera_pos.x-last_pos.x,
                              camera_pos.y-last_pos.y,
                              camera_pos.z-last_pos.z);

  i4_sound_man->set_listener_position(-camera_pos.y, camera_pos.z, camera_pos.x);
  
  new_cam_transform.inverse_transform_3x3(i4_3d_vector(0,0,1),cfv);
  new_cam_transform.inverse_transform_3x3(i4_3d_vector(0,-1,0),cuv); //negative y in viewspace is actually "up"

  //just in case they dont come out exactly correct
  cfv.normalize();
  cuv.normalize();

  i4_sound_man->set_listener_orientation(-cfv.y,cfv.z,cfv.x,    -cuv.y,cuv.z,cuv.x);

  i4_sound_man->set_listener_velocity(-cam_velocity.y,cam_velocity.z,cam_velocity.x);



  for (s=s1_sound_handle::first; s; s=s->next)
    s->dist_sqrd=get_dist_sqrd(s->x, s->y, s->z);



  // free any sounds that are done
  for (i=0; i<T_CHANNELS; i++)
  {
    if (channels[i].voice)
    {
      // streamed sound?
      if (channels[i].stream && !channels[i].stream->poll())
        channels[i].free();
      // is this a looping 3d sound?    
      else if (channels[i].loop_handler)
      {
        // see if it has gone out of hearing range
        s1_sound_handle *h=channels[i].loop_handler;
        float dist=h->dist_sqrd;

        if (dist>h->originator->hearable_distance_sqrd)
          channels[i].free();
        else if (show_sfx)
        {
          i4_warning("looping %s (dist=%f)", 
                     channels[i].originator->name,
                     sqrt(dist));
        }
      }
      else if (!channels[i].stream && !channels[i].voice->is_playing())
        channels[i].free();
      else if (show_sfx)
        i4_warning("non-looping %s (pos=%d)", 
                   channels[i].originator->name,
                   channels[i].voice->get_sound_position());
    }
  }


  for (s=s1_sound_handle::first; s; s=s->next)
  {
    // find out if this sound is in hearing range
    float dist_sqrd=s->dist_sqrd;
    if (dist_sqrd<s->originator->hearable_distance_sqrd && s->channel_on==-1)
    {
      s1_sfx_ref *ref=s->originator;

      int c=find_channel(ref, s);
      if (c!=-1 && ref->base_sfx)
      {
        i4_voice_class *v = i4_sound_man->duplicate_3d(ref->base_sfx);
        if (v)
        {
          v->set_looping(i4_T);
          channels[c].voice=v;
          channels[c].loop_handler=s;
          channels[c].originator=s->originator;        
          s->channel_on=c;

          v->set_3d_position(-s->y, s->z, s->x, i4_T);
          v->set_3d_velocity(-s->yv, s->zv, s->xv,  i4_T);
          v->set_frequency( i4_f_to_i(v->default_frequency * s->frequency_scale));

          v->play();
        }
        else
          channels[c].free();

      }
    }

  }

  // now update any 3d sounds & velocities we are playing
  for (i=0; i<T_CHANNELS; i++)
  {
    i4_voice_class *v=channels[i].voice;
    if (v)
    {
      s1_sound_handle *s=channels[i].loop_handler;
      if (s)
      {
        v->set_3d_position(-s->y, s->z, s->x,  i4_T);
        v->set_3d_velocity(-s->yv, s->zv,  s->xv, i4_T);
        v->set_frequency( i4_f_to_i(v->default_frequency * s->frequency_scale));
      }
    }
  }

  i4_sound_man->commit_3d_changes();

  pf_sfx_update.stop();  
}


void s1_sfx_ref::play_looping(s1_sound_handle &handle)
{
  if (handle.playing)
    return;

  // add the sound to the list of looping 3d sounds
  // we will determine which ones are heard in set_camera_pos
  handle.originator=this;
  handle.next=s1_sound_handle::first;
  s1_sound_handle::first=&handle;
  handle.playing=i4_T;
}


void s1_end_looping(s1_sound_handle &handle)
{
  if (!handle.playing)
    return;

  handle.playing=i4_F;
  int i=handle.channel_on;

  // if the sound was playing, stop it
  if (i!=-1)
    channels[i].free();

  // remove the sound from the list of 3d sounds
  if (&handle==s1_sound_handle::first)
    s1_sound_handle::first=s1_sound_handle::first->next;
  else
  {
    s1_sound_handle *last=0, *p=s1_sound_handle::first;
    for (;p;)
    {
      if (p==&handle)
      {
        last->next=handle.next;
        return; 
      }
      last=p;
      p=p->next;
    }
    i4_error("end_looping not in list");
  }

}


void s1_sfx_ref::play(float x, float y, float z)
{
  int i;
  if (!base_sfx && ((flags&S1_STREAMED)==0))
    return ;

  // sound is too far away
  if ((flags&S1_3D) && get_dist_sqrd(x,y,z)>hearable_distance_sqrd)
    return ;
   
  int use=find_channel(this, 0);

  if (use!=-1)
  {
    int fail=0;
    i4_voice_class *s=0;


    if (flags&S1_STREAMED)
    {
      char fn[100];      
      if (i4_sound_man==&i4_null_sound)
        fail=1;
      else
      {
        i4_file_class *fp=i4_open(add_sfx_path(name,fn));
        if (fp)
        {    
          i4_stream_wav_player *stream;
          stream=new i4_stream_wav_player(fp, 64*1024, i4_F, i4_T);
          if (stream->load_failed())
          {
            i4_warning("stream load failed");
            delete stream;
            fail=1;
          }
          else
          {
            channels[use].stream=stream;
            s=stream->i4_voice();
          }
        }
        else
          fail=1;
      }
    }
    else
      s=i4_sound_man->duplicate_3d(base_sfx);

      

    if (!fail && s)
    {
      if (flags & S1_3D)
        s->set_3d_position(-y,z,x,i4_T);

      if ((flags &S1_STREAMED)==0)
        s->play();

      channels[use].voice=s;
      channels[use].originator=this;
      channels[use].loop_handler=0;
    }
    else
      channels[use].free();

  }
}


void s1_save_sfx_list(i4_file_class *fp)
{

  if (fp)
  {
    for (s1_sfx_ref *s=s1_sfx_ref::first; s; s=s->next)
    {
      fp->printf("filename '%s', priority %d, defined at %s:%d ",
                 s->name, s->priority, s->file_defined_in, s->line_defined_on);
      
      if (s->flags & S1_3D)
        fp->printf("3D ");

      if (s->flags & S1_STREAMED)
        fp->printf("Streamed ");

      fp->printf("\n");
    }
  }
}
