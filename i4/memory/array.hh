/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_ARRAY_HH
#define I4_ARRAY_HH

// this template class manages an array of object
// it automaically expands when add() is called and not enough elements are
// available.  It also has an array reference operator allowing for transparent
// range checking.

#include "error/error.hh"
#include "memory/malloc.hh"
#include "search.hh"
#include <stdlib.h>

template <class T>
class i4_array
{
protected:
  T *entry;
  int used,entries,grow;
public:

  int size() const { return used; }
  int max_size() const { return entries; }
  T& operator[](int i) const 
  {
    I4_ASSERT(i>=0 && i<used, "i4_array::bad array reference");
    return entry[i]; 
  }    

  T& operator()(int i) const 
  {
    I4_ASSERT(i>=0 && i<used, "i4_array::bad array reference");
    return entry[i]; 
  }    

  void resize(int _entries, int _grow = -1)
  {
    if (_grow>=0)
      grow = _grow;

    entries = _entries;
    T* new_entry = (T*)i4_realloc(entry, sizeof(T)*entries, "grow array");
    I4_ASSERT(new_entry, "i4_array::out of memory");
    entry = new_entry;
  }

  i4_array(int entries, int grow = 0) : entries(entries), grow(grow), entry(0), used(0)
  {
    if (entries>0)
    {
      entry = (T*)i4_malloc(sizeof(T)*entries,"grow array");
      I4_ASSERT(entry, "i4_array::can't allocate entries");
    }
    else
      entry = 0;
  }

  void uninit()     // frees memory (use clear just to reset)
  {
    if (entry)
      i4_free(entry);
    entry=0;
    used=0;
    entries=0;
  }
  
  ~i4_array() 
  {
    uninit();
  }

  void grow_bigger()
  {
    if (grow)
    {
      entries += grow;

      T* new_entry = (T*)i4_realloc(entry, sizeof(T)*entries, "grow array");
        
      I4_ASSERT(new_entry, "i4_array::out of memory");

      entry = new_entry;
    }
    else
      I4_ASSERT(0, "i4_array::out of entries");
  }
  
  T *add_at(int ref)
  {
    if (used==entries)
      grow_bigger();

    for (int i=used; i>ref; i--)
      entry[i] = entry[i-1];
    used++;
    return entry+ref;    
  }

  T *add()
  {
    if (used==entries)
      grow_bigger();

    T *ret=entry+used;
    used++;
    return ret;
  }

  T* add_many(int num)
  {
    while (used+num>entries)
      grow_bigger();

    T *ret=entry+used;
    used+=num;
    return ret;
  }

  int add_at(T item, int ref)
  {
    T *q=add_at(ref);
    *q=item;
    return ref;
  }

  int add(T item)
  {
    T *q=add();
    *q=item;
    return used-1;
  }
  
  int add_array(const i4_array& tab,int ref = -1)
  {
    if (ref<0)
      ref += used+1;

    I4_ASSERT(ref>=0 && ref<=used,"i4_array::bad item referenced");

    if (used+tab.size() >= entries)
    {
      if (grow)
      {
        if (used+tab.size() >= entries+grow)
          entries = used+tab.size();
        else
          entries += grow;

        T* new_entry = (T*)realloc(entry, sizeof(T)*entries);
        
        I4_ASSERT(new_entry, "i4_array::out of memory");

        entry = new_entry;
      }
      else
        I4_ASSERT(0, "i4_array::out of entries");
    }

    int i;

    for (i=used-1; i>ref; i--)
      entry[i+tab.size()] = entry[i];
    for (i=0; i<tab.size(); i++)
      entry[ref+i] = tab.entry[i];

    used+=tab.size();

    return ref;
  }
  
  void remove(int ref)
  {
    I4_ASSERT(ref>=0 && ref<used, "i4_array::bad item deletion");

    used--;
    for (int i=ref; i<used; i++)
      entry[i] = entry[i+1];
  }
  
  void clear()
  {
    used = 0;
  }

  void sort(int (*compar)(const T *, const T *))
  {
    typedef int (*compare_type)(const void *x, const void *y);

    qsort(entry, used, sizeof(T), (compare_type)compar);
  }

  int binary_search(const T *find, int (*compar)(const T* a, const T* b))
  {
    if (size()==0) return -1;

    w32 res;

    if (i4_base_bsearch(find, res, entry, sizeof(T), (w32)size(), (i4_bsearch_compare_function_type)compar))
      return res;
    else
      return -1;                   
  }

};

#endif
