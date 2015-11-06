/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_FIXED_ARRAY_HH
#define I4_FIXED_ARRAY_HH

// this template class manages an array of object
// it automaically expands when add() is called and not enough elements are
// available.  It also has an array reference operator allowing for transparent
// range checking.

#include "error/error.hh"
#include "memory/malloc.hh"
#include "search.hh"
#include <stdlib.h>

template <class T>
class i4_fixed_array
{
protected:
  T *entry;
  int entries;
public:

  int size() const { return entries; }
  T& operator[](int i) const 
  {
    I4_ASSERT(i>=0 && i<entries, "i4_array::bad array reference");
    return entry[i]; 
  }    

  T& operator()(int i) const 
  {
    I4_ASSERT(i>=0 && i<entries, "i4_array::bad array reference");
    return entry[i]; 
  }    

  void resize(int new_size)
  {
    entry = (T*)i4_realloc(entry, sizeof(T) * new_size,"grow array");
    if (new_size)
      I4_ASSERT(entry, "i4_array::can't allocate entries");

    entries=new_size;
  }

  i4_fixed_array(int entries=0) : entries(entries)
  {
    entry=0;
    if (entries)
      resize(entries);
  }

  void uninit()     // frees memory (use clear just to reset)
  {
    if (entry)
      i4_free(entry);
    entry=0;
    entries=0;
  }
  
  ~i4_fixed_array() 
  {
    uninit();
  }
    

  void sort(int (*compar)(const T *, const T *))
  {
    typedef int (*compare_type)(const void *x, const void *y);
    qsort(entry, size(), sizeof(T), (compare_type)compar);
  }

  int binary_search(const T *find, int (*compar)(const T* a, const T* b))
  {
    if (size()==0) return -1;

    w32 res;

    if (i4_base_bsearch(find, res, entry, sizeof(T), (w32)size(), 
                        (i4_bsearch_compare_function_type)compar))
      return res;
    else
      return -1;                   
  }

};

#endif
