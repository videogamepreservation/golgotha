/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include <stdio.h>
#include "i4_make.hh"
#include <ctype.h>
#include "string.cc"
#include "error.cc"
#include "deps.cc"

#include <sys/types.h>
#include <sys/stat.h>
#ifdef __linux
#include <sys/wait.h>
#include <unistd.h>
#define MKDIR(x) mkdir(x, 0xffff)
#endif

#ifdef _WINDOWS
#include <direct.h>
#include <process.h>
#include <winsock.h>
#define MKDIR(x) _mkdir(x)
#endif

#include "array_tree.hh"


char *link_file="c:\\tmp\\link.lk";

mk_options_struct mk_options;

enum {MAX_LEN=1000};

char abs_path[MAX_LEN];
int current_build_type=0;

list failed_targets;
list files_to_clean;
list files_to_backup;

#ifdef _WINDOWS
int my_system (const char *command, int force=0)
{
  char *argv[4];

  if (current_build_type==BUILD_DEBUG ||
      current_build_type==BUILD_OPT ||
      current_build_type==BUILD_PROF ||
      force) 
  {   
    if (command == 0)
      return 1;
    argv[0] = "command";
    argv[1] = "/c";
    argv[2] = (char *) command;
    argv[3] = 0;


    OSVERSIONINFO os;
    os.dwOSVersionInfoSize=sizeof(os);
    GetVersionEx(&os);
    
    if (os.dwPlatformId==VER_PLATFORM_WIN32_NT)
      return system(command);
    else
      return _spawnv(_P_WAIT, "c:\\windows\\command.com", argv);

  }
  return 0;
}

#else


int my_system (const char *command, int force=0)
{
  if (current_build_type==BUILD_DEBUG ||
      current_build_type==BUILD_OPT ||
      current_build_type==BUILD_PROF ||
      force) 
  {   
    // warning- this breaks if you have a child process running already

    int pid, status;

    if (command == 0)
      return 1;
    pid = fork();
    if (pid == -1)
      return -1;
    if (pid == 0)
    {
      char *argv[4];
      argv[0] = "sh";
      argv[1] = "-c";
      argv[2] = (char*) command;
      argv[3] = 0;
      execv("/bin/sh", argv);
      exit(127);
    }
    do
    {
      wait(&status);
      return status;
    } while(1);
  }
  return 0;
}

#endif


void extract(char *fn)
{
  FILE *in=fopen(fn,"rb");
  while (1)
  {
    unsigned char l,s1,s2,s3,s4;
    if (fread(&l,1,1,in)!=1)
    {
      fclose(in);
      exit(0);
    }

    char fn[256],buf[4096];
    fread(fn, 1,l,in);
    FILE *out=fopen(fn,"wb");
    if (!out)
    {
      for (int i=0; i<l; i++)
        if (fn[i]=='\\' || fn[i]=='/')
        {
          int old=fn[i];
          fn[i]=0;
          MKDIR(fn);
          fn[i]=old;
        }

      out=fopen(fn,"wb");
    }

    if (out)
    {
      fread(&s1,1,1,in);     fread(&s2,1,1,in);     fread(&s3,1,1,in);     fread(&s4,1,1,in);
      int size=(s1<<24)|(s2<<16)|(s3<<8)|(s4);
    
      while (size)
      {
        int rsize=size>sizeof(buf) ? sizeof(buf) :size;
        fread(buf, 1,rsize,in);
        fwrite(buf, 1, rsize, out);
        size-=rsize;
      }
      fclose(out);    
    }
    else printf("extract: could not open %s for writing\n", fn);
  }
}

void mk_options_struct::get(int argc, char **argv)
{
  int i;
  quiet=1;

  for (i=1; i<argc; i++)
  {
    if (strcmp(argv[i],"/k")==0 || strcmp(argv[i],"-k")==0)
      continue_on_error=1;
    else if (strcmp(argv[i],"/f")==0 || strcmp(argv[i],"-f")==0)
    {
      i++;
      project_file=argv[i];
    }
    else if (strcmp(argv[i],"debug")==0)
      build_flags|=BUILD_DEBUG;
    else if (strcmp(argv[i],"profile")==0)
      build_flags|=BUILD_PROF;
    else if (strcmp(argv[i],"clean")==0)
      build_flags|=BUILD_CLEAN;
    else if (strcmp(argv[i],"backup")==0)
    {
      build_flags|=BUILD_BACKUP;
    }
    else if (strcmp(argv[i],"opt")==0 || strcmp(argv[i],"optimized")==0)
      build_flags|=BUILD_OPT;
    else if (strcmp(argv[i],"-d")==0)
      mk_debug_on=1;
    else if (strcmp(argv[i],"-show_deps")==0)
      show_deps=1;
    else if (strcmp(argv[i],"-v")==0)
    {
      verbose=1;
      quiet=0;
    }
    else if (strcmp(argv[i],"-v1")==0)
    {
      verbose=0;
      quiet=0;
    }
    else if (strcmp(argv[i],"-extract")==0)
      extract(argv[i+1]);
    else if (strcmp(argv[i],"-q")==0)
      quiet=1;
    else if (strcmp(argv[i],"-nl")==0 || strcmp(argv[i],"-no_libs")==0)
      no_libs=1;
    else if (strcmp(argv[i],"-sim_win32")==0)
      os=OS_WIN32;
    else if (strcmp(argv[i],"-static")==0)
      unix_libs_are_shared=0;
    else if (strcmp(argv[i],"-shared")==0)
      unix_libs_are_shared=1;
    else if (strcmp(argv[i],"-no_tmp")==0)
      no_tmp=1;
    else if (strcmp(argv[i],"-no_syms")==0)
      no_syms=1;
    else if (strcmp(argv[i],"-show_incs")==0)
      show_includes=1;
    else if (strcmp(argv[i],"-tmp")==0)
    {
      i++;
      tmp_dir=argv[i];
    }
    else if ((strcmp(argv[i],"-no_compile")==0) || (strcmp(argv[i],"-nc")==0))
      no_compile=1;    
    else if (strcmp(argv[i],"--help")==0 || 
             strcmp(argv[i],"-help")==0 ||
             strcmp(argv[i],"?")==0 ||
             strcmp(argv[i],"-?")==0)
    {
      printf("%s ussage :\n"
              "%s [-f filename] [-k,q,nl,v,v1,no_libs] [debug] [opt] [clean] [backup] [targets..]\n"
              "  -f overides default project.i4 for building\n"
              "  -k continues on error\n"
              "  debug build debug version of everything\n"
              "  opt builds optimized version of everything\n"
              "  backup backs up all the files referenced by project.i4 including\n"
              "    headers, ram files, and .cc files (saves to backup.i4)\n"
              "  if targets are not listed all targets list in project file are built\n"
              "  -q quiet, shows only the file being processed, and not the actual command\n"
              "  -v verbose show all commands executed\n"
              "  -v1 shows verbose paths (normally simplified when show compiling\n"
              "  -d show debug info relating to i4_make\n"
              "  -nl causes the compilation not to use library files (where possible)\n"
              "  -show_deps shows depdancies that are out of date\n"
              "  -no_compile/-nc runs through the make process without compiling/linking\n"
              "  -static  don't make libraries shared (unix only)\n"
              "  -no_syms don't include any symbol information in the object files or exes\n"
              "  -extract filename   extracts a backup into files (use i4_make backup)\n",
              argv[0]);
      exit(0);
    }

    else
      targets_to_build+=argv[i];
  }
  
  if (build_flags==0)
    build_flags=BUILD_DEBUG | BUILD_OPT;

  if (os==OS_WIN32) slash_char='\\';
  else slash_char='/';
}


