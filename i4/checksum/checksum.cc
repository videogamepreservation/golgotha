/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "arch.hh"

w32 i4_check_sum32(void *buf, w32 buf_len)
{
  w8 c1=0,c2=0,c3=0,c4=0;
  while (buf_len)
  {
    c1+=*((w8 *)buf);
    c2+=c1;
    buf=(void *)(((w8 *)buf)+1);
    c3+=c2;
    c4+=c3;
    buf_len--;
  }
  return (c1|(c2<<8)|(c3<<16)|(c4<<24));
}

w16 i4_check_sum16(void *buf, w32 buf_len)
{
  w8 c1=0,c2=0;

  while (buf_len)
  {
    c1+=*((w8 *)buf);
    buf=(void *)(((w8 *)buf)+1);
    c2+=c1;
  }
  return (c1|(c2<<8));
}
