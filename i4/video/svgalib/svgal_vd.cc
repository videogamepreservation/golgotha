/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "video/display.hh"
#include "video/svgalib/svgal_vd.hh"
#include "image/image8.hh"
#include "device/kernel.hh"

#include <string.h>
#include <vga.h>
#include <unistd.h>

svgalib_display_class svgalib_display_instance;

svgalib_display_class::svgalib_display_class()
{
  init=NOT_INITIALIZED;
  screen=NULL;
  mouse_notify.display=this;

  mouse_event_flags=0;
  mouse_pict=mouse_save=NULL;
  context=0;
}

svgalib_display_class::~svgalib_display_class()
{
  kernel.remove_device(&devs);
}


i4_image_class *svgalib_display_class::next_frame_screen(i4_rect_list_class *area_of_change)
{
  i4_rect_list_class::area *a=area_of_change->get_list();
 
  for (;a;a=a->next)   // add all of the area of change to the dirty list
    screen->dirty.add_area(a->x1,a->y1,a->x2,a->y2);

  return screen;
}


i4_image_class *svgalib_display_class::both_screens()
{
  return screen;
}

void svgalib_display_class::flush()
{
  int mw,mh;
  i4_rect_list_class no_clip(0,0,width()-1,height()-1);

  if (mouse_pict)
  {
    screen->clip_list.swap(&no_clip);

    mw=mouse_pict->width();
    mh=mouse_pict->height();
    mouse_save->set_pal(pal);

    mouse_save->copy_part(screen,0,0,mouse_x,mouse_y,mouse_x+mw-1,mouse_y+mh-1);  // save area behind mouse
    screen->copy_from_trans(mouse_pict,mouse_x,mouse_y,0);
  }

  i4_rect_list_class::area *a=screen->dirty.get_list(),*next;
  for (;a;a=next)
  {
    next=a->next;
    copy_part_to_vram(screen,a->x1,a->y1,a->x1,a->y1,a->x2,a->y2);
  }  
  screen->dirty.delete_list();

  if (mouse_pict)
  {    
    screen->copy_part(mouse_save,mouse_x,mouse_y,0,0,mw-1,mh-1);   // restore area behind mouse
    screen->clip_list.swap(&no_clip);
  }  
}


void svgalib_display_class::copy_part_to_vram(i4_image_class *im, int x, int y, int x1, int y1, int x2, int y2)
{
  unsigned long screen_off;
  int ys,ye,         // ystart, yend
        xs,xe,      
        page,last_page=-1,yy;
  long breaker;
  unsigned char *line_addr;
  int xres=width()-1,yres=height()-1;

  if (y>(int)yres || x>(int)xres) return ;


  if (y<0)
  { y1+=-y; y=0; }
  ys=y1;
  if (y+(y2-y1)>=(int)yres)
    ye=(int)yres-y+y1-1;
  else ye=y2;

  if (x<0)
  { x1+=-x; x=0; }
  xs=x1;
  if (x+(x2-x1)>=(int)xres)
    xe=(int)xres-x+x1-1;
  else xe=x2;
  if (xs>xe || ys>ye) return ;

  // find the memory offset for the scan line of interest
  screen_off=((long)y*(long)(xres+1));
  int bpl=screen->bytes_per_line();


  for (yy=ys;yy<=ye;yy++,screen_off+=(xres+1))
  {
    page=screen_off>>16;     // which page of 64k are we on?
    if (page!=last_page)     
    { last_page=page;
      vga_setpage(page);     // switch to new bank
    }

    line_addr=im->image_data()+yy*bpl+xs;

    // breaker is the number of bytes before the page split
    breaker=(long)0xffff-(long)(screen_off&0xffff)+1;


    // see if the slam gets split by the page break
    if (breaker>x+xe-xs)
    {
      void *dest=v_addr+(screen_off&0xffff)+x;
      int size=xe-xs+1;
      memcpy(dest,line_addr,size);
    }
    else if (breaker<=x)
    { last_page++;
      vga_setpage(last_page);
      memcpy(v_addr+x-breaker,line_addr,xe-xs+1); 
    }
    else
    {
      memcpy(v_addr+(screen_off&0xffff)+x,line_addr,breaker-x);
      last_page++;
      vga_setpage(last_page);
      memcpy(v_addr,line_addr+breaker-x,xe-xs-(breaker-x)+1);
    } 
    y++;   
  }
}


static int svga_modes[]={  // a list of a the SVGALIB supported modes we support
  G320x200x256,
/*  G320x240x256,       // modex only?
  G320x400x256,
  G360x480x256, */
  G640x480x256,
  G800x600x256,
  G1024x768x256,
  G1280x1024x256,
  -1
} ;


void svgalib_display_class::fill_amode(int mode)
{
  memset(&amode,0,sizeof(amode));
  amode.mode_num=mode;
  vga_modeinfo *m=vga_getmodeinfo(svga_modes[mode]);
  amode.bits_per_pixel=8;
  amode.bits_per_color=8;
  amode.assoc=this;
  amode.xres=m->width;
  amode.yres=m->height;
}


