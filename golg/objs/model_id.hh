/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_MODEL_ID_HH
#define G1_MODEL_ID_HH

#include "arch.hh"
#include "g1_limits.hh"
#include "memory/growheap.hh"
#include "init/init.hh"
#include "error/error.hh"
#include "memory/array.hh"

class i4_str;
class g1_quad_object_class;
class r1_render_api_class;
class r1_texture_manager_class;

typedef w16 g1_model_id_type;

// G3d model repository for the game
class g1_model_list_class : public i4_init_class
{
  friend g1_model_info_compare(const void *a, const void *b);
  struct model_info
  {
    g1_quad_object_class *model;
    char *name_start;
  } *array;


  i4_grow_heap_class *name_buffer;
  int   total_models;

  virtual void init();
  virtual void uninit();

public:
  void reset(i4_array<i4_str *> &model_names, r1_texture_manager_class *tmap);


  g1_quad_object_class *get_model(w16 handle) const
  { 
    if (handle>=total_models)
      i4_error("get_model : bad handle");

    return array[handle].model; 
  }

  g1_model_id_type find_handle(const char *name) const;
  void cleanup();
  
  char *get_model_name(w16 handle) const
  {
    return array[handle].name_start;
  }
};

extern g1_model_list_class g1_model_list_man;


class g1_model_ref
{
public:
  char *name;
  g1_model_id_type value;
  g1_model_ref *next;
  
  g1_model_ref(char *name=0);  // assumes name is a static value (it is not copied)
  void set_name(char *name);
  ~g1_model_ref();

  g1_model_id_type id() const { return value; }
  g1_quad_object_class *get() const { return g1_model_list_man.get_model(value); }
  g1_quad_object_class *operator()() const { return g1_model_list_man.get_model(value); }
};

#endif
