/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include <sioux.h>
#include <LowMem.h>
#include "draw_sprocket.hh"
#include "device/event.hh"
#include "device/keys.hh"

enum mac_modnum 
{
  MAC_COMMAND = 0x37,
  MAC_LSHIFT,
  MAC_CAPSLOCK,
  MAC_LOPTION,
  MAC_LCONTROL,
  MAC_RSHIFT,
  MAC_ROPTION,
  MAC_RCONTROL,
  MAC_LAST
};

const int mac_map[8] = 
{
  I4_COMMAND,
  I4_SHIFT_L,
  I4_CAPS,
  I4_ALT_L,
  I4_CTRL_L,
  I4_SHIFT_R,
  I4_ALT_R,
  I4_CTRL_R,
};

// save area for old event mask
short OldEventMask;

// new event mask for i4 system
const short MACEVENTS = mUpMask | mDownMask | keyUpMask | keyDownMask | autoKeyMask;

draw_sprocket_class draw_sprocket;


w32 draw_sprocket_class::device_manager_class::request_device(i4_event_handler_class *for_who, 
                                                              w32 event_types)
//{{{
{
  if ((event_types&i4_device_class::FLAG_MOUSE_BUTTON_UP) || 
      (event_types&i4_device_class::FLAG_MOUSE_BUTTON_DOWN) ||
      (event_types&i4_device_class::FLAG_MOUSE_MOVE) ||
      (event_types&i4_device_class::FLAG_KEY_PRESS) ||
      (event_types&i4_device_class::FLAG_KEY_RELEASE) ||
      (event_types&i4_device_class::FLAG_DISPLAY_CLOSE))
  {
    if (event_types&i4_device_class::FLAG_KEY_PRESS)
      add_agent(for_who,i4_device_class::FLAG_KEY_PRESS);

    if (event_types&i4_device_class::FLAG_KEY_RELEASE)
      add_agent(for_who,i4_device_class::FLAG_KEY_RELEASE);

    if (event_types&i4_device_class::FLAG_MOUSE_BUTTON_UP)
      add_agent(for_who,i4_device_class::FLAG_MOUSE_BUTTON_UP);

    if (event_types&i4_device_class::FLAG_MOUSE_BUTTON_DOWN)
      add_agent(for_who,i4_device_class::FLAG_MOUSE_BUTTON_DOWN);
    
    if (event_types&i4_device_class::FLAG_MOUSE_MOVE)
      add_agent(for_who,i4_device_class::FLAG_MOUSE_MOVE);

    if (event_types&i4_device_class::FLAG_DISPLAY_CLOSE)
      add_agent(for_who,i4_device_class::FLAG_DISPLAY_CLOSE);
    
    return (i4_device_class::FLAG_KEY_PRESS|
            i4_device_class::FLAG_KEY_RELEASE|
            i4_device_class::FLAG_MOUSE_BUTTON_UP|
            i4_device_class::FLAG_MOUSE_BUTTON_DOWN|
            i4_device_class::FLAG_MOUSE_MOVE|
            i4_device_class::FLAG_DISPLAY_CLOSE)&event_types;
  }
  return 0;
}
//}}}


void draw_sprocket_class::device_manager_class::release_device(i4_event_handler_class *for_who, 
                                                               w32 event_types)
//{{{
{
  if (event_types&i4_device_class::FLAG_KEY_PRESS)
    remove_agent(for_who,i4_device_class::FLAG_KEY_PRESS);

  if (event_types&i4_device_class::FLAG_KEY_RELEASE)
    remove_agent(for_who,i4_device_class::FLAG_KEY_RELEASE);

  if (event_types&i4_device_class::FLAG_MOUSE_BUTTON_UP)
    remove_agent(for_who,i4_device_class::FLAG_MOUSE_BUTTON_UP);

  if (event_types&i4_device_class::FLAG_MOUSE_BUTTON_DOWN)
    remove_agent(for_who,i4_device_class::FLAG_MOUSE_BUTTON_DOWN);
  
  if (event_types&i4_device_class::FLAG_MOUSE_MOVE)
    remove_agent(for_who,i4_device_class::FLAG_MOUSE_MOVE);

  if (event_types&i4_device_class::FLAG_DISPLAY_CLOSE)
    remove_agent(for_who,i4_device_class::FLAG_DISPLAY_CLOSE);
}
//}}}


