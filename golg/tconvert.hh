/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef TCONVERT_HH
#define TCONVERT_HH


#include "string/string.hh"
#include "material.hh"

enum { G1_TEXTURE_FILE,
       G1_ANIMATION_FILE };

g1_texture_handle convert_name_to_texture(const i4_const_str &name,
                                          g1_texture_mapper_class &tmapper);

w32 g1_convert_pal_filename_to_id(const i4_const_str &name);
void g1_create_best_palettes();

#endif
