/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "global_id.hh"
#include "error/error.hh"
#include "memory/malloc.hh"
#include "saver.hh"
#include "g1_object.hh"

#include <string.h>

sw32 g1_cur_num_map_objects;
#define PREASSIGN_SIG ((g1_object_class *)0xbadfeed)

g1_global_id_manager_class::g1_global_id_manager_class()
{
  init();
  claim_freespace();
}

i4_bool g1_global_id_manager_class::preassigned(w32 id) const
{
  return obj[id&ID_MASK]==PREASSIGN_SIG;
}


g1_global_id_reset_notifier *g1_global_id_reset_notifier::first=0;

g1_global_id_reset_notifier::g1_global_id_reset_notifier()
{
  next=first;
  first=this;
}


g1_global_id_reset_notifier::~g1_global_id_reset_notifier()
{
  if (first==this)
    first=first->next;
  else
  {
    g1_global_id_reset_notifier *p;
    for (p=first; p->next!=this; p=p->next);
    p->next=next;
  }
}


void g1_global_id_manager_class::init()
{
  // initialize ids
  for (w32 i=0; i<G1_MAX_OBJECTS; i++)
    obj_id[i] = ID_INCREMENT+i;

  memset(obj, 0, sizeof(obj));
  first_free=G1_MAX_OBJECTS;

  g1_cur_num_map_objects=0;

  for (g1_global_id_reset_notifier *p=g1_global_id_reset_notifier::first;
       p; p=p->next)
    p->reset();
}

void g1_global_id_manager_class::free_objects()
{
  w32 i = first_free,j=0;

  while (i<G1_MAX_OBJECTS)
  {
    j = *((w32*)(&obj[i]));                       // next one
    obj[i] = 0;
    i = j;
  }

  for (i=0; i<G1_MAX_OBJECTS; i++)
    if (obj[i])
    {
      delete obj[i];
      obj[i]=0;
    }

  first_free=G1_MAX_OBJECTS;
}

void g1_global_id_manager_class::claim_freespace()
{
  int i=G1_MAX_OBJECTS;
  while (i>0)
  {
    i--;
    if (!obj[i])
    {
      *((w32*)(&obj[i]))=first_free;
      first_free=i;
    }
  }
}

void g1_global_id_manager_class::assign(w32 id, g1_object_class *for_who)
{
  w32 index = id&ID_MASK;

  if (obj_id[index]!=id)
    i4_warning("assigning a possibly invalid id!");

  if (obj[index]!=PREASSIGN_SIG)
    i4_warning("assigning a previously assigned id!");

  obj[index] = for_who;
  //return i4_T;
}

w32 g1_global_id_manager_class::alloc(g1_object_class *for_who)
{
  if (first_free>=G1_MAX_OBJECTS)
  {
    claim_freespace();
    if (first_free>=G1_MAX_OBJECTS)
      i4_error("alloc object id : too many objects");
  }

  w32 id=first_free;
  first_free = *((w32*)(&obj[id]));
  
  obj[id] = (for_who) ? for_who : PREASSIGN_SIG;
  g1_cur_num_map_objects++;

  return obj_id[id];
}

void g1_global_id_manager_class::free(w32 id)
{
  if (!check_id(id))
  {
    i4_warning("free object id : bad id");
    return;
  }

  id &= ID_MASK;

#if 1
  // forces use of all ids first
  obj[id]=0;
#else
  *((w32*)(&obj[id]))=first_free;
  first_free=id;
#endif
  obj_id[id] += ID_INCREMENT;
  g1_cur_num_map_objects--;
}

g1_global_id_manager_class::remapper::remapper(g1_global_id_manager_class *gid) : gid(gid)
{
  map = (w32 *)i4_malloc(G1_MAX_OBJECTS*sizeof(*map), "global_id_remapping"); 
}
g1_global_id_manager_class::remapper::~remapper() 
{ 
  i4_free(map); 
}

g1_global_id_manager_class g1_global_id;



void g1_id_ref::save(g1_saver_class *fp)
{
  fp->write_global_id(id);
}

void g1_id_ref::load(g1_loader_class *fp)
{
  id=fp->read_global_id(); 
}

g1_id_ref::g1_id_ref(g1_object_class *o)
{
  id=o->global_id;
}

g1_id_ref& g1_id_ref::operator=(g1_object_class *o)
{
  if (o)
    id=o->global_id;
  else
    id=g1_global_id.invalid_id();

  return *this;
}

void g1_global_id_manager_class::debug(w32 flag_pass)
{
  w32 invalid[(G1_MAX_OBJECTS+31)/32];
  w32 num_free=0;

  memset(invalid,0,sizeof(invalid));

  w32 i = first_free;
  while (i<G1_MAX_OBJECTS)
  {
    invalid[i>>5] |= (1<< (i&31));                // mark bit
    i = *((w32*)(&obj[i]));                       // next one
    num_free++;                                   // count
  }


  for (i=0; i<G1_MAX_OBJECTS; i++)
    if ((invalid[i>>5] & (1<<(i&31)))==0)
      if (!obj[i])
      {
        invalid[i>>5] |= (1<< (i&31));
        num_free++;
      }
      else
        if (obj[i]->get_flag(flag_pass))
          i4_debug->printf("%d: [%s] team:%d [%c%c%c%c]\n", 
                           i, 
                           obj[i]->name(), 
                           obj[i]->player_num,
                           obj[i]->get_flag(g1_object_class::MAP_OCCUPIED)?'M':'m',
                           obj[i]->get_flag(g1_object_class::THINKING)?'T':'t',
                           obj[i]->get_flag(g1_object_class::SCRATCH_BIT)?'S':'s',
                           obj[i]->get_flag(g1_object_class::DELETED)?'D':'d'
                           );

  i4_debug->printf("Valid Objects: %d  Free Spaces: %d\n", G1_MAX_OBJECTS-num_free, num_free);

}

void g1_list_objects(w32 flag_pass=0xffffffff)
{
  g1_global_id.debug(flag_pass);
}
