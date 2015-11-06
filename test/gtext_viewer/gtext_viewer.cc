/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "window/window.hh"
#include "app/app.hh"
#include "main/main.hh"
#include "window/style.hh"
#include "file/file.hh"
#include "loaders/load.hh"
#include "window/wmanager.hh"
#include "math/transform.hh"
#include "gui/text.hh"
#include "gui/button.hh"
#include "gui/image_win.hh"
#include "loaders/load.hh"

#include "r1_res.hh"
#include "image/image16.hh"
#include "tex_cache.hh"
#include "tex_id.hh"
#include "mip.hh"

int load_gtext(i4_const_str texture_name, i4_image_class **images)
{
  i4_file_class *fp;
  tex_cache_header_t theader;
  mipheader_t header;

  fp=i4_open(r1_gets("texture_cache"));
  if (!fp) 
    return 0;

  theader.read(fp);
  delete fp;

  w32 id=r1_get_texture_id(texture_name);
  i4_str *fn = r1_texture_id_to_filename(id, r1_gets("local_texture_dir"));
  fp=i4_open(*fn);
  delete fn;
  if (!fp) 
    return 0;

  header.read(fp);

  int w=header.base_width, h=header.base_height, i; 
  i4_pal *pal;

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

   
    i4_image16 *im=new i4_image16(w,h, pal, (w8 *)data, w*2);
    im->dont_free_data=i4_T;
    
    images[i]=im;
    w/=2;
    h/=2;
  }
  delete fp;

  return header.num_mip_levels;
}

extern i4_image_class *global_im;
class test_app : public i4_application_class
{
public:
  void init()
  {
    i4_application_class::init();

    i4_image_class *im[10];
   
    int t = load_gtext("explosions2.tga", im);
    //i4_image_class *im=i4_load_image("/u/crack/golgotha/textures/moon.tga");
    
    int x=0;
    for (int i=0; i<t; i++)
    {
      i4_image_window_class *im_win=new i4_image_window_class(im[i], i4_T, i4_F);
      
      wm->add_child(x,0, im_win);
      x+=im[i]->width();

    }

  }  


  char *name() { return "test_app"; }
};

void i4_main(w32 argc, i4_const_str *argv)
{
  test_app test;
  test.run();
}

