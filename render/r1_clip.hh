/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef R1_CLIP_HH
#define R1_CLIP_HH

#include "r1_vert.hh"
#include "error/error.hh"
#include "tex_id.hh"

class r1_render_api_class;
class i4_draw_context_class;

struct r1_clip_vert_array
{
  int buf_size;
  int total;
  r1_vert *buf;

  r1_vert *add()
  {
    I4_ASSERT(total<buf_size, "clip array full");
    total++;
    return buf+total-1;
  }

  r1_clip_vert_array(int buf_size, r1_vert *buf, int total) : total(total), buf_size(buf_size),
    buf(buf) { ; }
};

// clips a a polygon, returns an array indexs to new verts (contained in the orignal v array)
int *r1_clip(r1_clip_vert_array *v_array,
             int *src, int t_src,             // indexes into vertex array for initial verts
             int *dst1, int *dst2,            // destination index arrays
             int &t_dst,
             float center_x, float center_y,
             i4_bool clip_code_and_project_done);

void r1_clip_render_lines(int t_lines, r1_vert *verts, 
                          float center_x, float center_y,
                          r1_render_api_class *api);

void r1_clip_clear_area(int x1, int y1, int x2, int y2, w32 color, float z, 
                        i4_draw_context_class &context,
                        r1_render_api_class *api);

/// clips to window dimensions, but leaves clip list to the z buffer
void r1_clip_render_textured_rect(float x1, float y1, float x2, float y2, float z, float a,
                                  int win_width, int win_height,
                                  r1_texture_handle handle,
                                  int frame,
                                  r1_render_api_class *api,
                                  float s1=0, float t1=0,
                                  float s2=1, float t2=1);
                                  
inline i4_float r1_clip_delta() { return 0.001; }

extern i4_float r1_near_clip_z;
extern i4_float r1_far_clip_z;

inline w8 r1_calc_outcode(i4_3d_vector &v)
{
  w8 clip_code = 0;

  if (v.x>(v.z))    clip_code |= 1;
  else
  if (v.x<-(v.z))   clip_code |= 2;  
  
  if (v.y>(v.z))    clip_code |= 4;
  else
  if (v.y<-(v.z))   clip_code |= 8; 
  
  if (v.z<r1_near_clip_z)     clip_code |=16;
  else if (v.z>r1_far_clip_z) clip_code |=32;

  return clip_code;
}

inline w8 r1_calc_ortho_outcode(i4_3d_vector &v)
{
  w8 clip_code = 0;

  if (v.x>1.0)    clip_code |= 1;
  else
  if (v.x<-1.0)   clip_code |= 2;  
  
  if (v.y>1.0)    clip_code |= 4;
  else
  if (v.y<-1.0)   clip_code |= 8; 
  
  if (v.z<r1_near_clip_z)     clip_code |=16;
  else if (v.z>r1_far_clip_z) clip_code |=32;  
  
  return clip_code;
}

enum {R1_CLIP_NO_CALC_OUTCODE=1, R1_CLIP_ORTHO=128, R1_CLIP_PROJECT_CLIPPED_POINTS=2};

inline w8 r1_calc_outcode(r1_vert *v)
{
  w8 clip_code=0;

  if (v->v.x>(v->v.z))    clip_code |= 1;
  else
  if (v->v.x<-(v->v.z))   clip_code |= 2;  
  
  if (v->v.y>(v->v.z))    clip_code |= 4;
  else
  if (v->v.y<-(v->v.z))   clip_code |= 8; 
  
  if (v->v.z<r1_near_clip_z)     clip_code |=16;
  else if (v->v.z>r1_far_clip_z) clip_code |=32;  
  
  v->outcode = clip_code;

  return clip_code;
}

inline w8 r1_calc_ortho_outcode(r1_vert *v)
{
  w8 clip_code=0;

  if (v->v.x>1.0)    clip_code |= 1;
  else
  if (v->v.x<-1.0)   clip_code |= 2;
  
  if (v->v.y>1.0)    clip_code |= 4;
  else
  if (v->v.y<-1.0)   clip_code |= 8;
  
  if (v->v.z<r1_near_clip_z)     clip_code |=16;
  else if (v->v.z>r1_far_clip_z) clip_code |=32;  
  
  v->outcode = clip_code;

  return clip_code;
}

#endif
