/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __REF_QUE_HPP_
#define __REF_QUE_HPP_

template <class T>
class i4_reference_que_class
{
  class node
  {
    friend class i4_reference_que_class<T>;
    T    *reference;
    node *last;
    public :
    node(T *ref, node *last) : reference(ref), last(last) {}
  } *first,*last;

  public :
  i4_reference_que_class()
  {
    first=last=0;
  }

  void enque(T *reference)
  {
    first=new node(reference,first);
    if (!last)
      last=first;
  }

  T *deque()
  {
    if (!last)       
      return 0;
    else
    {
      node *del=last;
      T    *p  =last->reference;
      last=last->last;
      delete del;
      if (!last)
        first=0;
      
      return p;
    }
  }
} ;

#endif
