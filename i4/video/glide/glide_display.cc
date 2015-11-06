/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "video/glide/glide_display.hh"
#include "image/image.hh"
#include "time/profile.hh"
#include "image/context.hh"
#include "image/image16.hh"

#ifdef _WINDOWS
#include "video/glide/glidefunc.cc"
#endif

i4_profile_class pf_glide_flush_lock_write("glide_flush :: lock(WRITE)");
i4_profile_class pf_glide_flush_mouse("glide_flush :: mouse draw");
i4_profile_class pf_glide_flush_mouse_save("glide_flush :: mouse save");
i4_profile_class pf_glide_flush_copy("glide_flush :: copy_vram");
i4_profile_class pf_glide_flush_flip("glide_flush :: page_flip");

i4_glide_display_class *i4_glide_display=0;

struct gr_buf_status_type
{
  sw8            state;  // -1 = not locked, else  
                         //    I4_FRAME_BUFFER_READ,
                         // or I4_FRAME_BUFFER_WRITE
  i4_image_class *im;
  int gr_lock_type;

} gr_buf[2];    // I4_FRONT_FRAME_BUFFER=0, I4_BACK_FRAME_BUFFER=1


i4_image_class *i4_glide_display_class::lock_frame_buffer(i4_frame_buffer_type type,
                                                          i4_frame_access_type access)
{
  if (gr_buf[type].state!=access)
  {
    if (gr_buf[type].state!=-1)
      unlock_frame_buffer(type);

    int gr_access = access==I4_FRAME_BUFFER_READ ? GR_LFB_READ_ONLY : GR_LFB_WRITE_ONLY;
    int gr_buf_t = type == I4_FRONT_FRAME_BUFFER ? GR_BUFFER_FRONTBUFFER : GR_BUFFER_BACKBUFFER;

    GrLfbInfo_t info;
    info.size = sizeof(GrLfbInfo_t);

    if (!grLfbLock(gr_access,
                   gr_buf_t,
                   GR_LFBWRITEMODE_565,
                   GR_ORIGIN_UPPER_LEFT,
                   FXFALSE,
                   &info)) 
      return 0;


    gr_buf[type].im->bpl=info.strideInBytes;
    gr_buf[type].im->data=info.lfbPtr;
    gr_buf[type].gr_lock_type=gr_access;
    gr_buf[type].state=access;
  }

  return gr_buf[type].im;
}

void i4_glide_display_class::unlock_frame_buffer(i4_frame_buffer_type type)
{
  if (gr_buf[type].state==-1)  // already unlocked?
    return;    

  int gr_buf_t = type == I4_FRONT_FRAME_BUFFER ? GR_BUFFER_FRONTBUFFER : GR_BUFFER_BACKBUFFER;
 
  grLfbUnlock(gr_buf[type].gr_lock_type, gr_buf_t);
  gr_buf[type].im->data=0;
  gr_buf[type].state=-1;
}


i4_bool procs_loaded = i4_F;

#ifdef _WINDOWS
void FreeTheGlideDll();
FxBool GetProcAddresses();
#else
void FreeTheGlideDll() { ; }
FxBool GetProcAddresses() { return 1; }
#endif

void i4_glide_display_class::init()
{
  if (procs_loaded)
  {
    GrHwConfiguration config;    
    grSstQueryBoards(&config);

    if (config.num_sst>0)
    {
      me.add_to_list("3dfx Native", 0, this, i4_display_list);
    }
  }
}


i4_glide_display_class::i4_glide_display_class()
{
  i4_glide_display=this;
  mcursor=0;
  mouse_save1=0;
  mouse_save2=0;
  prev_mouse_save=0;
  procs_loaded = GetProcAddresses();
  last_mouse_x=last_mouse_y=40;
}

i4_bool i4_glide_display_class::close()
{
  next_frame_copy.delete_list();

  if (context)
    delete context;
  context=0;

  for (int i=0; i<2; i++)
    if (gr_buf[i].im)
    {
      delete gr_buf[i].im;
      gr_buf[i].im=0;
    }

  if (fake_screen)
  {
    delete fake_screen;
    fake_screen=0;
  }
  
  if (grSstControl)
    grSstControl(GR_CONTROL_DEACTIVATE);

  if (grSstWinClose)
    grSstWinClose();

  if (grGlideShutdown)
    grGlideShutdown();
  
  destroy_window();

  return i4_T;
}


