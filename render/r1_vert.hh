/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef R1_VERT_HH
#define R1_VERT_HH

#include "arch.hh"
#include "math/num_type.hh"
#include "math/point.hh"

// when calling a render_api's function, you need only specify r1_vert parameters
// that will actually be used.  i.e. if you have alpha off, you do not need to specify a

class r1_3d_point_class : public i4_3d_vector
{
public:  
  float pad;
};

typedef struct //dont reorder this stuff
{
  r1_3d_point_class v;          // 3d position (not used for rendering)
  
  float px, py,         // projected x & y
    s,t,                // texture coordinates 0..1
    r,g,b,              // shading values, if white light mode r acts at white
    a,                  // alpha  0..1
    w;                  // 1/z      used for w-buffering and perspective correct tmapping    

  w8 outcode;           // used for clipping (calculated by r1_full_clip)
  w8 pad0;
  w8 pad1;
  w8 pad2; //pad so its 8 bytes

  void set_color(i4_float _r, i4_float _g, i4_float _b, i4_float _a=1.) { r=_r; g=_g; b=_b; a=_a; }
  void set_st(i4_float _s, i4_float _t) { s=_s; t=_t; }

  i4_3d_point_class *point() { return (i4_3d_point_class *)&v; }
  // creates a pointer to a i4_3d_point_class that represents the
  // same values in v
} r1_vert;

#endif
