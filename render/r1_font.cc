/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "r1_font.hh"
#include "image/context.hh"
#include "r1_api.hh"
#include "tmanage.hh"
#include "r1_clip.hh"

static inline int i4_is_black(w32 c)
{
  if ((c&0xff000000)==0)
    return 1;
  else return 0;
}

static inline int i4_scanline_empty(i4_image_class *im, int y)
{
  for (int x=0; x<im->width(); x++)  
    if (!i4_is_black(im->get_pixel(x,y)))
      return 0;

  return 1;
}


static inline int i4_colum_empty(i4_image_class *im, int x)
{
  for (int y=0; y<im->height(); y++)
    if (!i4_is_black(im->get_pixel(x,y)))
      return 0;

  return 1;
}


static inline int i4_image_has_alpha(i4_image_class *im)
{
  for (int y=0; y<im->height(); y++)
    for (int x=0; x<im->width(); x++)   
    {
      w32 cl=im->get_pixel(x,y);
      if (cl>>24)
        i4_warning("alpha %d", cl>>24);
    }

  return 1;
}

void r1_font_class::set_color(i4_color color)
{
  r=((color>>16)&0xff) * (1.0/255.0);
  g=((color>>8)&0xff) * (1.0/255.0);
  b=((color>>0)&0xff) * (1.0/255.0);
}

i4_bool r1_font_class::expand(i4_image_class *from,
                           i4_image_class *to, 
                           int start_ch)
{
  int x=0, y=0;

  int y_top=0, y_bottom=from->height()-1, x1=0,x2,i;

  while (y_top<=y_bottom && i4_scanline_empty(from, y_top))
    y_top++;

  if (y_top==y_bottom-1)
    i4_error("image is empty");

  while (i4_scanline_empty(from, y_bottom))
    y_bottom--;


  for (i=0; i<256; i++)
  {
    pos[i].w=4;
    pos[i].x=255;
  }


  int char_on=start_ch;

  largest_h=y_bottom-y_top;

  i4_draw_context_class context(0,0, to->width()-1, to->height()-1);
  do
  {
    while (x1!=from->width() && i4_colum_empty(from, x1)) x1++;
    if (x1<from->width())
    {
      x2=x1+1;
      while (x2!=from->width() && !(i4_colum_empty(from, x2))) x2++;
      int w=x2-x1+1;
      if (w>longest_w)
        longest_w=w;

      
      if (w+x>to->width())
      {
        y+=largest_h;
        if (y>to->height())
          return i4_F;
        x=0;
      }
            
      pos[char_on].x=x;
      pos[char_on].y=y;
      pos[char_on].w=w;
      from->i4_image_class::put_part(to, x,y, x1,y_top, x2,y_bottom, context);

      x+=w+1;

      x1=x2+1;
    }     
    char_on++;
  } while (x1<from->width() && char_on<255);

  

  texture=api->get_tmanager()->register_image(to);
  xs=1.0/to->width();
  ys=1.0/to->height();

  return i4_T;  
}



r1_font_class::r1_font_class(r1_render_api_class *api, i4_image_class *im, int start_ch)
  : api(api)
{ 
  memset(pos,0, sizeof(pos));

  i4_image_class *to=i4_create_image(64, 64, i4_pal_man.default_32());

  if (!expand(im, to, start_ch))
  {
    delete to;
    to=i4_create_image(128, 128, i4_pal_man.default_32());
    if (!expand(im, to, start_ch))
    {
      delete to;
      to=i4_create_image(256, 256, i4_pal_man.default_32());
      if (!expand(im, to, start_ch))
        i4_error("r1_font does not fit on a 256x256 texture");
    }    
  }
}


