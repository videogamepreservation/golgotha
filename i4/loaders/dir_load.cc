/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "loaders/dir_load.hh"
#include "checksum/checksum.hh"
#include "memory/malloc.hh"
#include "file/buf_file.hh"
#include "file/sub_section.hh"
#include <stdlib.h>

#include <string.h>


i4_bool i4_insert_sections(i4_file_class *in, i4_file_class *out,
                           int total_sections_to_insert,
                           i4_file_class **section_data,
                           char **section_names)
{
  w32 sig=in->read_32(); 
  if (sig!=i4_check_sum32("GOLG_SECTION_ID=32",17))
    i4_error("doesn't work on this");
  
  w32 t_sections=in->read_32();
  int copy_size=in->size()-8+t_sections*8;

  out->write_32(sig);
  out->write_32(t_sections + total_sections_to_insert);

  int i;
  for (i=0; i<t_sections; i++)
  {
    out->write_32(in->read_32());    // copy id unchanged
    int offset=in->read_32();
    out->write_32(offset + total_sections_to_insert * 8);
  }

  int offset2 = copy_size + out->tell() + total_sections_to_insert*8;

  for (i=0; i<total_sections_to_insert; i++)
  {
    out->write_32(i4_check_sum32(section_names[i], strlen(section_names[i])));
    out->write_32(offset2);
    offset2+=section_data[i]->size();
  }

  w8 buf[4096];
  while (copy_size)
  {
    int rs=sizeof(buf) > copy_size ? copy_size : sizeof(buf);
    in->read(buf, rs);
    out->write(buf, rs);
    copy_size-=rs;
  }
  

  for (i=0; i<total_sections_to_insert; i++)
  {
    int fsize=section_data[i]->size();
    while (fsize)
    {
      int rs=sizeof(buf) > fsize ? fsize : sizeof(buf);
      section_data[i]->read(buf, rs);
      out->write(buf, rs);
      fsize-=rs;
    }
  }  

  return i4_T;
}


i4_loader_class *i4_open_save_file(i4_file_class *in, i4_bool close_on_delete_or_fail)
{
  i4_loader_class *l=new i4_loader_class(in, close_on_delete_or_fail);

  if (l->error())
  {
    delete l;
    return 0;
  }

  return l;
}


int i4_loader_section_compare(const i4_loader_section_type *a, const i4_loader_section_type *b)
{
  return a->section_id>b->section_id ? 1 : (a->section_id<b->section_id ? -1 : 0);
}

i4_loader_class::i4_loader_class(i4_file_class *_in, i4_bool close_on_delete)
  : close_on_delete(close_on_delete),
    sections(512,512)
{
  seek_to_before_using=-1;


  in=new i4_buffered_file_class(_in, 0x1000, _in->tell());

  _error=0;

  w32 sig=in->read_32(); 
  int use_16;

  if (sig==i4_check_sum32("GOLG",4))
    use_16=1;
  else if (sig==i4_check_sum32("GOLG_SECTION_ID=32",17))
    use_16=0;
  else             // this is not a directoried file
  {
    _error=1;
    return ;
  }

  w32 t_sections=in->read_32(), i;

  last=0x7fffffff;

  for (i=0; i<t_sections; i++)
  {
    i4_loader_section_type *t=sections.add();
    if (use_16)
      t->section_id=in->read_16();
    else
      t->section_id=in->read_32();

    t->section_offset=in->read_32();
  }


  if (t_sections)
  {
    int offset=sections[0].section_offset;
    for (i=0; i<t_sections-1; i++)
      sections[i].section_size=sections[i+1].section_offset-sections[i].section_offset;

    sections[t_sections-1].section_size=in->size()-sections[t_sections-1].section_offset;
  }


  sections.sort(i4_loader_section_compare);
}

int i4_loader_class::find_section(w32 section_id)
{
  sw32 lo=0,hi=sections.size()-1,mid;

  mid=(lo+hi+1)/2;
  if (last<hi && sections[last+1].section_id==section_id)
    mid=last+1;
  else
  {
    while (sections[mid].section_id!=section_id)
    {
      if (sections[mid].section_id>section_id)
        hi=mid-1;
      else
        lo=mid+1;

    
      w32 last_mid=mid;
      mid=(hi+lo)/2;

      if (last_mid==mid)
        return -1;

    }
  }

  last=mid;

  return mid;
}


i4_bool i4_loader_class::goto_section(w32 section_id)
{
  int mid=find_section(section_id);
  if (mid==-1)
    return i4_F;

  if (in->tell()!=sections[mid].section_offset)
  {
    i4_warning("goto section : non-linear %d", section_id);
    in->seek(sections[mid].section_offset);
  }
  return i4_T;
}

i4_bool i4_loader_class::get_section_info(char *section_name, w32 &offset, w32 &size)
{
  w32 id=i4_check_sum32(section_name, strlen(section_name));
  int mid=find_section(id);
  if (mid!=-1)
  {
    offset=sections[mid].section_offset;
    size=sections[mid].section_size;
    return i4_T;
  }
  else return i4_F;

}

i4_bool i4_loader_class::goto_section(char *section_name)
{
  return goto_section(i4_check_sum32(section_name, strlen(section_name)));
}

static w32 i4_check_end=0;
void i4_loader_class::get_version(w16 &version, w16 &data_size)
{  
  version   = in->read_16();
  data_size = in->read_16();  


  //data_size is the size of the data + 4 bytes for the version and data size variables
  //since data_size is used to seek past the data, and we already read 4 bytes for
  //the version and data_size, return the data_size - 4
  data_size -= 4;

  i4_check_end=in->tell() + data_size;
}


i4_bool i4_loader_class::check_version(w16 version)
{
  w16 ver,add;
  ver=in->read_16();
  add=in->read_16();
  
  i4_check_end=in->tell()+add-4;

  if (ver!=version)
  {
    in->seek(i4_check_end);
    return i4_F;
  }
  return i4_T;
}


void i4_loader_class::end_version(I4_LF_ARGS)
{
  if (in->tell()!=i4_check_end)
    i4_warning("end_check_version did not read proper amount %s:%d", I4_FILE, I4_LINE);
}

w32 i4_loader_class::size ()
{ return in->size(); }

w32 i4_loader_class::tell () 
{ return in->tell(); }


w32 i4_loader_class::seek(w32 offset) 
{ 
  return in->seek(offset);
}

  
w32 i4_loader_class::read (void *buffer, w32 size)
{
  return in->read(buffer, size);
}


w32 i4_loader_class::write(const void *buffer, w32 size)
{
  i4_error("Bad bad");
  return 0;
}

i4_loader_class::~i4_loader_class()
{  
  if (close_on_delete && in)
    delete in;
}
