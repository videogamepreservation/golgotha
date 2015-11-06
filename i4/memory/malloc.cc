/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "arch.hh"

#include "memory/malloc.hh"
#include "error/error.hh"
#include "threads/threads.hh"
#include "time/profile.hh"
#include "init/init.hh"
#include "file/file.hh"
#include "file/static_file.hh"
#include "string/string.hh"

// define this to override the native memcpy to check for overwriting memory leaks
//#define i4_MEMCPY_CHECK
#define i4_FREE_CHECK

#ifdef i4_NEW_CHECK
#undef new
#endif

#undef i4_malloc
#undef i4_realloc

static i4_critical_section_class mem_lock;


#ifndef __MAC__

static i4_profile_class pf_malloc("i4_malloc");
static i4_profile_class pf_free("i4_free");


// declare the new()s before we include malloc.hh so that the new 
// macro doesn't mess up their definition
int i4_m_instance=0;
int i4_mem_break=-1;

#if (__linux && i4_NEW_CHECK)

void *operator new( size_t size, char *file, w32 line)
{  
  return i4_malloc(size, file, line);
}


void *operator new [](size_t size, char *file, w32 line)
{
  return i4_malloc(size, file, line);
}
#endif

#include <new.h>

#undef new


void *operator new( size_t size)
{  
  return i4_malloc(size,"unknown",0);
}

void operator delete(void *ptr)
{
  i4_free(ptr);
}


#endif

#ifdef i4_MEM_CHECK
#define i4_MEM_CLEAR
#endif


#include "memory/bmanage.hh"

extern void free_up_memory();

#ifdef i4_MEM_CHECK
// can be set in debugger, break mem fun will be called when this address is allocated
long break_mem_point=0;     
void break_mem_fun()
{
  printf("memory breakpoint\n");
}

#ifdef i4_MEMCPY_CHECK
// can set this memory range to check for mem copies over memory
w32 i4_check_min=0, i4_check_max=0;

extern "C" void *memcpy(void* dest, const void *src,size_t n)
{
  if (((w32)dest)<i4_check_max && ((w32)dest)+n>i4_check_min)
    break_mem_fun();

  bcopy(src,dest,n);
}
#endif

#endif


i4_block_manager_class bmanage[5];
int bmanage_total=0;


void inspect_memory()
{
  mem_lock.lock();
  for (int i=0;i<bmanage_total;i++)
    bmanage[i].inspect();
  mem_lock.unlock();
}


void small_allocation_summary(int &total, int *&list)
{
  int size=1;
  total=JM_SMALL_SIZE/4;
  list=(int *)i4_malloc(total*sizeof(int), __FILE__, __LINE__);
  
  for (;size<total;size++)
  {
    list[size]=0;
    int i,x;
    for (i=0;i<bmanage_total;i++)
    {
      small_block *s=bmanage[i].sblocks[size];
      while (s) 
      { 
        for (x=0;x<32;x++)
          if (s->alloc_list&(1<<x))
            list[size]++; 

        s=s->next; 
      }
    }
  }
}


void i4_malloc_uninit()
{
#ifdef i4_MEM_CHECK
  i4_mem_report("end.mem");
#endif

  mem_lock.lock();

  for (int i=0;i<bmanage_total;i++)
  {
    free(bmanage[i].addr);
  }
  bmanage_total=0;
  mem_lock.unlock();
}


void i4_mem_cleanup(int ret, void *arg)
{
  i4_malloc_uninit(); 
}

 
int i4_malloc_max_size=20000000;
int i4_malloc_min_size=0x1000;

char *not_enough_total_memory_message=
  "Memory manager : Sorry you do not have enough memory available to\n"
  "                 run this program.\n";


void i4_set_min_memory_required(int bytes)
{
  i4_malloc_min_size=bytes;
}

void i4_set_max_memory_used(int bytes)
{
  i4_malloc_max_size=bytes;
  if (i4_malloc_min_size< i4_malloc_max_size)
    i4_malloc_min_size=i4_malloc_max_size;
}


void i4_malloc_init()
{
  if (bmanage_total)
    i4_warning("warning : jmalloc_init called twice\n");
  else
  {
    void *mem;

    sw32 size=i4_malloc_max_size;
    for (mem=NULL;!mem && size>=i4_malloc_min_size;)
    {
      mem=malloc(size);
      if (!mem) size-=0x100;        
    }

    if (mem)
    {
      bmanage[bmanage_total].init(mem,size);
      bmanage_total++; 
      size-=0x1000;
    }  
    else
      i4_error(not_enough_total_memory_message);
  }
}



class i4_memory_init_class : public i4_init_class
{
public:
  int init_type() { return I4_INIT_TYPE_MEMORY_MANAGER; }

  void init() { i4_malloc_init(); }

  void uninit() { i4_malloc_uninit(); }

} i4_memory_init_instance;


long i4_available()
{
  mem_lock.lock();
  long size=0;
  for (int i=0;i<bmanage_total;i++) 
    size+=bmanage[i].available();
  mem_lock.unlock();
  return size;
}

