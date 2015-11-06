/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_STATUS_HH
#define I4_STATUS_HH

#include "arch.hh"
class i4_const_str;

class i4_status_class
{
public:
  virtual i4_bool update(float percent) = 0;   // percent should be 0..1, ret false for cancel
  virtual ~i4_status_class() { ; }
};

enum { I4_STATUS_ALLOW_CANCEL=1,
       I4_STATUS_UNKNOWN_TOTAL=2 };
    
// this is operating system dependant
i4_status_class *i4_create_status(const i4_const_str &description, int flags=0);


typedef i4_status_class *(*i4_status_create_function_type)(const i4_const_str &description,
                                                           int flags=0); 
void i4_set_status_create_function(i4_status_create_function_type fun);

class i4_idle_class
{  
public:
  static i4_idle_class *first;
  i4_idle_class *next;

  virtual void idle() = 0;

  i4_idle_class();
  virtual ~i4_idle_class();
};

#endif
