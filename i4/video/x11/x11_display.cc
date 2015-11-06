/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "video/x11/x11_display.hh"
#include "image/context.hh"
#define DEPTH 16

x11_display_class x11_display_instance;


x11_display_class::x11_display_class()
{
  shm_extension=0;
  mouse_save=screen=0;
  context=0;

  shm_image=0;
  ximage=0;
}



void x11_display_input_class::resize(int w, int h)
{
  x11_display_class *d=(x11_display_class *)i4_display;
  if (w!=d->width() || h!=d->height())
  {
    d->destroy_X_image();
    d->create_X_image(w,h);
  }
}

void x11_display_input_class::note_event(XEvent &xev)
{
  x11_display_class *d=(x11_display_class *)i4_display;
  if (d->shm_extension)
    d->shm_extension->note_event(xev);
}


i4_image_class *x11_display_class::get_back_buffer(int x1, int y1, int x2, int y2)
{
  if (x2<x1 || y2<y1) return 0;

  i4_image_class *im=i4_create_image(x2-x1+1, y2-y1+1, screen->pal);
  screen->put_part(im, 0,0, x1,y1,x2,y2);
  return im;
}

i4_bool x11_display_class::open_X_window(w32 width, w32 height, i4_display_class::mode *mode)
{  
  if (input.create_window(0,0,width,height, this, i4_F, get_visual(input.display)))
  {
    i4_pixel_format f;
    f.red_mask   = mode->red_mask;
    f.green_mask = mode->green_mask;
    f.blue_mask  = mode->blue_mask;
    f.alpha_mask = 0;

    f.calc_shift();

    f.lookup      = 0;
    f.pixel_depth = I4_16BIT;

    pal = i4_pal_man.register_pal(&f);


    if (shm_extension)
    {
      char *ds_name=getenv("DISPLAY");
      if (!ds_name) 
        ds_name="unix:0.0";
    
      if (!shm_extension->available(input.display,ds_name))
        shm_extension=0;
    }

    create_X_image(width, height);
    return i4_T;
  }
  else 
    return i4_F;
}



void x11_display_class::close_X_window()
{
  destroy_X_image();
  if (shm_extension) 
    shm_extension->shutdown(input.display);


  if (mouse_save)
  {
    delete mouse_save;
    mouse_save=0;
  }

  if (mouse_pict)
  {
    delete mouse_pict;
    mouse_pict=0;
  }
}


i4_bool x11_display_class::close()
{
  if (screen)
    close_X_window();

  return i4_T;
}

void x11_display_class::init()
{
  if (input.display || input.open_display())
  {
    me.add_to_list("X Windows", 0, this, i4_display_list);
    input.close_display();
  }
}

  // initialize_mode need not call close() to switch to another mode
i4_bool x11_display_class::initialize_mode()
{  
  memcpy(&cur_mode, &amode, sizeof(cur_mode));

  // this should open an X window
  return open_X_window(cur_mode.xres, cur_mode.yres, &cur_mode);
}


XVisualInfo *x11_display_class::get_visual(Display *display)
{
  return input.find_visual_with_depth(DEPTH);  
}

i4_display_class::mode *x11_display_class::get_first_mode(int driver_id)
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
      amode.bits_per_pixel=DEPTH;

      amode.red_mask    = v->red_mask;
      amode.green_mask  = v->green_mask;
      amode.blue_mask   = v->blue_mask;
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

      amode.assoc=this;          // so we know in 'initialize_mode' that we created this mode
      return &amode;
    }
    return 0;
  }
}





void x11_display_class::destroy_X_image()
{
  if (shm_extension)
  {
    if (shm_image)
      shm_extension->destroy_shm_image(input.display, shm_image);
    shm_image=0;
  }
  else
  {
    if (ximage)
      XDestroyImage(ximage);
    ximage=0;
  }
  if (screen)
  {
    delete screen;
    screen=0;
  }

  if (context)
  {
    delete context;
    context=0;
  }
}

