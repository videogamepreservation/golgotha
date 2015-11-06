/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#define I4 ".."
#define G1 "../../golg"

char *saturn_files[]= {0};

char *msvc_files[]= 
{
  I4 "/main/win_main.cc",
  I4 "/file/win32/get_dir.cc",

  0
};


char *linux_files[]= 
{
  I4  "/main/nrm_main.cc",
  I4 "/file/linux/get_dir.cc",
  I4 "/file/linux/linux_file.cc",
  0
} ;


char *common_files[]= 
{
  
  I4 "/file/file.cc",
  I4 "/file/buf_file.cc",
  I4 "/checksum/checksum.cc",

  I4 "/memory/bmanage.cc",
  I4 "/memory/malloc.cc",
  I4 "/memory/growheap.cc",
  
  I4 "/error/error.cc",
  I4 "/error/alert.cc",

  I4 "/string/string.cc",
  I4 "/init/init.cc",

  I4 "/time/profile.cc",

  "resource_save.cc",
  G1 "/drive_map.cc",
  0
} ;

char *includes[]= { I4 "/inc",
                    I4,
                    G1,
                    0 };

char *exe_base="resource_save";
