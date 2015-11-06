/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


#ifndef I4_SEARCH_HH
#define I4_SEARCH_HH

#include "arch.hh"

// binary search template for sorted array
//
//  if found,     returns i4_T and location of item in loc
//  if not found, returns i4_F and location of insertion point in loc
//

typedef int (*i4_bsearch_compare_function_type)(const void *key, const void *member);

i4_bool i4_base_bsearch(const void *member, w32 &loc, 
                        const void *array, w32 member_size, w32 size, 
                        i4_bsearch_compare_function_type compare);

template <class Key, class T>
i4_bool i4_bsearch(const Key* member, w32 &loc,
                   const T* array, w32 size, int (*compare)(const Key*, const T*))
{
  return i4_base_bsearch(member, loc, array, sizeof(T), size, 
                         (i4_bsearch_compare_function_type)compare);
}

#endif
