/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "video/win32/win32.hh"
#include "device/event.hh"
#include "main/win_main.hh"

class gdi_accelerator_class : public win32_accelerator_class
{
  I4_SCREEN_TYPE  *back_buffer;
  BITMAPINFOHEADER back_buffer_bmp;
  BITMAPINFO       dc_bmpinfo;
  i4_bool          win_is_open;
  HBITMAP          bitmap;

public:
  class gdi_mode : public i4_display_class::mode
  {
  public:
    gdi_accelerator_class *assoc;      // pointer to use, so we can confirm we created this mode

  };
  gdi_mode amode, cur_mode;


  virtual w16 width() const
  {
    return back_buffer->width();
  }
  
  virtual w16 height() const
  {
    return back_buffer->height();
  }
  
  void setup_back_buffer_bmp(w32 width, w32 height)
  {
    if (back_buffer)
    {
      delete back_buffer;
      DeleteObject(bitmap);
    }

    HDC         dc;
    dc=GetDC(win32_display_instance.window_handle);

    memset(&back_buffer_bmp,0,sizeof(back_buffer_bmp));
    back_buffer_bmp.biSize=sizeof(back_buffer_bmp);
    back_buffer_bmp.biWidth=width;
    back_buffer_bmp.biHeight=-height;
    back_buffer_bmp.biPlanes=1;
    back_buffer_bmp.biBitCount=I4_BYTES_PER_PIXEL*8;
    back_buffer_bmp.biCompression=BI_RGB;
    back_buffer_bmp.biSizeImage=width*height*I4_BYTES_PER_PIXEL;

    memcpy(&dc_bmpinfo.bmiHeader,&back_buffer_bmp,sizeof(back_buffer_bmp));

    void *bmp_data_address;
     
    bitmap=CreateDIBSection(dc,
                            &dc_bmpinfo,DIB_RGB_COLORS,
                            &bmp_data_address,           // where bitmap's data will be stored
                            0,                           // don't use a file
                            0);
    if (!bitmap)
      i4_error("Error CreateDIBSection failed!");

    i4_pal_handle_class pal;

#if (I4_SCREEN_DEPTH==15 || I4_SCREEN_DEPTH==16)
    pal.set_type(i4_pal_handle_class::ID_16BIT);
#else
#error add code here
#endif
    
    back_buffer=new I4_SCREEN_TYPE(width,height,
                                   pal,
                                   width*I4_BYTES_PER_PIXEL,   // bytes per line
                                   (w8 *)bmp_data_address);
    if (context)
      delete context;

    context=new i4_draw_context_class(0,0,width-1,height-1);
    context->both_dirty=new i4_rect_list_class;
    context->both_dirty->add_area(0,0,width-1,height-1);
    context->single_dirty=new i4_rect_list_class;
  }
  
  virtual void init()
  {
    back_buffer=0;
    win32_accelerator_class::init();
  }
  
  // called when windows gives us a WM_MOVE message, this tells everyone else interested
  virtual void move_screen(i4_coord x, i4_coord y)               
  {
    // We don't care about moves in this case
    // Because we are drawing an off-screen buffer, window moves don't affect anything
  }
  
  // called when windows gives us a WM_MOVE message, this tells everyone else interested
  virtual void resize_screen(i4_coord w, i4_coord h)             
  {
    // If the window resizes, we need to resize our off-screen buffer and send a message 
    // to everyone who is interested
    setup_back_buffer_bmp(w,h);

    delete context;
    context=new i4_draw_context_class(0,0,w-1,h-1);
    context->both_dirty=new i4_rect_list_class;
    context->both_dirty->add_area(0,0,w-1,h-1);
    context->single_dirty=new i4_rect_list_class;

    i4_display_change_event_class ev(&win32_display_instance,i4_display_change_event_class::SIZE_CHANGE);
    win32_display_instance.input.send_event_to_agents(&ev,i4_device_class::FLAG_DISPLAY_CHANGE);
  }
  

