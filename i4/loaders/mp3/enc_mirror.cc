/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "main/main.hh"
#include "error/alert.hh"
#include "error/error.hh"
#include "file/file.hh"
#include "string/str_checksum.hh"
#include "memory/malloc.hh"
#include "loaders/mp3_load.hh"
#include "loaders/wav_load.hh"
#include "status/status.hh"
#include <stdio.h>

i4_file_class *decomp_script=0;

void copy_file(char *f1, char *f2)
{
  i4_file_class *out=i4_open(f2, I4_WRITE);
  if (!out)
  {
    i4_warning("Could not open output file %s", f2);
    return;
  }

  i4_file_class *in=i4_open(f1);
  if (!in)
  {
    i4_warning("Could not open output file %s", f1);
    return;
  }

  printf("copying %s -> %s\n", f1, f2);

  char buf[4096];
  int size=in->size();
  while (size)
  {
    int t=in->read(buf, sizeof(buf));
    out->write(buf, t);
    size-=t;
  }


  delete in;
  delete out;
}

void encode_mp3(char *s, char *d)
{
  char cmd[256];
  printf("encoding %s -> %s\n", s,d);

  if (decomp_script)
    decomp_script->printf("(mp3_2_wav %s)\n",d);

  sprintf(cmd, "mp3enc -esr 32 -if %s -of %s -qual 9 > NUL", s,d);
  system(cmd);
}

void mcopy(const i4_const_str &s, const i4_const_str &d)
{
  i4_directory_struct df,sf;

  if (i4_get_directory(s, sf, i4_T))
  {
    char src_path[256], dest_path[256];
    i4_os_string(s, src_path, 256);
    i4_os_string(d, dest_path, 256);

    w8 *up_to_date=0;
    if (sf.tfiles)
    {
      up_to_date=(w8 *)i4_malloc(sf.tfiles * sizeof(w8), "");
      memset(up_to_date, 0, sf.tfiles * sizeof(w8));

      i4_mkdir(d);
      if (i4_get_directory(d, df, i4_T))
      {
        for (int i=0; i<sf.tfiles; i++)
        {
          for (int j=0; j<df.tfiles; j++)
            if (*sf.files[i]==*df.files[j])
              up_to_date[i]=1;
        }
      }
    }


    for (int i=0; i<sf.tfiles; i++)
    {
      if (!up_to_date[i])
      {
        i4_filename_struct fs;
        i4_split_path(*sf.files[i], fs);
        char src_fn[256], dst_fn[256];
        sprintf(src_fn, "%s/%s.%s", src_path, fs.filename, fs.extension);

        if (strcmp(fs.extension, "wav")==0 &&
            strstr(fs.filename, "_lp")==0)
        {
          sprintf(dst_fn, "%s/%s.mp3", dest_path, fs.filename);
          encode_mp3(src_fn, dst_fn);
        }
        else
        {
          sprintf(dst_fn, "%s/%s.%s", dest_path, fs.filename, fs.extension);
          copy_file(src_fn, dst_fn);
        }

      }
    }

    if (sf.tfiles)
      i4_free(up_to_date);


    for (int j=0; j<sf.tdirs; j++)
    {
      char dir[256], new_src_path[256], new_dst_path[256];
      i4_os_string(*sf.dirs[j], dir, 256);
      if (!(strcmp(dir,".")==0 || strcmp(dir,"..")==0))
      {
        sprintf(new_src_path, "%s/%s", src_path, dir);
        sprintf(new_dst_path, "%s/%s", dest_path, dir);

        mcopy(new_src_path, new_dst_path);
      }
    }


  }
}

void i4_main(w32 argc, i4_const_str *argv)
{
  i4_init();

  if (argc>2)
  {   
    if (argc==3)
      decomp_script=i4_open(argv[3],I4_WRITE);
    
    mcopy(argv[1], argv[2]);

    if (decomp_script)
      delete decomp_script;
  }
  else
    printf("Ussage : source_directory directory_to_mirror_to\n");
}

