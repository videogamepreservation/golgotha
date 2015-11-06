/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_QUE_HH
#define I4_QUE_HH

#include "arch.hh"

template <class T, w32 SIZE>
class i4_fixed_que
{
protected:
  T data[SIZE];
  w32 head, tail;
  
public:
  void reset() { head=tail=0; }

  i4_fixed_que() { reset(); }

  i4_bool empty() { return (i4_bool)(head==tail); }

  i4_bool full() { return (i4_bool)((head==SIZE-1 && tail==0) || head+1==tail); }

  i4_bool que(const T &object) 
  {
    w32 next_head=(head+1);
    if (next_head==SIZE) 
      next_head=0;

    if (next_head==tail)
      return i4_F;

    data[head]=object;
    head=next_head;
    return i4_T;
  }

  i4_bool deque(T &object)
  {
    if (empty())
      return i4_F;

    object=data[tail];
    tail++;
    if (tail==SIZE)
      tail=0;   
    return i4_T;
  }

  i4_bool front(T &object)
  {
    if (empty())
      return i4_F;

    object=data[tail];
    return i4_T;
  }
};

#endif
