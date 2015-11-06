/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "get_tmap.hh"
#include "tmapper.hh"
#include "error/error.hh"


g1_texture_mapper_class *g1_texture_mapper_class::list=0;

// creates a object rendering object
g1_texture_mapper_class *g1_get_tmapper(i4_display_class *display)
{
  int best_val=0;
  g1_texture_mapper_class *p, *best=0;

  for (p=g1_texture_mapper_class::list; p; p=p->next)
  {
    int stat=p->status(display);
    if (stat>best_val)
    {
      best=p;
      best_val=stat;
    }      
  }

  if (!best)
    i4_error("no texture mappers available");

  best->init(display);
  return best;
}