  virtual mode *current_mode()
  {
    return &cur_mode;
  }

  virtual i4_display_class::mode *get_first_mode()
  {
    memset(&amode,0,sizeof(amode));
    strcpy(amode.name,"Win32 GDI window");
    amode.flags=i4_display_class::mode::RESOLUTION_DETERMINED_ON_OPEN;

    amode.red_mask=(31<<10);
    amode.green_mask=(31<<5);
    amode.blue_mask=(31<<0);

    amode.bits_per_pixel=16;

    amode.xres=640;
    amode.yres=480;

    amode.assoc=this;          // so we know in 'initialize_mode' that we created this mode

    return &amode;
  }
  
  virtual i4_display_class::mode *get_next_mode(i4_display_class::mode *last_mode)
  {
    return 0;
  }
  
  virtual i4_bool initialize_mode(i4_display_class::mode *which_one)
  {
    if (!back_buffer)
    {
      memcpy(&cur_mode, which_one, sizeof(cur_mode));
      win32_display_instance.window_handle = CreateWindowEx(
                                                            0,
                                                            //                                                             WS_EX_TOPMOST,
                                                            win32_display_instance.class_name(),
                                                            win32_display_instance.class_name(),
                                                            WS_OVERLAPPEDWINDOW,
                                                            0,
                                                            0,
                                                            which_one->xres,
                                                            which_one->yres,
                                                            NULL,
                                                            NULL,
                                                            i4_win32_instance,
                                                            NULL );


      ShowWindow( win32_display_instance.window_handle, SW_SHOW );
      UpdateWindow( win32_display_instance.window_handle );

      setup_back_buffer_bmp(which_one->xres,
                            which_one->yres);
     

      return i4_T;
    } 
    else
      return i4_F;
  }
  
  virtual i4_bool close()
  {
    if (back_buffer)
    {
      DestroyWindow(win32_display_instance.window_handle);
      delete back_buffer;
      back_buffer=0;
      return i4_T;
    } else return i4_F;
  }
  
  virtual i4_image_class *get_screen()
  {
    return back_buffer;
  }
  
  virtual void flush()
  {
    PAINTSTRUCT paint_s;
    HDC         dc;
    dc=GetDC(win32_display_instance.window_handle);


    SelectObject(dc,bitmap);

    HDC hdcMem = CreateCompatibleDC(dc);
    bitmap = SelectObject(hdcMem,bitmap);


    for (i4_rect_list_class::area_iter a=context->single_dirty->list.begin();
         a!=context->single_dirty->list.end();
         ++a)
      context->both_dirty->add_area(a->x1, a->y1, a->x2, a->y2);

    context->single_dirty->delete_list();

    context->both_dirty->intersect_area(0,0,width()-1,height()-1);



    a=context->both_dirty->list.begin();
    for (;a!=context->both_dirty->list.end();++a)
      BitBlt(dc,a->x1,a->y1,
             a->x2-a->x1+1,a->y2-a->y1+1,
             hdcMem,a->x1,a->y1,SRCCOPY);

    // this is to reduce the flicker of the mouse cursor
    POINT p;
    GetCursorPos(&p);
    SetCursorPos(p.x, p.y);
  
    context->both_dirty->delete_list();
    bitmap = SelectObject(hdcMem,bitmap);         // windows says to do this

    DeleteDC(hdcMem);
    
    ReleaseDC(win32_display_instance.window_handle,dc);
  }
  
  virtual i4_bool set_mouse_shape(i4_cursor_class *cursor)
  {
    return i4_F;
  }
  
  virtual i4_bool realize_palette(i4_pal_handle_class pal_id)
  {
    return i4_T;   // pretend like this works for now
  }
  
  virtual w32 priority()
  {
    return 1;
  }
  
  gdi_accelerator_class() { context=0; }
};

static gdi_accelerator_class gdi_accelerator_instance;




