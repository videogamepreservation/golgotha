/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "file/buf_file.hh"
#include "memory/malloc.hh"
#include "error/error.hh"
#include <memory.h>



w32 i4_buffered_file_class::read(void *buffer, w32 size)
{
  w32 total_read=0;
  while (size)
  {
    if (offset>=buf_start && offset<buf_end)
    {
      w32 copy_size;
      if (buf_end-offset<size)
        copy_size=buf_end-offset;
      else copy_size=size;
      memcpy(buffer,((w8 *)buf)+offset-buf_start,copy_size);
      size-=copy_size;
      buffer=(void *)(((w8 *)buffer)+copy_size);
      offset+=copy_size;
      total_read+=copy_size;
    } else if (offset==buf_end)      // sequentially read more into the buffer
    {
      buf_start=offset;
      buf_end=offset+from->read(buf,buf_size);
      if (buf_end==buf_start)
        return total_read;
    } else                          // need to seek from file to a new spot
    {
      from->seek(offset);
      buf_start=buf_end=offset;
    }
  }
  return total_read;
}


w32 i4_buffered_file_class::write(const void *buffer, w32 size)
{
  w32 total_write=0;

  while (size)
  {      
    write_file=i4_T;
    if (offset>=buf_start && offset<=buf_end)
    {
      w32 copy_size;
      if (offset+size<buf_start+buf_size)
        copy_size=size;
      else
        copy_size=buf_start+buf_size-offset;

      memcpy(((w8 *)buf)+offset-buf_start,buffer,copy_size);

      size-=copy_size;
      buffer=(void *)(((w8 *)buffer)+copy_size);
      offset+=copy_size;
      total_write+=copy_size;

      if (offset>buf_end)
      {
        buf_end=offset;
        if (buf_end-buf_start==buf_size)
        {
          from->write(buf, buf_end-buf_start);
          buf_start=buf_end;
        }
      }

    }
    else if (buf_end!=buf_start)      // flush the buffer
    {
      from->write(buf, buf_end-buf_start);
      buf_start=buf_end;
    } else
    {
      from->seek(offset);
      buf_start=buf_end=offset;
    }
  }

  return total_write;
}


w32 i4_buffered_file_class::seek (w32 offset)
{
  i4_buffered_file_class::offset=offset;
  return offset;
}


w32 i4_buffered_file_class::size ()
{
  return from->size();
}


w32 i4_buffered_file_class::tell ()
{
  return offset;
}


i4_buffered_file_class::~i4_buffered_file_class()
{
  if (write_file && buf_start!=buf_end)
    from->write(buf, buf_end-buf_start);

  delete from;
  i4_free(buf);
}


i4_buffered_file_class::i4_buffered_file_class(i4_file_class *from, 
                                               w32 buffer_size,
                                               w32 current_offset)
  : from(from), buf_size(buffer_size), offset(current_offset)
{
  write_file=i4_F;
  buf=i4_malloc(buf_size,"file buffer");
  buf_start=buf_end=0;
}

struct callback_context
{
  i4_bool in_use;
  w32 prev_read;
  void *prev_context;
  i4_file_class::async_callback prev_callback;
  i4_buffered_file_class *bfile;
} ;


// Maximum number async reads going on at the same time
enum { MAX_ASYNC_READS = 4 };
static callback_context contexts[MAX_ASYNC_READS];
static w32 t_callbacks_used=0;

void i4_async_buf_read_callback(w32 count, void *context)
{
  callback_context *c=(callback_context *)context;
  c->bfile->offset+=count;

  i4_file_class::async_callback call=c->prev_callback;
  count += c->prev_read;
  void *ctext=c->prev_context;
  c->in_use=i4_F;

  t_callbacks_used--;
  
  call(count, ctext);
}



i4_bool i4_buffered_file_class::async_read (void *buffer, w32 size, 
                                            async_callback call,
                                            void *context)
{
  if (!(offset>=buf_start && offset<buf_end))
  {
    from->seek(offset);
    buf_start=buf_end=0;
  }


  if (t_callbacks_used>=MAX_ASYNC_READS)
    return i4_file_class::async_read(buffer, size, call, context);
  else
  {    
    w32 avail_size;

    if (offset>=buf_start && offset<buf_end)    
      avail_size=buf_end-offset;
    else
      avail_size=0;

    if (avail_size < size)
    {
      callback_context *c=0;
      for (w32 i=0; !c && i<MAX_ASYNC_READS; i++)
        if (!contexts[i].in_use)
        {
          c=contexts+i;
          c->in_use=i4_T;
        }

      if (c==0)
        i4_error("didn't find a free context");

      t_callbacks_used++;

      if (avail_size)
        c->prev_read=read(buffer,avail_size);
      else
        c->prev_read=0;

      c->prev_context=context;
      c->prev_callback=call;
      c->bfile=this;
      return from->async_read((w8 *)buffer + avail_size, size-avail_size, 
                              i4_async_buf_read_callback, c);
    }
    else
    {
      call(read(buffer,avail_size), context);
      return i4_T;
    }

  }
}


//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}

