/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_MODEL_DRAW_HH
#define G1_MODEL_DRAW_HH

#include "arch.hh"

class g1_object_class;
class g1_draw_context_class;
class g1_quad_object_class;

struct g1_model_draw_parameters
{
  enum
  {
    SUPPRESS_SPECIALS=1,
    NO_LIGHTING=2            // draws object full-bright
  };

  g1_quad_object_class *model;
  g1_quad_object_class *shadow_model;
  g1_quad_object_class *lod_model;

  w16 frame;           // current animation frame
  w16 animation;       // current animation number in model
  w32 flags;           // drawing modifiers

  g1_model_draw_parameters()
  {
    model           = 0;
    shadow_model    = 0;
    lod_model       = 0;
    frame           = 0;
    animation       = 0;
    flags           = 0;
  }
  
  void setup(const char *model_name, char *shadow_model_name=0, char *lod_model_name=0);
  void setup(w16 model_id, w16 shadow_model=0, w16 lod_model=0);
  void setup(g1_quad_object_class *model, 
             g1_quad_object_class *shadow_model=0,
             g1_quad_object_class *lod_model=0);
  
  float extent() const;     // gets extents of model_id from model_draw
};

void g1_model_draw(g1_object_class *_this,
                   g1_model_draw_parameters &params,
                   g1_draw_context_class *context);


void g1_editor_model_draw(g1_object_class *_this,
                          g1_model_draw_parameters &params,
                          g1_draw_context_class *context);    // calls model draw if in edit mode

#endif
