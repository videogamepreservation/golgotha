/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "image/image16.hh"
#include "memory/malloc.hh"
#include "palette/pal.hh"
#include "image/context.hh"

#include <string.h>

inline int iabs(int val) { return val>0?val:-val; }

i4_color i4_image16::get_pixel(i4_coord x, i4_coord y)
{    
  return i4_pal_man.convert_to_32(*paddr(x,y), pal);
}
  
void i4_image16::put_pixel(i4_coord x, i4_coord y, w32 color)
{
  w16 *addr=paddr(x,y);
  *addr=i4_pal_man.convert_32_to(color, &pal->source);
}




i4_image16::i4_image16(w16 _w, w16 _h, const i4_pal *_pal)
{
  w=_w;
  h=_h;
  bpl=_w*2;
  set_pal(_pal);
  data=i4_malloc(w*h*2, "");
}

i4_image16::i4_image16(w16 _w, w16 _h, const i4_pal *_pal,
                       void *_data, int _bpl)
{
  data=_data;
  bpl=_bpl;
  pal=_pal;
  w=_w;
  h=_h;

  dont_free_data=i4_T;
}


i4_image16::~i4_image16()
{
  if (!dont_free_data)
    i4_free(data);
}



void i4_image16::line(i4_coord ox1, i4_coord oy1, 
                          i4_coord ox2, i4_coord oy2, 
                          i4_color color, i4_draw_context_class &context)
{
  i4_coord x1,y1,x2,y2;
  i4_coord cx1,cy1,cx2,cy2;
  i4_bool skip;

  w16 tcolor=i4_pal_man.convert_32_to(color, &pal->source);

  for (i4_rect_list_class::area_iter c=context.clip.list.begin();c!=context.clip.list.end();++c)
  { 
    x1=ox1;
    y1=oy1;
    x2=ox2;
    y2=oy2;
    skip=i4_F;

    i4_coord i,xc,yc,er,n,m,xi,yi,xcxi,ycyi,xcyi;
    unsigned dcy,dcx;
    // check to make sure that both endpoint are on the screen

    cx1=c->x1;
    cy1=c->y1;
    cx2=c->x2;
    cy2=c->y2;

    // check to see if the line is completly clipped off
    if (!((x1<cx1 && x2<cx1) || (x1>cx2 && x2>cx2) || 
          (y1<cy1 && y2<cy1) || (y1>cy2 && y2>cy2)))
    {
 
      if (x1>x2)        // make sure that x1 is to the left
      {    
        i=x1; x1=x2; x2=i;  // if not swap points
        i=y1; y1=y2; y2=i;
      }  

      // clip the left side
      if (x1<cx1)
      {  
        int my=(y2-y1);       
        int mx=(x2-x1),b;
        if (!mx) skip=i4_T;
        if (my)
        {
          b=y1-(y2-y1)*x1/mx;      
          y1=my*cx1/mx+b;
          x1=cx1;      
        }
        else x1=cx1;
      }

      // clip the right side
      if (x2>cx2)
      {  
        int my=(y2-y1);       
        int mx=(x2-x1),b;
        if (!mx) skip=i4_T;
        else if (my)
        {
          b=y1-(y2-y1)*x1/mx;      
          y2=my*cx2/mx+b;
          x2=cx2;      
        }
        else x2=cx2;
      }

      if (y1>y2)        // make sure that y1 is on top
      {    
        i=x1; x1=x2; x2=i;  // if not swap points
        i=y1; y1=y2; y2=i;
      }  

      // clip the bottom
      if (y2>cy2)
      {  
        int mx=(x2-x1);       
        int my=(y2-y1),b;
        if (!my)
          skip=i4_T;
        else if (mx)
        {
          b=y1-(y2-y1)*x1/mx;      
          x2=(cy2-b)*mx/my;
          y2=cy2;
        }
        else y2=cy2;
      }

      // clip the top
      if (y1<cy1)
      {  
        int mx=(x2-x1);       
        int my=(y2-y1),b;
        if (!my) 
          skip=i4_T;
        else if (mx)
        {
          b=y1-(y2-y1)*x1/mx;      
          x1=(cy1-b)*mx/my;
          y1=cy1;
        }
        else y1=cy1;
      }


      // see if it got cliped into the box, out out
      if (x1<cx1 || x2<cx1 || x1>cx2 || x2>cx2 || y1<cy1 || y2 <cy1 || y1>cy2 || y2>cy2)
        skip=i4_T;     

      if (x1>x2)
      { xc=x2; xi=x1; }
      else { xi=x2; xc=x1; }

      if (!skip)
      {
        // assume y1<=y2 from above swap operation
        yi=y2; yc=y1;

        add_dirty(xc,yc,xi,yi,context);
        dcx=x1+context.xoff; dcy=y1+context.yoff;
        xc=(x2-x1); yc=(y2-y1);
        if (xc<0) xi=-1; else xi=1;
        if (yc<0) yi=-1; else yi=1;
        n=iabs(xc); m=iabs(yc);
        ycyi=iabs(2*yc*xi);
        er=0;
      
        
        if (n>m)
        {
          xcxi=iabs(2*xc*xi);
          for (i=0;i<=n;i++)
          {
            *paddr(dcx, dcy)=tcolor;

            if (er>0)
            {
              dcy+=yi;
              er-=xcxi;
            }
            er+=ycyi;
            dcx+=xi;
          }
        }
        else
        {
          xcyi=iabs(2*xc*yi);
          for (i=0;i<=m;i++)
          {
            *paddr(dcx, dcy)=tcolor;

            if (er>0)
            {
              dcx+=xi;
              er-=ycyi;
            }
            er+=xcyi;
            dcy+=yi;
          }
        }
      }
    }
  }
}


