/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "nrm_make.cc"

#include <unistd.h>

#define I4 "../i4"
#define G1 "../golg"
#define R1 "../render"

main(int argc, char **argv)
{
  list src, inc, libs;

  src 
    + I4 "/loaders/dir_load.cc"
    + I4 "/loaders/dir_save.cc"
    + I4 "/checksum/checksum.cc"

    + I4 "/file/file.cc"
    + I4 "/file/win32/win_file.cc"
    + I4 "/file/async.cc"
    + I4 "/init/init.cc"
    + I4 "/memory/malloc.cc"
    + I4 "/memory/bmanage.cc"
    + I4 "/memory/growheap.cc"
    + I4 "/file/win32/get_dir.cc"
    + I4 "/file/buf_file.cc"
    + I4 "/string/string.cc"
    + I4 "/error/error.cc"
    + I4 "/threads/win32/threads.cc"
    + I4 "/time/win_time.cc"
    + I4 "/time/profile.cc"
    + "max_object.cc"
    + "maxcomm.cc"
    + "util.cc"
    + "crkutil.cc";

  inc
    + I4 "/inc"
    + I4
    + G1
    + R1
    + "sdk_inc";

  libs
    + "$(MSDEV)/lib/kernel32.lib" 
    + "$(MSDEV)/lib/user32.lib" 
    + "$(MSDEV)/lib/gdi32.lib"
    + "$(MSDEV)/lib/comdlg32.lib"
    + "$(MSDEV)/lib/oldnames.lib"
    + "$(MSDEV)/lib/comctl32.lib"
    + "$(MSDEV)/lib/msvcrt.lib"
    + "$(MSDEV)/lib/winmm.lib"
    + "c:/maxsdk/lib/GEOM.LIB"
    + "c:/maxsdk/lib/UTIL.LIB"
    + "c:/maxsdk/lib/CORE.LIB";

  target_list t;

  t
    + target_t(src, inc, libs, "p1", MK_DEBUG | MK_DLL, 16)
    + target_t(src, inc, libs, "p1", MK_OPT   | MK_DLL, 16)
    ;

  t[0]->def_file=t[1]->def_file="plugin.def";
  t[0]->res + "plugin.rc";
  t[1]->res + "plugin.rc";

  make_win95(t);
}

