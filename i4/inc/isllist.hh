/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

//{{{ Intrusive singly linked list
//
//  Notes:
//    The templatized class MUST have member:
//      T* next;
//

#ifndef ISLLIST_HPP
#define ISLLIST_HPP
#include "arch.hh"      // for definition of i4_bool

template <class T>
class i4_isl_list
{
protected:
  typedef T* link;
  link list;

public:
  T *first() { return list; }
  i4_isl_list() : list(0) {}

  class iterator
  {
    friend class i4_isl_list<T>;
  protected:
    link node;

  public:
    iterator() : node(0) {}
    iterator(const iterator &p) : node(p.node) {}
    iterator(link p) : node(p) {}

    int operator==(const iterator &p) const { return (node == p.node); }
    int operator!=(const iterator &p) const { return (node != p.node); }
    
    T& operator*() const { return *node; }
    T* operator->() const { return node; }

    iterator& operator++()
    {
      node = node->next;
      return *this;
    }

    iterator& operator++(int)
    {
      node = node->next;
      return *this;
    }
  };
  
  iterator end()   const { return 0; }
  iterator begin() const { return list; }

  i4_bool empty() const { return begin() == end(); }

  iterator insert_after(iterator _pos, T &item)
  {
    item.next = (*_pos.node).next;
    _pos.node->next = &item;

    return &item;
  }
  void erase_after(iterator _pos)
  {
    _pos.node->next = _pos.node->next->next;
  }
  
  void insert_end(T &item)
  {
    item.next=0;
    if (!list)
      list=&item;
    else
    {
      link last=list;
      while (last->next)
        last=last->next;
      last->next=&item;
    }    
  }
  
  void insert(T& item) { item.next = list; list = &item; }
  void erase() { list = list->next; }

  void destroy_all()
  {
    link p;
    
    while (list)
    {
      p = list;
      erase();
      delete p;
    }
  }
  
  iterator find(T *item)
  {
    iterator p=begin();
    for (;p!=end();++p)
      if (p.node==item)
        return p;
    return end();
  }
  
  void swap(i4_isl_list &other)
  {
    link other_list=other.list;
    other.list=list;
    list=other_list;
  }
  
  T *find_and_unlink(T *item)
  {
    iterator p=begin(), last=end();
    for (;p!=end(); ++p)
      if (p.node==item)
      {      
        if (last!=end())
          erase_after(last);
        else
          erase();    
    
        return item;
      }
      else last=p;
    return 0;
  }
};

#endif