i4_display_class::mode *svgalib_display_class::get_first_mode()
{
  if (!available()) return NULL;

  fill_amode(0);
  return &amode;
}


i4_display_class::mode *svgalib_display_class::get_next_mode(mode *last_mode)
{
  if (!last_mode || ((svga_mode *)last_mode)->assoc!=this) return NULL;   // make sure we created the last mode
  if (svga_modes[((svga_mode *)last_mode)->mode_num+1]==-1) return NULL;     // any mode modes left?
  fill_amode(  ((svga_mode *)last_mode)->mode_num+1);
  return &amode;
}


static unsigned char def_mouse[]=
    { 0,2,0,0,0,0,0,0,
      2,1,2,0,0,0,0,0,
      2,1,1,2,0,0,0,0,
      2,1,1,1,2,0,0,0,

      2,1,1,1,1,2,0,0,
      2,1,1,1,1,1,2,0,
      0,2,1,1,2,2,0,0,
      0,0,2,1,1,2,0,0,
      0,0,2,1,1,2,0,0,
      0,0,0,2,2,0,0,0 };    // 8x10


  // initialize_mode need not call close() to switch to another mode
i4_bool svgalib_display_class::initialize_mode(mode *which_one)
{
  if (!which_one || ((svga_mode *)which_one)->assoc!=this) return i4_F;

  cur_svga_mode=svga_modes[((svga_mode *)which_one)->mode_num];
  vga_setmode(cur_svga_mode);

  v_addr=vga_getgraphmem();
  if (pal.is_32bit())          // palette has not been created yet
  {
    w32 initial_pal[256];
    int x;
    for (x=0;x<256;x++)                       // initialize to a gray scale pal
      initial_pal[x]=(x<<16)|(x<<8)|x;

    pal=i4_pal_man.register_pal(i4_pal_handle_class::ID_8BIT,initial_pal);
  }

  realize_palette(pal);

  if (screen) 
    delete screen;

  screen=new i4_image8_clippable(which_one->xres,which_one->yres,pal);


  if (mouse_pal.is_32bit())
  {
    w32 mpal[256];
    memset(mpal,0,sizeof(mpal));
    mpal[0]=0;
    mpal[1]=0xffffff;
    mpal[2]=0x080808;
    mouse_pal=i4_pal_man.register_pal(i4_pal_handle_class::ID_8BIT,mpal);
  } 

  if (!mouse_event_flags)
    mouse_event_flags=local_devices()->request_device(&mouse_notify,i4_device_class::FLAG_MOUSE_MOVE);

  if (mouse_pict)    // has the mouse already been initialized?
  {
    delete mouse_pict;  mouse_pict=NULL;
    delete mouse_save;  mouse_save=NULL;
  }

  if (mouse_event_flags)
  {
    mouse_pict=new i4_image8(8,10,mouse_pal);    // the mouse pict has it's own palette to ensure it will be displayed properly
    mouse_save=new i4_image8(8,10,pal);    // the save image (for behinf the mouse) uses the screen palette
    memcpy(mouse_pict->image_data(),def_mouse,8*10);
  } 

  
}

  // should be called before a program quits
i4_bool svgalib_display_class::close()
{
  if (mouse_event_flags)
  {
    mouse_event_flags=0;
    local_devices()->release_device(&mouse_notify,i4_device_class::FLAG_MOUSE_MOVE);
  }

  if (screen) 
  {
    delete screen;
    screen=NULL;
  }

  if (mouse_pict)
  {
    delete mouse_pict;
    mouse_pict=NULL;
  }

  if (mouse_save)
  {
    delete mouse_save;
    mouse_save=NULL;
  }

  vga_setmode(TEXT);


}


i4_bool svgalib_display_class::available()
{
  if (init==NOT_INITIALIZED)
  {
    if (ioperm(0x3b4, 0x3df - 0x3b4 + 1, 1)!=0)   // make sure we have permission to vga ports
      init=INITIALIZED_AND_NOT_AVAILABLE;
    else if (vga_init()==-1)
      init=INITIALIZED_AND_NOT_AVAILABLE;
    else 
      init=INITIALIZED_AND_AVAILABLE;
  } 

  return (i4_bool)(init==INITIALIZED_AND_AVAILABLE);
}



i4_bool svgalib_display_class::realize_palette(i4_pal_handle_class pal_id)
{
  if (pal_id.is_8bit())
  {
    w32 *data=i4_pal_man.get_pal(pal_id);
    for (int i=0;i<256;i++,data++)
      vga_setpalette(i,(((*data)>>18)&0x3f),
		       (((*data)>>10)&0x3f),
		       (((*data)>>2)&0x3f));
    pal=pal_id;
    if (screen)
      screen->set_pal(pal);
    return i4_T;
  } else return i4_F;
}

