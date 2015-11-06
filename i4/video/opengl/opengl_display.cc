/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "image/depth.hh"
#include "image/image32.hh"

#include "video/opengl/opengl_display.hh"

#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>

int glx_attriblist[] = {
  GLX_RGBA,               /* I want only TrueColor visuals */
  GLX_LEVEL, 0,           /* What is this? */
  GLX_DOUBLEBUFFER,       /* I definitely need this. */
  GLX_AUX_BUFFERS, 0,     /* What is this? */
  GLX_RED_SIZE, 8,
  GLX_GREEN_SIZE, 8,
  GLX_BLUE_SIZE, 8, 
  GLX_ALPHA_SIZE, 0,
  GLX_DEPTH_SIZE, 16,     /* I want a min 32-bit deep buffer */
  GLX_STENCIL_SIZE, 0, 

  GLX_ACCUM_RED_SIZE, 0,
  GLX_ACCUM_BLUE_SIZE, 0,
  GLX_ACCUM_GREEN_SIZE, 0,
  GLX_ACCUM_ALPHA_SIZE, 0,

  GLX_TRANSPARENT_INDEX_VALUE_EXT, 0,
  GLX_TRANSPARENT_RED_VALUE_EXT, 0,
  GLX_TRANSPARENT_GREEN_VALUE_EXT, 0,
  GLX_TRANSPARENT_BLUE_VALUE_EXT, 0, 
  GLX_TRANSPARENT_ALPHA_VALUE_EXT, 0,

  None /* end of list */
};


opengl_display_class::opengl_display_class()
{
  know_if_available=i4_F;
}

i4_bool opengl_display_class::set_mouse_shape(i4_cursor_class *cursor)
{
  if (mcursor)
    delete mcursor;

  if (cursor && cursor->pict)
    mcursor=cursor->copy();
  else 
    mcursor=0;

  return i4_T;
}



i4_bool opengl_display_class::available()
{
  if (!know_if_available)
  {
    if (input.open_display())
    {
      if (glXQueryExtension(input.display, &GLX_error_base, &GLX_event_base))
        known_availability=i4_T;          
      else
        known_availability=i4_F;
    }
    else
      known_availability=i4_F;

    know_if_available=i4_T;
    input.close_display(); 
  }

  return know_if_available;
}

void opengl_display_class::remove_cursor(sw32 x, sw32 y, i4_image_class *mouse_save)
{
  
  /*      glRasterPos2d(a->x1 *2 / (float)(width()) -1,  
                  (height()-a->y1) *2/ (float)height() -1 );

    glPixelStorei(GL_UNPACK_ROW_LENGTH, width());

    glDrawPixels(a->x2-a->x1+1, a->y2-a->y1+1, GL_RGBA, GL_UNSIGNED_BYTE, 
                 ((I4_SCREEN_TYPE *)fake_screen)->local_sub_data(a->x1, a->y1));
                 */
}

void opengl_display_class::save_and_draw_cursor(sw32 x, sw32 y, i4_image_class *&mouse_save)
{
}



XVisualInfo *opengl_display_class::get_visual(Display *display)
{
  int i,j;
  XVisualInfo dummy, *x_visinfo;
  int num_vis;
  int screen_list[16]; /* if you've got more than 16 screens, yer sick */

  x_visinfo = XGetVisualInfo(display, 0, &dummy, &num_vis);

  int num_screens = 0;
  for (i=0 ; i<num_vis ; i++)
  {
    for (j=0 ; j<num_screens ; j++)
      if (screen_list[j] == x_visinfo[i].screen) break;
    if (j == num_screens)
      screen_list[num_screens++] = x_visinfo[i].screen;
  }

  int usable_screens=0;
  for (j=0 ; j<num_screens ; j++)
    if (glXChooseVisual(input.display, screen_list[j], glx_attriblist))
      usable_screens++;

  if (usable_screens)
    return glXChooseVisual(input.display, screen_list[0], glx_attriblist);
  else
    return 0;
}


opengl_display_class opengl_display_instance;



