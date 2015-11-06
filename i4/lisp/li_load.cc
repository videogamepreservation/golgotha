/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "lisp/lisp.hh"
#include "loaders/dir_load.hh"
#include "loaders/dir_save.hh"
#include "lisp/li_load.hh"

li_type_number *li_load_type_info(i4_loader_class *fp, li_environment *env)
{
  int t_types=fp->read_16(), i;
  if (!t_types)
    return 0;

  
  li_type_number *remap=(li_type_number *)i4_malloc(sizeof(li_type_number) * t_types, "");
  memset(remap, 0, sizeof(li_type_number) * t_types);

  for (i=1; i<t_types; i++)
  {
    char buf[300];
    int l=fp->read_16();
    if (l>sizeof(buf)) 
      li_error(env, "load type name too long");

    fp->read(buf, l);
   
    for (int j=1; j<li_max_types(); j++)
      if (li_valid_type(j))
        if (strcmp(buf, li_get_type(j)->name())==0)
          remap[i]=j;    
  }

  for (i=1; i<t_types; i++)
  {
    w32 skip=fp->read_32();

    if (remap[i])
    {
      //      i4_warning("%d : remap for %s", i, li_get_type(remap[i])->name());
      li_get_type(remap[i])->load(fp, remap, env);
    }
    else
      fp->seek(fp->tell() + skip);
  }


  return remap;
}


void li_free_type_info(li_type_number *remap)
{
  if (remap)
    i4_free(remap);

  for (int i=1; i<li_max_types(); i++)
    if (li_valid_type(i))
      li_get_type(i)->load_done();
}

void li_save_type_info(i4_saver_class *fp, li_environment *env)
{
  int t_types=1, i;
  for (i=1; i<li_max_types(); i++)
    if (li_valid_type(i))
      t_types++;

  // save the name and number of each type
  fp->write_16(t_types);
  for (i=1; i<li_max_types(); i++)
  {
    if (li_valid_type(i))
    {
      char *n=li_get_type(i)->name();
      int nl=strlen(n)+1;
      fp->write_16(nl);
      fp->write(n,nl);
    }
    else 
      fp->write_16(0);
  }

  for (i=1; i<li_max_types(); i++)
  {
    if (li_valid_type(i))
    {
      int handle=fp->mark_size();
      li_get_type(i)->save(fp, env);
      fp->end_mark_size(handle);
    }

  }
}



li_object *li_load_typed_object(char *type_name, i4_loader_class *fp, 
                                li_type_number *type_remap,
                                li_environment *env)
{
  int type=li_find_type(type_name);
  if (!type)
    li_error(env,"no type %s", type_name);
  else
  {
    li_object *o=li_load_object(fp, type_remap, env);
    if (!o || o->type()!=type)
      return li_new(type);
    else
      return o;
  }

  return 0;
}

li_object *li_load_typed_object(int type, i4_loader_class *fp, li_type_number *type_remap,
                                li_environment *env)
{  
  li_object *o=li_load_object(fp, type_remap, env);
  if (!o || o->type()!=type) 
  {
    if (type)   
      return li_new(type);
    else return 0;
  }
  else
    return o;
}
