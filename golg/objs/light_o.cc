/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/light_o.hh"
#include "saver.hh"
#include "objs/model_draw.hh"
#include "map.hh"
#include "math/num_type.hh"
#include "object_definer.hh"
#include "map_man.hh"
#include "flare.hh"
#include "time/profile.hh"
#include "map_vert.hh"

static i4_profile_class pf_light_occupy("light::occupy"), pf_light_unoccupy("light::unoccupy");

g1_object_type g1_lightbulb_type;
void g1_light_object_init();

g1_object_definer<g1_light_object_class>
g1_light_object_def("lightbulb", 
                    g1_object_definition_class::EDITOR_SELECTABLE,
                    g1_light_object_init);

void g1_light_object_init()
{
  g1_lightbulb_type = g1_light_object_def.type;
}


void g1_light_object_class::setup(float _x, float _y, float _h, 
                                  float _r, float _g, float _b, float _white,
                                  float min_light_contribute, 
                                  float linear_contribute, 
                                  float geometric_contribute)
{
  r=_r;
  g=_g;
  b=_b;
  x=lx=_x;
  y=ly=_y;
  h=lh=_h;

  int occ=get_flag(MAP_OCCUPIED);

  if (occ)
    unoccupy_location();

  if (add_intensities)
    i4_free(add_intensities);


  c1=min_light_contribute;
  c2=linear_contribute;
  c3=geometric_contribute;

  if (c3<0.0016)  // maximum radius is 25 squares for now
    c3=0.0016;

  change_radius=i4_f_to_i(sqrt((32-c1)/c3));
  
  sw32 w=change_radius*2+1,h=change_radius*2+1;

  add_intensities=(w32 *)i4_malloc(w*h*sizeof(w32),
                                   "light map restore");  
  
  if (occ)
    occupy_location();
  
}

g1_light_object_class::~g1_light_object_class()
{
  if (add_intensities)
    i4_free(add_intensities);
}

g1_light_object_class::g1_light_object_class(g1_object_type id,
                                             g1_loader_class *fp)
  : g1_object_class(id,fp)
{
  add_intensities=0;

  if (fp && fp->check_version(DATA_VERSION))
  {
    float _r, _g, _b, _white, _c1, _c2, _c3;
    fp->read_format("fffffff", &_r, &_g, &_b, &_white, &_c1, &_c2, &_c3);
    setup(x,y,h, _r, _g, _b, _white, _c1, _c2, _c3);

    fp->end_version(I4_LF);
  }
  else
  {
    r=g=b=1; 
    white=1;    
    c1=0.1;    c2=0.25;    c3=0.5;    
    h=2;
  }

  draw_params.setup("lightbulb");
}


void g1_light_object_class::save(g1_saver_class *fp)
{
  g1_object_class::save(fp);
  fp->start_version(DATA_VERSION);
  fp->write_format("fffffff", &r,&g,&b, &white, &c1,&c2,&c3);
  fp->end_version();
}

void g1_light_object_class::draw(g1_draw_context_class *context)
{  
  g1_editor_model_draw(this, draw_params, context);
}


void g1_light_object_class::move(float nx, float ny, float nh)
{
  unoccupy_location();
  lx=x; ly=y; lh=h;
  x=nx; y=ny; h=nh;

  occupy_location();
}


void g1_light_object_class::think()
{
}

i4_bool g1_light_object_class::occupy_location()
{
  if (!add_intensities)
  {
    i4_warning("call light::setup before occupy_location");
    
    return i4_F;
  }
  
  pf_light_occupy.start();
  if (g1_object_class::occupy_location())
  {

    
    sw32 ix=i4_f_to_i(x), iy=i4_f_to_i(y);
    w32 *a=add_intensities;

    for (int ty=-change_radius+iy; ty<=change_radius+iy; ty++)
    {
      sw32 start_x=ix-change_radius;
      if (start_x<0) start_x=0;
      g1_map_vertex_class *v=g1_get_map()->vertex(start_x,ty);

      for (int tx=-change_radius+ix; tx<=change_radius+ix; tx++, a++)
      {
        if (tx>=0 && ty>=0 && tx<=g1_get_map()->width() && ty<=g1_get_map()->height())
        {
          i4_3d_vector normal;
          i4_float tz;
          w32 old_rgb=v->dynamic_light;

          v->get_normal(normal, tx, ty);
          tz=v->get_height();

          i4_3d_vector dir=i4_3d_vector(x-tx,
                                        y-ty, 
                                        h-tz);

          i4_float dist=sqrt(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
          i4_float odist=1.0/dist;

          dir.x*=odist;  // normalize the light direction vector
          dir.y*=odist;
          dir.z*=odist;

          i4_float ndl = normal.dot(dir);
          i4_float atten =  1.0/(c1 + c3*dist*dist);

          if (atten>1) atten=1;

          i4_float intensity=ndl*atten;
          if (intensity<0) intensity=0;

          sw32 ra,ga,ba, or,og,ob;

          ra=i4_f_to_i(intensity * r * 255);    // calculate how much to add to the current light
          ga=i4_f_to_i(intensity * g * 255);
          ba=i4_f_to_i(intensity * b * 255);

          or=(old_rgb>>16)&255;              // grab the old light values
          og=(old_rgb>>8)&255;
          ob=(old_rgb)&255;
      
          if (ra+or>255) ra=255-or;           // adjust for overflow
          if (ga+og>255) ga=255-og;
          if (ba+ob>255) ba=255-ob;

          *a=(ra<<16)|(ga<<8)|ba;           // store the added amount so we can subtract out later
        
          v->dynamic_light=((ra+or)<<16) | ((ga+og)<<8) | (ba+ob);
          v->light_sum|=0x80000000;
          v++;
        }
      }
    }
    pf_light_occupy.stop();
    return i4_T;
  }
  else
  {
    pf_light_occupy.stop();
    return i4_F;
  }
}

void g1_light_object_class::unoccupy_location()
{ 
  if (!add_intensities)
    return ;

  pf_light_unoccupy.start();
  g1_object_class::unoccupy_location();

  sw32 w=change_radius*2+1,h=change_radius*2+1;
  sw32 ix=i4_f_to_i(x), iy=i4_f_to_i(y);
  w32 *a=add_intensities;

  for (int ty=-change_radius+iy; ty<=change_radius+iy; ty++)
  {
    sw32 start_x=ix-change_radius;
    if (start_x<0) start_x=0;
    g1_map_vertex_class *v=g1_get_map()->vertex(start_x,ty);
    for (int tx=-change_radius+ix; tx<=change_radius+ix; tx++, a++)
    {
      if (tx>=0 && ty>=0 && tx<=g1_get_map()->width() && ty<=g1_get_map()->height())      
      {
        v->dynamic_light-=*a;
        v++;
        v->light_sum=0x80000000;
      }
    }
  }
  
  pf_light_unoccupy.stop();
}

