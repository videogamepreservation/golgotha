/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "video/display.hh" 
#include "video/x11/x11devs.hh"
#include "device/kernel.hh"
#include "palette/pal.hh"
#include "error/error.hh"
#include "memory/malloc.hh"
#include "device/keys.hh"
#include "time/profile.hh"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

x11_display_class x11_display_instance;


static Cursor CreateNullCursor(Display *display, Window root)
// makes a null cursor
{
  Pixmap cursormask; 
  XGCValues xgc;
  GC gc;
  XColor dummycolour;
  Cursor cursor;

  cursormask = XCreatePixmap(display, root, 1, 1, 1/*depth*/);
  xgc.function = GXclear;
  gc =  XCreateGC(display, cursormask, GCFunction, &xgc);
  XFillRectangle(display, cursormask, gc, 0, 0, 1, 1);
  dummycolour.pixel = 0;
  dummycolour.red = 0;
  dummycolour.flags = 04;
  cursor = XCreatePixmapCursor(display, cursormask, cursormask,
                               &dummycolour,&dummycolour, 0,0);
  XFreePixmap(display,cursormask);
  XFreeGC(display,gc);
  return cursor;
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

  mouse_save=new I4_SCREEN_TYPE(mouse_pict->width(),mouse_pict->height(),screen->get_pal());

  mouse_hot_x=cursor->hot_x;
  mouse_hot_y=cursor->hot_y;
  return i4_T;
}


enum { devices_handled =
       i4_device_class::FLAG_MOUSE_BUTTON_UP   |
       i4_device_class::FLAG_MOUSE_BUTTON_DOWN |
       i4_device_class::FLAG_MOUSE_MOVE        |
       i4_device_class::FLAG_KEY_PRESS         |
       i4_device_class::FLAG_KEY_RELEASE       |
       i4_device_class::FLAG_DISPLAY_CHANGE };


void x11_display_class::x11_device_manager::get_x_time(w32 xtick, i4_time_class &t)
{
  if (need_first_time)
  {
    first_time=xtick;
    need_first_time=i4_F;
  }

  w32 xtime=xtick-first_time;
  
  i4_time_class now;
  i4_time_class x_now(xtime);
              
  sw32 diff=x_now.milli_diff(now);

  if (diff>x11_start_time_best_diff)
  {
    x11_start_time_best_diff=diff;
    x11_start_time=xtime;
    i4_start_time=now;
  }
              
  t=i4_start_time;
  t.add_milli(xtime-x11_start_time);
}

