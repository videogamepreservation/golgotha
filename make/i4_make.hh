/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __I4_MAKE_HH
#define __I4_MAKE_HH

#include "table.hh"

enum {
  BUILD_DEBUG  =1<<0,
  BUILD_OPT    =1<<1,
  BUILD_PROF   =1<<2,
  BUILD_CLEAN  =1<<3,
  BUILD_BACKUP =1<<4,
  BUILD_LAST   =BUILD_BACKUP
};


enum os_type { 
  OS_WIN32,
  OS_LINUX
};

enum
{
  WIN32_CONSOLE_APP,
  WIN32_WINDOWED_APP
};


// global options
struct mk_options_struct
{
  int show_deps;
  int build_flags;
  int continue_on_error;
  int no_libs;
  int verbose,quiet;
  int unix_libs_are_shared;
  int no_compile;
  int no_tmp;
  int no_syms;
  int show_includes;

  char *project_file;
  char *target_name;
  char *tmp_dir;
  list targets_to_build;
  list targets_built;
  char slash_char;
  
  os_type os;

  int is_unix() { if (os==OS_LINUX) return 1; else return 0; }

  mk_options_struct()
  {
    project_file="project.i4";
    continue_on_error=0;
    build_flags=0;
    target_name=0;
    verbose=0;
    quiet=0;
    no_libs=0;
    show_deps=0;
    unix_libs_are_shared=0;
    no_compile=0;
    tmp_dir=0;
    no_tmp=0;
    no_syms=0;
    show_includes=0;
  }

  void get(int argc, char **argv);
};

list mk_global_defines;

// options dependant on each target
struct mk_target_struct
{
  char *def_file;

  char *target;
  char *target_type;
  char *outdir;

  list dlls, libs, src, inc, defines;


  int app_type;
  char *cc_flags, *link_flags;

  void reset()
  {
    app_type=WIN32_CONSOLE_APP;
    def_file=0;
    target=0;
    cc_flags=0;
    link_flags=0;
    target=0;
    target_type=0;    
    outdir=0;

    dlls.clear();
    libs.clear();    
    src.clear();
    inc.clear();
    defines.clear();
  }

  mk_target_struct()
  {
    reset();
  }


};

extern mk_options_struct mk_options;

#endif
