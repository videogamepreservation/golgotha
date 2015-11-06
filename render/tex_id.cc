/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "tex_id.hh"
#include "string/string.hh"
#include "file/file.hh"
#include "string/str_checksum.hh"
#include "r1_res.hh"

r1_texture_ref *r1_texture_ref::first=0;

i4_str *r1_texture_id_to_filename(w32 id, 
                                  const i4_const_str &out_dir)
{
  return i4_const_str("%S/%x.gtx").sprintf(100,
                                          &out_dir,
                                          id);

}

w32 r1_get_texture_id(const i4_const_str &full_filename)
{
  if (!full_filename.null())
  {
    i4_filename_struct fn;
    i4_split_path(full_filename, fn);
    w32 id=i4_str_checksum(fn.filename);
    return id;
  }
  else return 0;
}



r1_texture_ref::~r1_texture_ref()
{
  if (this==first)
    first=first->next;
  else
  {
    r1_texture_ref *p, *last=0;    
    for (p=first; p!=this;)
      last=p; p=p->next;
    last->next=next;
  }  
}
