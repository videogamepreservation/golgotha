/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_INT_SQRT
#define I4_INT_SQRT


// From Graphics Gems II. p 387

inline w32 i4_int_sqrt(w32 n)
{
  w32 next=n/2, current;
  if (n<=1)
    return n;
  do
  {
    current=next;
    next=(next+n/next)/2;
  } while (next<current);
  return current;
}


#endif