void opengl_display_class::flush()
{
  sw32 mouse_x=input.mouse_x, mouse_y=input.mouse_y;

 
  context->both_dirty->intersect_area(0,0,width()-1,height()-1);

  i4_image_class **cur_save;

  // if this is the first time to save, or last save wave to mouse_save2 then use 1
  if (!prev_mouse_save || prev_mouse_save==mouse_save2)
    cur_save=&mouse_save1;
  else
    cur_save=&mouse_save2;
 
  save_and_draw_cursor(mouse_x - (sw32)mcursor->hot_x, mouse_y - (sw32)mcursor->hot_y, *cur_save);


  if (prev_mouse_save)
    remove_cursor(last_mouse_x, last_mouse_y, prev_mouse_save);

  prev_mouse_save=*cur_save;
  last_mouse_x=mouse_x - (sw32)mcursor->hot_x;
  last_mouse_y=mouse_y - (sw32)mcursor->hot_y;


  i4_rect_list_class::area_iter a=context->both_dirty->list.begin();
  for (;a!=context->both_dirty->list.end();++a)
    next_frame_copy.add_area(a->x1, a->y1, a->x2, a->y2);


  glDisable(GL_TEXTURE_2D);  
  glDepthFunc(GL_ALWAYS);
  glDisable(GL_BLEND);
  glColorMask(1,1,1,0);


  glViewport(0,0, width(), height());
  glPixelZoom(1,-1);

  a=next_frame_copy.list.begin();
  for (;a!=context->both_dirty->list.end();++a)
  {  
    float x,y;
    get_pl_pos(a->x1, a->y1, x,y);
    glRasterPos2d(x, y);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, width());

    glDrawPixels(a->x2-a->x1+1, a->y2-a->y1+1, GL_RGBA, GL_UNSIGNED_BYTE, 
                 ((I4_SCREEN_TYPE *)fake_screen)->local_sub_data(a->x1, a->y1));
    
  }  

  glFlush();

  glXSwapBuffers(input.display, input.mainwin);


  next_frame_copy.swap(context->both_dirty);
  context->both_dirty->delete_list();

}


i4_bool opengl_display_class::close()
{
  input.close_display();
  if (fake_screen)
  {
    delete fake_screen;
    fake_screen=0;

    if (context)
    {
      delete context;
      context=0;
    }
  }
  return i4_T;
}


i4_display_class::mode *opengl_display_class::get_first_mode()
{
  if (!input.open_display())
    return 0;
  else
  {
    XVisualInfo *v = get_visual(input.display);
    if (v)
    {
      strcpy(amode.name,"Standard X11 window");
      amode.flags=mode::RESOLUTION_DETERMINED_ON_OPEN;
      amode.bits_per_pixel=I4_SCREEN_DEPTH;

      amode.red_mask    = v->red_mask<<8;
      amode.green_mask  = v->green_mask<<8;
      amode.blue_mask   = v->blue_mask<<8;
      XFree((char *)v);

      Window root;
      int x,y;
      unsigned w,h,bord,depth;
      if (XGetGeometry(input.display,
                       RootWindow(input.display,DefaultScreen(input.display)),&root, 
                       &x,&y,&w,&h,&bord,&depth)==0)
        i4_error("can't get root window attributes");

      amode.xres=w;
      amode.yres=h;

      return &amode;
    }
    return 0;
  }
}

i4_bool opengl_display_class::initialize_mode(mode *m)
{
  sw32 width=m->xres;
  sw32 height=m->yres;

  if (input.create_window(0,0,width,height, this, i4_F, get_visual(input.display)))
  {
    int GLX_major, GLX_minor;

    if (glXQueryVersion(input.display, &GLX_major, &GLX_minor))
      i4_warning("GLX v%d.%d\n", GLX_major, GLX_minor);

    i4_pixel_format f;
    f.red_mask   = m->red_mask;
    f.green_mask = m->green_mask;
    f.blue_mask  = m->blue_mask;
    f.alpha_mask=0;
    f.calc_shift();

    f.lookup      = 0;
    f.pixel_depth = I4_16BIT;

    pal = i4_pal_man.register_pal(&f);

    fake_screen=new I4_SCREEN_TYPE(width, height, pal);
    
    context=new i4_draw_context_class(0,0, width-1, height-1);
    context->both_dirty=new i4_rect_list_class;
    context->single_dirty=new i4_rect_list_class;


    return i4_T;
  }
  else 
    return i4_F;
}