enum { BUF_SIZE=1024*1000 };
char *buf=0;
int boff=0;

char *buf_alloc(char *p, int len)
{
  if (!buf || boff+len>BUF_SIZE)
  {
    buf=(char *)malloc(BUF_SIZE);
    boff=0;
  }

  memcpy(buf+boff,p,len);
  boff+=len;
  return buf+boff-len;
}


char *get_abs_path(char *filename, int force=0);


void add_to_backup(char *fname)
{
  char *full_name=get_abs_path(fname,1);   // force a full name

  if (files_to_backup.find(full_name)==-1)
    files_to_backup.add(full_name);
}


void pcwd()
{
  char buf[100];
  getcwd(buf, 100);
  printf("cwd = %s\n",buf);
}

char start_path[256];

void set_abs_path(char *filename)
{ 
  char tmp[MAX_LEN];
  strcpy(tmp, filename);

  char *last_slash=0, *p=tmp, *q;
  for (; *p; p++)
    if (*p=='/' || *p=='\\')
      last_slash=p;

  if (last_slash)
  {  
    last_slash[1]=0;
    strcpy(abs_path, get_abs_path(tmp));

    if (mk_options.no_tmp)
    {
      chdir(start_path);
      chdir(abs_path);
    }
  }

}

char *get_abs_path(char *filename, int force)
{
  if (mk_options.no_tmp && !force)
    return filename;
  else
  {
      
    char tmp[MAX_LEN];
    strcpy(tmp, abs_path);
    char *s, *d;

    if ((filename[0]=='/' || filename[0]=='\\') || (filename[1]==':'))
      return filename;

    for (s=filename, d=tmp + strlen(tmp); *s; )
    {
      if (s[0]=='.' && s[1]=='.' && (s[2]=='/' || s[2]=='\\'))
      {
        d-=2;
        while (*d!='/' && *d!='\\') 
        {
          d--;
          if (d<tmp) mk_error("bad file path %s\n",filename);
        }
        d++;
        s+=3;
      }
      else 
      {
        while (*s && *s!='/' && *s!='\\')
        {
          *d=*s;
          s++;
          d++;
        }

        if (*s)
        {
          *d=*s;
          d++;
          s++;
        }
      }
    }
    *d=0;

    for (d=tmp; *d; d++)
      if (mk_options.os==OS_LINUX && *d=='\\')
        *d='/';
      else if (mk_options.os==OS_WIN32 && *d=='/')
        *d='\\';
  

    return buf_alloc(tmp, strlen(tmp)+1);
  }
}

enum { EXT_O,
       EXT_LIB,
       EXT_DLL,
       EXT_PLUGIN,
       EXT_EXE,
       EXT_DEP,
       EXT_PCH,
       EXT_PDB,
       EXT_RAM_FILE,
       EXT_RC_RES
};


void show_command(char *cmd, int force=0)
{
  if (current_build_type==BUILD_DEBUG ||
      current_build_type==BUILD_OPT ||
      current_build_type==BUILD_PROF ||
      force) 
  {
    if (mk_options.verbose)
      printf("%s",cmd);
    else
    {
      for (;*cmd;)
      {
        if ((*cmd=='/' || *cmd=='\\') || (cmd[0]=='_' && cmd[1]=='_'))
        {
          char *start=cmd, *c, *d;
          for (c=cmd; *c && *c!=' '; c++)
            if (*c=='/' || *c=='\\')
              start=c+1;
            else if (c[0]=='_' && c[1]=='_')
              start=c+2;
     
          for (d=start; d!=c; d++)
            printf("%c",*d);

          cmd=c;
        }
        else
        {
          printf("%c", *cmd);
          cmd++;
        }    
      }
    }
    printf("\n");
  }
}

