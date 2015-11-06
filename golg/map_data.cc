/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "map_data.hh"
#include <string.h>

g1_map_data_class *g1_map_data_class::first=0;

g1_map_data_class::g1_map_data_class(char *name) 
  : name(name)
{
  if (!first)
  {
    next=first;
    first=this;
  }
  else 
  {
    g1_map_data_class *last=0;
    g1_map_data_class *p;
    for (p=first; p && strcmp(p->name,name)>0;)
    {
      last=p;
      p=p->next;
    }

    if (last)
    {
      p->next=last->next;
      last->next=p;
    }
    else
    {
      next=first;
      first=this;
    }      
  }
}


g1_map_data_class::~g1_map_data_class()
{
  if (this==first)
    first=first->next;
  else
  {
    g1_map_data_class *p;
    for (p=first; p->next!=this; p=p->next);
    p->next=next;
  }
}
