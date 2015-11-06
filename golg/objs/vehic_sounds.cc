/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/vehic_sounds.hh"
#include "sound/sfx_id.hh"

class g1_average_sound : public s1_sound_handle
{
public:
  int t_contributors;

  void add_to_average(const i4_3d_vector& p)
  {
    x+=p.x; y+=p.y; z+=p.z;
    t_contributors++;
  }
  void reset();
};
 
g1_average_sound g1_average_rumbles[G1_T_RUMBLES];

S1_SFX(heli, "rumble/helicopter_lp.wav",            S1_3D, 40);
S1_SFX(jet , "rumble/jet_lp.wav",                   S1_3D, 40);
S1_SFX(ground, "rumble/peon_tank_lp.wav",           S1_3D, 40);
S1_SFX(missile, "misc/missile_in_flight_22khz.wav", S1_3D, 40); 
S1_SFX(s_stank, "rumble/supertank_lp.wav",          S1_3D, 50);


void g1_add_to_sound_average(g1_rumble_type type, const i4_3d_vector& v)
{
  i4_3d_vector pos;
  s1_get_camera_pos(pos);
  float r=20;
  pos -= v;
  if (pos.dot(pos)<r*r)
    g1_average_rumbles[type].add_to_average(v);
}


void g1_add_to_sound_average(g1_rumble_type type, const i4_3d_vector& v, const i4_3d_vector& vel)
{
  i4_3d_vector pos;
  s1_get_camera_pos(pos);
  float r=20;
  pos -= v;
  if (pos.dot(pos)<r*r)
  {
    g1_average_sound *s=g1_average_rumbles+type;
    s->add_to_average(v);
    s->xv=vel.x;
    s->yv=vel.y;
    s->zv=vel.z;
  }
}



void g1_average_sound::reset()
{
  t_contributors=0;
  x=0;
  y=0;
  z=0;
  frequency_scale=1.0;
}

void g1_reset_sound_averages()
{
  for (int i=0; i<G1_T_RUMBLES; i++)  
    g1_average_rumbles[i].reset();
 
}

void g1_recalc_sound_averages()
{
  g1_average_sound *s=g1_average_rumbles;
  for (int i=0; i<G1_T_RUMBLES; i++,s++)
  {
    if (s->t_contributors)
    {
      if (s->t_contributors!=1)
      {
        s->xv=s->yv=s->zv=0;    // can't average velocity

        float oo_t=1.0/(float)s->t_contributors;
        s->x*=oo_t;
        s->y*=oo_t;
        s->z*=oo_t;
      }
      
      if (!g1_average_rumbles[i].playing)
      {
        switch (i)
        {
          case G1_RUMBLE_GROUND :
            ground.play_looping(g1_average_rumbles[i]);
            break;

          case G1_RUMBLE_HELI :
            heli.play_looping(g1_average_rumbles[i]);
            break;

          case G1_RUMBLE_JET :
            jet.play_looping(g1_average_rumbles[i]);
            break;

          case G1_RUMBLE_MISSILE :
            missile.play_looping(g1_average_rumbles[i]);
            break;
        }
      }
    }
    else if (g1_average_rumbles[i].playing)
      s1_end_looping(g1_average_rumbles[i]);

  }
}


void g1_stop_sound_averages()
{
  for (int i=0; i<G1_T_RUMBLES; i++)  
    if (g1_average_rumbles[i].playing)
      s1_end_looping(g1_average_rumbles[i]);
}
