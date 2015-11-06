/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


/*  

Example ussage of threads :

void test(void *arg)
{
.. do something..
}

my_main()
{
  i4_add_thread(test,10*1024);
}

*/

#ifndef I4_THREADS_HH
#define I4_THREADS_HH

#include "arch.hh"

typedef void (*i4_thread_func_type)(void *context);

i4_bool i4_threads_supported();

// this will start the thread right away, regardless of how many threads are running
void i4_add_thread(i4_thread_func_type fun, w32 stack_size=50*1024, void *context=0);

void i4_thread_yield();
void i4_wait_threads();  // waits for all threads to terminate (don't call from a thread!)
int  i4_get_thread_id();
int  i4_get_main_thread_id(); 


void i4_suspend_other_threads();  // stops all of threads from running
void i4_resume_other_threads();   // resumes execution of other threads

int i4_get_first_thread_id();
i4_bool i4_get_next_thread_id(int last_id, int &id);
void i4_get_thread_stack(int thread_id, void *&base, void *&top);


enum i4_thread_priority_type { I4_THREAD_PRIORITY_HIGH,
                               I4_THREAD_PRIORITY_NORMAL,
                               I4_THREAD_PRIORITY_LOW };
       
void i4_set_thread_priority(int thread_id, i4_thread_priority_type priority);


class i4_critical_section_class
{
  w32 data[6];  // to store operating system depandant data, enlarge if needed
public:
  i4_critical_section_class();
  void I4_FAST_CALL lock();
  void I4_FAST_CALL unlock();
  ~i4_critical_section_class();
};


class i4_signal_object
{
  w32 data[1]; // to store operating system depandant data, enlarge if needed
public:
  i4_signal_object(char *name);
  void wait_signal();
  void signal();
  ~i4_signal_object();
};


#endif
