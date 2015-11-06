/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#define I4 "../.."

#include "stdmak.cc"

common()
{
  exe_base = "sndtst";

  files
    + I4 "/file/file.cc"
    + I4 "/file/buf_file.cc"
    + I4 "/checksum/checksum.cc"
  
    + I4 "/memory/malloc.cc"
    + I4 "/memory/bmanage.cc"
    + I4 "/memory/growheap.cc"
  
    + I4 "/image/image.cc"
    + I4 "/image/image32.cc"
    + I4 "/loaders/load.cc"
    + I4 "/loaders/pcx_load.cc"
    + I4 "/loaders/bmp_load.cc"
    + I4 "/loaders/tga_load.cc"
    + I4 "/loaders/fli_load.cc"
    + I4 "/loaders/bmp_write.cc"
    + I4 "/loaders/wav_load.cc"
  
    + I4 "/palette/pal.cc"
    + I4 "/error/error.cc"
    + I4 "/error/alert.cc"
    + I4 "/device/keys.cc"
    + I4 "/area/rectlist.cc"
    + I4 "/video/display.cc"
  
    + I4 "/window/cursor.cc"     // needs image/load, image/image
    + I4 "/window/window.cc"     // needs window/cursor 
    + I4 "/window/wmanager.cc"
    + I4 "/window/colorwin.cc"
    + I4 "/window/dragwin.cc"
    + I4 "/window/style.cc"
    + I4 "/window/mwmstyle.cc"

    + I4 "/menu/menu.cc"
    + I4 "/menu/boxmenu.cc"
    + I4 "/menu/menuitem.cc"
    + I4 "/menu/textitem.cc"
    + I4 "/gui/button.cc"
    + I4 "/gui/butbox.cc"

    + I4 "/string/string.cc"
    + I4 "/font/plain.cc"

    + I4 "/device/device.cc"
    + I4 "/device/kernel.cc"
    + I4 "/init/init.cc"
    
    + I4 "/time/profile.cc"
    
    + I4 "/app/app.cc"
    + I4 "/quantize/median.cc"
    + I4 "/quantize/histogram.cc"

    + I4 "/sound/sound.cc"

    + "sndtst.cc"
    ;

  linux_files
    + I4 "/file/linux/linux_file.cc"
    + I4 "/file/linux/get_dir.cc"
    + I4 "/main/nrm_main.cc"
    + I4 "/video/x11/x11devs.cc"
    + I4 "/video/x11/mitshm.cc"
    + I4 "/time/unixtime.cc"
    + I4 "/time/timedev.cc"
    + I4 "/threads/linux/threads.cc"
    + I4 "/sound/linux/linux_sound.cc"
    ;

  win95_files
    + I4 "/time/win_time.cc"
    + I4 "/video/win32/win32.cc"
    + I4 "/video/win32/gdi_draw.cc"
    + I4 "/time/win32_timedev.cc"
    + I4 "/threads/win32/threads.cc"
    + I4 "/file/win32/win_file.cc"
    + I4 "/file/win32/get_dir.cc"
    + I4 "/main/win_main.cc"
    + I4 "/sound/dsound/direct_sound.cc"
    ;

  includes
    + I4
    + I4 "/inc"
    ;
}
