/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "error/error.hh"
#include "remove_man.hh"
#include "g1_object.hh"

const int MAX_REMOVES = 100;

g1_remove_manager_class g1_remove_man;
 

void g1_remove_manager_class::request_remove(g1_object_class *obj)
{
  if (obj->get_flag(g1_object_class::MAP_OCCUPIED))
    i4_error("deleted object while still on map");

  if (!g1_global_id.check_id(obj->global_id))
    i4_error("removing a piece with invalid id (name=%s)!", obj->name());

  obj->flags |= g1_object_class::DELETED;

  if (obj->flags & g1_object_class::THINKING)
    obj->stop_thinking();
  
  if (removes<MAX_REMOVES)
  {
    remove_list[removes] = obj;
    removes++;
  }
}

void g1_remove_manager_class::init()
{
  remove_list = 
    (g1_object_class**)i4_malloc( MAX_REMOVES*sizeof(g1_object_class*), "remove_list" );

  I4_ASSERT(remove_list, 
            "g1_remove_manager_class - Couldn't create removal list!");

  removes = 0;
}


void g1_remove_manager_class::uninit()
{
  I4_TEST(removes==0, "g1_remove_manager - Not all removals processed!");

  i4_free(remove_list);
}



void g1_remove_manager_class::process_requests()
{
  I4_ASSERT(removes<MAX_REMOVES, 
            "g1_remove_manager - Removes previously exceeded maximum removes");

  for (w32 j=0; j<removes; j++)
  {
    i4_isl_list<g1_reference_class> *ref_list = &remove_list[j]->ref_list;

    while (!ref_list->empty())
    {
      i4_isl_list<g1_reference_class>::iterator i=ref_list->begin();      
      ref_list->erase();

      i->ref = 0;
    }

    delete remove_list[j];
  }
  removes = 0;
}

