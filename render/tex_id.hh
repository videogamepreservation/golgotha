/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __R1_TEXTURE_ID_HH_
#define __R1_TEXTURE_ID_HH_

#include "arch.hh"
typedef sw16 r1_texture_handle;   

class i4_const_str;
class i4_str;
i4_str *r1_texture_id_to_filename(w32 id, const i4_const_str &out_dir);
w32     r1_get_texture_id(const i4_const_str &full_filename);


struct r1_texture_ref
{
public:
  static r1_texture_ref *first;

  char *name;
  r1_texture_handle texture_handle;
  
  r1_texture_ref(char *texture_name) :  name(texture_name)
  { texture_handle=0; next=first; first=this; }
  
  ~r1_texture_ref();
  
  r1_texture_ref *next;
  
  r1_texture_handle get()  { return texture_handle; }
};


#endif
