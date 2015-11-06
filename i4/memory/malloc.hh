/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __I4_MALLOC_HPP_
#define __I4_MALLOC_HPP_

#include "arch.hh"

#if 0 //( __linux && DEBUG )
#define i4_NEW_CHECK 1
#define i4_MEM_CHECK 1
#endif


void *i4_malloc(w32 size, char *file, int line);
void *i4_realloc(void *old_memory, w32 new_size, char *file, int line);

// if new checking, all news are logged by file and line number
// these logs can be obtained by calling i4_mem_report
// this is avaiable only under gcc

#ifdef i4_NEW_CHECK
#include <stdlib.h>
extern void *operator new( size_t size, char *file, w32 line);
//extern void *operator new [](size_t size, char *file, w32 line);
#define new new(__FILE__,__LINE__)

#define i4_malloc(size, reason) i4_malloc(size, __FILE__, __LINE__)
#define i4_realloc(old, new_size, reason) i4_realloc(old, new_size, __FILE__, __LINE__);
#else
#define i4_malloc(size, reason) i4_malloc(size, 0, 0)
#define i4_realloc(old, new_size, reason) i4_realloc(old, new_size, 0,0)

#endif

void i4_set_max_memory_used(int bytes);
void i4_set_min_memory_required(int bytes);

void i4_free(void *ptr);

void i4_mem_report(char *filename);
long i4_allocated();
long i4_available();
long i4_largest_free_block();

// for debugging purposes only...
int valid_ptr(void *ptr);        // returns 1 if is an address returned by i4_malloc
int valid_memory(void *ptr);     // returns 1 if address is in memory space managed by i4

#endif