i4_bool draw_sprocket_class::device_manager_class::process_events()
//{{{
{
  i4_bool ret = i4_F;

  Point p;

  // Update mouse position
  omx = mx;
  omy = my;
  ::GetMouse(&p);
  mx = p.h;
  my = p.v;
  if (mx!=omx || my!=omy)
  {
    i4_mouse_move_event_class ev(mx,my);
    send_event_to_agents(&ev,i4_device_class::FLAG_MOUSE_MOVE);
    ret = i4_T;
  }

#if 0
  if (Button())
  {
    i4_display_close_event_class quit(&draw_sprocket);
    send_event_to_agents(&quit,FLAG_DISPLAY_CLOSE);

    return i4_T;
  }
  return i4_F;
#endif

  // get mac modifiers
  unsigned char km[16];
  int i;

  GetKeys((unsigned long*)&km);
  for (i=0; i<MAC_LAST-MAC_COMMAND; i++)
  {
    char last = mac_modifier[i];
    mac_modifier[i] = (km[ (i+MAC_COMMAND) >> 3 ] >> ((i+MAC_COMMAND) & 7)) & 1;
    if (mac_modifier[i] != last)
    {
      w16 key, key_code;

      key_code = mac_map[i];
      if (mac_modifier[i])
      {
        key = i4_key_translate(key_code,1,shift_state);
        i4_key_press_event_class ev(key, key_code);
        send_event_to_agents(&ev,FLAG_KEY_PRESS);    
        ret = i4_T;
      }
      else 
      {
        key = i4_key_translate(key_code,0,shift_state);
        i4_key_release_event_class ev(key, key_code);
        send_event_to_agents(&ev,FLAG_KEY_RELEASE);    
        ret = i4_T;
      }
    }
  }

  EventRecord MacEv;
  
  // process mac events
  while (GetOSEvent(MACEVENTS,&MacEv))
  {
    switch (MacEv.what)
    {
      case keyDown :
      case keyUp :
      case autoKey :
      {
        // process key events

        w16 key, key_code;

        switch ((MacEv.message & keyCodeMask)>>8)
        {
          case 0x7d:   key_code=I4_DOWN; break;
          case 0x7e:   key_code=I4_UP;  break;
          case 0x7b:   key_code=I4_LEFT;  break;
          case 0x7c:   key_code=I4_RIGHT;  break;
          case 0x47:   key_code=I4_NUM_LOCK;  break;
          case 0x73:   key_code=I4_HOME;  break;
          case 0x77:   key_code=I4_END;  break;
          case 0x33:   key_code=I4_BACKSPACE;  break;
          case 0x30:   key_code=I4_TAB;  break;
          case 0x24:   key_code=I4_ENTER;  break;
          case 0x39:   key_code=I4_CAPS;  break;
          case 0x35:   key_code=I4_ESC;  break;
          case 0x7a:   key_code=I4_F1; break;
          case 0x78:   key_code=I4_F2;break;
          case 0x63:   key_code=I4_F3; break;
          case 0x76:   key_code=I4_F4; break;
          case 0x60:   key_code=I4_F5; break;
          case 0x61:   key_code=I4_F6; break;
          case 0x62:   key_code=I4_F7; break;
          case 0x64:   key_code=I4_F8; break;
          case 0x65:   key_code=I4_F9; break;
          case 0x6d:   key_code=I4_F10; break;
          case 0x67:   key_code=I4_F11; break;
          case 0x6f:   key_code=I4_F12; break;
          case 0x69:   key_code=I4_F13; break;
          case 0x6b:   key_code=I4_F14; break;
          case 0x71:   key_code=I4_F15; break;
          case 0x72:   key_code=I4_INSERT; break;
          case 0x75:   key_code=I4_DEL; break;
          case 0x74:   key_code=I4_PAGEUP; break;
          case 0x79:   key_code=I4_PAGEDOWN; break;
          
          case 0x52:   key_code=I4_KP0; break;
          case 0x53:   key_code=I4_KP1; break;
          case 0x54:   key_code=I4_KP2; break;
          case 0x55:   key_code=I4_KP3; break;
          case 0x56:   key_code=I4_KP4; break;
          case 0x57:   key_code=I4_KP5; break;
          case 0x58:   key_code=I4_KP6; break;
          case 0x59:   key_code=I4_KP7; break;
          case 0x5b:   key_code=I4_KP8; break;
          case 0x5c:   key_code=I4_KP9; break;
          case 0x51:   key_code=I4_KPEQUAL; break;
          case 0x4b:   key_code=I4_KPSLASH; break;
          case 0x43:   key_code=I4_KPSTAR; break;
          case 0x4e:   key_code=I4_KPMINUS; break;
          case 0x45:   key_code=I4_KPPLUS; break;
          case 0x4c:   key_code=I4_KPENTER; break;
          case 0x41:   key_code=I4_KPPERIOD; break;
          
          case 0x00:   key_code='A'; break;
          case 0x0b:   key_code='B'; break;
          case 0x08:   key_code='C'; break;
          case 0x02:   key_code='D'; break;
          case 0x0e:   key_code='E'; break;
          case 0x03:   key_code='F'; break;
          case 0x05:   key_code='G'; break;
          case 0x04:   key_code='H'; break;
          case 0x22:   key_code='I'; break;
          case 0x26:   key_code='J'; break;
          case 0x28:   key_code='K'; break;
          case 0x25:   key_code='L'; break;
          case 0x2e:   key_code='M'; break;
          case 0x2d:   key_code='N'; break;
          case 0x1f:   key_code='O'; break;
          case 0x23:   key_code='P'; break;
          case 0x0c:   key_code='Q'; break;
          case 0x0f:   key_code='R'; break;
          case 0x01:   key_code='S'; break;
          case 0x11:   key_code='T'; break;
          case 0x20:   key_code='U'; break;
          case 0x09:   key_code='V'; break;
          case 0x0d:   key_code='W'; break;
          case 0x07:   key_code='X'; break;
          case 0x10:   key_code='Y'; break;
          case 0x06:   key_code='Z'; break;
          
          case 0x32:   key_code='`'; break;
          case 0x12:   key_code='1'; break;
          case 0x13:   key_code='2'; break;
          case 0x14:   key_code='3'; break;
          case 0x15:   key_code='4'; break;
          case 0x17:   key_code='5'; break;
          case 0x16:   key_code='6'; break;
          case 0x1a:   key_code='7'; break;
          case 0x1c:   key_code='8'; break;
          case 0x19:   key_code='9'; break;
          case 0x1d:   key_code='0'; break;
          case 0x1b:   key_code='-'; break;
          case 0x18:   key_code='='; break;

          case 0x21:   key_code='['; break;
          case 0x1e:   key_code=']'; break;
          case 0x2a:   key_code='\\'; break;
          
          case 0x29:   key_code=';'; break;
          case 0x27:   key_code='\''; break;
          
          case 0x2b:   key_code=','; break;
          case 0x2f:   key_code='.'; break;
          case 0x2c:   key_code='/'; break;
          case 0x31:   key_code=' '; break;
          
          default:     key_code = 0; break;
        }

        if (key_code)
        {
          if (MacEv.what!=keyUp)
          {
            key = i4_key_translate(key_code,1,shift_state);
            i4_key_press_event_class ev(key, key_code);
            send_event_to_agents(&ev,FLAG_KEY_PRESS);    
            ret = i4_T;
          }
          else 
          {
            key = i4_key_translate(key_code,0,shift_state);
            i4_key_release_event_class ev(key, key_code);
            send_event_to_agents(&ev,FLAG_KEY_RELEASE);    
            ret = i4_T;
          }
        }
      } break;
      
      case mouseUp :
      {
        i4_mouse_button_up_event_class down(i4_mouse_button_up_event_class::LEFT);
        send_event_to_agents(&down,FLAG_MOUSE_BUTTON_UP);
        ret = i4_T;
      } break;

      case mouseDown :
      { 
        i4_mouse_button_down_event_class down(i4_mouse_button_down_event_class::LEFT);
        send_event_to_agents(&down,FLAG_MOUSE_BUTTON_DOWN);
        ret = i4_T;
      } break;
    }
  }

  return ret;
}
//}}}


