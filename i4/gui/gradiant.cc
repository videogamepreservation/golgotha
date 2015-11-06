/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "image/image.hh"
#include "palette/pal.hh"
#include "image/context.hh"

void i4_gradiant_bar(i4_image_class *im, int x1, int y1, int x2, int y2,
                     i4_color start_color, i4_color end_color,
                     i4_draw_context_class &context)
{
  context.add_both_dirty(x1,y1,x2,y2);

  float sr=(start_color>>16)&0xff, sg=(start_color>>8)&0xff, sb=(start_color)&0xff;
  float er=(end_color>>16)&0xff, eg=(end_color>>8)&0xff, eb=(end_color)&0xff;
  
  im->add_dirty(x1,y1,x2,y2, context);

  float t=1.0/(x2-x1+1);
  float r_step=(er-sr)*t;
  float g_step=(eg-sg)*t;
  float b_step=(eb-sb)*t;


  int w=(x2-x1+1);
  int h=(y2-y1+1);
  if (w*h*4<32*1024)  // do it fast if it's small than 32k
  {        
    i4_image_class *fast=i4_create_image(w,h, i4_pal_man.default_no_alpha_32());
    
    for (int x=0; x<w; x++)
    {
      w32 c=(((int)sr)<<16) | (((int)sg)<<8) | (((int)sb));

      w32 *sl=((w32 *)fast->data)+x;
      for (int y=0; y<h; y++)
      {
        *sl=c;
        sl+=w;
      }

      sr+=r_step;  sg+=g_step;  sb+=b_step;
    }

    fast->put_image(im, x1,y1, context);
    delete fast;
  }
  else
  { 
    for (int x=x1; x<=x2; x++)
    {
      w32 c=(((int)sr)<<16) | (((int)sg)<<8) | (((int)sb));
      im->bar(x,y1,x,y2, c, context);

      sr+=r_step;  sg+=g_step;  sb+=b_step;
    }
  }
}