char *make_out_name(char *filename, int ext_type=-1, int use_full_path=1, char *outdir=0)
{
  char tmp[MAX_LEN], *s, *d;

  if (ext_type==-1)
  {
    int l=strlen(filename);
    if (l>4)
    {
      if (filename[l-3]=='.' && filename[l-2]=='r' && filename[l-1]=='c')
        ext_type=EXT_RC_RES;
      else if (filename[l-3]=='.' && filename[l-2]=='c' && filename[l-1]=='c')
        ext_type=EXT_O;
      else
      {
        printf("make_out_name : don't know what extension to use for %s",filename);
        exit(0);
      }


    } else       
    {
      printf("make_out_name : filename to short : %s",filename);
      exit(0);

    }
  }

  if (mk_options.no_tmp)
  {
    d=tmp;
    char *start=filename;
    for (s=filename; *s && *s!='.'; s++);
    if (*s)
    {
      memcpy(d,filename, s-filename);
      d+=s-filename;
    }
    else
    {
      strcpy(d,filename);
      d+=strlen(d);
    }
    *d=0;
  }
  else
  {
    if (outdir)
    {
      strcpy(tmp,outdir);
      use_full_path=0;
    }
    else
    {
      if (mk_options.tmp_dir)
        sprintf(tmp,"%s%c", mk_options.tmp_dir, mk_options.slash_char);
      else
      {
        char *t=getenv("I4_TMP");
        if (t)
          sprintf(tmp,"%s%c", t, mk_options.slash_char);      
        else
        {
          if (mk_options.os==OS_LINUX)    
            strcpy(tmp,"/tmp/");
          else
            strcpy(tmp,"c:\\tmp\\");
        }
      }
    
    }
    d=tmp+strlen(tmp);


    if (!use_full_path)
    {
      char *start=filename;
      for (s=filename; *s; s++)
        if (*s=='\\' || *s=='/' || *s==':')
          start=s+1;

      strcpy(d,start);
      d+=strlen(d);
    }
    else
    {
      for (s=filename; *s; s++)
      {
        if (*s=='\\' || *s=='/' || *s==':')
        {
          d[0]=d[1]='_';
          d++;
        } 
        else if (*s=='.')
        {
          d[0]='-';
        } else *d=*s;
        d++;    
      }
    }
  }
  
  if (current_build_type==BUILD_DEBUG)
    strcpy(d,"_debug");
  else if (current_build_type==BUILD_OPT)
    strcpy(d,"_opt");
  else if (current_build_type==BUILD_PROF)
    strcpy(d,"_prof");
  else if (current_build_type==BUILD_CLEAN)
    strcpy(d,"_*");
  d+=strlen(d);

  if (mk_options.os==OS_LINUX)
  {
    switch (ext_type)
    {
      case EXT_O :  strcpy(d,".o"); break;
      case EXT_LIB :  
      {
        if (mk_options.unix_libs_are_shared)
          strcpy(d,".so");
        else
          strcpy(d,".a"); 
      } break;

      case EXT_DEP :  strcpy(d,".dep"); break;
      case EXT_PLUGIN:
      case EXT_DLL :  strcpy(d,".dll"); break;
      case EXT_RAM_FILE : strcpy(d,".cc"); break;
    }
  }
  else
  {
    switch (ext_type)
    {
      case EXT_O :  strcpy(d,".obj"); break;
      case EXT_LIB :  strcpy(d,".lib"); break;
      case EXT_EXE :  strcpy(d,".exe"); break;
      case EXT_DEP :  strcpy(d,".dep"); break;
      case EXT_PDB :  strcpy(d,".pdb"); break;
      case EXT_PCH :  strcpy(d,".pch"); break;
      case EXT_PLUGIN:
      case EXT_DLL :  strcpy(d,".dll"); break;
      case EXT_RAM_FILE : strcpy(d,".cc"); break;
      case EXT_RC_RES :  strcpy(d,".res"); break;
    }

  }

  
  char *ret=buf_alloc(tmp, strlen(tmp)+1);

  if (current_build_type==BUILD_CLEAN)
  {
    char *full_name=get_abs_path(ret,1);   // force a full name
    if (files_to_clean.find(full_name)==-1)
      files_to_clean.add(full_name);
  }

  return ret;
}



unsigned long check_sum32(void *buf, int buf_len)
{
  unsigned char c1=0,c2=0,c3=0,c4=0;
  while (buf_len)
  {
    c1+=*((unsigned char *)buf);
    c2+=c1;
    buf=(void *)(((unsigned char *)buf)+1);
    c3+=c2;
    c4+=c3;
    buf_len--;
  }
  return (c1|(c2<<8)|(c3<<16)|(c4<<24));
}


class mk_file_mod_node
{
public:
  int left, right;
  unsigned long checksum;
  unsigned long mod_time;
  
  int operator>(const mk_file_mod_node &b) const { return  (checksum > b.checksum); }
  int operator<(const mk_file_mod_node &b) const { return  (checksum < b.checksum); } 
  mk_file_mod_node(unsigned long checksum) : checksum(checksum) {}
  mk_file_mod_node() {}
};


i4_array_tree<mk_file_mod_node, 5000> mod_tree;


void set_mod_time(char *filename, int time_to_set)
{
  mk_file_mod_node n(check_sum32(filename,strlen(filename)));
  int i=mod_tree.find(n);
  if (i!=-1)
  {
    mod_tree.get(i).mod_time=time_to_set;
    return ;
  }

  n.mod_time=time_to_set;
  mod_tree.add(n);
 
}

int get_mod_time(char *filename, int force_stat=0)
{
  mk_file_mod_node n(check_sum32(filename,strlen(filename)));
  int i=mod_tree.find(n);
  if (i!=-1 && !force_stat)
    return mod_tree.get(i).mod_time;

  unsigned long t;
#ifdef _WINDOWS
  struct _stat s;
  if (_stat(filename, &s)==0)
   t=s.st_mtime;
  else
    return 0;
#elif __linux
   struct stat s;
  if (stat(filename, &s)==0)
    t=s.st_mtime;
  else
    return 0;
#else
#error define os here
#endif

  if (i!=-1)
    mod_tree.get(i).mod_time=t;

  n.mod_time=t;
  mod_tree.add(n);
  return t;
}


enum { ALREADY_UP_TO_DATE,
       CHANGED,
       BUILD_ERROR,
       NO_MORE_TARGETS
};

void clean_file(char *outname)
{
  char cmd[200];
  if (outname[strlen(outname)-1]!='*')
  {
#ifdef _WINDOWS
    sprintf(cmd,"del %s", outname);
#else
    sprintf(cmd,"rm %s", outname);
#endif


    show_command(cmd,1);

    my_system(cmd, 1);
  }
}

int build_file(char *filename, 
               mk_target_struct &target,
               mk_target_struct &top_target)
               
