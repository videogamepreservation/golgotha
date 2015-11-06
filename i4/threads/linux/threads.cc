/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include <stdio.h>
#include <pthread.h>
#include "error/error.hh"
#include "threads/threads.hh"
#include "init/init.hh"
#include <sched.h>

static w32 i4_thread_count=0;
static i4_critical_section_class i4_thread_lock;
int i4_main_thread_id;

void i4_wait_threads()  // waits for all threads to terminate (don't call from a thread!)
{
  while (i4_thread_count!=0)
    i4_thread_yield();
}
 
static i4_critical_section_class i4_thread_start_lock;
static i4_thread_func_type i4_thread_to_start;
static pthread_t handle;

void *i4_thread_starter(void *arg)
{
  i4_thread_func_type start=i4_thread_to_start;
  i4_thread_start_lock.unlock();

  start(arg);
  
  i4_thread_lock.lock();
  i4_thread_count--;
  i4_thread_lock.unlock();

  pthread_exit(0);
  
  return 0;  
}



void i4_add_thread(i4_thread_func_type fun, w32 stack_size, void *arg_list)
{
  i4_thread_lock.lock();
  i4_thread_count++;
  i4_thread_lock.unlock();

  i4_thread_start_lock.lock();
  i4_thread_to_start=fun;
  
  pthread_create(&handle, 0, i4_thread_starter, arg_list);
}



void i4_thread_yield()
{
  sched_yield();
}


int i4_get_thread_id()
{
  return pthread_self();
}



i4_critical_section_class::i4_critical_section_class(void)
{
  pthread_mutex_init(&mutex,0);
}

i4_critical_section_class::~i4_critical_section_class(void)
{
	pthread_mutex_destroy(&mutex);
}

void i4_critical_section_class::lock(void)
{   
	pthread_mutex_lock(&mutex);
}

void i4_critical_section_class::unlock(void)
{
	pthread_mutex_unlock(&mutex);
}