static GrFog_t    fogtable[GR_FOG_TABLE_SIZE];

i4_bool i4_glide_display_class::initialize_mode()
{
  if (!create_window())
    return i4_F;  

  grGlideInit();
  GrHwConfiguration config;    
  grSstQueryHardware(&config);
  if (config.num_sst<=0)
  {
    i4_error("Couldnt find your 3dfx board");
  }
  grSstSelect( 0 );
 
  grSstWinOpen(window_handle(),
               tmp.glide_mode,
               GR_REFRESH_60Hz,
               GR_COLORFORMAT_ARGB,
               GR_ORIGIN_UPPER_LEFT,
               2, 1 );

  grDepthBufferMode( GR_DEPTHBUFFER_WBUFFER );
  grDepthBufferFunction( GR_CMP_LEQUAL);
  grDepthMask( FXTRUE );
  grCullMode(GR_CULL_DISABLE);
  //  grCullMode( GR_CULL_POSITIVE );
  grTexMipMapMode(GR_TMU0, GR_MIPMAP_DISABLE, FXFALSE);
  grHints(GR_HINT_STWHINT, GR_STWHINT_W_DIFF_TMU0 |
          GR_STWHINT_ST_DIFF_TMU0);

  guColorCombineFunction( GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB );
  grTexCombineFunction(GR_TMU0, GR_TEXTURECOMBINE_DECAL);
  grAlphaCombine(GR_COMBINE_FUNCTION_BLEND_OTHER,GR_COMBINE_FACTOR_ONE,
                 GR_COMBINE_LOCAL_NONE,GR_COMBINE_OTHER_CONSTANT,0);
  grAlphaBlendFunction(GR_BLEND_ONE,GR_BLEND_ZERO,GR_BLEND_ZERO,GR_BLEND_ZERO);

  //  grTexClampMode(GR_TMU0,GR_TEXTURECLAMP_WRAP,GR_TEXTURECLAMP_WRAP);


  //grGlideShamelessPlug(1);
 // setup fog
  //grFogMode( GR_FOG_WITH_TABLE );
	//grFogColorValue( 0x00666666 );
	w32 i;
  w32 fog_density=0;
  for (i=0;i<64;i++)
  {
    fog_density += 4;
    if (fog_density>255) fog_density=255;
    fogtable[i] = (w8)fog_density;
  }
  //guFogGenerateExp(fogtable,0.001f);
	//grFogTable( fogtable );	

  i4_pixel_format fmt;
  fmt.pixel_depth=I4_16BIT;
  fmt.red_mask=tmp.red_mask;
  fmt.green_mask=tmp.green_mask;
  fmt.blue_mask=tmp.blue_mask;
  fmt.alpha_mask=0;
  fmt.calc_shift();

  pal = i4_pal_man.register_pal(&fmt);

  fake_screen=i4_create_image(tmp.xres, tmp.yres, pal);
  gr_buf[0].im=i4_create_image(tmp.xres, tmp.yres, pal, 0, 0);
  gr_buf[0].state=-1;

  gr_buf[1].im=i4_create_image(tmp.xres, tmp.yres, pal, 0, 0);
  gr_buf[1].state=-1;

    
  context=new i4_draw_context_class(0,0,tmp.xres-1,tmp.yres-1);
  context->both_dirty=new i4_rect_list_class;

  memcpy(&cur_mode, &tmp, sizeof(cur_mode));  
  
  return i4_T;
}

void i4_glide_display_class::remove_cursor(sw32 x, sw32 y, i4_image_class *mouse_save)
{
  if (mouse_save && mcursor)
  {
    i4_draw_context_class no_clip_context(0,0, width()-1, height()-1);      

    pf_glide_flush_lock_write.start();    
    i4_image_class *draw_to=lock_frame_buffer(I4_BACK_FRAME_BUFFER, I4_FRAME_BUFFER_WRITE);
    pf_glide_flush_lock_write.stop();

    int w=mouse_save->w, h=mouse_save->h;
    ((i4_image16 *)mouse_save)->put_part_one_pixel_at_a_time(draw_to,
                                                             x,y,0,0,w-1,h-1, no_clip_context);
    unlock_frame_buffer(I4_BACK_FRAME_BUFFER);
  }
}