i4_bool x11_display_class::x11_device_manager::process_events()       
// returns true if an event was dispatched
{
  i4_bool motion_occured=i4_F;
  sw32 final_x, final_y;

  if (parent->display)
  {
    while (XPending(parent->display))
    {
      XEvent xev;
      XNextEvent(parent->display,&xev);

      if (parent->shm_extension)
        parent->shm_extension->note_event(xev);

      switch (xev.type)
      { 
        case Expose :
                  { 
          if (parent->context)
          {
            parent->context->add_both_dirty(xev.xexpose.x,
                                            xev.xexpose.y,
                                            xev.xexpose.x+xev.xexpose.width,
                                            xev.xexpose.y+xev.xexpose.height);
          }
        } break;
          
        //{{{  not implemented yet
        case ConfigureNotify :
        {
          XFlush(parent->display);

          int new_width=xev.xconfigure.width&~3;  // must be word alligned
          int new_height=xev.xconfigure.height;
          if (new_width!=xev.xconfigure.width)
            XResizeWindow(parent->display,parent->mainwin,new_width,xev.xconfigure.height);

          XFlush(parent->display);

          parent->destroy_X_image();
          parent->create_X_image(new_width, new_height);
   
          i4_display_change_event_class d_change(parent,
                                                 i4_display_change_event_class::SIZE_CHANGE);
          
          send_event_to_agents(&d_change,FLAG_DISPLAY_CHANGE);
        } break;
        
        case MotionNotify : 
                  {
          motion_occured=i4_T;
          final_x=xev.xmotion.x;
          final_y=xev.xmotion.y;
        } break;
          
        case ButtonRelease :
                  {   
          i4_mouse_button_up_event_class::btype but;
          switch (xev.xbutton.button)
          {
            case 1 : but=i4_mouse_button_up_event_class::LEFT; break;
            case 3 : but=i4_mouse_button_up_event_class::RIGHT; break;
            case 2 : but=i4_mouse_button_up_event_class::CENTER; break;
          }
          i4_mouse_button_up_event_class up(but);
          send_event_to_agents(&up,FLAG_MOUSE_BUTTON_UP);
        } break;
          
        case ButtonPress :
                  {   
          i4_mouse_button_down_event_class::btype but;
          switch (xev.xbutton.button)
          {
            case 1 : but=i4_mouse_button_down_event_class::LEFT; break;
            case 3 : but=i4_mouse_button_down_event_class::RIGHT; break;
            case 2 : but=i4_mouse_button_down_event_class::CENTER; break;
          }
          i4_mouse_button_down_event_class down(but);
          send_event_to_agents(&down,FLAG_MOUSE_BUTTON_DOWN);
        } break;
          
        case KeyPress :
        case KeyRelease :
                  {
          char buf;
          KeySym ks;
          XLookupString(&xev.xkey,&buf,1,&ks,NULL); 
          w16 key, key_code;

          switch (ks)
          { 
            case XK_Down :    key_code=I4_DOWN; break;
            case XK_Up :      key_code=I4_UP;  break;
            case XK_Left :    key_code=I4_LEFT;  break;
            case XK_Right :     key_code=I4_RIGHT; break;
            case XK_Control_L :   
            { 
              key_code=I4_CTRL_L; 
              if (xev.type==KeyPress)
                modifier_state|=I4_MODIFIER_CTRL_L;  
              else
                modifier_state&=~I4_MODIFIER_CTRL_L;  
            } break;

            case XK_Control_R :   
            { 
              key_code=I4_CTRL_R; 
              if (xev.type==KeyPress)
                modifier_state|=I4_MODIFIER_CTRL_R;  
              else
                modifier_state&=~I4_MODIFIER_CTRL_R;
            } break;

            case XK_Alt_L :   
            { 
              key_code=I4_ALT_L; 
              if (xev.type==KeyPress)
                modifier_state|=I4_MODIFIER_ALT_L;  
              else
                modifier_state&=~I4_MODIFIER_ALT_L;
            } break;

            case XK_Alt_R :   
            { 
              key_code=I4_ALT_R; 
              if (xev.type==KeyPress)
                modifier_state|=I4_MODIFIER_ALT_R;  
              else
                modifier_state&=~I4_MODIFIER_ALT_R;
            } break;


            case XK_Shift_L :   
            { 
              key_code=I4_SHIFT_L; 
              if (xev.type==KeyPress)
                modifier_state|=I4_MODIFIER_SHIFT_L;  
              else
                modifier_state&=~I4_MODIFIER_SHIFT_L;
            } break;

            case XK_Shift_R :
            { 
              key_code=I4_SHIFT_R; 
              if (xev.type==KeyPress)
                modifier_state|=I4_MODIFIER_SHIFT_R;  
              else
                modifier_state&=~I4_MODIFIER_SHIFT_R;
            } break;


            case XK_Num_Lock :          key_code=I4_NUM_LOCK;  break;
            case XK_Home :    key_code=I4_HOME;  break;
            case XK_End :     key_code=I4_END;  break;
            case XK_BackSpace :         key_code=I4_BACKSPACE;  break;
            case XK_Tab :   key_code=I4_TAB;  break;
            case XK_Return :    key_code=I4_ENTER;  break;
            case XK_Caps_Lock :         key_code=I4_CAPS;  break;
            case XK_Escape :    key_code=I4_ESC;  break;
            case XK_F1 :                key_code=I4_F1; break;
            case XK_F2 :                key_code=I4_F2; break;
            case XK_F3 :                key_code=I4_F3; break;
            case XK_F4 :                key_code=I4_F4; break;
            case XK_F5 :                key_code=I4_F5; break;
            case XK_F6 :                key_code=I4_F6; break;
            case XK_F7 :                key_code=I4_F7; break;
            case XK_F8 :                key_code=I4_F8; break;
            case XK_F9 :                key_code=I4_F9; break;
            case XK_F10 :               key_code=I4_F10; break;
            case XK_Insert :            key_code=I4_INSERT; break;
            case XK_Page_Up :           key_code=I4_PAGEUP; break;
            case XK_Page_Down :         key_code=I4_PAGEDOWN; break;
            case XK_Delete :            key_code=I4_DEL; break;

            case XK_KP_0 :              key_code=I4_KP0; break;
            case XK_KP_1 :              key_code=I4_KP1; break;
            case XK_KP_2 :              key_code=I4_KP2; break;
            case XK_KP_3 :              key_code=I4_KP3; break;
            case XK_KP_4 :              key_code=I4_KP4; break;
            case XK_KP_5 :              key_code=I4_KP5; break;
            case XK_KP_6 :              key_code=I4_KP6; break;
            case XK_KP_7 :              key_code=I4_KP7; break;
            case XK_KP_8 :              key_code=I4_KP8; break;
            case XK_KP_9 :              key_code=I4_KP9; break;

            case ' ':                   key_code=I4_SPACE; break;
            case '`':
            case '~':                   key_code='`'; break;
            case '-':
            case '_':                   key_code='-'; break;
            case '=':
            case '+':                   key_code='='; break;
            case '[':
            case '{':                   key_code='['; break;
            case ']':
            case '}':                   key_code=']'; break;
            case '\\':
            case '|':                   key_code='\\'; break;
            case ';':
            case ':':                   key_code=';'; break;
            case '\'':
            case '"':                   key_code='\''; break;
            case ',':
            case '<':                   key_code=','; break;
            case '.':
            case '>':                   key_code='.'; break;
            case '/':
            case '?':                   key_code='/'; break;

            case ')':                   key_code='0'; break;
            case '!':                   key_code='1'; break;
            case '@':                   key_code='2'; break;
            case '#':                   key_code='3'; break;
            case '$':                   key_code='4'; break;
            case '%':                   key_code='5'; break;
            case '^':                   key_code='6'; break;
            case '&':                   key_code='7'; break;
            case '*':                   key_code='8'; break;
            case '(':                   key_code='9'; break;

            default :
              if ((ks>=XK_A && ks<=XK_Z) || (ks>=XK_0 && ks<=XK_9))
                key_code = ks;
              else if (ks>=XK_a && ks<=XK_z)
                key_code = ks + 'A' - 'a';
              else
                key_code=0;
          }

          if (key_code)
          {
            if (xev.type==KeyPress)
            {
              i4_time_class t;
              get_x_time(xev.xkey.time, t);
              
              key = i4_key_translate(key_code,1,modifier_state);
              i4_key_press_event_class ev(key, key_code, modifier_state, t);

              send_event_to_agents(&ev,FLAG_KEY_PRESS);    
            }
            else 
            {
              key = i4_key_translate(key_code,0, modifier_state);

              i4_time_class t;
              get_x_time(xev.xkey.time, t);
              
              i4_key_release_event_class ev(key, key_code, modifier_state, t);
              send_event_to_agents(&ev,FLAG_KEY_RELEASE);    
            }
          }
        } break;
      }
    }
  }
  
          
  if (motion_occured && (final_x!=parent->mouse_x || final_y!=parent->mouse_y))
  {    
    i4_mouse_move_event_class move(parent->mouse_x, parent->mouse_y, final_x, final_y);
    send_event_to_agents(&move,FLAG_MOUSE_MOVE);

    if (parent->mouse_locked)
    {
      XWarpPointer(parent->display, None, parent->mainwin, 0,0, 0,0, 
                   parent->mouse_x, 
                   parent->mouse_y);
    }
    else
    {
      parent->mouse_x=final_x;
      parent->mouse_y=final_y;
    }   
  }

  return i4_F;
}


