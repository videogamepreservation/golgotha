/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "object_definer.hh"
#include "math/random.hh"
#include "map_vert.hh"
#include "map_man.hh"
#include "tick_count.hh"
#include "objs/model_draw.hh"
#include "loaders/load.hh"
#include "image/image.hh"
#include "map_vert.hh"
#include "objs/model_id.hh"
#include "time/profile.hh"

static i4_profile_class pf_cloud_shadow("Cloud Shadow");

i4_image_class *cloud_im=0;
int cloud_ref=0;

g1_model_ref cloud_model("cloud2");

inline float interp(float c1, float c2, float r)
{
  return (c2-c1)*r+c1;
}

static float xfract(float x)
{
  return x-(int)x;
}

class g1_cloud_class :public g1_object_class
{
public:
  int ticks_till_move;
  static int cloud_count;


  float x_offset;
  float y_offset;

  float cloud_x() { return x*2-g1_map_width/2.0; }
  float cloud_y() { return y*2; }


  g1_cloud_class(g1_object_type id, g1_loader_class *fp)
    : g1_object_class(id,fp)
  {    

    ticks_till_move=0;
    draw_params.setup("lightbulb");
    draw_params.flags|=g1_model_draw_parameters::NO_LIGHTING;
    
    if (!cloud_ref)
    {
      cloud_im=i4_load_image("bitmaps/cloud.tga");
      if (!cloud_im) i4_error("couldn't load cload.tga");
    }


    y_offset=x_offset=0;

    cloud_ref++;
    draw_params.setup(cloud_model.id());
  }

  ~g1_cloud_class()
  {
    cloud_ref--;
    if (!cloud_ref)
    {
      delete cloud_im;
      cloud_im=0;
    }
  }

  void apply_shadow()
  {
    pf_cloud_shadow.start();   
    int iw=cloud_im->width(), ih=cloud_im->height();
    int mw=g1_map_width, mh=g1_map_height;
    float rx=xfract(x_offset), ry=xfract(y_offset); 
   
    g1_map_vertex_class *v=g1_verts;



    int dy=(int)(y_offset);
    w32 *d=(w32 *)cloud_im->data;


    for (int vy=0; vy<=mh; vy++)
    {

      int dx=(int)x_offset;
      w32 *d=(w32 *)cloud_im->data + dx + dy*65;

      for (int vx=0; vx<=mw; vx++, v++)
      {
        float x1=interp(d[0]&0xff, d[1]&0xff, rx);
        float x2=interp(d[iw]&0xff, d[iw+1]&0xff, rx);
        int yc=(int)interp(x1,x2, ry);

        v->shadow_subtract=yc;
        v->light_sum|=0x80000000;

        if (dx==iw-1)
        {
          dx=0;
          d-=iw-1;
        }
        else
        {
          dx++;
          d++;
        }
      }

      dy++;
      if (dy==ih-1)
        dy=0;

    }

    pf_cloud_shadow.stop();
  }

  i4_bool occupy_location()
  {
    if (cloud_count)
      return i4_F;

    if (g1_object_class::occupy_location_center())
    {
      cloud_count++;
      apply_shadow();
      return i4_T;
    }
    else return i4_F;
  }



  void unoccupy_location()
  {
    cloud_count--;

    g1_map_vertex_class *v=g1_verts;
    int mw=g1_map_width, mh=g1_map_height;

    for (int vy=0; vy<mh; vy++)
    {
      for (int vx=0; vx<mw; vx++, v++)
      {
        v->shadow_subtract=0;
        v->light_sum|=0x80000000;
      }
    }

    g1_object_class::unoccupy_location();
  }


  void think()
  {
    if (!ticks_till_move)
    {
      ticks_till_move=0;
      g1_map_vertex_class *v=g1_verts;


      y_offset+=1.0/8.0;
      if (y_offset>=64)
        y_offset-=64;

      apply_shadow();
    }
    else ticks_till_move--;

    request_think();
  }

  void draw(g1_draw_context_class *context)
  {  
    // don't do anything because this is done by draw_sky
  }



};


int g1_cloud_class::cloud_count=0;


g1_object_definer<g1_cloud_class>
g1_cloud_def("cloud_shadow", g1_object_definition_class::EDITOR_SELECTABLE);

