/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#define I4 "../i4"
#define G1 "../golg"

#include "stdmak.cc"

common()
{
  exe_base = "cd_maker";

  files
    + I4 "/file/file.cc"
    + I4 "/file/buf_file.cc"
    + I4 "/checksum/checksum.cc"
  
    + I4 "/memory/malloc.cc"
    + I4 "/memory/bmanage.cc"
    + I4 "/memory/growheap.cc"
  
    + I4 "/error/error.cc"
    + I4 "/error/alert.cc"
  
    + I4 "/string/string.cc"

    + I4 "/init/init.cc"
    
    + I4 "/time/profile.cc"
    
    + G1 "/drive_map.cc"
    + G1 "/g1_file.cc"

    + "cd_maker.cc"
    ;

  linux_files
    + I4 "/file/linux/get_dir.cc"
    + I4 "/file/linux/linux_file.cc"
    + I4 "/main/nrm_main.cc"
    ;

  includes
    + I4
    + I4 "/inc"
    + G1
    ;
}
