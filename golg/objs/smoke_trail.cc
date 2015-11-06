/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/smoke_trail.hh"
#include "saver.hh"
#include "g1_render.hh"
#include "map.hh"
#include "map_man.hh"
#include "object_definer.hh"
#include "draw_context.hh"

g1_object_definer<g1_smoke_trail_class>
g1_smoke_trail_def("smoke_trail",
                   g1_object_definition_class::HAS_ALPHA);

void g1_smoke_trail_class::setup(i4_float start_x, i4_float start_y, i4_float start_h,
                                 i4_float start_width, i4_float end_width,
                                 i4_color start_color, i4_color end_color)
{
  lx=x=start_x;
  ly=y=start_y;
  lh=h=start_h;

  ticks_advanced=0;

  for (int i=0; i<TAIL_LENGTH; i++)
    tspots[i]=i4_3d_point_class(start_x, start_y, start_h);

  sc=start_color;
  ec=end_color;
  sw=start_width;
  ew=end_width;
}

g1_smoke_trail_class::g1_smoke_trail_class(g1_object_type id,
                                           g1_loader_class *fp)
  : g1_object_class(id, fp)
{
  if (fp && fp->check_version(DATA_VERSION))
  {
    ticks_advanced=fp->read_16();

    sc=fp->read_32();
    ec=fp->read_32();

    sw=fp->read_float();
    ew=fp->read_float();

    int t_spots=fp->read_8();
    for (int i=0; i<t_spots; i++)
    {
      i4_float x,y,z;
      x=fp->read_float();
      y=fp->read_float();
      z=fp->read_float();
      if (i<TAIL_LENGTH)
        tspots[i]=i4_3d_point_class(x,y,z);
    }

    fp->end_version(I4_LF);
  }
}

void g1_smoke_trail_class::save(g1_saver_class *fp)
{
  g1_object_class::save(fp);

  fp->start_version(DATA_VERSION);

  fp->write_16(ticks_advanced);

  fp->write_32(sc);
  fp->write_32(ec);

  fp->write_float(sw);
  fp->write_float(ew);

  fp->write_8(TAIL_LENGTH);
  for (int i=0; i<TAIL_LENGTH; i++)
  {
    fp->write_float(tspots[i].x);
    fp->write_float(tspots[i].y);
    fp->write_float(tspots[i].y);
  }

  fp->end_version();
}


void g1_smoke_trail_class::update_head(i4_float nx, i4_float ny, i4_float nh)
{
  sw32 ix=(sw32)nx,iy=(sw32)ny;

  if (ix>=0 && ix<g1_get_map()->width() && iy>=0 && iy<g1_get_map()->height())
  {
    unoccupy_location();

    if (ticks_advanced<=TAIL_LENGTH)
      ticks_advanced++;

    for (int i=TAIL_LENGTH-1; i>0; i--)
      tspots[i]=tspots[i-1];

    tspots[0]=i4_3d_point_class(lx,ly,lh);

    lx=x; ly=y; lh=h;

    x=nx; y=ny; h=nh;
  
    occupy_location();
  }
}

void g1_smoke_trail_class::draw(g1_draw_context_class *context)
{
  //w32 cur_frame=context->tmap->get_frame_counter();

  i4_float fr=g1_render.frame_ratio;

  i4_3d_point_class s[TAIL_LENGTH+1];
  s[0].x=(x-lx)*fr + lx;     s[0].y=(y-ly)*fr + ly;      s[0].z=(h-lh)*fr + lh;

  s[1].x=(lx-tspots[0].x)*fr + tspots[0].x;
  s[1].y=(ly-tspots[0].y)*fr + tspots[0].y;
  s[1].z=(lh-tspots[0].z)*fr + tspots[0].z;


  for (int i=0; i<TAIL_LENGTH-1; i++)
  {
    s[i+2].x=(tspots[i].x-tspots[i+1].x)*fr + tspots[i+1].x;
    s[i+2].y=(tspots[i].y-tspots[i+1].y)*fr + tspots[i+1].y;
    s[i+2].z=(tspots[i].z-tspots[i+1].z)*fr + tspots[i+1].z;
  }
  
  g1_render.add_translucent_trail(context->transform, s, ticks_advanced, sw, ew, 1, 0, sc, ec);

}

void g1_smoke_trail_class::think()
{
}

