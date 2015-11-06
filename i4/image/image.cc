/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "image/image.hh"
#include "error/error.hh"
#include "image/image32.hh"
#include "image/image16.hh"
#include "image/image8.hh"
#include "palette/pal.hh"
#include "area/rectlist.hh"
#include "image/context.hh"


#ifndef abs
#define abs(x) ((x)<0 ? -(x) : (x))
#endif


// void i4_image_class::widget(i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2, i4_color bright, i4_color med, i4_color dark, i4_draw_context_class &context)
// {
//   add_dirty(x1,y1,x2,y2,context);      // to keep from creating a dirty for each operation below

//   bar(x1,y1,x2,y1,bright,context);
//   bar(x1,y1+1,x1,y2,bright,context);
//   bar(x2,y1+1,x2,y2,dark,context);
//   bar(x1+1,y2,x2-1,y2,dark,context);
//   bar(x1+1,y1+1,x2-1,y2-1,med,context);
// }

void i4_image_class::put_pixel(i4_coord x, i4_coord y, w32 color, i4_draw_context_class &context)
{
  for (i4_rect_list_class::area_iter c=context.clip.list.begin();c!=context.clip.list.end();++c)
  {      
    if (x>=c->x1 && x<=c->x2 && y>=c->y1 && y<=c->y2)
      put_pixel(x + context.xoff, y + context.yoff, color);
  }
}


w32 i4_image_class::get_pixel(i4_coord x, i4_coord y,  i4_draw_context_class &context)
{
  return get_pixel(x + context.xoff, y + context.yoff);
}

  
void i4_image_class::xor_bar(i4_coord x1,    i4_coord y1, 
                             i4_coord x2,    i4_coord y2, 
                             i4_color color, i4_draw_context_class &context)
{
  for (i4_rect_list_class::area_iter c=context.clip.list.begin();c!=context.clip.list.end();++c)
  {
    i4_coord lx1,ly1,lx2,ly2;

    if (x1<c->x1) lx1=c->x1; else lx1=x1;
    if (y1<c->y1) ly1=c->y1; else ly1=y1;
    if (x2>c->x2) lx2=c->x2; else lx2=x2;
    if (y2>c->y2) ly2=c->y2; else ly2=y2;

    if (!(lx1>lx2 || ly1>ly2))
    {
      add_dirty(lx1,ly1,lx2,ly2,context);
      for (;ly1<=ly2; ly1++)
        for (int x=lx1; x<=lx2; x++)
          put_pixel(x + context.xoff, ly1 + context.yoff, get_pixel(x,ly1)^0xffffff);

    }
  }
}



void i4_image_class::bar(i4_coord x1,    i4_coord y1, 
                         i4_coord x2,    i4_coord y2, 
                         i4_color color, i4_draw_context_class &context)
{
  for (i4_rect_list_class::area_iter c=context.clip.list.begin();c!=context.clip.list.end();++c)
  {    
    i4_coord lx1,ly1,lx2,ly2;

    if (x1<c->x1) lx1=c->x1; else lx1=x1;
    if (y1<c->y1) ly1=c->y1; else ly1=y1;
    if (x2>c->x2) lx2=c->x2; else lx2=x2;
    if (y2>c->y2) ly2=c->y2; else ly2=y2;

    if (!(lx1>lx2 || ly1>ly2))
    {
      add_dirty(lx1,ly1,lx2,ly2,context);
      for (;ly1<=ly2; ly1++)
        for (int x=lx1; x<=lx2; x++)
          put_pixel(x+ context.xoff, ly1+ context.yoff, color);

    }
  }
}