draw_sprocket_class::device_manager_class::device_manager_class()
  : shift_state(0)
//{{{
{
  int i;

  for (i=0; i<8; i++)
    mac_modifier[i] = 0;
}
//}}}


draw_sprocket_class::draw_sprocket_class()
  : i4_display_class(), context(0), screen(0)
//{{{
{
  /* tell SIOUX(stdio simulator) to shut up */
//  SIOUXSettings.autocloseonquit = true;
  SIOUXSettings.asktosaveonclose = false;

  DSpStartup();

  // open window & initialize toolbox routines to allow DrawSprockets to work?
  printf("Draw Sprocket Initialization\n");

  // save & set event mask
  OldEventMask = LMGetSysEvtMask();
  SetEventMask(MACEVENTS);
}
//}}}


draw_sprocket_class::~draw_sprocket_class()
//{{{
{
  // restore event mask
  SetEventMask(OldEventMask);

  DSpShutdown();
}
//}}}


draw_sprocket_class::draw_sprocket_mode_class 
*draw_sprocket_class::new_mode(DSpContextReference context)
//  create new video mode info struct from draw sprocket context
//{{{
{
  DSpContextAttributes attr;
  OSErr err;
  
  err = DSpContext_GetAttributes( context, &attr );
  
  if (err != noErr)
    return 0;

  if (attr.displayBestDepth != (8*I4_BYTES_PER_PIXEL))
    return 0;

  draw_sprocket_mode_class *m = new draw_sprocket_mode_class(context);
  
  m->xres = attr.displayWidth;
  m->yres = attr.displayHeight;
  m->bits_per_pixel = attr.displayBestDepth;
  m->bits_per_color = 8;
  m->flags = mode::PAGE_FLIPPED;
  m->name[0] = 0;

  return m;
}
//}}}