void i4_image16::put_part(i4_image_class *to, 
                              i4_coord _x,  i4_coord _y,                              
                              i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2, 
                              i4_draw_context_class &context)
{
  const i4_pixel_format *from_format=&get_pal()->source;
  const i4_pixel_format *to_format=&to->get_pal()->source;

  
  if ((to_format->pixel_depth!=I4_16BIT ||
      to_format->red_mask != from_format->red_mask ||
      to_format->green_mask != from_format->green_mask ||
      to_format->blue_mask != from_format->blue_mask) &&
      (from_format->alpha_mask==0 || to_format->alpha_mask))
    // depths aren't the same do it the slow way
    i4_image_class::put_part(to, _x, _y, x1,y1,x2,y2, context);
  else
  {  
    for (i4_rect_list_class::area_iter c=context.clip.list.begin();
         c!=context.clip.list.end();
         ++c)
    { 
      i4_coord lx1,ly1,lx2,ly2,x=_x,y=_y;

      if (x1<0) { lx1=0; _x+=-x1; } else lx1=x1;
      if (y1<0) { ly1=0; _y+=-y1; } else ly1=y1;
      if (x2>=width())  lx2=width()-1;   else lx2=x2;
      if (y2>=height()) ly2=height()-1;  else ly2=y2;
  
      if (!(lx1>lx2 || ly1>ly2))
      {
        if (x<c->x1)
        { lx1+=(c->x1-x); x=c->x1; }

        if (y<c->y1)
        { ly1+=(c->y1-y); y=c->y1; }


        if (x+lx2-lx1+1>c->x2)
          lx2=c->x2-x+lx1; 

        if (y+ly2-ly1+1>c->y2)
          ly2=c->y2-y+ly1; 


        w16 *source=paddr(lx1, ly1);
        w16 *dest=((i4_image16 *)to)->paddr(x + context.xoff,
                                            y + context.yoff);
        
        int copy_width=((lx2-lx1)+1)*2;

        if (!(lx1>lx2 || ly1>ly2))
        {
          to->add_dirty(x,y,x+(lx2-lx1+1),y+(ly2-ly1+1),context);
          while (ly1<=ly2)
          {              
            memcpy(dest, source, copy_width);
            dest=((w16 *)((w8 *)dest + to->bpl));
            source=((w16 *)((w8 *)source + bpl));
            ly1++;
          }
        }
      }
    }
  }
}



void i4_image16::put_part_one_pixel_at_a_time(i4_image_class *to, 
                                              i4_coord _x,  i4_coord _y,
                                              i4_coord x1, i4_coord y1,
                                              i4_coord x2, i4_coord y2, 
                                              i4_draw_context_class &context)
{
  const i4_pixel_format *from_format=&get_pal()->source;
  const i4_pixel_format *to_format=&to->get_pal()->source;

  
  if ((to_format->pixel_depth!=I4_16BIT ||
      to_format->red_mask != from_format->red_mask ||
      to_format->green_mask != from_format->green_mask ||
      to_format->blue_mask != from_format->blue_mask) &&
      (from_format->alpha_mask==0 || to_format->alpha_mask))
    // depths aren't the same do it the slow way
    i4_image_class::put_part(to, _x, _y, x1,y1,x2,y2, context);
  else
  {  
    for (i4_rect_list_class::area_iter c=context.clip.list.begin();
         c!=context.clip.list.end();
         ++c)
    { 
      i4_coord lx1,ly1,lx2,ly2,x=_x,y=_y;

      if (x1<0) { lx1=0; _x+=-x1; } else lx1=x1;
      if (y1<0) { ly1=0; _y+=-y1; } else ly1=y1;
      if (x2>=width())  lx2=width()-1;   else lx2=x2;
      if (y2>=height()) ly2=height()-1;  else ly2=y2;
  
      if (!(lx1>lx2 || ly1>ly2))
      {
        if (x<c->x1)
        { lx1+=(c->x1-x); x=c->x1; }

        if (y<c->y1)
        { ly1+=(c->y1-y); y=c->y1; }


        if (x+lx2-lx1+1>c->x2)
          lx2=c->x2-x+lx1; 

        if (y+ly2-ly1+1>c->y2)
          ly2=c->y2-y+ly1; 


        w16 *source=paddr(lx1, ly1);
        w16 *dest=((i4_image16 *)to)->paddr(x + context.xoff,
                                            y + context.yoff);
        
        int copy_width=((lx2-lx1)+1)*2;

        if (!(lx1>lx2 || ly1>ly2))
        {
          to->add_dirty(x,y,x+(lx2-lx1+1),y+(ly2-ly1+1),context);
          while (ly1<=ly2)
          {
            for (int fx=0; fx<copy_width/2; fx++)
              dest[fx]=source[fx];
              
            dest=((w16 *)((w8 *)dest + to->bpl));
            source=((w16 *)((w8 *)source + bpl));
            ly1++;
          }
        }
      }
    }
  }
}