void i4_image_class::line(i4_coord ox1, i4_coord oy1, 
                          i4_coord ox2, i4_coord oy2, 
                          i4_color color, i4_draw_context_class &context)
{
  i4_coord x1,y1,x2,y2;
  i4_coord cx1,cy1,cx2,cy2;
  i4_bool skip;

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
        n=abs(xc); m=abs(yc);
        ycyi=abs(2*yc*xi);
        er=0;
      
        
        if (n>m)
        {
          xcxi=abs(2*xc*xi);
          for (i=0;i<=n;i++)
          {
            put_pixel(dcx, dcy, color);

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
          xcyi=abs(2*xc*yi);
          for (i=0;i<=m;i++)
          {
            put_pixel(dcx, dcy, color);

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


void i4_image_class::put_part(i4_image_class *to, 
                              i4_coord _x,  i4_coord _y,                              
                              i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2, 
                              i4_draw_context_class &context)
{
  for (i4_rect_list_class::area_iter c=context.clip.list.begin();c!=context.clip.list.end();++c)
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


      const i4_pixel_format *from_format=&get_pal()->source;
      const i4_pixel_format *to_format=&to->get_pal()->source;
      if (from_format->alpha_mask==0 || to_format->alpha_mask)
      {
        if (!(lx1>lx2 || ly1>ly2))
        {
          to->add_dirty(x,y,x+(lx2-lx1+1),y+(ly2-ly1+1),context);
          while (ly1<=ly2)
          {
            int tx=x;
            for (int fx=lx1;fx<=lx2; fx++, tx++)
            {     
              w32 c=get_pixel(fx, ly1);          
              to->put_pixel(tx+ context.xoff, y+ context.yoff, c);
            }
            ly1++;
            y++;
          }
        }
      }
      else
      {
        if (!(lx1>lx2 || ly1>ly2))
        {
          to->add_dirty(x,y,x+(lx2-lx1+1),y+(ly2-ly1+1),context);
          while (ly1<=ly2)
          {
            int tx=x;
            for (int fx=lx1;fx<=lx2; fx++, tx++)
            {     
              w32 c=get_pixel(fx, ly1);
              float a=c>>24;
              if (a)
              {
                a/=255.0;

                int r=(c>>16)&0xff, g=(c>>8)&0xff, b=c&0xff;
                w32 tc=to->get_pixel(tx, y, context);
                int tr=(tc>>16)&0xff, tg=(tc>>8)&0xff, tb=tc&0xff;
                
                r=(int)((r-tr)*a + tr);
                g=(int)((g-tg)*a + tg);
                b=(int)((b-tb)*a + tb);

                to->put_pixel(tx+ context.xoff, y+ context.yoff, (r<<16) | (g<<8) | b);
              }
            }
            ly1++;
            y++;
          }
        }
      }
    }
  }
}



void i4_image_class::put_part_trans(i4_image_class *to, 
                                    i4_coord _x,  i4_coord _y,                              
                                    i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2,
                                    i4_color trans_color,
                                    i4_draw_context_class &context)
{
  for (i4_rect_list_class::area_iter c=context.clip.list.begin();c!=context.clip.list.end();++c)
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


      const i4_pixel_format *from_format=&get_pal()->source;
      const i4_pixel_format *to_format=&to->get_pal()->source;

      if (!(lx1>lx2 || ly1>ly2))
      {
        to->add_dirty(x,y,x+(lx2-lx1+1),y+(ly2-ly1+1),context);
        while (ly1<=ly2)
        {
          int tx=x;
          for (int fx=lx1;fx<=lx2; fx++, tx++)
          {     
            w32 c=get_pixel(fx, ly1);
            if (c!=trans_color)
              to->put_pixel(tx+context.xoff, y+ context.yoff, c);
          }
          ly1++;
          y++;
        }
      }
    }
  }
}



i4_image_class *i4_create_image(int width, int height, const i4_pal *pal)
{
  switch (pal->source.pixel_depth)
  {
    case I4_32BIT :
      return new i4_image32(width, height, pal);
      break;

    case I4_16BIT :
      return new i4_image16(width, height, pal);
      break;
      
      
    case I4_8BIT :
      return new i4_image8(width, height, pal);
      break;
      
    default:      
      i4_error("don't know how");      
  }
  


  return 0;
}

i4_image_class *i4_create_image(int width, int height,
                                const i4_pal *pal,
                                void *data,
                                int bpl)
{
  switch (pal->source.pixel_depth)
  {
    case I4_32BIT :
      return new i4_image32(width, height, pal, data, bpl);
      break;

    case I4_16BIT :
      return new i4_image16(width, height, pal, data, bpl);
      break;
      
    case I4_8BIT :
      return new i4_image8(width, height, pal, data, bpl);
      break;
      
    default:      
      i4_error("don't know how");      
  }
  return 0;
}

i4_image_class *i4_image_class::copy()
{
  i4_image_class *im=i4_create_image(width(), height(), pal);

  for (int y=0; y<h; y++)
    for (int x=0; x<w; x++)
      im->put_pixel(x,y, get_pixel(x,y));

  return im;
}


void i4_image_class::add_single_dirty(i4_coord x1, i4_coord y1, i4_coord x2, 
                                      i4_coord y2, i4_draw_context_class &context)
{
  context.add_single_dirty(x1+context.xoff,y1+context.yoff,
                           x2+context.xoff,y2+context.yoff);
}
  
void i4_image_class::add_dirty(i4_coord x1, i4_coord y1, i4_coord x2, 
                               i4_coord y2, i4_draw_context_class &context)
{
  context.add_both_dirty(x1+context.xoff,y1+context.yoff,
                         x2+context.xoff,y2+context.yoff);
}

void i4_image_class::rectangle(i4_coord x1, i4_coord y1,
                               i4_coord x2, i4_coord y2, i4_color color, 
                               i4_draw_context_class &context)
{
  bar(x1,y1,x1,y2,color,context);
  bar(x2,y1,x2,y2,color,context);
  bar(x1+1,y1,x2-1,y1,color,context);
  bar(x1+1,y2,x2-1,y2,color,context);
}
