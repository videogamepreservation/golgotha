/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef _STATISTICS_HH_
#define _STATISTICS_HH_

#include "arch.hh"
#include "math/num_type.hh"
#include "time/time.hh"

class i4_parent_window_class;

class g1_statistics_counter_class
{

public:
  enum {TOTAL_POLYS, OBJECT_POLYS, TERRAIN_POLYS,OBJECTS,
        TERRAIN, SPRITES, SFXS, FRAMES, LAST};

  sw32 counter_array[LAST];
  w32 current_counter;
  w32 t_frames;
  
  i4_time_class last_update_time;
  
  w32 get_value(w32 count_type)
  {
    if (count_type<LAST)
      return counter_array[count_type];
    else
      return 0;
  }
  
  void set_value(w32 count_type, w32 value)
  {
    if (count_type<LAST)
      counter_array[count_type] = value;    
  }

  void set_current_counter(w32 count_type)
  {
    current_counter = count_type;
  }

  w32 get_current_counter()
  {
    return current_counter;
  }

  void increment_current_counter()
  {
    if (current_counter<LAST)
      counter_array[current_counter]++;
  }

  void increment(w32 inc)
  {
    if (inc<LAST)
      counter_array[inc]++;
  }

  void add(w32 inc, w32 amount)
  {
    if (inc<LAST)
      counter_array[inc] += amount;
  }

  void reset()
  {
    w32 i;
    for (i=0;i<LAST;i++)
      counter_array[i] = 0;

    t_frames=0;
    last_update_time.get();
  }

  void show();
};

extern g1_statistics_counter_class g1_stat_counter;

#endif
