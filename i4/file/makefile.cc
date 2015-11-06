/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#define I4 "../../i4"

#include "stdmak.cc"

common()
{
  exe_base = "file_test";

  files 
    + I4 "/error/error.cc"
    + I4 "/memory/malloc.cc"
    + I4 "/memory/growheap.cc"
    + I4 "/init/init.cc"
    + I4 "/file/file.cc"
    + I4 "/file/buf_file.cc"
    + I4 "/string/string.cc"

    + "file_test.cc"
    ;

  linux_files 
    + I4 "/file/linux/linux_file.cc"
    + I4 "/file/linux/get_dir.cc"
    ;

  win95_files 
    + I4 "/file/win32/win_file.cc"
    + I4 "/file/win32/get_dir.cc"
    ;

  mac_files
    + I4 "/file/mac/mac_file.cc"
    + I4 "/file/mac/get_dir.cc"
    + I4 "/file/mac/dirent.cc"
    ;

  includes
    + I4 "/inc"
    + I4
    ;
}
