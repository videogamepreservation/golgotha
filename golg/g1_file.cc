/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "error/alert.hh"
#include "string/str_checksum.hh"
#include "threads/threads.hh"
#include "file/file.hh"
#include "memory/array.hh"
#include "file/file_man.hh"
#include "main/main.hh"
#include "checksum/checksum.hh"

i4_critical_section_class cd_file_lock;
static char *cd_image="golgotha.cd";


struct g1_dir_entry
{
  w32 offset;
  w32 checksum;
  w32 length;
};


int g1_dir_entry_compare(const g1_dir_entry *a, const g1_dir_entry *b)
{
  if (a->checksum<b->checksum)
    return -1;
  else if (a->checksum>b->checksum)
    return 1;
  else return 0;
}


// this is the only file that is actually used by the game, the rest is just seeking
i4_file_class *g1_single_file=0;


void g1_cd_file_callback(w32 count, void *context);

class g1_cd_file : public i4_file_class
{
public:
  w32 offset;
  w32 end_offset;
  w32 start_offset;
  i4_file_class::async_callback callback;
  void *context;
  i4_file_class *use_file;


  virtual w32 read (void *buffer, w32 size)
  {
    cd_file_lock.lock();

    if (use_file->tell()!=offset)
      use_file->seek(offset);
    if (offset+size>end_offset)
      size=end_offset-offset;

    w32 ret=use_file->read(buffer,size);
    offset+=ret;

    cd_file_lock.unlock();
    return ret;
  }

  virtual w32 write(const void *buffer, w32 size)
  {
    return 0;
  }

  virtual w32 seek (w32 _offset) 
  {
    offset=start_offset+_offset;
    if (offset>end_offset)
      offset=end_offset;

    return offset;
  }

  virtual w32 size ()
  {
    return end_offset-start_offset;
  }

  virtual w32 tell ()
  {
    return offset-start_offset;
  }

  ~g1_cd_file()
  {
    if (use_file!=g1_single_file)
      delete use_file;
  }

  g1_cd_file(w32 start, w32 length, i4_file_class *fp)
  {
    use_file=fp;
    start_offset=start;
    offset=start_offset;
    end_offset=start_offset+length;
  }

};

void g1_cd_file_callback(w32 count, void *context)

{
  cd_file_lock.unlock();
  ((g1_cd_file *)context)->callback(count, ((g1_cd_file *)context)->context);
}



class g1_file_manager_class : public i4_file_manager_class
{
  i4_array<g1_dir_entry> entries;
  char current_cd_file[100];

public:
  int g1_file_manager_class::find_checksum(w32 id)
  {
    g1_dir_entry find; 
    find.checksum=id;
    return entries.binary_search(&find, g1_dir_entry_compare);    
  }

  virtual i4_file_class *open(const i4_const_str &name, w32 flags)
  {
    if (flags & (I4_WRITE|I4_APPEND|I4_SUPPORT_ASYNC))
      return 0;

    char tmp[256];
    int k=0;

    // convert slashes to a commonf format
    for (i4_const_str::iterator i=name.begin(); i!=name.end(); )
    {
      int c=i.get().value();
      if (c=='\\')
        tmp[k++]='/';
      else
        tmp[k++]=c;
      ++i;
    }


    int handle=find_checksum(i4_check_sum32(tmp,k));
    if (handle>=0)
    {
      if (flags & I4_SUPPORT_ASYNC)
      {
        i4_file_class *fp=i4_open(current_cd_file, flags);
        return new g1_cd_file(entries[handle].offset, entries[handle].length, fp);
      }

      return new g1_cd_file(entries[handle].offset, entries[handle].length, g1_single_file);
    }
    else return 0;
  }

  void set_cd_file()
  {
    if (cd_image)
    {
      g1_single_file=i4_open(cd_image);
      if (!g1_single_file)
        i4_warning("could not open cd image file");
      else
      {  
        strcpy(current_cd_file, cd_image);

        int tfiles = g1_single_file->read_32();

        for (int i=0; i<tfiles; i++)
        {
          g1_dir_entry *e=entries.add();
          e->offset=g1_single_file->read_32();
          e->checksum=g1_single_file->read_32();
          e->length=g1_single_file->read_32();
        }


        int netfirst=0;
        for (int j=1; j<i4_global_argc; j++)
          if (i4_global_argv[j]=="-netfirst")
            netfirst=1;

        if (netfirst)
          i4_add_file_manager(this, i4_F);
        else
          i4_add_file_manager(this, i4_T);
      }
    }
  }


  void init()
  {  
    set_cd_file();
  }

  void uninit()
  {
    entries.uninit();
    if (g1_single_file)
    {
      i4_remove_file_manger(this);
      delete g1_single_file;
    }
  }

  g1_file_manager_class()
    : entries(0,512)
  {
  }

} g1_file_manager_class_instance;



void g1_set_cd_image(char *filename)
{
  cd_image=filename;
  g1_file_manager_class_instance.set_cd_file();
}
