/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "error/error.hh"
#include "time/profile.hh"
#include "file/file.hh"
#include "time/time.hh"
#include "file/file.hh"
#include "time/profile_stack.hh"
#include "memory/malloc.hh"
#include "string/string.hh"
#include "threads/threads.hh"

#include <stdio.h>

#ifdef I4_PROFILING


i4_profile_stack_struct i4_profile_stack;
i4_profile_stack_item i4_profile_stack_struct::stack[I4_MAX_PROFILE_STACK_DEPTH];
i4_profile_class *i4_profile_class::list;
int i4_profile_on=0;
int i4_profile_stack_top=0;



void i4_profile_stack_struct::overflow()
{
  i4_debug->printf("items on stack end :\n");
  for (int i=top()-1; i>top()-30; i--)
    i4_debug->printf("  '%s'\n",stack[i].item->name);
  
  i4_error("profile stack overflow");



}

void i4_profile_stack_struct::underflow()
{
  i4_error("unbalanced profile start/stop");
}

i4_profile_class::i4_profile_class(char *debug_name)
  : name(debug_name)
{  
  for (i4_profile_class *p=list; p; p=p->next)
    if (p==this)
      i4_error("already in list?");

  total_time=0;
  next=list;
  list=this;
  active=1;
}




void i4_profile_class::called_start()
{
  w64 current_clock=i4_get_system_clock();
  if (i4_get_thread_id()==i4_get_main_thread_id())
  {
    i4_profile_stack_item *top=i4_profile_stack.get_top();
    if (top)
      top->item->total_time+=current_clock-top->start_clock;


    i4_profile_stack.push(this, current_clock);
  }

}

void i4_profile_class::called_stop()
{
  w64 current_clock=i4_get_system_clock();

  if (i4_get_thread_id()==i4_get_main_thread_id())
  {
    i4_profile_stack_item *me=i4_profile_stack.pop();
    if (me->item!=this)
      i4_error("profile stop unmatched %s (%s on stack)", 
               name, me->item->name);

    total_time+=current_clock - me->start_clock;
    
    i4_profile_stack_item *top=i4_profile_stack.get_top();
    if (top)
      top->start_clock=current_clock;
  }

}



i4_profile_class::~i4_profile_class()
{
  if (this==list)
    list=next;
  else
  {
    i4_profile_class *last=0;
    i4_profile_class *p;

    for (p=list; p && p!=this; )
    {
      last=p;
      p=p->next;
    }

    if (p!=this)      
      i4_warning("could not find profile entry to unlink %s", name);
    else
      last->next=next;
  }
  
}


// clear out any profile timing information we have so far
void i4_profile_clear()
{
  i4_profile_stack.flush_stack();
  for (i4_profile_class *p=i4_profile_class::list; p; p=p->next)
    p->total_time=0;

}

static int profile_compare(const void *a, const void *b)
{
  i4_profile_class *ap=*((i4_profile_class **)a);
  i4_profile_class *bp=*((i4_profile_class **)b);

  if (ap->total_time<bp->total_time)
    return 1;
  else if (ap->total_time>bp->total_time)
    return -1;
  else return 0;               
}


static int prev_on;
void i4_profile_report_start()
{
  i4_profile_clear();
  prev_on=i4_profile_on;
  i4_profile_on=i4_T;
}


// print a report to about timing since the last clear or program start
void i4_profile_report(char *filename)
{
  i4_profile_stack.flush_stack();

  double total_time=0;
  int t=0,i=0;
  i4_profile_class *p;

  for (p=i4_profile_class::list; p; p=p->next, t++)
    total_time+=(double)(sw64)p->total_time;    

  // put them all in a list and sort them by clocks
  i4_profile_class **plist;
  plist=(i4_profile_class **)i4_malloc(sizeof(i4_profile_class *)*t,"");
  for (p=i4_profile_class::list; p; p=p->next)
    plist[i++]=p;


  qsort(plist, t, sizeof(i4_profile_class *), profile_compare);


  double oo_total=1.0/total_time;
  char buf[100];

  i4_file_class *fp=i4_open(filename, I4_WRITE);
  if (fp)
  {
    sprintf(buf,"%2.2f total clocks\n", total_time);
    fp->write(buf, strlen(buf));

    for (i=0; i<t; i++)
    {
      double percent= (sw64)plist[i]->total_time * oo_total;
      double t_clocks=(sw64)plist[i]->total_time;

      sprintf(buf,"%2.2f%%  %2.0f clocks  %s\n", percent * 100.0, t_clocks, plist[i]->name);
      fp->write(buf, strlen(buf));
    }
    delete fp;
  }
  



  i4_free(plist);
  i4_profile_on=prev_on;
}

#endif





