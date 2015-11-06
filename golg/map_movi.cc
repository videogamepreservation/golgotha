/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "map.hh"
#include "g1_object.hh"
#include "m_flow.hh"



i4_bool g1_map_class::start_movie()
{
  if (!current_movie || !current_movie->current())
    return i4_F;
    
  current_movie->start();
  movie_in_progress=i4_T;
  
  g1_object_class *o[G1_MAX_OBJECTS];
  int t=make_object_list(o,G1_MAX_OBJECTS);
  
  for (int i=0; i<t; i++)
    o[i]->request_think();

  tick_time.get();

  return i4_T;
}

void g1_map_class::stop_movie()
{
  if (current_movie)
    current_movie->stop();

  movie_in_progress=i4_F;
}

i4_bool g1_map_class::advance_movie_with_time()
{
  if (current_movie)
  {
    g1_movie_flow_class::advance_status stat;
    stat=current_movie->advance_movie_with_time();
    if (stat==g1_movie_flow_class::DONE)
    {
      movie_in_progress=i4_F;
      return i4_F;
    }

    if (stat==g1_movie_flow_class::NEXT_SCENE)
    {
      g1_object_class *o[G1_MAX_OBJECTS];
      int t=make_object_list(o,G1_MAX_OBJECTS);
  
      for (int i=0; i<t; i++)
        o[i]->request_think();
    }
    return i4_T;
  }
  else
    return i4_F;
}