{
  char *p=filename, *last_dot=0, cmd[MAX_LEN], inc[MAX_LEN], def[MAX_LEN];
  inc[0]=0;
  def[0]=0;
  int i;

  for (p=filename; *p; p++)
    if (*p=='.' && p[1]!='.')
      last_dot=p;

  if (last_dot)
  {
    if (strcmp(last_dot,".cc")==0)
    {
      char *source_name=get_abs_path(filename);

      char *outname=make_out_name(source_name,EXT_O);
      if (failed_targets.find(outname)!=-1)
        return BUILD_ERROR;

      switch (current_build_type)
      {
        case BUILD_BACKUP:
        {
          add_to_backup(source_name);
          list *dep=get_deps(source_name, &target.inc);
          if (dep)
          {
            for (i=0; i<dep->size(); i++)
              add_to_backup((*dep)[i]);
          }
        } break;


        case BUILD_OPT :
        case BUILD_DEBUG :
        case BUILD_PROF :
          int rebuild=0;

          int m1=get_mod_time(outname);
          if (m1)
          {
            list *dep=get_deps(source_name, &target.inc);
            if (mk_options.show_includes && dep)
            {
              printf( "%s\n",filename);
              for (i=0; i<dep->size(); i++)
                printf( "    %s\n",(*dep)[i]);
            }
            
            if (dep)
            {
              for (i=0; !rebuild && i<dep->size(); i++)
              {
                int src_mod_time=get_mod_time( (*dep)[i]);
                if (src_mod_time>m1)
                {
                  if (mk_options.show_deps)
                    printf( "%s newer than %s\n", (*dep)[i], outname);
                  rebuild=1;
                }
              }
            }
          } else rebuild=1;
              
          if (rebuild)
          {
            if (mk_options.os==OS_LINUX)
            {
              for (i=0; i<target.inc.size(); i++)
                sprintf(inc+strlen(inc), "-I%s ",target.inc[i]);

              for (i=0; i<target.defines.size(); i++)
                sprintf(def+strlen(def), "-D%s ", target.defines[i]);

              for (i=0; i<mk_global_defines.size(); i++)
                sprintf(def+strlen(def), "-D%s ", mk_global_defines[i]);

              char *sym_string = mk_options.no_syms ? "" : "-g ";
              
              sprintf(cmd, "g++ %s%s%s%s-c %s -o %s",
                      sym_string,
                      current_build_type==BUILD_DEBUG ? "-DDEBUG " : 
                      current_build_type==BUILD_OPT ? "-O2 " :
                      "-O2 -pg ", // profile version
                      def, inc, source_name, outname);
            }
            else 
            {
              for (i=0; i<target.inc.size(); i++)
                sprintf(inc+strlen(inc), "/I %s ",target.inc[i]);
              
              for (i=0; i<target.defines.size(); i++)
                sprintf(def+strlen(def), "/D%s ",target.defines[i]);

              for (i=0; i<mk_global_defines.size(); i++)
                sprintf(def+strlen(def), "/D%s ",mk_global_defines[i]);

              char *dll_def=(strcmp(target.target_type,"dll")==0 || 
                             strcmp(target.target_type,"plugin")==0) ? "/DBUILDING_DLL " : "";
              
              sprintf(cmd, "cl /GX /EHa /D_WINDOWS %s%s%s%s/c /Tp %s /Fo%s /Fd%s",
                      dll_def,
                      current_build_type==BUILD_DEBUG ? "/MD /Zi /Od /DDEBUG /nologo " : 
                      current_build_type==BUILD_OPT ? "/MD /Zi /Ox /nologo " :
                      "/MD /Zi /Ox /nologo /Gh " // profile version
                      ,
                      def, inc, source_name, outname,
                      //                      make_out_name(get_abs_path(target.target), EXT_PCH),
                      make_out_name(get_abs_path(target.target), EXT_PDB));


            }

            if (mk_options.quiet && current_build_type!=BUILD_CLEAN)
              printf("Compiling %s (%s)\n", filename, 
                     current_build_type==BUILD_DEBUG ? "debug" : 
                     current_build_type==BUILD_OPT ? "opt" :
                     "profile");
            else
              show_command(cmd);

            if (mk_options.no_compile)
            {
              set_mod_time(outname, 0x7fffffff);
              return CHANGED;
            }
            else
            {
              if (my_system(cmd)==0)
              {
                get_mod_time(outname,1);  // get time of output file
                return CHANGED;
              }
              else 
              {
                failed_targets.add(outname);
                return BUILD_ERROR;
              }
            }

          } else return ALREADY_UP_TO_DATE;
          break;
        
      }

      return ALREADY_UP_TO_DATE;
    }
    else if (strcmp(last_dot,".rc")==0)
    {
      add_to_backup(filename);
      char *source_name=get_abs_path(filename);

      char *outname=make_out_name(source_name,EXT_RC_RES);
      if (failed_targets.find(outname)!=-1)
        return BUILD_ERROR;

      
      if (get_mod_time(source_name)<get_mod_time(outname))
        return ALREADY_UP_TO_DATE;

      sprintf(cmd, "rc /fo%s ",outname);
      for (i=0; i<target.inc.size(); i++)
        sprintf(cmd+strlen(cmd), "/i %s ",target.inc[i]);
      sprintf(cmd+strlen(cmd),"%s", source_name);

      if (mk_options.quiet && current_build_type!=BUILD_CLEAN)
        printf("Compiling %s (%s)", source_name, 
               current_build_type == BUILD_DEBUG ? "debug" : 
               current_build_type == BUILD_OPT ? "opt" :
               "profile");
      else
        show_command(cmd);

      if (mk_options.no_compile)
      {
        set_mod_time(outname, 0x7fffffff);
        return CHANGED;
      }
      else
      {
        if (my_system(cmd)==0)
        {
          get_mod_time(outname, 1);
          return CHANGED;
        }
        else
        {
          failed_targets.add(outname);
          return BUILD_ERROR;
        }
      }
    }
    else
      mk_error("don't know how to build files with extension '%s' : %s",last_dot, filename);
  }
  else 
    mk_error("don't know how to build files without extensions '%s'", filename);

  return BUILD_ERROR;
}

