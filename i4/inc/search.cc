/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "inc/search.hh"

i4_bool i4_base_bsearch(const void *member, w32 &loc,
                        const void *array, w32 member_size, w32 size, 
                        int (*compare)(const void *a, const void *b))
{
  w32 l=0,r=size,m;
  int c;

  while (l<r)
  {
    m = (l+r)/2;

    if ((c = compare(member, (void*)((w8*)array+member_size*m)))==0)
    {
      loc = m;
      return i4_T;
    }
    else if (c<0)
      r = m;
    else
      l = m+1;
  }
  loc = l;
  if (l<size && compare(member, (void*)((w8*)array+member_size*l))==0)
    return i4_T;
  return i4_F;
}
