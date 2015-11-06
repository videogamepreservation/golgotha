/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "reference.hh"
#include "g1_object.hh"


void g1_reference_class::remove_ref()
{
  i4_isl_list<g1_reference_class>::iterator i=ref->ref_list.begin(), last=ref->ref_list.end();

  for (; i!=ref->ref_list.end(); ++i)
  {
    if ((&*i)==this)
    {
      if (last==ref->ref_list.end())
        ref->ref_list.erase();
      else        
        ref->ref_list.erase_after(last);
      ref=0;
      return;
    }
    last=i;
  }
  i4_error("remove reference : not found");
}

void g1_reference_class::reference_object(g1_object_class *object_being_referenced)
{
  if (ref)
    remove_ref();

  if (object_being_referenced)
  {
    ref=object_being_referenced;

    ref->ref_list.insert(*this);
  }
  else 
    ref=0;
}

g1_reference_class::~g1_reference_class()
{
  if (ref) remove_ref();
}