int build_lib(int object_files_have_changed, 
              mk_target_struct &target,
              mk_target_struct &top_target,
              int dll=0)
{
  char tmp[50000];
  int i;

  if (target.src.size()==0)
    return ALREADY_UP_TO_DATE;



  char *lname=make_out_name(get_abs_path(target.target), 
                            dll ? EXT_DLL : EXT_LIB, 1, target.outdir);

  if (failed_targets.find(lname)!=-1)
    return BUILD_ERROR;

  if (dll)
  {
    if (top_target.dlls.find(lname)==-1)
      top_target.dlls.add(lname);
  }
  else if (!dll && top_target.libs.find(lname)==-1)
    top_target.libs.add(lname);

  int ltime=get_mod_time(lname);     
  if (!ltime)
    object_files_have_changed=1;


  if (mk_options.os==OS_LINUX)
  {
    char *sym_string = mk_options.no_syms ? "" : "-g ";
    char shared_string[100];
    
    if (mk_options.unix_libs_are_shared)
      sprintf(shared_string,  "-shared -Wl,-soname,%s ", lname);
    else
      shared_string[0]=0;
    
    if (dll || mk_options.unix_libs_are_shared)
      sprintf(tmp, "g++ %s%s -g -o %s ",
              sym_string,
              shared_string,
              lname);
    else
      sprintf(tmp, "ar rucs %s ", lname);

    char *d=tmp+strlen(tmp);
    for (i=0; i<target.src.size(); i++)
    {
      char *oname=make_out_name(get_abs_path(target.src[i]));
      sprintf(d, "%s ", oname);
      if (!object_files_have_changed && get_mod_time(oname)>ltime)
      {
        if (mk_options.show_deps)
          printf( "%s newer than %s\n",oname, lname);
        object_files_have_changed=1;
      }

      d+=strlen(d);
    }
  }
  else
  {
    if (dll)
      sprintf(tmp,
              "link /MACHINE:IX86 /DEBUG /debugtype:coff /debugtype:both /DLL "
              "/nologo /PDB:%s /OUT:%s @%s%s", 
              make_out_name(get_abs_path(lname), EXT_PDB, 0),
              lname, link_file, 
              mk_options.quiet ? "> c:\\tmp\\null" : "");
    else
      sprintf(tmp, "lib /nologo /OUT:%s @%s%s", lname, link_file, 
              mk_options.quiet ? "> c:\\tmp\\null" : "");
    FILE *fp=fopen(link_file, "wb");
   

    for (i=0; i<target.src.size(); i++)
    {
      char *oname=make_out_name(get_abs_path(target.src[i]));
      fprintf(fp, "%s\n", oname);

      if (!object_files_have_changed && get_mod_time(oname)>ltime)
      {
        if (mk_options.show_deps)
          printf( "%s newer than %s\n",oname, lname);

        object_files_have_changed=1;
      }

    }

    if (dll)
    {
      for (i=0; i<target.libs.size(); i++)
      {
        char *oname=target.libs[i];
        fprintf(fp, "%s\n", oname);


        if (!object_files_have_changed && get_mod_time(oname)>ltime)
        {
          if (mk_options.show_deps)
            printf( "%s newer than %s\n",oname, lname);

          object_files_have_changed=1;
        }
      }

      if (target.def_file)
        sprintf(tmp+strlen(tmp), " /def:\"%s\"", target.def_file);
    }

    fclose(fp);
  }



  if (object_files_have_changed)
  {
    if (mk_options.quiet && (current_build_type!=BUILD_CLEAN && current_build_type!=BUILD_BACKUP))
      printf("Creating library %s\n", target.target);
    else
      show_command(tmp);

    if (mk_options.no_compile)
    {
      set_mod_time(lname, 0x7fffffff);
      return CHANGED;
    }
    else
    {
      if (my_system(tmp)==0)
      {
        get_mod_time(lname, 1);  // get time of output file

        // don't need to relink executable if using shared libraries
        if (mk_options.is_unix() && mk_options.unix_libs_are_shared)
          return ALREADY_UP_TO_DATE;

        return CHANGED;
      }
      else
      {
        failed_targets.add(lname);       // don't try to build this again
        return BUILD_ERROR;
      }
    }
  }
  else 
    return ALREADY_UP_TO_DATE;
}

int build_exe(int deps_have_changed, mk_target_struct &target)
{
  char tmp[50000];
  int i;

  char *ename=make_out_name(get_abs_path(target.target), EXT_EXE, 0, target.outdir);
  int etime=get_mod_time(ename);
  if (!etime)
    deps_have_changed=1;
  
  FILE *fp;
  if (mk_options.os==OS_LINUX)
  {
    char *sym_str=mk_options.no_syms ? "" : "-g ";
    
    
    sprintf(tmp, "g++ -rdynamic %s%s-o %s ",
            sym_str,
            current_build_type==BUILD_DEBUG ? "" : 
            current_build_type==BUILD_OPT ? "-O2 " :
            "-O2 -pg ",
            ename);
  }
  else
  {
    sprintf(tmp, "link  /SUBSYSTEM:%s /MACHINE:IX86 /DEBUG "
            "/NOLOGO /PDB:%s /NODEFAULTLIB /OUT:%s @%s",       // /INCREMENTAL:NO
            target.app_type==WIN32_CONSOLE_APP ? "CONSOLE" : "WINDOWS",
            make_out_name(get_abs_path(target.target), EXT_PDB, 0),
            ename, link_file);

    fp=fopen(link_file,"wb");
    if (!fp) mk_error("could not open %s for writing\n",link_file);
  }


  char *d=tmp+strlen(tmp);
    
  for (i=0; i<target.src.size(); i++)
  {
    char *oname=make_out_name(get_abs_path(target.src[i]));
    if (!deps_have_changed && get_mod_time(oname)>etime)
    {
      if (mk_options.show_deps)
        printf( "%s newer than %s\n",oname, ename);
      deps_have_changed=1;
    }

    if (mk_options.os==OS_LINUX)
      sprintf(d, "%s ", oname);
    else
      fprintf(fp, "%s\n", oname);

    d+=strlen(d);
  }


  for (i=target.libs.size()-1; i>=0; i--)
  {     
    if (mk_options.os==OS_LINUX)
      sprintf(d, "%s ", target.libs[i]);
    else
      fprintf(fp, "%s\n", target.libs[i]);

    if (!deps_have_changed && get_mod_time(target.libs[i])>etime)
    {
      if (mk_options.show_deps)
        printf( "%s newer than %s\n", target.libs[i], ename);
      deps_have_changed=1;
    }

    d+=strlen(d);
  }

  if (mk_options.os==OS_WIN32)
    fclose(fp);

  if (deps_have_changed)
  {
    if (current_build_type!=BUILD_CLEAN && current_build_type!=BUILD_BACKUP)
    {
      if (mk_options.quiet)
        printf("Linking %s\n", target.target);
      else
        printf("%s\n", tmp);
    }

    if (mk_options.no_compile)
    {
      set_mod_time(ename, 0x7fffffff);
      return CHANGED;
    }
    else
    {
      if (my_system(tmp)==0)
      {
        get_mod_time(ename, 1);  // get time of output file
        return CHANGED;
      }
      else
        return BUILD_ERROR;
    }
  }
  else return ALREADY_UP_TO_DATE;
  
}