void i4_glide_display_class::save_and_draw_cursor(sw32 x, sw32 y, i4_image_class *&mouse_save)
{
  if (mcursor)
  {
    pf_glide_flush_mouse_save.start();

    if (mouse_save && (mouse_save->width()!=mcursor->pict->width() ||
                       mouse_save->height()!=mcursor->pict->height()))
    {
      delete mouse_save;
      mouse_save=i4_create_image(mcursor->pict->width(), mcursor->pict->height(), pal);
    }
    else if (!mouse_save)
      mouse_save=i4_create_image(mcursor->pict->width(), mcursor->pict->height(), pal);

    i4_draw_context_class no_clip_context(0,0, width()-1, height()-1);
    sw32 x1=x, y1=y;
    sw32 x2=x1+mcursor->pict->width()-1, y2=y1+mcursor->pict->height()-1;


    if (x2 > width()-1)
      x2=width()-1;
    if (y2 > height()-1)
      y2=height()-1;
    if (x<width()-1 && y<height()-1)
    {
      i4_image16 *im=(i4_image16 *)lock_frame_buffer(I4_BACK_FRAME_BUFFER, I4_FRAME_BUFFER_READ);
      im->put_part_one_pixel_at_a_time(mouse_save, 0,0, x1,y1,x2,y2, no_clip_context);
      unlock_frame_buffer(I4_BACK_FRAME_BUFFER);
    }
           
    i4_image16 *dest=(i4_image16 *)lock_frame_buffer(I4_BACK_FRAME_BUFFER, I4_FRAME_BUFFER_WRITE);
    mcursor->pict->put_image_trans(dest,  x1,y1, mcursor->trans, no_clip_context);
    unlock_frame_buffer(I4_BACK_FRAME_BUFFER);

    pf_glide_flush_mouse_save.stop();
  }
}

void i4_glide_display_class::flush()
{
  i4_rect_list_class::area_iter a;

  sw32 mouse_x, mouse_y;
  get_mouse_pos(mouse_x, mouse_y);


  context->both_dirty->intersect_area(0,0,width()-1,height()-1);
  w16 *middle_buffer=(w16 *)fake_screen->data;

  pf_glide_flush_lock_write.start();
  i4_image_class *bbuf=lock_frame_buffer(I4_BACK_FRAME_BUFFER, I4_FRAME_BUFFER_WRITE);
  int bpl=bbuf->bpl;
  void *bptr = bbuf->data;
  pf_glide_flush_lock_write.stop();

  pf_glide_flush_copy.start();
  a=context->both_dirty->list.begin();
  for (;a!=context->both_dirty->list.end();++a)
    next_frame_copy.add_area(a->x1, a->y1, a->x2, a->y2);


  a=next_frame_copy.list.begin();
  for (;a!=context->both_dirty->list.end();++a)
  {
    for (w32 y=a->y1; y<=a->y2; y++)
    {
      w32 sx=a->x1, ex=(a->x2);

      w16 *src=(w16 *)(((w16 *)middle_buffer)+y*cur_mode.xres + sx);
      w16 *dst=(w16 *)(((w8 *)bptr)+y*bpl + sx*2);

      while (sx<=ex)
      {
        *(dst++)=*(src++);
        sx++;
      }
    }    
  }  

  next_frame_copy.swap(context->both_dirty);
  context->both_dirty->delete_list();

  pf_glide_flush_copy.stop();


  pf_glide_flush_mouse.start();

  i4_image_class **cur_save;
  // if this is the first time to save, or last save wave to mouse_save2 then use 1
  if (!prev_mouse_save || prev_mouse_save==mouse_save2)
    cur_save=&mouse_save1;
  else
    cur_save=&mouse_save2;

  save_and_draw_cursor(mouse_x - (sw32)mcursor->hot_x, 
                       mouse_y - (sw32)mcursor->hot_y, *cur_save);


  pf_glide_flush_mouse.stop();

  unlock_frame_buffer(I4_BACK_FRAME_BUFFER);
  
  pf_glide_flush_flip.start();
  //  while (grBufferNumPending());
  grBufferSwap(0);
  pf_glide_flush_flip.stop();

  pf_glide_flush_mouse.start();
  if (prev_mouse_save)
    remove_cursor(last_mouse_x, last_mouse_y, prev_mouse_save);
  pf_glide_flush_mouse.stop();

  prev_mouse_save=*cur_save;
  last_mouse_x=mouse_x - (sw32)mcursor->hot_x;
  last_mouse_y=mouse_y - (sw32)mcursor->hot_y;
}