void i4_image16::put_part_trans(i4_image_class *to, 
                                    i4_coord _x,  i4_coord _y,                              
                                    i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2,
                                    i4_color trans_color,
                                    i4_draw_context_class &context)
{
  const i4_pixel_format *from_format=&get_pal()->source;
  const i4_pixel_format *to_format=&to->get_pal()->source;

  
  if ((to_format->pixel_depth!=I4_16BIT ||
      to_format->red_mask != from_format->red_mask ||
      to_format->green_mask != from_format->green_mask ||
      to_format->blue_mask != from_format->blue_mask) &&
      (from_format->alpha_mask==0 || to_format->alpha_mask))
    // depths aren't the same do it the slow way
    i4_image_class::put_part(to, _x, _y, x1,y1,x2,y2, context);
  else
  {
    i4_color tcolor=i4_pal_man.convert_32_to(trans_color, &pal->source);
    
    for (i4_rect_list_class::area_iter c=context.clip.list.begin();
         c!=context.clip.list.end();++c)
    { 
      i4_coord lx1,ly1,lx2,ly2,x=_x,y=_y;

      if (x1<0) { lx1=0; _x+=-x1; } else lx1=x1;
      if (y1<0) { ly1=0; _y+=-y1; } else ly1=y1;
      if (x2>=width())  lx2=width()-1;   else lx2=x2;
      if (y2>=height()) ly2=height()-1;  else ly2=y2;
  
      if (!(lx1>lx2 || ly1>ly2))
      {
        if (x<c->x1)
        { lx1+=(c->x1-x); x=c->x1; }

        if (y<c->y1)
        { ly1+=(c->y1-y); y=c->y1; }


        if (x+lx2-lx1+1>c->x2)
          lx2=c->x2-x+lx1; 

        if (y+ly2-ly1+1>c->y2)
          ly2=c->y2-y+ly1; 


        if (!(lx1>lx2 || ly1>ly2))
        {
          to->add_dirty(x,y,x+(lx2-lx1+1),y+(ly2-ly1+1),context);

          w16 *source=paddr(lx1, ly1);
          w16 *dest=((i4_image16 *)to)->paddr(x + context.xoff,
                                              y + context.yoff);
          int w=(lx2-lx1+1);
          
          for (; ly1<=ly2; ly1++)
          {
            for (int j=0; j<w; j++)
            {
              w16 c=source[j];
              if (c!=tcolor)
                dest[j]=c;
            }
            
            source=(w16 *)((w8 *)source + bpl);
            dest=(w16 *)((w8 *)dest + to->bpl);
          }
        }
      }
    }
  }
}


void i4_image16::bar(i4_coord x1,    i4_coord y1, 
                     i4_coord x2,    i4_coord y2, 
                     i4_color color, i4_draw_context_class &context)
{
  w16 tcolor=i4_pal_man.convert_32_to(color, &pal->source);
    
  for (i4_rect_list_class::area_iter c=context.clip.list.begin();c!=context.clip.list.end();++c)
  {    
    i4_coord lx1,ly1,lx2,ly2;

    if (x1<c->x1) lx1=c->x1; else lx1=x1;
    if (y1<c->y1) ly1=c->y1; else ly1=y1;
    if (x2>c->x2) lx2=c->x2; else lx2=x2;
    if (y2>c->y2) ly2=c->y2; else ly2=y2;

    if (!(lx1>lx2 || ly1>ly2))
    {
      w16 *d=paddr(lx1 + context.xoff, ly1+context.yoff);
      int count=lx2-lx1+1;
      
      add_dirty(lx1,ly1,lx2,ly2,context);
      for (;ly1<=ly2; ly1++)
      {
        for (int x=0; x<count; x++)
          d[x]=tcolor;
        
        d=(w16 *)((w8 *)d + bpl);
      }
    }
  }
}