int build_target(mk_target_struct &target,
                 mk_target_struct &top_target)
//                  list &exe_src,
//                  list &exe_inc,
//                  list &exe_def,
//                  list &lib_files,
//                  list &dll_libs)
{
  int i;
  int change=0;

  if (mk_options.targets_built.find(target.target)==-1)
  {
    mk_options.targets_built.add(target.target);

    if (strcmp(target.target_type,"list")==0)
    {
      for (i=0; i<target.src.size(); i++)        
      {
        char *fname=get_abs_path(target.src[i]);
        if (top_target.src.find(fname)==-1) 
          top_target.src.add(fname);
      }
    }
          
//     if (strcmp(target.target_type,"executable")==0 || strcmp(target.target_type,"plugin")==0)
//     {
//       for (i=0; i<target.src.size(); i++)
//       {
//         char *fn=target.src[i];
//         if (fn[strlen(fn)-1]=='.')
//           printf("adding to src_file with no extension\n");

//         target.src+=exe_src[i];
//       }
//     }

    int ret=ALREADY_UP_TO_DATE;


    for (i=0; i<target.src.size(); i++)
    {
      mk_debug("checking depends for %s\n", target.src[i]);

      int result=build_file(target.src[i], target, top_target); 
      if (result==BUILD_ERROR && !mk_options.continue_on_error)
        return BUILD_ERROR;

      if (result==CHANGED)
        change=1;
    } 
         
    if (strcmp(target.target_type,"lib")==0)
    {
      if (mk_options.no_libs)     // they don't want to use libs, add files to top target
      {
        for (i=0; i<target.src.size(); i++)        
        {
          char *fname=get_abs_path(target.src[i]);
          if (top_target.libs.find(fname)==-1) 
            top_target.libs.add(fname);
        }
      }
      else
      {
        switch (build_lib(change, target, top_target, 0))
        {
          case BUILD_ERROR : return BUILD_ERROR; break;
          case CHANGED : change=1; break;
        }
      }
    }  

    if (strcmp(target.target_type,"executable")==0)
    {
      switch (build_exe(change, top_target)) 
      {
        case BUILD_ERROR : return BUILD_ERROR; break;
        case CHANGED : change=1; break;
      }
    }  


    if (strcmp(target.target_type,"dll")==0)
    {
      switch (build_lib(change, target, top_target, 1))
      {
        case BUILD_ERROR : return BUILD_ERROR; break;
        case CHANGED : change=1; break;
      }
    }  

    if (strcmp(target.target_type,"plugin")==0)
    {
      switch (build_lib(change, target, top_target, 1))
      {
        case BUILD_ERROR : return BUILD_ERROR; break;
        case CHANGED : change=1; break;
      }
    }  
  }
    

  if (change)
    return change;
  else
    return ALREADY_UP_TO_DATE;

}




char skip_white(char *&p)
{ 
  while (*p && (*p==' ' || *p=='\n' || *p=='\t' || *p=='\r' || *p=='#' || *p==28))
  {
    if (*p=='#')
    {
      while (*p && *p!='\n') 
        p++;            
    }
    else if (*p==' ' || *p=='\r' || *p=='\t' || *p==28)
      p++;
    else if (*p=='\n')
    {
      p++;
      line_on++;
    }
    else
    {
      char c=*p;
      p++;
      return c;
    }
  } 
  return 0;
}

int token_char(char p)
{
  return isalnum(p) || p=='_' || p=='.' || p=='\\' || p=='/' || p==':';
}


char tmp[2000];

char *get_token(char *&p)
{
  skip_white(p); 
  if (*p)
  {
    if (p[0]==']' || p[0]=='[' || p[0]=='=')      
    {
      tmp[0]=p[0]; tmp[1]=0; p++;
      return buf_alloc(tmp,2);
    }
    else if (p[0]=='"')
    {
      int len=0;
      char *c=tmp;
      p++;
    
      while (*p && *p!='"') 
      {
        *(c++)=*(p++);
        len++;
      }
      *c=0;

      p++;      
      return buf_alloc(tmp, len+1);


    } else if (token_char(*p))
    {
      int len=0;
      char *c=tmp;
     
      while (*p && token_char(*p)) 
      {
        *(c++)=*(p++);
        len++;
      }
      *c=0;

      return buf_alloc(tmp, len+1);
    }
    else
    {
      mk_error("unexpected char '%c'", p[0]);
      return 0;
    }
  }
  else return 0;

}

void next_line(char *&p)
{
  int l=line_on;
  while (l==line_on)
  {
    skip_white(p);
    if (l==line_on)
      get_token(p);
  }
}


int build_ram_file(char *name, char *out_name, mk_target_struct &top_target)
{
  char *data_name=get_abs_path(name);
  char *cc_name=make_out_name(data_name, EXT_RAM_FILE, 1);

  if (top_target.src.find(cc_name)==-1)
    top_target.src.add(cc_name);
  
  int m1=get_mod_time(cc_name);
  if (!m1 || m1<get_mod_time(data_name))
  {
    show_command(form("Generating ram file .cc %s", cc_name));

    FILE *fp=fopen(data_name, "rb");
    if (!fp) 
    {
      printf( "%s : Unable to open file to genertate ram file\n", data_name);
      return 0;
    }

    fseek(fp, 0, SEEK_END);
    int fsize=ftell(fp);
    fseek(fp, 0, SEEK_SET);

    unsigned char *buf=(unsigned char *)malloc(fsize);
    fread(buf, fsize, 1, fp);
    
    fclose(fp);

    unsigned long id=check_sum32(cc_name, strlen(cc_name)) +
      check_sum32(out_name, strlen(out_name));

    fp=fopen(cc_name, "wb");
    fprintf(fp, "#include \"file/ram_file_man.hh\"\n\nunsigned char i4_%x_ptr[%d]={\n",
            id, fsize);
   
    for (int i=0; i<fsize-1; i++)
    {
      if ((i%15)==0)
        fprintf(fp, "\n");
      fprintf(fp, "%d, ", buf[i]);
    }
    if (fsize)
      fprintf(fp, "%d", buf[fsize-1]);


    fprintf(fp,"};\n\ni4_openable_ram_file_info i4_%x(\"%s\", i4_%x_ptr, %d);",
            id, out_name, id, fsize);
    fclose(fp);

    get_mod_time(cc_name, 1);

    return CHANGED;
  }
  else return ALREADY_UP_TO_DATE;
}

