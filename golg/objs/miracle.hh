/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_MIRACLE_CLASS_HH
#define G1_MIRACLE_CLASS_HH

#include "g1_object.hh"

class g1_miracle_class : public g1_object_class
{
 
public:
  enum {DATA_VERSION=1};

  enum {NUM_ARC_POINTS=10};
  
  typedef struct
  {
    i4_3d_vector lposition,position;
  } arc_point_struct;
  
  struct arc_point_list_struct
  {
    arc_point_struct arc_points[NUM_ARC_POINTS];
  };

  void new_arc_points(i4_3d_vector src, i4_3d_vector dest, arc_point_struct *dest_points);
  void copy_old_points(arc_point_struct *dest_points);

  i4_array<arc_point_list_struct> *electric_arcs;

  g1_miracle_class(g1_object_type id, g1_loader_class *fp);
  ~g1_miracle_class();
  
  virtual void save(g1_saver_class *fp);
    
  enum {LIGHTNING=1};
  
  w32 miracle_type;
  
  g1_typed_reference_class<g1_convoy_class> convoy;   // the convoy it is killing

  void post_think();
  void think();
  void draw(g1_draw_context_class *context);  
  
  void setup(g1_convoy_class *convoy_to_attack, w32 _miracle_type, i4_3d_vector pos);

  i4_float occupancy_radius() const { return 0.4; }

  g1_voice_class  *miracle_sound;
};

#endif
