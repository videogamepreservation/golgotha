/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "arch.hh"
#include "memory/malloc.hh"
#include <string.h>


class i4_bit_array_class
{
  w8 static_data[8];
  w8 *data;
  public :
  i4_bit_array_class(w32 length=32, void *bit_data)
  {
    if (length<=64) data=static_data;
    else data=(w8 *)i4_malloc((length+7)/8,"bit array");
    memcpy(data,bit_data,(length+7)/8);
  }

  i4_bit_array_class(w32 length=32)
  {
    if (length<=64) data=static_data;
    else data=(w8 *)i4_malloc((length+7)/8,"bit array");
    memset(data,0,(length+7)/8);
  }

  i4_bool get(w32 x)
  {
    if (data[x/8]&(1<<(x%8)))
      return i4_T;
    else return i4_F;
  }

  void set(w32 x, i4_bool value)
  {
    if (value==i4_F)
      data[x/8]&=~(1<<(x%8));
    else data[x/8]|=(1<<(x%8));
  }

  ~i4_bit_array_class() 
  {
    if (static_data!=data)
      i4_free(data);
  }
} ;