char *load_file(char *fn, int set_current_file)
{
  char *abs_filename=get_abs_path(fn);

  FILE *fp=fopen(abs_filename, "rb");
  if (!fp) return 0;
  
  fseek(fp, 0, SEEK_END);
  int fsize=ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char *buf=(char *)malloc(fsize+1);
  fread(buf, fsize, 1, fp);
  buf[fsize]=0;

  fclose(fp);

  if (set_current_file)
  {
    file_on=abs_filename;
    line_on=0;
    file_contents=buf;
  }

  return buf;
}

int get_target(mk_target_struct &target,
               mk_target_struct &top_target,
               char *&p, 
               char *target_name_to_match=0)
{
  int ret=ALREADY_UP_TO_DATE;
  while (1)
  {
    char *left_brace=get_token(p);
    if (left_brace)
    {
      if (strcmp(left_brace, "["))
        mk_error("expecting '['");

      target.reset();
      target.target_type=get_token(p);
      target.target=get_token(p);

      if (strcmp(get_token(p), "]"))
        mk_error("expecting ']'");

    // should we use this target or skip it
      if (target_name_to_match && strcmp(target.target, target_name_to_match))
      {
        mk_debug("skipping section %s\n", target.target);
        skip_white(p);
        while (*p && *p!='[')
        {        
          get_token(p);
          skip_white(p);
        }    
      }
      else
      {
        mk_debug("processing section %s\n", target.target);
        skip_white(p);
        while (*p && *p!='[')
        {     
          char *t=get_token(p);
          
          if (strcmp(t, "win32")==0)
          {
            if (mk_options.os!=OS_WIN32)
            {
              mk_debug("%s : skipping line, wrong os\n", t);
              next_line(p);
            }
          }
          else if (strcmp(t, "linux")==0)
          {
            if (mk_options.os!=OS_LINUX)
            {
              mk_debug("%s : skipping line, wrong os\n", t);
              next_line(p);
            }
          }
          else if (strcmp(t, "opt")==0)
          {
            if (current_build_type!=BUILD_OPT)
            {
              mk_debug("%s : skipping line, wrong build type\n", t);
              next_line(p);
            }
          }
          else if (strcmp(t, "debug")==0)
          {
            if (current_build_type!=BUILD_DEBUG)
            {
              mk_debug("%s : skipping line, wrong build type\n", t);
              next_line(p);
            }
          }
          else if (strcmp(t, "profile")==0)
          {
            if (current_build_type!=BUILD_PROF)
            {
              mk_debug("%s : skipping line, wrong build type\n", t);
              next_line(p);
            }
          }
          else if (strcmp(t, "hostname")==0)
          {
            t=get_token(p);
            char name[200];
            gethostname(name,200);
            if (strcmp(t, name)!=0)
              next_line(p);            
          }
          else if (strcmp(t, "nothostname")==0)
          {
            t=get_token(p);
            char name[200];
            gethostname(name,200);
            if (strcmp(t, name)==0)
              next_line(p);            
          }          
          else if (strcmp(t, "link_as_windowed_app")==0)
            top_target.app_type=WIN32_WINDOWED_APP;
          else if (strcmp(t, "target_dir")==0)
          {
            char *dir=get_token(p);
            MKDIR(dir);
            target.outdir=form("%s/",dir);
          }
          else if (strcmp(t, "add_to_plugin")==0)
          {
            char *fn=get_token(p);

            char *file=get_abs_path(fn);
            char *d=file+strlen(file);
            while (d>file && *d!='.') d--;

            if (strcmp(top_target.target_type,"plugin")==0)
            {
              if (strcmp(d,".def")==0)
                top_target.def_file=file;            
              else if (strcmp(d,".a")==0 || 
                       (strstr(file, ".so")!=0) ||
                       strcmp(d,".lib")==0 ||
                       strcmp(d,".res")==0)
              {
                if (strcmp(d,".res")==0)
                  add_to_backup(file);

                if (top_target.libs.find(file)==-1) 
                  top_target.libs.add(file);
              }
              else if (top_target.src.find(file)==-1)
                top_target.src.add(file);
            }
          }
          else if (strcmp(t, "add_to_executable")==0)
          {
            char *fn=get_token(p);

            char *file=get_abs_path(fn);
            char *d=file+strlen(file);
            while (d>file && *d!='.') d--;

            if (strcmp(top_target.target_type,"executable")==0 || 
                strcmp(top_target.target_type,"plugin")==0)
            {
              if (strcmp(d,".def")==0)
                top_target.def_file=file;            
              else if (strcmp(d,".a")==0 || 
                       (strstr(file, ".so")!=0) ||
                       strcmp(d,".lib")==0 ||
                       strcmp(d,".res")==0)
              {
                if (strcmp(d,".res")==0)
                  add_to_backup(file);

                if (top_target.libs.find(file)==-1) 
                  top_target.libs.add(file);
              }
              else if (top_target.src.find(file)==-1)
                top_target.src.add(file);
            }
          }
          else if (strcmp(t,"backup")==0)
          {            
            char *fname=get_token(p);
            add_to_backup(fname);
          }
          else if (strcmp(t,"add_include_directory")==0)
          {
            char *i=get_abs_path(get_token(p), 1);

            if (target.inc.find(i)==-1) 
              target.inc.add(i);

            if (top_target.inc.find(i)==-1)
              top_target.inc.add(i);

            mk_debug("add_include_directory (dir=%s)\n", i);
          }
          else if (strcmp(t,"add_define")==0)
          {
            char *i=get_token(p);

            if (target.defines.find(i)==-1) 
              target.defines.add(i);

            mk_debug("add_define (define=%s)\n", i);
          }
          else if (strcmp(t,"add_global_define")==0)
          {
            char *i=get_token(p);

            if (mk_global_defines.find(i)==-1) 
              mk_global_defines.add(i);

            mk_debug("add_global_define (define=%s)\n", i);
          }
          else if (strcmp(t, "ram_file")==0)
          {
            char *in_name=get_token(p);
            int l=line_on;
            skip_white(p);

            if (current_build_type==BUILD_BACKUP)
            {
              add_to_backup(in_name);
              if (line_on==l)
                get_token(p);
            }
            else
            {
              if (line_on==l)              
                build_ram_file(in_name, get_token(p), top_target); 
              else
                build_ram_file(in_name, in_name, top_target);
            }


          }
          else if (strcmp(t, "use")==0)
          {

            t=get_token(p);
            char *use_file=0;


            if (strcmp(t,"file")==0)
            {
              if (strcmp(get_token(p),"="))
                mk_error("expected = after file");
              use_file=get_token(p);
              t=get_token(p);

              add_to_backup(use_file);
            }

            mk_debug("use (file=%s) (target=%s)\n", use_file, t);


            mk_target_struct tar;
            int cur_line=line_on;   // save current line & file info
            char *cur_file=file_on;
            char *cur_contents=file_contents;

            char old_abs[MAX_LEN];
            char *f;
            if (use_file)
            {
              f=load_file(use_file, 1);
              if (!f)
                mk_error("could not load project file '%s'", use_file);
              
              strcpy(old_abs, abs_path);
              set_abs_path(use_file);
            }
            else f=file_contents;


            if (get_target(tar, top_target, f,t)==NO_MORE_TARGETS)
                mk_error("could not find target '%s'", t);
            
            // add include paths to local target
            for (int i=0; i<tar.inc.size(); i++)
              if (target.inc.find(tar.inc[i])==-1)
                target.inc.add(tar.inc[i]);

            ret=build_target(tar, top_target);
              
            if (use_file)
            {
              file_contents=cur_contents;
              file_on=cur_file;
              set_abs_path(old_abs);
            }

            line_on=cur_line;
              
           
          }
          else 
          {
            mk_debug("add to target %s\n",t);
            target.src.add(t);
          }
        
          skip_white(p);
        }    
        return ret;
      } 
    }
    else return NO_MORE_TARGETS;
  }
}

