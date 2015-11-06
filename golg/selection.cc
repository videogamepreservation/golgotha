/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "selection.hh"
#include "memory/malloc.hh"
#include "g1_object.hh"

g1_selection_list::~g1_selection_list()
{
  if (list)
    i4_free(list);
}

g1_selection_list::g1_selection_list()
{
  list=0;
  list_size=0;
  t=0;
}

void g1_selection_list::add(g1_object_class *o)
{
  if (t==list_size)
  {
    list_size+=64;
    list=(g1_object_class **)i4_realloc(list, 
                                       list_size * sizeof(g1_object_class *),
                                       "selection list");
  }

  list[t++]=o;
  o->flags |= (~g1_object_class::SELECTED);
}

void g1_selection_list::remove(g1_object_class *o)
{
  for (w32 i=0; i<t; i++)
  {
    if (list[i]==o)
    {
      t--;
      memmove(list+i, list+i+1, t-i);
      o->flags &= (~g1_object_class::SELECTED);
      return ;
    }
  }
  i4_error("list remove : object not found");
}

void g1_selection_list::clear()
{
  for (w32 i=0; i<t; i++)
    list[i]->flags &= (~g1_object_class::SELECTED);
  t=0;
}
