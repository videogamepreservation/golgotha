/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "image/image.hh"
#include "loaders/load.hh"
#include "image_man.hh"
#include "string/string.hh"
#include "app/app.hh"
#include "g1_tint.hh"
#include "video/display.hh"

// this image is used if the correct image could not be loaded
static i4_image_class *default_image=0;
static int images_have_been_loaded=0;


g1_image_ref *g1_image_ref::first=0;

g1_image_ref::g1_image_ref(const char *filename)
{
  image_name=filename;
  next=first;
  first=this;
  im=0;
}

g1_image_ref::~g1_image_ref()
{
  if (first==this)
    first=first->next;
  else
  {
    g1_image_ref *f=first;
    while (f->next!=this)
      f=f->next;
    f->next=next;    
  }
}

void g1_image_ref::load()
{
  im=i4_load_image(image_name);
  if (!im)
  {
    i4_warning("could not load %s", image_name);
    im=default_image;
  }

}

void g1_image_ref::set_image(const char *filename)
{
  cleanup();

  image_name=filename;

  if (images_have_been_loaded)
  {
    im=i4_load_image(image_name);
    if (!im)
    {
      i4_warning("could not load %s", image_name);
      im=default_image;
    }
  }
}

void g1_image_ref::cleanup()
{
  if (im && im!=default_image)
  {
    delete im;
    im=0;
  }
 
}


void g1_load_images()
{
  i4_image_class *old_default_image=default_image;

  if (default_image)
    delete default_image;
  default_image=i4_load_image("bitmaps/default.tga");
  if (!default_image)
    i4_error("could not load default image bitmaps/default.tga");


  for (g1_image_ref *f=g1_image_ref::first; f; f=f->next)
  {
    f->cleanup();
    f->load();

  }

  images_have_been_loaded=1;
}


void g1_unload_images()
{
  for (g1_image_ref *f=g1_image_ref::first; f; f=f->next)
    f->cleanup();

  if (default_image)
  {
    delete default_image;
    default_image=0;
  }
  images_have_been_loaded=0;
}


g1_team_icon_ref::g1_team_icon_ref(const char *filename)
  : g1_image_ref(filename)
{
  for (int i=0; i<G1_MAX_PLAYERS; i++)
    tinted_icons[i]=0;
}

void g1_team_icon_ref::cleanup()
{
  g1_image_ref::cleanup();
  for (int i=0; i<G1_MAX_PLAYERS; i++)
  {
    if (tinted_icons[i] && tinted_icons[i]!=default_image)
      delete tinted_icons[i];
    tinted_icons[i]=0;
  }
}


static float tr,tg,tb;

inline w32 tint_color(w32 color)
{
  int r=i4_f_to_i(((color>>16)&0xff)*tr);
  int g=i4_f_to_i(((color>>8)&0xff)*tg);
  int b=i4_f_to_i(((color>>0)&0xff)*tb);

  return (r<<16)|(g<<8)|b;
}

void g1_team_icon_ref::load()
{
  g1_image_ref::load();

  
  const i4_pal *pal=i4_current_app->get_display()->get_palette();
  
  for (int i=0; i<G1_MAX_PLAYERS; i++)
  {
    int w=im->width(), h=im->height();
    i4_image_class *out=i4_create_image(w,h, pal);

    tr=g1_player_tint_data[i].r;
    tg=g1_player_tint_data[i].g;
    tb=g1_player_tint_data[i].b;

    for (int y=0; y<h; y++)
      for (int x=0; x<w; x++)
      {
        w32 color=im->get_pixel(x,y);
        out->put_pixel(x,y, tint_color(color));        
      }

    tinted_icons[i]=out;    
  }
}
