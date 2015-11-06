/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "stdmak.cc"
#define I4 "../.."

list files[10],linux_files[10],win95_files;
list linux_libs,win95_libs,mac_libs,libs;
list objs, cleans;
list exename,exeopt;
list pretext;

list includes;
list defines;
list exe_base,dll;

common()
{
  exe_base = "dll_test";
  dll
    + "test1.dll"
    + "test2.dll"
    ;

  linux_files[0]
    + I4 "/dll/linux_dll.cc"
    ;

  win95_files
    + I4 "/dll/win32_dll.cc"
    ;

  includes
    + I4
    + I4 "/inc"
    ;

  files[0]
    + "test.cc"
    ;

  files[1]
    + "test1.cc"
    ;

  files[2]
    + "test2.cc"
    ;
}

void make_linux()
//{{{
{
  list targets;

  for (i=0; i<dll.size()+1; i++)
    linux_files[i] += files[i];

  exename = form("$(LTMP)/%s",exe_base[0]);

  targets
    + exename
    + dll
    ;

  linux_libs
    "dl"
    ;

  // linux
  linux.cc.includes
    + includes
    ;

  linux.cc.defines
    + defines
    ;

  linux.cc_opt.includes = linux.cc.includes;
  linux.cc_opt.defines = linux.cc.defines;

  linux.link.libs
    + linux_libs
    ;

  linux.linux.

  use_unix_files();
  makefile("Makefile");
  echo(pretext);
  echo(config);
  linux.start();
  target("all", targets);

  linux.cc(linux_files[0],objs);
  linux.link(objs,exename[0]);
  cleans += objs;
  linux.clean(cleans);
}
//}}}


void make_win95(int bit_depth)
//{{{
{
  win95_files += files;

  win95_libs
    + "$(MSDEV)/lib/kernel32.lib" 
    + "$(MSDEV)/lib/user32.lib" 
    + "$(MSDEV)/lib/gdi32.lib"
    + "$(MSDEV)/lib/comdlg32.lib"
    + "$(MSDEV)/lib/oldnames.lib"
    + "$(MSDEV)/lib/msvcrt.lib"
    + "$(MSDEV)/lib/winmm.lib"
    + "$(MSDEV)/lib/dsound.lib"
    + "$(MSDEV)/lib/d3drm.lib"
    + "$(MSDEV)/lib/ddraw.lib"
    ;

  // dos
  exename = form("$(DOSTMP)/%s_%d.exe",exe_base[0],bit_depth);
  exeopt =  form("$(DOSTMP)/%so_%d.exe",exe_base[0],bit_depth);

  win95.cc.win95_includes
    + "$(MSDEV)/include"
    ;

  win95.cc_opt.win95_includes
    + "$(MSDEV)/include"
    ;

  win95.cc.includes
    + includes
    ;

  win95.cc_opt.includes
    + includes
    ;

  win95.cc.defines
    + defines
    + form("I4_SCREEN_DEPTH=%d",bit_depth)
    + "WIN32"
    + "DEBUG"
    + "_WINDOWS"
    ;

  win95.cc_opt.defines
    + defines
    + form("I4_SCREEN_DEPTH=%d",bit_depth)
    + "WIN32"
    + "_WIN32"
    + "_WINDOWS"
    ;

  win95.link.libs
    + win95_libs
    ;

  use_dos_files();
  makefile(form("makefile_%d.mak",bit_depth));
  echo(pretext);
  echo(config);
  win95.start();
  target("all", exename);
  target("opt", exeopt);
  win95.cc(win95_files,objs);
  win95.cc_opt(win95_files,opt_objs);
  win95.link(objs,exename);
  win95.link(opt_objs,exeopt);
  win95.clean(objs + opt_objs);
}
//}}}


void make_saturn()
//{{{
{
  exename = form("$(DOSTMP)/%s",exe_base);
  exeopt =  form("$(DOSTMP)/%so",exe_base);

  saturn_files += files;

  saturn.cc.defines 
    + "I4_SCREEN_DEPTH=15"
    ;

  saturn.cc.includes + includes;

  //  generate Saturn dependencies

  use_saturn_files();
  makefile("makefile.sat");
  echo(pretext);
  echo(config);
  saturn.start();

  target("all", exename);
  //  target("opt", exeopt);

  saturn.cc(saturn_files,objs);
  saturn.link(objs,exename);

  //  saturn.cc_opt(saturn_filelist,opt_objs);
  //  saturn.link(opt_objs,exeopt);
  opt_objs.clear();

  saturn.clean(objs + opt_objs);
}
//}}}


void make_mac()
//{{{
{
  list targets;

  mac_files += files;

  exename = form("\"%s.mac\"",exe_base[0]);
  exeopt = form("\"%so.mac\"",exe_base[0]);

  targets
    + exename
    ;

  libs.clear();
  libs
    + "{SharedLibraries}InterfaceLib"
    + "{SharedLibraries}MathLib"
    + "{MWPPCLibraries}MWCRuntime.Lib"
    + "{MWPPCLibraries}SIOUX.PPC.Lib"
    + "{MWPPCLibraries}ANSI C.PPC.Lib"
    + "{MWPPCLibraries}ANSI C++.PPC.Lib"
    + "{Sprockets}lib:DrawSprocketLib"
    + mac_libs
    ;

  mac.cc.mac_includes
    + "{MacLinks}i4"
    + "{Sprockets}inc"
    ;

  mac.cc.includes
    + includes
    ;

  mac.cc.defines
    + "__MAC__"
    + "I4_SCREEN_DEPTH=15"
    + defines
    ;

  mac.cc_opt.includes = mac.cc.mac_includes;
  mac.cc_opt.includes = mac.cc.includes;
  mac.cc_opt.defines = mac.cc.defines;

  mac.link.libs
    + libs
    ;

  use_mac_files();
  makefile("makefile.make");
  echo(pretext);
  echo(config);
  mac.start();
  target("all", targets);
  target("opt", exeopt);
  mac.cc(mac_files,objs);
  mac.cc_opt(mac_files,opt_objs);
  mac.link(objs,exename);
  mac.link(opt_objs,exeopt);
  mac.clean(objs + opt_objs);
}
//}}}


void main(int argc, char **argv)
//{{{
{
  init();
  
  common();

  if (argc<=1 || strcmp(argv[1], "linux")==0)
    make_linux();
  if (argc<=1 || strcmp(argv[1], "win95")==0 || strcmp(argv[1], "win96_16")==0)
    make_win95(16);
  if (argc==2 && strcmp(argv[1], "win95_15")==0)
    make_win95(15);
  if (argc<=1 || strcmp(argv[1], "mac")==0)
    make_mac();
  if (argc<=1 || strcmp(argv[1], "saturn")==0)
    make_saturn();
}
//}}}


//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
