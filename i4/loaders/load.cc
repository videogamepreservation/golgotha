/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "loaders/load.hh"
#include "memory/malloc.hh"
#include "file/file.hh"
#include "image/image.hh"
#include "error/alert.hh"

#include <stdlib.h>

static i4_image_loader_class *loader_list=NULL;
static w16 max_header_need=0;

void i4_image_loader_class::init()
{
  i4_image_loader_class *who=this;

  who->next=loader_list;
  loader_list=who;
  w16 mh=who->max_header_size();
  if (mh>max_header_need)
    max_header_need=mh;
}

void close_file(i4_file_class *fp)
{
  delete fp;
}

i4_image_class *i4_load_image(const i4_const_str &filename,
                              i4_status_class *status)
{
  i4_file_class *fp;
  fp=i4_open(filename);

  if (!fp)
    return NULL;
  
  i4_image_class *im=i4_load_image(fp, status);
  if (!im)
    i4_alert(i4gets("load_failed"),200,&filename);

  delete fp;
  return im;
}


i4_image_class *i4_load_image(i4_file_class *fp,
                              i4_status_class *status)
{
  if (max_header_need==0)   // cannot identify any formats
    return NULL;

  i4_image_loader_class *il=loader_list;
  i4_image_class *im=0;
  

  w8 *buf=(w8 *)i4_malloc(max_header_need,"sig buf");
  w16 size_read=fp->read(buf,max_header_need);
 
  for (;!im && il;il=il->next)
  {
    if (size_read>=il->max_header_size())
    {
      if (il->recognize_header(buf))
      {
	fp->seek(0);
	im=il->load(fp, status);
      }
    }    
  }

  i4_free(buf);
  return im;
}
