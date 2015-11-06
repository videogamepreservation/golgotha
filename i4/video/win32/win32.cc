/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "device/keys.hh"
#include "device/event.hh"
#include "device/kernel.hh"
#include "main/win_main.hh"

#include "video/win32/win32_input.hh"
#include "video/display.hh"
#include "image/context.hh"

class gdi_input : public win32_input_class
{
public:
  gdi_input() : win32_input_class(0) {}
  virtual void redraw(int x1, int y1, int x2, int y2);
  virtual void resize(sw32 w, sw32 h);
};



// void win32_display_class::get_desktop_format(i4_pixel_format &fmt)
// {

//       save = GetPixel(hdc,0,0);
//       SetPixel(hdc,0,0,RGB(0x08,0x08,0x08));
//       color = GetPixel(hdc,0,0) & 0xffffff;

//       switch (color)
//          {
//          //
//          // 0x000000 = 5-5-5
//          //

//          case 0x000000:
            
//             desktop_R_bitmask = 0x007c00;
//             desktop_G_bitmask = 0x0003e0;
//             desktop_B_bitmask = 0x00001f;
//             break;

//          //
//          // 0x000800 = 5-6-5
//          //

//          case 0x000800:
            
//             desktop_R_bitmask = 0x00f800;
//             desktop_G_bitmask = 0x0007e0;
//             desktop_B_bitmask = 0x00001f;
//             break;

//          //
//          // 0x080808 = 8-8-8
//          //

//          case 0x080808:

//             desktop_R_bitmask = 0xff0000;
//             desktop_G_bitmask = 0x00ff00;
//             desktop_B_bitmask = 0x0000ff;
//             break;

//          default:

//             if ((desktop_bpp == 15) || (desktop_bpp == 16))
//                {
//                desktop_R_bitmask = 0x00f800;
//                desktop_G_bitmask = 0x0007e0;
//                desktop_B_bitmask = 0x00001f;
//                }
//             break;
//          }

//       SetPixel(hdc,0,0,save);



class win32_display_class : public i4_display_class
{
public:
  gdi_input input;

  i4_draw_context_class *context;
  i4_image_class *backbuf;  
  HBITMAP          bitmap;

  i4_image_class *get_screen() { return backbuf; }
  i4_draw_context_class *get_context() { return context; }

  virtual i4_image_class *create_image(int w, int h, i4_bool in_vram, const i4_pal *pal)
  {
    return i4_create_image(w,h,pal);
  }


  void flush()
  {
    PAINTSTRUCT paint_s;
    HDC         dc;
    dc=GetDC(input.get_window_handle());


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
    
    ReleaseDC(input.get_window_handle(), dc);

  }


  w16 width() const { return backbuf->width(); }
  w16 height() const { return backbuf->height(); }


  i4_display_class::mode cur_mode, tmp_mode;

  i4_display_class::mode *current_mode() { return &cur_mode; }
    
  
  i4_display_class::mode *get_first_mode(int driver_id) 
  {
    strcpy(tmp_mode.name, "GDI window");
    tmp_mode.flags=mode::BACK_BUFFER | mode::RESOLUTION_DETERMINED_ON_OPEN;
    tmp_mode.xres=tmp_mode.yres=0;

    tmp_mode.bits_per_pixel=16;
    tmp_mode.red_mask=31<<10;
    tmp_mode.green_mask=31<<5;
    tmp_mode.blue_mask=31<<0;

    return &tmp_mode;
  }

  virtual mode *get_next_mode()  {    return 0; }

  i4_image_class *lock_frame_buffer(i4_frame_buffer_type type,
                                            i4_frame_access_type access)
  {
    return backbuf;
  }

  void unlock_frame_buffer(i4_frame_buffer_type type) { ; }

  i4_bool create_bmp(int w, int h)
  {
    cur_mode.xres=w;
    cur_mode.xres=h;

    if (context)
      delete context;
    context=new i4_draw_context_class(0,0, w-1, h-1);
    context->both_dirty=new i4_rect_list_class;
    context->single_dirty=new i4_rect_list_class;

    HDC dc=GetDC(input.get_window_handle());

    BITMAPINFOHEADER bmp;
    memset(&bmp, 0, sizeof(bmp));
    bmp.biSize=sizeof(bmp);

    bmp.biWidth=w;
    bmp.biHeight=-h;
    bmp.biPlanes=1;
    bmp.biBitCount=16;
    bmp.biCompression=BI_RGB;
    bmp.biSizeImage=w*h*2;

    BITMAPINFO bmp_info;
    memcpy(&bmp_info, &bmp, sizeof(bmp));

    void *data_address;
         
    bitmap=CreateDIBSection(dc,
                            &bmp_info, DIB_RGB_COLORS,
                            &data_address,           // where bitmap's data will be stored
                            0,                           // don't use a file
                            0);

    backbuf->w=w;
    backbuf->bpl=w*2;
    backbuf->h=h;
    backbuf->data=data_address;   

    if (!bitmap)
    {
      i4_warning("Error CreateDIBSection failed!");
      input.destroy_window();
      return i4_F;
    }

    return i4_T;
  }
  

  i4_bool initialize_mode()
  {
    memcpy(&cur_mode, &tmp_mode, sizeof(tmp_mode));
    if (!input.create_window(0,0, cur_mode.xres, cur_mode.yres, this, i4_F))
      return i4_F;

    int x,y,w,h;
    input.get_window_area(x,y,w,h);
    cur_mode.xres=w;
    cur_mode.yres=h;


    i4_pixel_format fmt;
    fmt.red_mask=cur_mode.red_mask;
    fmt.green_mask=cur_mode.green_mask;
    fmt.blue_mask=cur_mode.blue_mask;
    fmt.alpha_mask=0;
    fmt.pixel_depth=I4_16BIT;
    fmt.calc_shift();

    pal=i4_pal_man.register_pal(&fmt);
    backbuf=i4_create_image(cur_mode.xres, cur_mode.yres, pal,
                            0, cur_mode.xres*2);

    return create_bmp(cur_mode.xres, cur_mode.yres);
  }

  i4_bool close()
  {
    if (backbuf)
    {
      input.destroy_window();
      delete backbuf;
      DeleteObject(bitmap);
      delete context;
      return i4_T;
    }


    return i4_F;
  }

  

  virtual i4_bool set_mouse_shape(i4_cursor_class *cursor) { return i4_F; }


  virtual i4_bool display_busy() const { return i4_F; }

  win32_display_class() 
  {
    backbuf=0;
    context=0;
  }

  void init()
  {    
    i4_display_list_struct *s=new i4_display_list_struct;
    s->add_to_list("Windowed GDI", 0, this, i4_display_list);
  }             

  virtual i4_refresh_type update_model() { return I4_BLT_REFRESH; }
  virtual i4_bool lock_mouse_in_place(i4_bool yes_no) { return i4_F; }
};

win32_display_class win32_display_instance;


void gdi_input::redraw(int x1, int y1, int x2, int y2)
{
  if (win32_display_instance.backbuf)
    win32_display_instance.context->both_dirty->add_area(x1,y1,x2,y2);
}

void gdi_input::resize(sw32 w, sw32 h)
{
  if (win32_display_instance.backbuf)
  {
    w=(w+3)&(~3);
    win32_display_class *wi=&win32_display_instance;
    DeleteObject(wi->bitmap);
    wi->create_bmp(w,h);
  }
}
