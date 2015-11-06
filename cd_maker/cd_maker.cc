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
#include <stdio.h>

struct cd_file
{
    
  i4_str *name;
  w32 length;
  w32 checksum;

  void add(const i4_const_str &str)
  {
    char tmp[256];
    i4_os_string(str, tmp,256);
    for (char *c=tmp; *c; c++)
      if (*c=='\\')
        *c='/';
    
    name=new i4_str(tmp);
    i4_file_class *fp=i4_open(str);
    I4_ASSERT(fp,"no fp");
    length=fp->size();
    delete fp;
  }

} flist[10000];
 

int flist_compare(const void *a, const void *b)
{
  if (((cd_file *)a)->checksum > ((cd_file *)b)->checksum)
    return 1;
  else if (((cd_file *)a)->checksum < ((cd_file *)b)->checksum)
    return -1;
  else return 0;

}

void i4_main(w32 argc, i4_const_str *argv)
{
  i4_init();



  if (argc>1)
    i4_string_man.load(argv[1]);
  else
    i4_string_man.load("..\cd_maker\resource.res");

  i4_debug->printf("Building CD image golgotha.cd");
  

  i4_const_str *files=i4_string_man.get_array("cd_files");

 
  
  int i, tfiles=0,j;

  for (i=0; !files[i].null(); i++)
  {
    i4_directory_struct d;
    if (i4_get_directory(files[i], d))
    {
      for (j=0; j<d.tfiles; j++)
      {
        i4_str *r=i4_const_str("%S/%S").sprintf(150,&files[i],d.files[j]);

        flist[tfiles++].add(*r);
        delete r;
      }
    }
    else
    {
      i4_file_status_struct return_stat;
      if (i4_get_status(files[i], return_stat))              
        flist[tfiles++].add(files[i]);
      else
        i4_alert("File does not exsist : %S",100,&files[i]);
    }
  }

  i4_file_class *out=i4_open("golgotha.cd", I4_WRITE);
  if (!out)
    i4_error("could not open output file");


  out->write_32(tfiles);

  w32 offset=4 +             // total files
             (12 * tfiles);  // directory

  for (i=0; i<tfiles; i++)
    flist[i].checksum = i4_str_checksum(*flist[i].name);
  
  qsort(flist, tfiles, sizeof(flist[0]), flist_compare);


  for (i=0; i<tfiles; i++)
  {    
    out->write_32(offset);
    offset+=flist[i].length;

    if (i!=0 && flist[i-1].checksum==flist[i].checksum)
      i4_error("2 files have same checkum");

    out->write_32(flist[i].checksum);
    out->write_32(flist[i].length);      
  }

  int tsize=0;
  for (i=0; i<tfiles; i++)
  {
    i4_file_class *fp=i4_open(*flist[i].name);
    
    if(fp)
    {
      int off=out->tell();

      char fn[100];
      i4_os_string(*flist[i].name, fn, 100);
      printf("%d bytes Adding '%s' checksum = %x, off=%d\n",
             flist[i].length,  fn, flist[i].checksum, off);

      tsize+=flist[i].length;

      if (flist[i].length)
      {
        w8 buf[64*1024];

        int size=flist[i].length;
        while (size)
        {
          int to_read=size>sizeof(buf) ? sizeof(buf) : size;

          if (fp->read(buf, to_read)!=to_read)
            i4_error("file read premature");
                    
          if (out->write(buf, to_read)!=to_read)
            i4_error("write premature, out of space?");

          size-=to_read;
        }
      }

      delete fp;
    }
    else 
    {
      char buf[100];
      i4_os_string(*flist[i].name, buf, 100);
      i4_error("%s : file not found", buf);
    }
  }
  delete out;

  printf("Total size= %d\n", tsize);


  i4_free(files);

  i4_uninit();
}


