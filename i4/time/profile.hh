/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef PROFILE_HH
#define PROFILE_HH

#include "error/error.hh"

// This class is used for code profiling, it properly hanldes recursion.

// Example Ussage :
//
// i4_profiler_class ph_file_open("File open");
// i4_profiler_class ph_file_close("File close");
//
// void some_function()
// {
//   ph_file_open.start();
//   open();
//   ph_file_open.stop();
//
//   ph_file_close.start();
//   close();
//   ph_file_close.stop();
// }
//
// void main_function()
// {
//   some_function();
//   //other stuff
//   some_function();
//   i4_profile_report()
// }

#include "arch.hh"

#define I4_PROFILING 1

#ifdef I4_PROFILING



extern int i4_profile_on, i4_profile_stack_top;


class i4_profile_class
{
public:
  static i4_profile_class *list;  
  i4_profile_class *next;            

  int active;
  char *name; 
  w64 total_time;

  i4_profile_class(char *debug_name);

  void called_start();     // not inlined to save program space
  void called_stop();

  void start()
  {
    // this is inlined so when profiling is off, there is no call overhead
    if (active && i4_profile_on)
      called_start();
 }

  void stop()
  {
    if (active && i4_profile_stack_top)
      called_stop();
  }

  ~i4_profile_class();
};



// clear out any profile timing information we have so far
void i4_profile_clear();                

void i4_profile_report_start();  // marks the start of a profile

// print a report to about timing since the last clear or program start
void i4_profile_report(char *filename); 

#else

void i4_profile_clear() { ; }
void i4_profile_report_start() { ; }
void i4_profile_report(char *filename) { ; }

class i4_profile_class
{
public:
  i4_profile_class(char *debug_name) {}  // make these inline so they go away
  void start()                       {}
  void stop()                        {}
};
#endif



#endif