x11_display_class::x11_device_manager::x11_device_manager()
{
  modifier_state=0;
  need_first_time=1;
  x11_start_time_best_diff=0;
  parent=&x11_display_instance;
}


x11_display_class::x11_display_class()
{
  mouse_locked=i4_F;
  screen=0;
  context=0;
  shm_extension=0;
  shm_image=0;
  ximage=0;
  display=0;
}


i4_image_class *x11_display_class::get_screen()
{
  return screen;
}


void db_show()
{
  x11_display_instance.get_context()->add_both_dirty(0,0,
                                                x11_display_instance.get_screen()->width(),
                                                x11_display_instance.get_screen()->height());
  x11_display_instance.flush();
  x11_display_instance.get_context()->add_both_dirty(0,0,
                                                x11_display_instance.get_screen()->width(),
                                                x11_display_instance.get_screen()->height());
}


static i4_profile_class x11_flush_ph("X11 video flush (XPutImage)");


void x11_display_class::flush()   // the actual work of the function is in copy_part_to_vram
{  
  int mw,mh;
  i4_coord rmx,rmy;
  x11_flush_ph.start();
  sw32 mx=mouse_x, my=mouse_y;
  if (mx<0) mx=0;
  if (my<0) my=0;

  rmx=mx-mouse_hot_x;
  rmy=my-mouse_hot_y;  

  if (mouse_pict)
  {
    mw=mouse_pict->width();
    mh=mouse_pict->height();

    if (pal.source_type()==I4_8BIT)
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

  x11_flush_ph.stop();
}


// Dave : have the name return a string appropriate to the extensions
// available on the Xserver, i.e. MITSHM or the page-flip thingy
char *x11_display_class::name() const
{
  char buf[200];
  if (getenv("DISPLAY"))
    sprintf(buf,"X Server : %s",getenv("DISPLAY"));
  else strcpy(buf,"X Server : localhost");
  return buf;
}


i4_display_class::mode *x11_display_class::get_first_mode()
{
  //  Check to see if we need to open the display, 
  // since get_first_mode() is valid while a display is
  i4_bool need_close=i4_F;
  if (!display)
  {
    need_close=i4_T;
    if (!open_display())
      return 0;
  }

  //  Check to see if they have an 8 bit visual on the display
  int items;
  XVisualInfo vis_info;

  if (I4_SCREEN_DEPTH==8)
  {
    vis_info.depth=8;
    vis_info.c_class=PseudoColor;
  }
  else if (I4_SCREEN_DEPTH==15)
  {
    vis_info.depth=16;
    vis_info.c_class=TrueColor;
  } 
  else if (I4_SCREEN_DEPTH==16)
  {
    vis_info.depth=16;
    vis_info.c_class=TrueColor;
  }
  else if (I4_SCREEN_DEPTH==32)
  {
    vis_info.depth=24;
    vis_info.c_class=TrueColor;
  }

  XVisualInfo *return_visual=XGetVisualInfo(display,
      VisualClassMask | VisualDepthMask, &vis_info,&items);

  amode.red_mask    = return_visual->red_mask;
  amode.green_mask  = return_visual->green_mask;
  amode.blue_mask   = return_visual->blue_mask;


  XFree((char *)return_visual);
  if (items==0)
  {
    if (need_close)
      close_display();
    return 0;
  }
  
  strcpy(amode.name,"Standard X11 window");
  amode.flags=mode::RESOLUTION_DETERMINED_ON_OPEN;
  amode.bits_per_pixel=I4_SCREEN_DEPTH;



  Window root;
  int x,y;
  unsigned w,h,bord,depth;
  if (XGetGeometry(display,RootWindow(display,DefaultScreen(display)),&root, 
                   &x,&y,&w,&h,&bord,&depth)==0)
    i4_error("can't get root window attributes");

  amode.xres=w;
  amode.yres=h;

  amode.assoc=this;          // so we know in 'initialize_mode' that we created this mode

  if (need_close)
    close_display();

  return &amode;
}


i4_display_class::mode *x11_display_class::get_next_mode(mode *last_mode)
{
  return NULL;   // for now only straight X stuff
                 // this should return modes for direct screen access
}


void x11_display_class::close_X_window()
{
  destroy_X_image();
  if (shm_extension) 
    shm_extension->shutdown(display);

  XFreeGC(display,gc);
  XFree((char *)my_visual);
  XCloseDisplay(display);
  display=0;

  if (I4_SCREEN_DEPTH==8)
    XFreeColormap(display,xcolor_map);

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


void x11_display_class::destroy_X_image()
{
  if (shm_extension)
  {
    if (shm_image)
      shm_extension->destroy_shm_image(display, shm_image);
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
    shm_image=shm_extension->create_shm_image(display,
                                              mainwin,
                                              gc,
                                              my_visual->visual,
                                              my_visual->depth,
                                              w,h);   // actual width and height from X
    screen=new I4_SCREEN_TYPE(w,h,pal,
                              w*I4_BYTES_PER_PIXEL,
                              (w8 *)shm_image->data);
  }
  else
  {

    ximage = XCreateImage(  display,
                          my_visual->visual,
                          my_visual->depth,
                          ZPixmap,
                          0,
                          (char *)i4_malloc(width*height*I4_BYTES_PER_PIXEL,"Ximage"),
                          width, 
                          height,
                          32,
                          0 );

    screen=new I4_SCREEN_TYPE(width,height,pal,
                              width*I4_BYTES_PER_PIXEL,             // bytes per line
                              (w8 *)ximage->data);
  }

  if (context)
    delete context;

  context=new i4_draw_context_class(0,0,screen->width()-1,screen->height()-1);
  context->both_dirty=new i4_rect_list_class;
  context->single_dirty=new i4_rect_list_class;
}


i4_bool x11_display_class::open_X_window(w32 width, w32 height, i4_display_class::mode *mode)
// assumes that the display has already been grabbed
{
  if (!open_display()) return i4_F;

  width=(width+3)&(~3);

  XVisualInfo vis_info;
  int items;
  XEvent report;
  XTextProperty winName;
  char *win_name,*icon_name;
  if (i4_global_argv)
    win_name=icon_name=i4_global_argv[0];
  else 
    win_name=icon_name="...";

  // locate a visual same depth as i4_SCREEN_DEPTH

  if (I4_SCREEN_DEPTH==8)
  {
    vis_info.depth=8;
    vis_info.c_class=PseudoColor;
  }
  else if (I4_SCREEN_DEPTH==15)
  {
    vis_info.depth=16;
    vis_info.c_class=TrueColor;
  }
  else if (I4_SCREEN_DEPTH==16)
  {
    vis_info.depth=16;
    vis_info.c_class=TrueColor;
  }
  else if (I4_SCREEN_DEPTH==32)
  {
    vis_info.depth=24;
    vis_info.c_class=TrueColor;
  }


  my_visual   = XGetVisualInfo(display,VisualClassMask | VisualDepthMask, &vis_info,&items);
  screen_num  = DefaultScreen(display);

  if (items>0)
  {
    XMatchVisualInfo(display, screen_num, vis_info.depth, vis_info.c_class, &vis_info);
    my_visual->visual = vis_info.visual;
  } else return i4_F;


  Colormap tmpcmap;
  
  tmpcmap = XCreateColormap(display, 
      XRootWindow(display,my_visual->screen),my_visual->visual, 
      AllocNone);

  int attribmask = CWColormap | CWBorderPixel;
  XSetWindowAttributes attribs;
  attribs.border_pixel = 0;
  attribs.colormap = tmpcmap;
  
  width=(width+3)&(0xffffffff-3);        
  mainwin=XCreateWindow(display,
      XRootWindow(display,my_visual->screen),
      0,0,
      width,height,
      0,
      my_visual->depth,
      InputOutput,
      my_visual->visual,
      attribmask,
      &attribs);
  XFreeColormap(display,tmpcmap);

  XSelectInput(display,mainwin, 
         KeyPressMask     | VisibilityChangeMask | ButtonPressMask | ButtonReleaseMask |
         ButtonMotionMask | PointerMotionMask    | KeyReleaseMask  | ExposureMask      | 
               StructureNotifyMask);

  XGCValues values;
  gc=XCreateGC(display,mainwin,0,&values);
  XSetBackground(display,gc,BlackPixel(display,screen_num));

  XMapWindow(display,mainwin);
  do
  { XNextEvent(display, &report);
  } while (report.type!= Expose);     // wait for our window to pop up

  /*
  if (I4_SCREEN_DEPTH==8)
    xcolor_map=XCreateColormap(display,mainwin,my_visual->visual,AllocAll);  
    */
  
  i4_pixel_format f;
  f.red_mask   = mode->red_mask;
  f.green_mask = mode->green_mask;
  f.blue_mask  = mode->blue_mask;
  f.alpha_mask  = 0;

  f.calc_shift();

  f.lookup      = 0;
  f.pixel_depth = I4_16BIT;

  pal = i4_pal_man.register_pal(&f);


  if (shm_extension)
  {
    char *ds_name=getenv("DISPLAY");
    if (!ds_name) 
      ds_name="unix:0.0";
    
    if (!shm_extension->available(display,ds_name))
      shm_extension=0;
  }

  create_X_image(width, height);

  return i4_T;
}



i4_bool x11_display_class::initialize_mode(mode *which_one)
{
  if ((((x11_mode *)which_one)->assoc)==this)
  {
    memcpy(&cur_mode,which_one,sizeof(cur_mode));
    // this should open an X window

    open_X_window(cur_mode.xres, cur_mode.yres, which_one);

    XDefineCursor(display,mainwin, CreateNullCursor(display,mainwin));

    i4_kernel.add_device(&devs);

  } else return i4_F;  
  return i4_T;
}


i4_bool x11_display_class::close()
{
  if (screen)
  {
    close_X_window();
    i4_kernel.remove_device(&devs);
  }
  // close the X window, or return to normal mode of operation 
  // prepare for end of program or next mode set
}


i4_bool x11_display_class::open_display()
{
  // get the default display name from the environment variable DISPLAY
  char *ds_name=getenv("DISPLAY");
  if (!ds_name || ds_name[0]==0)
    ds_name="unix:0.0";

  // check the command line args for another display
  w32 i=1;
  for (;i<i4_global_argc;i++)
  {
    if (!strcmp(i4_global_argv[i],"-display"))
    {
      i++;
      ds_name=i4_global_argv[i];
    }
  }
  display=XOpenDisplay(ds_name);  
  if (!display)
    return i4_F;
  else return i4_T;
}


void x11_display_class::close_display()
{
  XCloseDisplay(display);
  display=NULL;
}


i4_bool x11_display_class::available()
{
  if (display)    // arleady open
    return i4_T;

  // determine if the display is available
  if (open_display())
  {
    close_display();
    return i4_T;
  } else
   return i4_F;
}


i4_bool x11_display_class::realize_palette(i4_pal_handle_class pal_id)
// loads the palette into hardware registers, 
// return i4_F if pal_id is wrong color depth for display
// use i4_pal_manager_class::get_pal() to retrieve palette memory
{
  /*  if (pal_id.is_8bit())
  {    
    XColor color;
    color.flags=DoRed|DoBlue|DoGreen;
    w32 *p=i4_pal_man.get_pal(pal_id);
    int i;
    for (i=0;i<256;i++,p++)
    {
      color.pixel=i;      
      color.red=  ((((*p)&0x00ff0000)>>16)<<8)|0xff; 
      color.green=((((*p)&0x0000ff00)>>8)<<8)|0xff; 
      color.blue= ((((*p)&0x000000ff))<<8)|0xff; 
      XStoreColor(display,xcolor_map,&color);
    }
    XSetWindowColormap(display,mainwin,xcolor_map);
    pal=pal_id;
    if (screen)
       screen->set_pal(pal);
  } */
  return i4_T;
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
    XPutImage(display,mainwin,gc,ximage,x1,y1,x,y,x2-x1+1,y2-y1+1);
    return i4_T;
  }
}

