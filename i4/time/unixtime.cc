/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "time/time.hh"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

void i4_time_class::get()
{
  struct timezone tz={0,0};
  struct timeval tv;

  gettimeofday( (struct timeval *)&tv,&tz);
             
  time.unix_time.sec=tv.tv_sec;
  time.unix_time.usec=tv.tv_usec;
}


void i4_time_class::add_milli(sw32 milli_sec)
{

  sw32 new_usec;

  if (milli_sec>0)
  {
    if (milli_sec>1000)
    {
      int sec_add=milli_sec/1000;
      time.unix_time.sec+=sec_add;
      milli_sec-=sec_add*1000;
    }

    new_usec=time.unix_time.usec+milli_sec*1000;
    if (new_usec>1000*1000)
    {
      time.unix_time.sec++;
      new_usec-=1000*1000;
    }
  } else
  {
    if (milli_sec<-1000)
    {
      int sec_add=milli_sec/1000;
      time.unix_time.sec+=sec_add;
      milli_sec-=sec_add*1000;
    }

    new_usec=time.unix_time.usec-milli_sec*1000;
    if (new_usec<0)
    {
      new_usec+=1000*1000;
      time.unix_time.sec--;
    }
  }
  time.unix_time.usec=new_usec;
}


i4_bool i4_time_class::operator >(const i4_time_class &other) const 
{
  return (other.time.unix_time.sec<time.unix_time.sec || 
          (other.time.unix_time.sec==time.unix_time.sec && 
           other.time.unix_time.usec<time.unix_time.usec));
}


i4_bool i4_time_class::operator <(const i4_time_class &other) const 
{
  return (other.time.unix_time.sec>time.unix_time.sec || 
          (other.time.unix_time.sec==time.unix_time.sec && 
           other.time.unix_time.usec>time.unix_time.usec));
}



sw32 i4_time_class::milli_diff(const i4_time_class &past_time) const
{
  return (time.unix_time.usec-past_time.time.unix_time.usec)/1000 + 
    (time.unix_time.sec-past_time.time.unix_time.sec)*1000;
}

i4_time_class::i4_time_class(sw32 milli_sec)
{
  sw32 sec=milli_sec/1000;
  time.unix_time.sec=sec;
  milli_sec-=sec*1000;
  time.unix_time.usec=milli_sec*1000;
}


w64 i4_get_system_clock()
{
  w32 low, high;

  asm volatile ("      .byte 0x0f
                       .byte 0x31
                           movl %%eax, %0
                           movl %%edx, %1"
                : /* no outputs */
                : /* two "m" (memory) inputs */ "m" (low), "m" (high)
                : /* two trashed registers */ "eax", "edx");


  return (((w64)high)<<32) | low;
}



int i4_win_clocks_per_sec = -1;

int i4_get_clocks_per_second()
{
  if (i4_win_clocks_per_sec==-1)
  {
    w64 _start = i4_get_system_clock();
  
    i4_time_class now,start;
    while (now.milli_diff(start) < 1000) now.get();
  
    w64 end = i4_get_system_clock();

    i4_win_clocks_per_sec = end - _start;
  }
  
  return i4_win_clocks_per_sec;
}

void i4_sleep(int seconds) { sleep(seconds); }
void i4_milli_sleep(int milli_seconds) { usleep(milli_seconds*1000); }