i4_image_class *i4_glide_display_class::get_screen()
{      
  return fake_screen;
}

i4_bool i4_glide_display_class::set_mouse_shape(i4_cursor_class *cursor)
{
  if (mcursor)
    delete mcursor;
  
  mcursor=cursor->copy(get_palette());
  
  return i4_T;
}

void i4_glide_display_class::uninit()
{
  if (mcursor)
    delete mcursor;
  if (mouse_save1)
    delete mouse_save1;
  if (mouse_save2)
    delete mouse_save2;

  mouse_save1 = 0;
  mouse_save2 = 0;
  mcursor = 0;
}

i4_glide_display_class::~i4_glide_display_class()
{
  FreeTheGlideDll();
  procs_loaded = i4_F;
}

i4_display_class::mode *i4_glide_display_class::get_first_mode(int driver_id) 
{ 
  tmp.glide_mode=GR_RESOLUTION_320x200;
  tmp.xres=320;
  tmp.yres=200;
  tmp.flags=mode::PAGE_FLIPPED;
  tmp.red_mask=31<<11;
  tmp.green_mask=63<<5;
  tmp.blue_mask=31;
  strcpy(tmp.name, "320 X 200 16bit");
  return &tmp;
}

i4_display_class::mode *i4_glide_display_class::get_next_mode() 
{
  switch (tmp.glide_mode)
  {
    case GR_RESOLUTION_320x200 :
      tmp.glide_mode=GR_RESOLUTION_320x240;
      tmp.yres=240;
      break;
      
    case GR_RESOLUTION_320x240 :
      tmp.glide_mode=GR_RESOLUTION_400x256;
      tmp.xres=400; tmp.yres=256;
      break;

    case GR_RESOLUTION_400x256 :
      tmp.glide_mode=GR_RESOLUTION_512x384;
      tmp.xres=512; tmp.yres=384;
      break;


    case GR_RESOLUTION_512x384 :
      tmp.glide_mode=GR_RESOLUTION_640x200;
      tmp.xres=640; tmp.yres=200;
      break;

    case GR_RESOLUTION_640x200 :
      tmp.glide_mode=GR_RESOLUTION_640x350;
      tmp.xres=640; tmp.yres=350;
      break;

    case GR_RESOLUTION_640x350 :
      tmp.glide_mode=GR_RESOLUTION_640x400;
      tmp.xres=640; tmp.yres=400;
      break;

    case GR_RESOLUTION_640x400 :
      tmp.glide_mode=GR_RESOLUTION_640x480;
      tmp.xres=640; tmp.yres=480;
      break;

    case GR_RESOLUTION_640x480 :
      tmp.glide_mode=GR_RESOLUTION_800x600;
      tmp.xres=800; tmp.yres=600;
      break;

    case GR_RESOLUTION_800x600 :
      tmp.glide_mode=GR_RESOLUTION_960x720;
      tmp.xres=960; tmp.yres=720;
      break;

    case GR_RESOLUTION_960x720 :
      tmp.glide_mode=GR_RESOLUTION_856x480;
      tmp.xres=856; tmp.yres=480;
      break;

    case GR_RESOLUTION_856x480 :
      tmp.glide_mode=GR_RESOLUTION_512x256;
      tmp.xres=512; tmp.yres=256;
      break;


    case GR_RESOLUTION_512x256 :
      return 0;

  }

  sprintf(tmp.name, "%d X %d 16bit", tmp.xres, tmp.yres);

  return &tmp;
}

