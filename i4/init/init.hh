/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

//{{{ Global Initializers Class
//
// all classes that need initialization at the begining of main should derive themselves from
// this no class should allocate memory or interact with other classes before the "init stage" 
// which follows the "construction stage"
//
//$Id: init.hh,v 1.10 1998/06/22 17:44:16 jc Exp $

#ifndef __i4INIT_HPP_
#define __i4INIT_HPP_

#include "arch.hh"

// these type numbers determine the order i4_init_class'es are initialized in
// deinitialization occurs in the reverse order
enum {
  I4_INIT_TYPE_MEMORY_MANAGER,     // main i4 memory manager
  I4_INIT_TYPE_THREADS,            // initialized thread info
  I4_INIT_TYPE_LISP_MEMORY,        // for lisp object allocations - uses i4 memory manager,
  I4_INIT_TYPE_LISP_BASE_TYPES,    // adds lisp types into the system (li_int.. etc)
  I4_INIT_TYPE_LISP_FUNCTIONS,     // adds lisp fuinctions (li_load & any user li_automatic..
  I4_INIT_TYPE_STRING_MANAGER,
  I4_INIT_TYPE_FILE_MANAGER,
  I4_INIT_TYPE_DLLS,
  I4_INIT_TYPE_OTHER
};

       
class i4_init_class
{
public:
  static i4_init_class *first_init; 
  i4_init_class *next_init;

  virtual int init_type() { return I4_INIT_TYPE_OTHER; }

  virtual void init()   {}
  virtual void uninit() {}

  i4_init_class();
  virtual ~i4_init_class();
};

// should be called at the begining of main (after memory manager is initialized)
void i4_init();

// should be called at the end of main (after memory manager is deinitialized)
void i4_uninit();                            

// used to report errors when things should be executed only when i4 is initialized
i4_bool i4_is_initialized();

#endif

