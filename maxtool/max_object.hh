/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef M1_MAX_OBJECT_HH
#define M1_MAX_OBJECT_HH

#include "obj3d.hh"
#include "memory/array.hh"
#include "string/string.hh"

class i4_saver_class;

class m1_poly_object_class : public g1_quad_object_class
{
  void save_quads(i4_saver_class *fp);
  void save_texture_names(i4_saver_class *fp);
  void save_vert_animations(i4_saver_class *fp);
  void save_mount_points(i4_saver_class *fp);
  void save_specials(i4_saver_class *fp);
  void calc_quad_normal(g1_vert_class *v, g1_quad_class &q);

public:
  void calc_texture_scales();
  void calc_vert_normals();

  enum { INVALID_QUAD = (1<<7) };  // g1_quad_class flag (*NOTE: bit 7 must be available)
  enum { MAX_ANIMATIONS=20 };
  enum { MAX_TEXTURE_NAMES=MAX_ANIMATIONS * 200 };

  enum { POLY_SELCTED=1 };

  i4_array<i4_str *> texture_names;
  i4_array<i4_str *> animation_names;
  i4_array<i4_str *> mount_names;
  i4_array<w32>      vert_flags;

  i4_array<animation_class>      anim_store;
  i4_array<g1_vert_class>        vert_store;
  i4_array<g1_quad_class>        quad_store;
  i4_array<i4_3d_vector>         mount_store;
  i4_array<w32>                  mount_id_store;
  i4_array<g1_texture_animation> special_store;
  
  ~m1_poly_object_class()
  //{{{
  {
    w32 i;
    for (i=0; i<texture_names.size(); i++)
      delete texture_names[i];
      
    for (i=0; i<animation_names.size(); i++)
      delete animation_names[i];
      
    for (i=0; i<mount_names.size(); i++)
      delete mount_names[i];
  }
  //}}}

  int get_poly_vert_flag(int poly_num, int flag)
  //{{{
  { 
    if (poly_num>=vert_flags.size()) 
      return 0; 
    else return vert_flags[poly_num]&flag ? 1 : 0;
  }
  //}}}

  void set_poly_vert_flag(int poly_num, int flag, int value)
  //{{{
  {
    while (vert_flags.size()<=poly_num) vert_flags.add();
    if (value)
      vert_flags[poly_num] |= flag;
    else
      vert_flags[poly_num] &= ~flag;
  }
  //}}}

  void add_animation(m1_poly_object_class *other, w32 animation_number);
  void delete_animation(w32 animation_number);

  void add_frame(w32 anim, w32 frame);
  void remove_frame(w32 anim, w32 frame);
  
  w32 add_vertex();
  void remove_vertex(w32 num);

  w32 add_quad();
  void remove_quad(w32 num);

  w32 add_mount();
  void remove_mount(w32 num);

  w32 add_special(w32 quad_number);
  void remove_special(w32 num);

  void save(i4_saver_class *fp);
  void change_animation_name(w32 animation_number, const i4_const_str &st);

  m1_poly_object_class() 
    : anim_store(0,1), vert_store(0,100), quad_store(0,100), 
      mount_store(0,10), mount_id_store(0,10), special_store(0,10),
      texture_names(0, 32), animation_names(0,32), vert_flags(0,32), mount_names(0,32) {}
};
                                    
#endif

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
