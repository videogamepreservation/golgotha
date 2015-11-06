/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


#include "image/image.hh"
#include "window/cursor.hh"
#include "string/string.hh"
#include "loaders/load.hh"
#include "image/context.hh"

i4_cursor_class *i4_load_cursor(char *cursor_name,
                                i4_string_manager_class *sman)
{
  i4_const_str st=sman->get(cursor_name);    // looks up the resource
  sw32 trans,hot_x,hot_y;

  i4_const_str::iterator s=st.begin();
  i4_str *fn=s.read_string();
  trans=s.read_number();
  hot_x=s.read_number();
  hot_y=s.read_number();


  i4_image_class *im=i4_load_image(*fn);  // try to load up the cursor image
  if (!im) 
  {
    delete fn;  
    return 0;
  }
  else 
  {
    i4_cursor_class *c=new i4_cursor_class(im,(i4_color)trans,hot_x,hot_y);
    delete im;
    delete fn;
    return c;
  }

}



i4_cursor_class::i4_cursor_class(i4_image_class *_pict, 
                                 i4_color trans, 
                                 i4_coord hot_x, i4_coord hot_y,
                                 const i4_pal *convert_to) 
: trans(trans),
  hot_x(hot_x),hot_y(hot_y)
{
  if (convert_to)
  {
    int w=_pict->width(), h=_pict->height();
    pict=i4_create_image(w,h, convert_to);
      
    i4_draw_context_class context(0,0, w-1,h-1);
    pict->clear(trans, context);
    _pict->put_image_trans(pict, 0,0, trans, context);
  }
  else pict=_pict->copy();
}

i4_cursor_class::~i4_cursor_class() 
{
  if (pict)
    delete pict;
}

i4_cursor_class::i4_cursor_class()
{
  pict = 0;
}

i4_cursor_class *i4_cursor_class::copy(const i4_pal *convert_to)
{
  return new i4_cursor_class(pict, trans, hot_x, hot_y, convert_to);
}

