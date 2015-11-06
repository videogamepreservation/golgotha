/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "string/str_checksum.hh"

w32 i4_str_checksum(const i4_const_str &str)
{
  i4_const_str::iterator i=str.begin();

  int len=str.length();
  
  w8 c1=0,c2=0,c3=0,c4=0;

  while (len)
  {
    c1+=i.get().value();
    c2+=c1;
    c3+=c2;
    c4+=c3;
    ++i;
    len--;
  }

  return (c1|(c2<<8)|(c3<<16)|(c4<<24));
}