draw_sprocket_class::mode *draw_sprocket_class::get_first_mode()
//{{{
{
  return get_next_mode(0);
}
//}}}


draw_sprocket_class::mode *draw_sprocket_class::get_next_mode(mode *last_mode)
//{{{
{
  DSpContextReference cont;
  CAST_PTR(dsm, draw_sprocket_mode_class, last_mode);
  draw_sprocket_mode_class *ret;
  OSErr err;

  if (dsm == 0)
  {
    // get first screen through Display Manager

    gd = DMGetFirstScreenDevice(0);

    err = DMGetDisplayIDByGDevice( gd, &display_id, 0);
    
    if (err != noErr)
      return 0;

    // start screen enumeration through Display Manager
    err = DSpGetFirstContext( display_id, &cont );
    
    if (err != noErr)
      return 0;
  }
  else
  {
    // iterate screen enumeration through Display Manager

    err = DSpGetNextContext( dsm->ref, &cont );

    if (err != noErr)
      return 0;
  }

  while (cont && (ret = new_mode(cont))==0)
  {
    // continue to get valid modes
    err = DSpGetNextContext( cont, &cont );

    if (err != noErr)
      return 0;
  }

  return ret;
}
//}}}


void draw_sprocket_class::setup_back_buffer()
//  get memory pointer to back buffer for direct drawing & page flipping
//{{{
{
  CGrafPtr back;
  w8 *data;
  w32 rowbytes;

  // get CGrafPtr to back buffer
  DSpContext_GetBackBuffer(DSp_ref, kDSpBufferKind_Normal, &back);

  // get pixmap of port
  PMH = back->portPixMap;

  // ensure pixels stay in memory
  LockPixels(PMH);

  // get number of physical bytes per row
  rowbytes = (*PMH)->rowBytes & 0x3FFF;

  // get pointer to screen memory
  data = (w8 *)GetPixBaseAddr(PMH);

  i4_pal_handle_class pal;
//#if (I4_SCREEN_DEPTH==15)
  //pal.set_type(i4_pal_handle_class::ID_15BIT);
//#elif (I4_SCREEN_DEPTH==16)
  pal.set_type(i4_pal_handle_class::ID_16BIT);
//#else
//#error add code here
//#endif

  // setup screen with pointer into pixmap
  if (!screen)
  {
    screen=new I4_SCREEN_TYPE(wx,wy,
                              pal,
                              rowbytes,   // bytes per line
                              (w8 *)data);
  }
  else
    screen->local_set_data((w8*)data);

#if 0
  //{{{ Debugging under 16 bit to show colors

  int i,j;

  for (j=0; j<1<<(16 - 8); j++)
    for (i=0; i<1<<8; i++)
      *((w16 *)(data + (j*rowbytes) + i*2)) = (j<<8) + i;

  for (j=0; j<100; j++)
    for (i=0; i<20; i++)
    {
      *((w16 *)(data + (j*rowbytes) + (i+340)*2)) = 0x001f;
      *((w16 *)(data + (j*rowbytes) + (i+320)*2)) = 0x03e0;
      *((w16 *)(data + (j*rowbytes) + (i+300)*2)) = 0x7c00;
    }

  //}}}
#endif
}
//}}}