void x11_display_class::create_X_image(w32 width, w32 height)
{
  destroy_X_image();

  if (shm_extension)
  {          
    w16 w=width,h=height;
    shm_image=shm_extension->create_shm_image(input.display,
                                              input.mainwin,
                                              input.gc,
                                              input.my_visual->visual,
                                              input.my_visual->depth,
                                              w,h);   // actual width and height from X
    screen=i4_create_image(w,h,pal,
                           (w8 *)shm_image->data,
                           w*(DEPTH/8));
  }
  else
  {

    ximage = XCreateImage(input.display,
                          input.my_visual->visual,
                          input.my_visual->depth,
                          ZPixmap,
                          0,
                          (char *)i4_malloc(width*height*(DEPTH/8),"Ximage"),
                          width, 
                          height,
                          32,
                          0 );

    screen=i4_create_image(width, height, pal,
                           (w8 *)ximage->data,
                           width*(DEPTH/8));            // bytes per line

  }

  if (context)
    delete context;

  context=new i4_draw_context_class(0,0,screen->width()-1,screen->height()-1);
  context->both_dirty=new i4_rect_list_class;
  context->single_dirty=new i4_rect_list_class;
}


void x11_display_class::flush()   // the actual work of the function is in copy_part_to_vram
{  
  int mw,mh;
  i4_coord rmx,rmy;
  sw32 mx=input.mouse_x, my=input.mouse_y;
  if (mx<0) mx=0;
  if (my<0) my=0;

  rmx=mx-mouse_hot_x;
  rmy=my-mouse_hot_y;  

  if (mouse_pict)
  {
    mw=mouse_pict->width();
    mh=mouse_pict->height();

    if (pal->source.pixel_depth==I4_8BIT)
      mouse_save->set_pal(pal);
      

    i4_draw_context_class save_context(0,0,mw-1,mh-1);
    screen->put_part(mouse_save,0,0,rmx,rmy,rmx+mw-1,rmy+mh-1,
        save_context);  // save area behind mouse

    mouse_pict->put_image_trans(screen,rmx,rmy,mouse_trans,*context);
  }

  i4_rect_list_class::area_iter a;
  for (a=context->single_dirty->list.begin(); 
       a!=context->single_dirty->list.end();
       ++a)
    context->both_dirty->add_area(a->x1, a->y1, a->x2, a->y2);

  context->single_dirty->delete_list();


  context->both_dirty->intersect_area(0,0,width()-1,height()-1);

  a=context->both_dirty->list.begin();
  i4_bool error=i4_F;
  for (;a!=context->both_dirty->list.end();++a)
    error=(i4_bool)(error & copy_part_to_vram(a->x1,a->y1,a->x1,a->y1,a->x2,a->y2));
  
  if (!error)
    context->both_dirty->delete_list();

  if (mouse_pict)
    mouse_save->put_part(screen,rmx,rmy,0,0,mw-1,mh-1,*context);   // restore area behind mouse


}


i4_bool x11_display_class::copy_part_to_vram(i4_coord x, i4_coord y, 
                                             i4_coord x1, i4_coord y1, 
                                             i4_coord x2, i4_coord y2)
// this should copy a portion of an image to the actual screen at offset x, y
{
  if (shm_image)
    return shm_image->copy_part_to_vram(shm_extension,x,y,x1,y1,x2,y2);
  else
  {
    XPutImage(input.display, input.mainwin, input.gc, ximage,x1,y1,x,y,x2-x1+1,y2-y1+1);
    return i4_T;
  }
}


i4_bool x11_display_class::set_mouse_shape(i4_cursor_class *cursor)
{
  if (mouse_pict)
  {
    delete mouse_pict;
    delete mouse_save;
  }

  mouse_trans=cursor->trans;
  mouse_pict=cursor->pict->copy();

  mouse_save=i4_create_image(mouse_pict->width(),mouse_pict->height(),screen->get_pal());

  mouse_hot_x=cursor->hot_x;
  mouse_hot_y=cursor->hot_y;
  return i4_T;
}


