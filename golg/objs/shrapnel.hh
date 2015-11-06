/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef SHRAPNEL_HH
#define SHRAPNEL_HH

#include "g1_object.hh"
#include "player_type.hh"
#include "path.hh"
#include "objs/model_draw.hh"

class g1_solid_class;
class g1_map_piece_class;
class g1_particle_emitter_class;

class g1_shrapnel_class : public g1_object_class
{
public:
  w16 shrap_model_id;

  enum { MAX_SHRAPNEL_PIECES=5 };

  typedef struct
  {
    i4_3d_vector lposition,position,velocity;    
    w8 type;
  } shrapnel_piece;
  
  shrapnel_piece shrapnel_pieces[MAX_SHRAPNEL_PIECES];
  w32 num_shrapnel_pieces;
  w32 shrapnel_time;

  g1_shrapnel_class(g1_object_type id, g1_loader_class *fp);
  virtual void save(g1_saver_class *fp);
  
  virtual void setup(i4_float sx, i4_float sy, i4_float sz, w32 num_pieces,
                     int type);
  
  virtual void think();
  void draw(g1_draw_context_class *context);
};

#endif