i4_bool draw_sprocket_class::initialize_mode(mode *m)
//{{{
{
  OSErr err;
  DSpContextAttributes attr;
  CAST_PTR(dsm, draw_sprocket_mode_class, m);

  DSp_ref = dsm->ref;

  err = DSpContext_GetAttributes( DSp_ref, &attr );
  
  if (err != noErr)
    return i4_F;

  // enable page flipping option
  attr.contextOptions |= kDSpContextOption_PageFlip;

  err = DSpContext_Reserve(DSp_ref, &attr);
  if (err != noErr)
    return i4_F;

  // fade out mac scrren
  err = DSpContext_FadeGammaOut(0, 0);
  if (err != noErr)
    return i4_F;

  // change mode
  err = DSpContext_SetState(DSp_ref, kDSpContextState_Active);
  if (err != noErr)
    return i4_F;

  // fade game in
  err = DSpContext_FadeGammaIn(0, 0);
  if (err != noErr)
    return i4_F;

  wx = attr.displayWidth;  
  wy = attr.displayHeight;

  setup_back_buffer();

  if (context)
    delete context;

  // setup context for window
  context=new i4_draw_context_class(0,0,wx-1,wy-1);
  context->both_dirty=new i4_rect_list_class;
  context->single_dirty=new i4_rect_list_class;
  
  return i4_T;
}
//}}}


i4_bool draw_sprocket_class::close()
//{{{
{
  OSErr err;

  if (PMH)
    // free pixels to Mac Heap
    UnlockPixels(PMH);

  err = DSpContext_FadeGammaOut(0, 0);

  // return to normal mode
  err = DSpContext_SetState(DSp_ref, kDSpContextState_Inactive);

  err = DSpContext_FadeGammaIn(0, 0);

  if (context)
    delete context;
  context = 0;

  if (screen)
    delete screen;
  screen = 0;
  
  return i4_T;
}
//}}}


void draw_sprocket_class::flush()
//{{{
{
  OSErr err;

  PixMapHandle PMH2 = PMH;
  w8 *data;
  w32 rowbytes;
  Rect r;

  // save off pointers to current screen
  rowbytes = (*PMH2)->rowBytes & 0x3FFF;
  data = (w8 *)GetPixBaseAddr(PMH2);

  // swap screens
  err = DSpContext_SwapBuffers(DSp_ref, 0, 0);
  setup_back_buffer();

  // copy areas that changed
  i4_rect_list_class::area_iter a=context->both_dirty->list.begin();
  for (;a!=context->both_dirty->list.end();++a)
  {
    r.left = a->x1;
    r.top = a->y1;
    r.right = a->x2+1;
    r.bottom = a->y2+1;

    CopyBits((BitMap *)*PMH2, (BitMap *)*PMH, &r, &r, srcCopy, 0);
  }

  // free memory for old screen
  if (PMH2)
    UnlockPixels(PMH2);

  context->single_dirty->delete_list();
  context->both_dirty->delete_list();
}
//}}}


i4_bool draw_sprocket_class::available()
//{{{
{
  // should do more checks
  return i4_T;
}
//}}}


i4_bool draw_sprocket_class::realize_palette(i4_pal_handle_class pal_id)
//{{{
{
  return i4_F;
}
//}}}


i4_bool draw_sprocket_class::set_mouse_shape(i4_cursor_class *cursor)
//{{{
{
  // not implemented yet
  return i4_F;
}
//}}}


//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
