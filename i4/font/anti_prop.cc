/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "font/anti_prop.hh"
#include "image/image.hh"
#include "palette/pal.hh"
#include "image/context.hh"

/*  Printable english characters 
  !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /   0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?   @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O   P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _   `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o   p   q   r   s   t   u   v   w   x   y   z   {   |   }

  "
  */

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



class i4_anti_image_class : public i4_image_class
{
public:

  w32 current_color;


  virtual i4_image_class *copy() 
  {
    i4_anti_image_class *i=new i4_anti_image_class(w,h);
    memcpy(i->data, data, w*h);
    return i;
  }
  
  virtual w16 width() { return w; }
  virtual w16 height() { return h; }

  void set_color(w32 c) { current_color=c; }

  virtual void put_pixel(i4_coord x, i4_coord y, w32 color)
  {
    *((w8 *)data+x+y*w)=color>>24;
  }

  virtual w32  get_pixel(i4_coord x, i4_coord y)
  {
    return ((*((w8 *)data+x+y*w))<<24) | current_color;
  }

  i4_anti_image_class(w16 _w, w16 _h)
  {
    w=_w;
    h=_h;
    bpl=w;
    i4_pixel_format fmt;
    fmt.default_format();
    pal=i4_pal_man.register_pal(&fmt);
    data=i4_malloc(w*h,"");
    
    current_color=0xffffff;
  }

  ~i4_anti_image_class()
  {
    i4_free(data);
  }

};

void i4_anti_proportional_font_class::put_string(i4_image_class *screen, 
                                                 sw16 x, sw16 y, 
                                                 const i4_const_str &string, 
                                                 i4_draw_context_class &context)
{
  if (!string.null())
  {
    for (i4_const_str::iterator p=string.begin(); p!=string.end(); ++p)
    {
      char ch=p.get().ascii_value();
      if (offsets[ch])
      {
        aim->put_part(screen, x,y, offsets[ch], 0, offsets[ch]+widths[ch]-1, aim->height()-1, 
                      context);

        x+=widths[ch];
      }
      else x+=4;
    }
  }
}


void i4_anti_proportional_font_class::put_character(i4_image_class *screen, 
                                                    sw16 x, sw16 y, 
                                                    const i4_char &c, 
                                                    i4_draw_context_class &context)
{
  int ch=c.ascii_value();
  int xo=offsets[ch];
  if (xo)
    aim->put_part(screen, x,y, xo,0,xo+widths[ch]-1,aim->height(), context);
}


i4_anti_proportional_font_class::i4_anti_proportional_font_class(i4_image_class *im, 
                                                                 int start_ch)
{
  memset(offsets, 0, sizeof(offsets));
  longest_w=2;

  
  //  i4_image_has_alpha(im);

  int y_top=0, y_bottom=im->height()-1, x1=0,x2,i;
  for (i=0; i<256; i++)
    widths[i]=4;

  while (y_top<=y_bottom && i4_scanline_empty(im, y_top))
    y_top++;

  if (y_top==y_bottom-1)
    i4_error("image is empty");

  while (i4_scanline_empty(im, y_bottom))
    y_bottom--;

  int char_on=start_ch;
  int x=1;

  do
  {
    while (x1!=im->width() && i4_colum_empty(im, x1)) x1++;
    if (x1<im->width())
    {
      x2=x1+1;
      while (x2!=im->width() && 
             !(i4_colum_empty(im, x2) && i4_colum_empty(im, x2+1))) x2++;
      widths[char_on]=x2-x1+1;
      if (widths[char_on]>longest_w)
        longest_w=widths[char_on];

      offsets[char_on]=x;
      x+=x2-x1+1;
      x1=x2+2;
    }     
    char_on++;
  } while (x1<im->width());

  h=y_bottom-y_top+1;
  aim=new i4_anti_image_class(x, h);

  x1=0; x=1;
  i4_draw_context_class context(0,0, aim->width()-1, aim->height()-1);
  char_on=start_ch;
  do
  {
    while (x1!=im->width() && i4_colum_empty(im, x1)) x1++;
    if (x1<im->width())
    {
      x2=x1+1;
      while (x2!=im->width() && 
             !(i4_colum_empty(im, x2) && i4_colum_empty(im, x2+1))) x2++;

      im->i4_image_class::put_part(aim, x, 0, x1, y_top, x2, y_bottom, context);
      x+=x2-x1+1;
      x1=x2+2;
    }     
    char_on++;
  } while (x1<im->width());
}


void i4_anti_proportional_font_class::set_color(i4_color color)
{
  aim->set_color(color);
}

i4_anti_proportional_font_class::~i4_anti_proportional_font_class() 
{ 
  delete aim; 
}