void r1_font_class::put_character(i4_image_class *screen, 
                                  sw16 x, sw16 y, 
                                  const i4_char &c, 
                                  i4_draw_context_class &context)
{
  int ch=c.value();
  if (pos[ch].x!=255)
  {
    r1_vert v[4];
  
    v[0].px=x;              v[0].py=y;
    v[1].px=x+pos[ch].w;    v[1].py=y;
    v[2].px=x+pos[ch].w;    v[2].py=y+largest_h;
    v[3].px=x;              v[3].py=y+largest_h;
  

    float xp=pos[ch].x * xs;
    float yp=pos[ch].y * ys;
    float wp=pos[ch].w * xs;
    float hp=largest_h * ys;

    v[0].s=xp; v[0].t=yp;
    v[1].s=xp+wp; v[1].t=yp;
    v[2].s=xp+wp; v[2].t=yp+hp;
    v[3].s=xp; v[3].t=yp+hp;


    float w=1.0/r1_near_clip_z;
    v[0].w=w; v[0].v.z=r1_near_clip_z;
    v[1].w=w; v[1].v.z=r1_near_clip_z;
    v[2].w=w; v[2].v.z=r1_near_clip_z;
    v[3].w=w; v[3].v.z=r1_near_clip_z;

    v[0].r=r; v[0].g=g; v[0].b=b;  v[0].a=1;
    v[1].r=r; v[1].g=g; v[1].b=b;  v[1].a=1;
    v[2].r=r; v[2].g=g; v[2].b=b;  v[2].a=1;
    v[3].r=r; v[3].g=g; v[3].b=b;  v[3].a=1;

    api->use_texture(texture, 256, 0);

    i4_rect_list_class::area_iter cl=context.clip.list.begin();
    for (; cl != context.clip.list.end(); ++cl)
      if (x>=cl->x1 && y>=cl->y1 && v[1].px<=cl->x2 && v[2].py<=cl->y2)      
        api->render_poly(4, v);
  }
}




void r1_font_class::put_string(i4_image_class *screen, 
                               sw16 x, sw16 y, 
                               const i4_const_str &string, 
                               i4_draw_context_class &context)
{
  r1_vert v[4];
  float w=1.0/r1_near_clip_z;
  v[0].w=w; v[0].v.z=r1_near_clip_z;
  v[1].w=w; v[1].v.z=r1_near_clip_z;
  v[2].w=w; v[2].v.z=r1_near_clip_z;
  v[3].w=w; v[3].v.z=r1_near_clip_z;

  v[0].r=r; v[0].g=g; v[0].b=b;  v[0].a=1;
  v[1].r=r; v[1].g=g; v[1].b=b;  v[1].a=1;
  v[2].r=r; v[2].g=g; v[2].b=b;  v[2].a=1;
  v[3].r=r; v[3].g=g; v[3].b=b;  v[3].a=1;

  if (!string.null())
  {
    api->use_texture(texture, 256, 0);

    for (i4_const_str::iterator p=string.begin(); p!=string.end(); ++p)
    {      
      int ch=p.get().value();
      if (pos[ch].x!=255)
      {   
        i4_rect_list_class::area_iter cl=context.clip.list.begin();
        for (; cl != context.clip.list.end(); ++cl)
        {  
          v[0].px=x;                  v[0].py=y;
          v[1].px=x+pos[ch].w;        v[1].py=y;
          v[2].px=x+pos[ch].w;        v[2].py=y+largest_h;
          v[3].px=x;                  v[3].py=y+largest_h;   

          float xp=pos[ch].x * xs;
          float yp=pos[ch].y * ys;
          float wp=pos[ch].w * xs;
          float hp=largest_h * ys;


          v[0].s=xp; v[0].t=yp;
          v[1].s=xp+wp; v[1].t=yp;
          v[2].s=xp+wp; v[2].t=yp+hp;
          v[3].s=xp; v[3].t=yp+hp;

          if (x>=cl->x1 && y>=cl->y1 && v[1].px<=cl->x2 && v[2].py<=cl->y2)
            api->render_poly(4, v);
        }
      }


      x+=pos[ch].w;
    }
  }
}

