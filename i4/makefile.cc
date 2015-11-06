/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "nrm_make.cc"

#define I4 "."
#define GLIDE_INC "/usr/local/glide/include"

enum
{
  GOLG,
  LIBS
};


list lib_name;
list files[LAST+1][LIBS];
list includes;
list defines;

list blank;


common()
{
  lib_name 
    + "libgolgi4"
    ;

  files[LINUX] [GOLG]
    + I4 "/video/glide/glide_display.cc"
    + I4 "/video/x11/x11_input.cc"

    + I4 "/file/linux/linux_file.cc"
    + I4 "/file/linux/get_dir.cc"

    + I4 "/main/nrm_main.cc"
    
    
    + I4 "/time/unixtime.cc"
    + I4 "/time/timedev.cc"
    
    + I4 "/music/stream.cc"
    + I4 "/file/linux/unix_open.cc"
    
    + I4 "/dll/linux_dll.cc"
    ;

/* hiya */

  files[SGI]   [GOLG]
    + I4 "/video/x11/x11_display.cc"
    
    + I4 "/video/x11/x11_input.cc"
    + I4 "/video/x11/mitshm.cc"
    
    + I4 "/file/async.cc"
    + I4 "/file/linux/linux_file.cc"
    + I4 "/threads/linux/threads.cc"
    + I4 "/file/linux/get_dir.cc"
    
    + I4 "/main/nrm_main.cc"
    + I4 "/time/unixtime.cc"
    + I4 "/time/timedev.cc"
    + I4 "/music/stream.cc"
    + I4 "/file/linux/unix_open.cc"
    ;

  files[MSVC]  [GOLG]
    + I4 "/file/win32/win_file.cc"
    + I4 "/file/win32/get_dir.cc"
    + I4 "/time/win_time.cc"
    
    + I4 "/video/win32/win32_input.cc"
    + I4 "/video/win32/display_dialog.cc"
    
    + I4 "/main/win_main.cc"
    + I4 "/time/timedev.cc"
    
    + I4 "/video/win32/dx5.cc"
    + I4 "/video/win32/dx5_error.cc"
    + I4 "/video/win32/dx5_mouse.cc"
    + I4 "/video/win32/dx5_util.cc"

    + I4 "/video/glide/glide_display.cc"
    
    + I4 "/file/async.cc"
    + I4 "/threads/win32/threads.cc"
    + I4 "/music/stream.cc"
    
    + I4 "/file/win32/win_open.cc"
    
    + I4 "/dll/win32_dll.cc"
    ;

  files[COMMON][GOLG]
    + I4 "/inc/search.cc"

    + I4 "/file/file.cc"
    + I4 "/file/buf_file.cc"
    + I4 "/checksum/checksum.cc"
    
    + I4 "/memory/malloc.cc"
    + I4 "/memory/bmanage.cc"
    + I4 "/memory/growheap.cc"
    
    + I4 "/image/image.cc"
    + I4 "/image/image32.cc"
    + I4 "/loaders/load.cc"
    
    + I4 "/loaders/tga_write.cc"
    + I4 "/loaders/bmp_write.cc"
    
    + I4 "/palette/pal.cc"
    + I4 "/error/error.cc"
    + I4 "/error/alert.cc"
    + I4 "/device/keys.cc"
    + I4 "/area/rectlist.cc"
    + I4 "/video/display.cc"
    + I4 "/sound/sound.cc"
    
    + I4 "/window/cursor.cc"     // needs image/load, image/image
    + I4 "/window/window.cc"     // needs window/cursor 
    + I4 "/window/wmanager.cc"
    + I4 "/window/colorwin.cc"
    + I4 "/window/dragwin.cc"
    + I4 "/window/style.cc"
    
    + I4 "/menu/menu.cc"
    + I4 "/menu/pull.cc"
    + I4 "/menu/key_item.cc"
    
    + I4 "/menu/boxmenu.cc"
    + I4 "/menu/menuitem.cc"
    + I4 "/menu/textitem.cc"
    + I4 "/gui/button.cc"
    + I4 "/gui/butbox.cc"
    + I4 "/gui/text_scroll.cc"
    + I4 "/gui/image_win.cc"
    
    + I4 "/string/str_checksum.cc"
    + I4 "/string/string.cc"
    + I4 "/font/plain.cc"                             
    
    + I4 "/device/device.cc"
    + I4 "/device/kernel.cc"
    + I4 "/init/init.cc"
    
    + I4 "/time/profile.cc"

    + I4 "/app/app.cc"
    + I4 "/quantize/median.cc"
    + I4 "/quantize/histogram.cc"
    + I4 "/math/spline.cc"
    
    + I4 "/network/net_prot.cc"
    + I4 "/network/login.cc"      // for get username
    
    + I4 "/status/status.cc"
    
    + I4 "/status/gui_stat.cc"
    
    + I4 "/gui/scroll_bar.cc"
    + I4 "/gui/create_dialog.cc"
    + I4 "/gui/slider.cc"
    + I4 "/gui/smp_dial.cc"
    + I4 "/gui/text_input.cc"
    + I4 "/time/gui_prof.cc"      // to show profiling

    + I4 "/loaders/jpg/jccoefct.cc"  
    + I4 "/loaders/jpg/jcmaster.cc"
    + I4 "/loaders/jpg/jdapistd.cc"  
    + I4 "/loaders/jpg/jdhuff.cc"       
    + I4 "/loaders/jpg/jdmerge.cc"   
    + I4 "/loaders/jpg/jfdctflt.cc"     
    + I4 "/loaders/jpg/jidctint.cc"  
    + I4 "/loaders/jpg/jquant2.cc"
    + I4 "/loaders/jpg/jccolor.cc"   
    + I4 "/loaders/jpg/jcomapi.cc"      
    + I4 "/loaders/jpg/jdatasrc.cc"  
    + I4 "/loaders/jpg/jdinput.cc"      
    + I4 "/loaders/jpg/jdphuff.cc"   
    + I4 "/loaders/jpg/jfdctfst.cc"     
    + I4 "/loaders/jpg/jidctred.cc"  
    + I4 "/loaders/jpg/jutils.cc"
    + I4 "/loaders/jpg/jcdctmgr.cc"  
    + I4 "/loaders/jpg/jcparam.cc"      
    + I4 "/loaders/jpg/jdcoefct.cc"  
    + I4 "/loaders/jpg/jdmainct.cc"     
    + I4 "/loaders/jpg/jdpostct.cc"  
    + I4 "/loaders/jpg/jfdctint.cc"     
    + I4 "/loaders/jpg/jmemmgr.cc"   
    + I4 "/loaders/jpg/wrtarga.cc"
    + I4 "/loaders/jpg/jchuff.cc"    
    + I4 "/loaders/jpg/jcphuff.cc"      
    + I4 "/loaders/jpg/jdcolor.cc"   
    + I4 "/loaders/jpg/jdmarker.cc"     
    + I4 "/loaders/jpg/jdsample.cc"  
    + I4 "/loaders/jpg/jidctflt.cc"     
    + I4 "/loaders/jpg/jmemnobs.cc"
    + I4 "/loaders/jpg/jcmarker.cc"  
    + I4 "/loaders/jpg/jdapimin.cc"     
    + I4 "/loaders/jpg/jddctmgr.cc"  
    + I4 "/loaders/jpg/jdmaster.cc"     
    + I4 "/loaders/jpg/jerror.cc"    
    + I4 "/loaders/jpg/jidctfst.cc"     
    + I4 "/loaders/jpg/jquant1.cc"

    ;

  includes
    + I4 "/inc"
    + I4
    + GLIDE_INC
    ;
}

void main(int argc, char **argv)
//{{{
{
  list plat_exe[LAST], plat_includes[LAST];

  list exe_common, inc_common;

  char **str;

  common();
 
  plat_includes[MSVC] 
    + includes
    + "c:/glide/win32/include"
    + "$(MSDEV)/include";

  plat_includes[LINUX] + includes;
  plat_includes[SGI] + includes;

  
  for (int i=0; i<LAST; i++)
  {    
    target_list t;

    for (int l=0; l<LIBS; l++)
    {
      list src;

      src += files[i][l] + files[COMMON][l];
      t += new target_t(src, plat_includes[i], blank, 
			lib_name[l], MK_DEBUG | MK_LIB, 16);

      t += new target_t(src, plat_includes[i], blank, 
			lib_name[l], MK_OPT | MK_LIB, 16);
    }

    if (i==MSVC && (argc<=1 || strcmp(argv[1], "win95")==0))
      make_win95(t);

    if ((i==LINUX && (argc<=1 || strcmp(argv[1], "linux")==0)) ||
	(i==SGI && (argc<=1 || strcmp(argv[1], "sgi")==0)))
      make_unix(t,i);
  }
}
//}}}


//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
