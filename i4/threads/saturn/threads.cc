/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "threads/threads.hh"

// this will start the thread right away, regardless of how many threads are running
void i4_add_thread(i4_thread_func_type fun, w32 stack_size, void *arg_list)
{
  fun(arg_list);
}

// this will start the thread after all other threads have exited, useful for queing
// up things that to disk access but don't want to do seek-thrashing
void i4_que_thread(i4_thread_func_type fun, w32 stack_size, void *arg_list)
{
  fun(arg_list);
}


void i4_end_of_thread() { ; }
void i4_end_of_que_thread() { ; }
void i4_thread_yield() { ; }
void i4_wait_threads() { ; }  // waits for all threads to terminate (don't call from a thread!)

