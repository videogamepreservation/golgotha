/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "threads.hh"

// stubs while golgatha transitions away from threads

void i4_add_thread(i4_thread_func_type fun, w32 stack_size, void *arg_list)
{}

void i4_que_thread(i4_thread_func_type fun, w32 stack_size, void *arg_list)
{}

void i4_end_of_thread()
{}

void i4_end_of_que_thread()
{}

void i4_thread_yield()
{}

void i4_wait_threads()
{}

void i4_set_thread_priority(int thread_id, i4_thread_priority_type priority) 
{}

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
