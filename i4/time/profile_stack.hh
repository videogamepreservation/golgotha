/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_PROFILE_STACK_HH
#define I4_PROFILE_STACK_HH

#include "time/profile.hh"
#include "time/time.hh"

struct i4_profile_stack_item
{
  w64 start_clock;
  i4_profile_class *item;  
};

enum { I4_MAX_PROFILE_STACK_DEPTH=1000 };

extern int i4_profile_stack_top;

struct i4_profile_stack_struct
{
  static i4_profile_stack_item stack[I4_MAX_PROFILE_STACK_DEPTH];

  void overflow();
  void underflow();

  int top() { return i4_profile_stack_top; }

  void push(i4_profile_class *item, w64 current_clock)
  {
    if (top()==I4_MAX_PROFILE_STACK_DEPTH)
      overflow();

    stack[top()].item=item;
    stack[top()].start_clock=current_clock;
    i4_profile_stack_top++;
  }

  i4_profile_stack_item *pop()
  {
     if (!top())
       underflow();
    i4_profile_stack_top--;
    return stack+top();
  }

  i4_profile_stack_item *get_top() 
  { 
    if (top()) 
      return stack+top()-1;
    else return 0;
  }

  // this adds any time remaining on the stack into the profile classes
  // called before reports are formed
  void flush_stack()
  {
    i4_profile_stack_item *t=get_top();
    if (t)
    {
      w64 current_clock=i4_get_system_clock();
      t->item->total_time+=current_clock-t->start_clock;
      t->start_clock=current_clock;
    }
      
  }

  
  

};


#endif