long i4_largest_free_block()
{
  mem_lock.lock();
  long l=0;
  for (int i=0;i<bmanage_total;i++)
  {
    long t=bmanage[i].largest_free_block();
    if (t>l)
      l=t;
  }
  mem_lock.unlock();
  return l;
}

long i4_allocated()
{
  mem_lock.lock();
  
  long size=0;
  for (int i=0;i<bmanage_total;i++) 
    size+=bmanage[i].allocated();
  
  mem_lock.unlock();
  return size;
}


void *i4_malloc(w32 size, char *file, int line)
{
  pf_malloc.start();

  if (size==0)
    i4_warning("tried to malloc 0 bytes");

  i4_m_instance++;
  if (i4_m_instance == i4_mem_break)
    i4_warning("i4 mem break");

#ifdef i4_MEM_CHECK
  char reason[200];

  sprintf(reason, "%s:%d (inst=%d)", file, line, i4_m_instance);
#else
  char *reason="unknown";
#endif

//   printf("Instance %d: %s\n", i4_m_instance, reason);

//  I4_TEST(i4_is_initialized(), "i4_malloc called before i4_init");

  if (!bmanage_total) 
  {
    void *ret=malloc(size);
    return ret;
  }

  size=(size+3)&(0xffffffff-3);
  mem_lock.lock();

  do
  {
    for (int i=0;i<bmanage_total;i++)
    {
      void *a=bmanage[i].alloc(size, reason);
      if (a) 
      {
        mem_lock.unlock();
        pf_malloc.stop();
        return a;
      }
    }

#if 1//def i4_MEM_CHECK    
    mem_lock.unlock();
    i4_mem_report("no_mem");
#endif

    mem_lock.unlock();
    i4_error("Out of memory!");
    //    free_up_memory();
  } while (1);  
}


void i4_free(void *ptr)
{
  pf_free.start();

  I4_TEST(i4_is_initialized(), "i4_free called after i4_uninit()");

  if (!bmanage_total) 
  { 
    free(ptr); 
    pf_free.stop();
    return ; 
  }

#ifdef i4_FREE_CHECK
  I4_ASSERT(valid_ptr(ptr), "invalid free!");
#endif

  mem_lock.lock();
  for (int i=0;i<bmanage_total;i++)
    if (ptr>=(void *)bmanage[i].sfirst)  // is the pointer in this block?
    {
      if (ptr<=(void *)bmanage[i].slast)  // is it in static space?
      {
        bmanage[i].free(ptr);
        mem_lock.unlock();
        pf_free.stop();
        return ;
      } 
    }
 
  mem_lock.unlock();
  
  i4_error("i4_free : bad pointer\n");
  pf_free.stop();
}


void *i4_realloc(void *ptr, w32 size, char *file, int line)
{  
  if (!ptr) 
  {
    // malloc is already lock protected
    return i4_malloc(size, file, line);
  }


  if (!bmanage_total) 
  { 
    // thread protect the c library realloc
    mem_lock.lock();
    void *ret=realloc(ptr,size); 
    mem_lock.unlock();
    return ret;
  }

  if (size==0) 
  { 
    // free is already lock protected
    i4_free(ptr); 
    return NULL; 
  }

  sw32 old_size=0;
  for (int i=0;i<bmanage_total;i++)
    if (ptr>=(void *)bmanage[i].sfirst && 
        ptr<=(void *)(((char *)bmanage[i].sfirst)+bmanage[i].block_size))
    {
      old_size=bmanage[i].pointer_size(ptr);  
      if (ptr<=(void *)bmanage[i].slast)
      {
        void *nptr=i4_malloc(size, file, line);
        if ((sw32)size>old_size)
          memcpy(nptr,ptr,old_size);
        else memcpy(nptr,ptr,size);

        bmanage[i].free(ptr);


        return nptr;
      }
    }

  i4_error("jrealloc : bad pointer\n");
  return NULL;
}


void dmem_report()
{
  i4_mem_report("debug.mem");
}

static i4_static_file_class mem_report;

void i4_mem_report(char *filename)
{
  i4_file_class *fp=mem_report.open(filename);

  if (fp)
  {
    for (int i=0;i<bmanage_total;i++)
      bmanage[i].report(fp);
  
    fp->printf("Total available=%d, allocated=%d\n", i4_available(), i4_allocated());    
  }
}


long small_ptr_size(void *ptr)
{
  return ((small_block *)(((long *)ptr)[-1]))->size;
}


int valid_ptr(void *ptr)
{
  if (!bmanage_total) { return 0; }
  for (int i=0;i<bmanage_total;i++)
    if (ptr>=(void *)bmanage[i].sfirst)  // is the pointer in this block?
    {
      if (ptr<=(void *)bmanage[i].slast)
      {
        int ret=bmanage[i].valid_ptr(ptr);
        return ret;
      }
    }

  return 0;
}




int valid_memory(void *ptr)
{
  for (int i=0; i<bmanage_total; i++)
    if (ptr>=(void *)bmanage[i].sfirst)  // is the pointer in this block?
      if (ptr<=(void *)bmanage[i].slast)
        return 1;

  return 0;
}
