/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "image/image.hh"
#include "tex_cache.hh"
#include "tex_id.hh"
#include "mip.hh"
#include "file/file.hh"
#include "r1_res.hh"
#include "memory/malloc.hh"

int r1_load_gtext(w32 id, i4_image_class **images)
{
  i4_file_class *fp;
  tex_cache_header_t theader;
  mipheader_t header;

  fp=i4_open(r1_get_cache_file());
  if (!fp) 
    return 0;

  theader.read(fp);
  delete fp;


  i4_str *fn = r1_texture_id_to_filename(id, r1_get_decompressed_dir());
  fp=i4_open(*fn);
  delete fn;
  if (!fp) 
    return 0;

  header.read(fp);

  int w=header.base_width, h=header.base_height, i; 
  const i4_pal *pal;

  if (header.flags & R1_MIP_IS_TRANSPARENT)   
    pal=i4_pal_man.register_pal(&theader.chroma_format);
  else if (header.flags & R1_MIP_IS_ALPHATEXTURE)
    pal=i4_pal_man.register_pal(&theader.alpha_format);
  else
    pal=i4_pal_man.register_pal(&theader.regular_format);

  for (i=0; i<header.num_mip_levels; i++)
  {
    fp->seek(header.offsets[i]+8);
    
    w16 *data=(w16 *)i4_malloc(w*h*2,"");
    fp->read(data,w*h*2);

   
    i4_image_class *im=i4_create_image(w,h, pal, (w8 *)data, w*2);
    im->dont_free_data=i4_F;
    
    images[i]=im;
    w/=2;
    h/=2;
  }
  delete fp;

  return header.num_mip_levels;
}
