/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_GROW_ARRAY_HH
#define I4_GROW_ARRAY_HH


#include "arch.hh"
#include "memory/malloc.hh"
#include "error/error.hh"

template <class T>
class i4_grow_array
//{{{
{
protected:
  T *entry;
  w32 used,entries,grow;
  char *name;
public:

  int size() const { return used; }

  T& operator[](int i) { return entry[i];  }

  i4_grow_array(w32 entries, char *name, w32 grow = 0) 
    : entries(entries), name(name), grow(grow), entry(0), used(0)
  {
    if (entries>0)
      entry = (T*)i4_malloc(sizeof(T)*entries, name);
    else
      entry = (T*)i4_malloc(sizeof(T)*grow, name);
  }

  w32 add(T item)
  {
    if (used>=entries)
    {
      if (grow)
      {
        entries += grow;
        T* new_entry = (T*)i4_realloc(entry, sizeof(T *)*entries, name);
        entry = new_entry;
      }
      else
        i4_error("i4_grow_array '%s' out of entries",name);
    }
    entry[used] = item;
    used++;
    return used-1;
  }

  void clear() { used = 0; }
  ~i4_grow_array() { i4_free(entry); }
};



#endif