void backup()
{
  if (files_to_backup.size())
  {  
    FILE *out=fopen("backup.i4", "wb");
    if (out)
    {
      int t_files=0;
      for (int i=0; i<files_to_backup.size(); i++)
      {
        FILE *in=fopen(files_to_backup[i], "rb");
        if (in)
        {        
          char *fn=files_to_backup[i];
          if (fn[1]==':') fn+=2;
          if (fn[0]=='\\' || fn[0]=='/') fn++;        

          unsigned char l=strlen(fn)+1, s1,s2,s3,s4;
          fwrite(&l, 1, 1, out);
          fwrite(fn, l, 1, out);

          fseek(in, 0, SEEK_END);
          int size=ftell(in);
          fseek(in, 0, SEEK_SET);
          s1=(size&0xff000000)>>24;
          s2=(size&0xff0000)>>16;
          s3=(size&0xff00)>>8;
          s4=(size&0xff)>>0;
          fwrite(&s1, 1,1, out);
          fwrite(&s2, 1,1, out);
          fwrite(&s3, 1,1, out);
          fwrite(&s4, 1,1, out);

          char buf[4096];
          int rsize=0;
          do
          {
            rsize=fread(buf, 1, 4096, in);
            fwrite(buf, rsize, 1, out);
          } while (rsize);
          t_files++;
          fclose(in);

        }
        else printf("backup: couldn't open %s\n", files_to_backup[i]);
      }

      fclose(out);
      printf("backed up %d files to backup.i4\n", t_files);
    }
    else printf("backup: couldn't backup.i4 for writing\n");    
  }
}

int i4_make_main(int argc, char **argv)
{
  setbuf(stdout, 0);

#ifdef _WINDOWS

  mk_options.os=OS_WIN32;
  _getcwd(abs_path, sizeof(abs_path));

#elif __linux

  mk_options.os=OS_LINUX;
  getcwd(abs_path, sizeof(abs_path));

#else

  #error add new os here 
#endif



  strcat(abs_path, "/");

  mk_options.get(argc, argv);
  
  char *file_start=load_file(mk_options.project_file, 1), *b;
  if (!file_start)
    mk_error("Project file not found : '%s'", mk_options.project_file);


  mk_target_struct target;  
  getcwd(start_path, 256);
          
  for (current_build_type=1; current_build_type<=BUILD_LAST; current_build_type*=2)
  {           
    if (mk_options.build_flags & current_build_type)
    {  
      if (current_build_type==BUILD_BACKUP)
        add_to_backup(mk_options.project_file);

      if (!mk_options.targets_to_build.size())
      {  
        b=file_start;
        int ret;

        while (ret=get_target(target, target, b)!=NO_MORE_TARGETS)
        {
          if (build_target(target, target)==BUILD_ERROR)
            if (!mk_options.continue_on_error)
            {
              perror("Stopping : ");
              exit(-1);
            }
                
          target.reset();
          mk_options.targets_built.clear();
        }
      }

      for (int i=0; i<mk_options.targets_to_build.size(); i++)
      {
        b=file_start;
        if (get_target(target, target, b, mk_options.targets_to_build[i])==NO_MORE_TARGETS)
          mk_error("can't find target '%s' to build\n", mk_options.targets_to_build[i]);
        else if (build_target(target, target)==BUILD_ERROR)
          if (!mk_options.continue_on_error)
            return -1;

        target.reset();
        mk_options.targets_built.clear();
      }

      if (current_build_type==BUILD_CLEAN)
      {
        for (int i=0; i<files_to_clean.size(); i++)
          clean_file(files_to_clean[i]);
        files_to_clean.clear();
      }

    }
  }


  if (mk_options.build_flags & BUILD_BACKUP)
    backup();

  return 0;
}


int main(int argc, char **argv)
{
  return i4_make_main(argc, argv);
}

