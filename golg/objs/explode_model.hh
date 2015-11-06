/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef EXPLODE_MODEL_HH
#define EXPLODE_MODEL_HH

#include "arch.hh"
#include "math/num_type.hh"
#include "objs/map_piece.hh"

class g1_object_class;
class g1_draw_context_class;

class g1_explode_vertex_class
{
public:
  i4_3d_vector v;
  i4_float s,t;
};

class g1_explode_face_class
{
public:
  w8    tint_handle; //0 for no tint

  enum { MAX_VERTS=g1_quad_class::MAX_VERTS };
  w32 num_verts() const { return (indices[MAX_VERTS-1]==0xffff)? MAX_VERTS-1 : MAX_VERTS; }
  
  g1_quad_class::vertex_ref_type indices[MAX_VERTS];

  i4_3d_vector angles, l_angles, angular_vel;
  i4_3d_vector pos, l_pos, vel;
};

enum g1_stage_type { G1_APPLY_WHOLE, G1_APPLY_SING };  // same force for each piece?



struct g1_explode_params
{
  float gravity;
  struct stage
  {
    g1_stage_type type;
    int ticks;
    float force;     // f= force/(dist*dist)negative force sucks, postive expels, 0 does nothing
    float air_friction;
    
    void setup(int _ticks, float _force, g1_stage_type t=G1_APPLY_SING, float _air_friction=0.99)
    { ticks=_ticks; force=_force; type=t; air_friction=_air_friction; }
    
  };

  i4_3d_vector initial_vel;
  int t_stages, current_stage;
  stage stages[4];
  
  g1_explode_params();
};

class g1_explode_model_class : public g1_object_class
{
  void grab_model(g1_object_class *o, i4_3d_vector &obj_center);
public:
  g1_explode_params params;
  
  enum { DATA_VERSION=1 };
  i4_3d_vector center;
  
  g1_explode_model_class(g1_object_type id, g1_loader_class *fp);
  ~g1_explode_model_class();
  
  i4_float occupancy_radius() const {return 0.f;}

  g1_explode_vertex_class *vertices;
  sw16 num_vertices;

  g1_explode_face_class   *faces;
  sw16 num_faces;

  virtual void setup(g1_object_class *source_obj,
                     const i4_3d_vector &center,
                     g1_explode_params &params);
                       
  virtual void save(g1_saver_class *fp);
  virtual void draw(g1_draw_context_class *context);
  virtual void think();
};

#endif


