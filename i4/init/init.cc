/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "init/init.hh"
#include "error/error.hh"


i4_init_class *i4_init_class::first_init=0;
static i4_bool i4_inited=i4_F;

void i4_show_init_list()
{
  for (i4_init_class *f=i4_init_class::first_init; f; f=f->next_init)
    i4_warning("%p\n",f);
}

void i4_init()
{
  I4_ASSERT(!i4_inited, "i4 already initialized");


  for (int t=0; t<=I4_INIT_TYPE_OTHER; t++)
  {
    i4_init_class *i=i4_init_class::first_init;  
    for (;i;i=i->next_init)
      if (i->init_type()==t)
        i->init();

    if (t==I4_INIT_TYPE_MEMORY_MANAGER)
      i4_inited=i4_T;   // ok to allocate memory after this stage
  }


}

void i4_uninit()
{
  I4_ASSERT(i4_inited, "i4_uninit() without i4_init()");

  for (int t=I4_INIT_TYPE_OTHER; t>=0; t--)
  {
    i4_init_class *i=i4_init_class::first_init;  
    for (;i;i=i->next_init)
      if (i->init_type()==t)
        i->uninit();
  }

  i4_inited=i4_F;
}


i4_init_class::~i4_init_class()
{
  i4_init_class *last=0, *i=first_init;

  for (;i && i!=this;)
  {
    last=i;
    i=i->next_init;
  }
  
  if (!i)
    i4_error("couldn't find init to remove");

  if (last)
    last->next_init=next_init;
  else
    first_init=next_init;

  next_init=0;
}


i4_bool i4_is_initialized()
{
  return i4_inited;
}

i4_init_class::i4_init_class()
{
  this->next_init=first_init;
  first_init=this;
}
