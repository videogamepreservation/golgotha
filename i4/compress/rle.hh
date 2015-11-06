/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_RLE_HH
#define I4_RLE_HH

#include "file/file.hh"

template <class T>
class i4_rle_class
{
  T last_value;
  T raw[256];
  int run_count;
  int raw_count;
  i4_file_class *fp;
  
public:
  i4_rle_class(i4_file_class *fp) : fp(fp)
  { 
    run_count=0; 
    raw_count=0;
  }

  void flush()
  {
    if (run_count)
    {
      fp->write_8(run_count);
      *fp << last_value;
      run_count=0;
    }
    else if (raw_count)
    {
      fp->write_8(128 | raw_count);
      for (int i=0; i<raw_count; i++)
        *fp << raw[i];
      raw_count=0;
    }
  }

  void write(T v)
  {
    if (run_count)
    {
      if (last_value==v && run_count!=127)
        run_count++;
      else
      {
        flush();
        raw_count=1;
        raw[0]=v;
      }        
    }
    else if (raw_count>1 && raw[raw_count-1]==raw[raw_count-2] && raw[raw_count-1]==v)
    {
      raw_count-=2;
      run_count=0;
      flush();
      run_count=3;
      last_value=v;
    }
    else 
    {
      if (raw_count==127) flush();
      raw[raw_count++]=v;
    }   
  }

  T read()
  {
    if (run_count)
    {
      run_count--;
      return last_value;
    }
    else if (raw_count)
    {
      raw_count--;
      T ret;
      *fp >> ret;
      return ret;
    }
    else 
    {
      int v=fp->read_8();
      if (v&128)
      {
        raw_count=v&(~128);
      }
      else 
      {
        run_count=v;
        *fp >> last_value;
      }
      return read();
    }
  }

  ~i4_rle_class() { flush(); }
};

#endif
