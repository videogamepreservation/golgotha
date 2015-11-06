/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "video/win32/dx5_util.hh"
#include "video/win32/dx5_mouse.hh"
#include "image/context.hh"
#include "video/win32/dx5_error.hh"

#include <ddraw.h>

dx5_mouse_class::dx5_mouse_class(i4_bool page_flipped)
  : page_flipped(page_flipped)
{
  cursor.pict=0;
}

void dx5_mouse_class::set_cursor(i4_cursor_class *c)
{
  if (cursor.pict)
  {
    delete cursor.pict;
    cursor.pict=0;
  }

  if (c && c->pict)
  {
    int cw=c->pict->width(), ch=c->pict->height();

    i4_draw_context_class context(0,0, cw-1, ch-1);
    
    i4_dx5_image_class *dx5_image = new i4_dx5_image_class(cw, ch, DX5_VRAM | DX5_CLEAR);
    
    //setup the color-key value for the mouse (black)
    DDCOLORKEY colorkey;
    memset(&colorkey,0,sizeof(DDCOLORKEY));

    //i4_color converted_transparent = c->pict->pal.pal->convert(c->trans,&dx5_common.i4_fmt_565);

    colorkey.dwColorSpaceLowValue  = 0;//converted_transparent;
    colorkey.dwColorSpaceHighValue = 0;//converted_transparent;

    dx5_image->surface->SetColorKey(DDCKEY_SRCBLT,&colorkey);

    cursor      = *c;
    cursor.pict = dx5_image;

    dx5_image->lock();
    c->pict->put_image_trans(cursor.pict, 0,0, c->trans, context);
    dx5_image->unlock();    
  }
}

dx5_mouse_class::~dx5_mouse_class()
{
  if (cursor.pict)
  {
    delete cursor.pict;
    cursor.pict=0;
  }
}


void dx5_mouse_class::save_and_draw(int x, int y)
{
  if (!cursor.pict) return ;

  if (!current.save_buffer ||
      current.save_buffer->width() != cursor.pict->width() ||
      current.save_buffer->height() != cursor.pict->height())
  {
    if (current.save_buffer)
      delete current.save_buffer;

    current.save_buffer=new i4_dx5_image_class(cursor.pict->width(), cursor.pict->height(),
                                               DX5_VRAM);
  }

  current.x=x - cursor.hot_x;  current.y=y - cursor.hot_y;
  if (current.x<0) current.x=0;
  if (current.y<0) current.y=0;
  

  RECT src;
  src.left   = current.x;
  src.right  = current.x + cursor.pict->width();
  src.top    = current.y;
  src.bottom = current.y + cursor.pict->height();

  i4_dx5_check(current.save_buffer->surface->BltFast(0,0, dx5_common.back_surface, &src,DDBLTFAST_NOCOLORKEY));

  
  i4_dx5_check(dx5_common.back_surface->BltFast(current.x, current.y,
                                                ((i4_dx5_image_class *)cursor.pict)->surface,0 ,
                                                DDBLTFAST_SRCCOLORKEY));

  if (page_flipped)
  {
    save_struct tmp=current;
    current=last;
    last=tmp;
    tmp.save_buffer=0;  
  }

}


void dx5_mouse_class::restore()
{  

  if (current.save_buffer)
  {
    RECT src;
    src.left   = 0;
    src.right  = cursor.pict->width();
    src.top    = 0;
    src.bottom = cursor.pict->height();

    dx5_common.back_surface->BltFast(current.x, current.y, current.save_buffer->surface, &src,
                                          DDBLTFAST_NOCOLORKEY); 
  }
}
