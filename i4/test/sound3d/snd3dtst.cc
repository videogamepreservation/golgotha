/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "drive_map.hh"
#include "main/main.hh"
#include "sound/sound3d.hh"

#ifdef WIN32

#include "app/app.hh"

class sound_test_class : public i4_application_class
//{{{
{
public:
  I4_EVENT_HANDLER_NAME("m1_utility_app_class");

  virtual w32 max_memory() const { return 1000 * 1024; }
  
  sound_test_class(char *resource_file, void *resource_buffer) 
  {
    memory_init();

    g1_map_drives();

    resource_init(resource_file,resource_buffer);
    display_init();
  }
};
//}}}

sound_test_class *app;

#endif

int my_sleep(int val)
//{{{
{
  int val2 = -1;

#ifdef __linux
  val *= 10;
#else
  val *= 10;
#endif

  for (int i=0; i<val; i++)
    val2 *= val2;

  return val2;
}
//}}}

void i4_main(w32 argc, i4_const_str *argv)
//{{{
{
#ifdef WIN32
  app = new sound_test_class("resource.res",0);
#endif
  i4_3d_voice_class *snd;
  int i;

  i4_init();

#ifndef WIN32
  g1_map_drives();
#endif

  i4_string_man.load(0, "resource.res");

  i4_3d_sound_man->load_sounds(16);

  i4_3d_sound_parameters param(11025,63);
  param.looping = i4_T;

  snd = i4_3d_sound_man->play(0, param);

  for (i=-1000; i<1000; i+=50)
  {
    snd->position.x = i;
    snd->update();
    my_sleep(40000);
  }
  snd->position.z = 100;
  snd->velocity.x = 100;

  for (i=-1000; i<1000; i+=50)
  {
    snd->position.x = i;
    snd->update();
    my_sleep(40000);
  }

  snd->position.x = 100;
  snd->velocity.z = 100;
  snd->velocity.x = 0;

  for (i=-1000; i<1000; i+=50)
  {
    snd->position.z = i;
    snd->update();
    my_sleep(40000);
  }
  i4_uninit();
}
//}}}

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
