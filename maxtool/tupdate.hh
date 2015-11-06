/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef M1_TUDATE_HH
#define M1_TUDATE_HH


#include "string/string.hh"
#include "status/status.hh"

// this function checks the times of the textures with the
// compressed mip version and updates it if it is newer
// return true if texture was updated
i4_bool m1_update_texture(const i4_const_str &texture_filename,
                          i4_bool force_all,
                          i4_status_class *stat,
                          float start, float end);

// this calls m1_update_texture on all the textures in an object model file
// returns true if one or more textures were updated
i4_bool m1_update_object_textures(const i4_const_str &object_filename, 
                                  i4_bool force_all=i4_F,
                                  const i4_const_str *copy_to=0);


void m1_convert_all_in_dir(const i4_const_str &dir);

#endif
