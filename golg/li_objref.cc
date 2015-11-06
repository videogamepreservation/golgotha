/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "li_objref.hh"
#include "file/file.hh"
#include "g1_object.hh"
#include "lisp/lisp.hh"
#include "lisp/li_init.hh"
#include "saver.hh"
#include "error/error.hh"
#include "g1_render.hh"
#include "g1_object.hh"
#include "draw_context.hh"

li_type_number li_g1_ref_type_number;
li_object_pointer g1_null_ref;

li_g1_ref::li_g1_ref(g1_object_class *o)
  : li_object(li_g1_ref_type_number) 
{
  if (o)
    _global_id=o->global_id;
  else
    _global_id=g1_global_id.invalid_id();
}

li_g1_ref *li_g1_null_ref()
{
  return li_g1_ref::get(g1_null_ref.get(),0);
}

class li_g1_ref_function : public li_type_function_table
{
  virtual void print(li_object  *o, i4_file_class *stream)   
  { 
    li_g1_ref *r=li_g1_ref::get(o,0);
    g1_object_class *v=r->value();

    stream->printf("(g1_object_class ");
    if (v)
      stream->printf(" %s (id=%d))", v->name(), v->global_id);
    else
      stream->printf(" null)");
  }

  virtual char *name() { return "object_ref"; }

  virtual li_object *create(li_object *params, li_environment *env) 
  { 
    g1_object_class *o=0;
    if (params)
      o=li_g1_ref::get(li_eval(li_car(params,env), env),env)->value();

    if (o)
      return new li_g1_ref(o->global_id);
    else
      return li_g1_null_ref();
  }

  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env)
  {
    ((g1_saver_class *)fp)->write_global_id(li_g1_ref::get(o, env)->_global_id);
  }

  virtual li_object *load_object(i4_loader_class *fp,  li_type_number *type_remap,
                                 li_environment *env)
  {
    return new li_g1_ref(((g1_loader_class *)fp)->read_global_id());
  }   
};





static w32 half_bright(w32 color)
{
  w32 r=((color&0xff0000)>>16)/4;
  w32 g=((color&0xff00)>>8)/4;
  w32 b=((color&0xff)/4);
  return (r<<16)|(g<<8)|b;
}

inline float raise_dist() { return 0.05; }

void li_g1_ref::draw(g1_object_class *start, w32 color, g1_draw_context_class *context)
{
  g1_object_class *o=value();
  if (o)
    g1_render.render_3d_line(i4_3d_point_class(start->x, start->y, start->h+raise_dist()),
                             i4_3d_point_class(o->x, o->y, o->h+raise_dist()),
                             color, half_bright(color),
                             context->transform);
}


// ******************************** li_g1_ref_list ******************************


void li_g1_ref_list::draw(g1_object_class *start, w32 color, g1_draw_context_class *context)
{
  int t=size();
  for (int i=0; i<t; i++)
  {
    g1_object_class *o=value(i);
    if (o)
      g1_render.render_3d_line(i4_3d_point_class(start->x, start->y, start->h+raise_dist()),
                               i4_3d_point_class(o->x, o->y, o->h+raise_dist()),
                               color, half_bright(color),
                               context->transform);
  }
}

w32 li_g1_ref_list::get_id(int list_num)
{
  I4_ASSERT(list && list_num<list[0], "bad list_num");
  return list[list_num+1];
}

g1_object_class *li_g1_ref_list::value(int list_num)
{
  w32 id=get_id(list_num);
  if (g1_global_id.check_id(id))
    return g1_global_id.get(id);
  else
    return 0;   
}


int li_g1_ref_list::find(w32 id)
{
  int t=size()+1;
  for (int i=1; i<t; i++)     // object is already in list
    if (list[i]==id)
      return i-1;

  return -1;  
}

int li_g1_ref_list::find(g1_object_class *o)
{
  return find(o->global_id);
}


void li_g1_ref_list::add(w32 id)
{ 
  if (find(id)==-1)
  {
    int t=size()+2;
    list=(w32 *)i4_realloc(list, t * sizeof(w32),"li_g1_ref_list::list");
    list[0]=t-1;
    list[t-1]=id;
  }
}

void li_g1_ref_list::add(g1_object_class *obj)
{
  add(obj->global_id);
}

li_g1_ref_list *li_g1_ref_list::clone()
{
  li_g1_ref_list *c=new li_g1_ref_list;
  
  int t=size();
  for (int i=0; i<t; i++)
  {
    g1_object_class *o=value(i);
    if (o)
      c->add(o);
  }
  return c;
}

void li_g1_ref_list::remove(w32 id)
{
  int pos=find(id);

  if (pos==-1) 
    i4_error("not in list");

 
  for (int i=pos+1; i<list[0]; i++)
    list[i]=list[i+1];

  list[0]--;
}

void li_g1_ref_list::remove(g1_object_class *o)
{
  remove(o->global_id);
}

void li_g1_ref_list::free()
{
  if (list) 
  {
    i4_free(list);
    list=0;
  }
}

void li_g1_ref_list::compact()
{
  int t=size();
  for (int i=0; i<t; i++)
  {
    w32 id=get_id(i);
    if (!g1_global_id.check_id(id))
    {
      for (int j=i; j<t; j++)
        list[j+1]=list[j+2];
      
      t--;
      list[0]--;
    }
  }
}

li_type_number li_g1_ref_list_type_number;

class li_g1_ref_list_function : public li_type_function_table
{
  virtual void free(li_object   *o) 
  {
    li_g1_ref_list *p=li_g1_ref_list::get(o,0);
    p->free();
  }

  virtual void print(li_object  *o, i4_file_class *stream)   
  { 
    li_g1_ref_list *r=li_g1_ref_list::get(o,0);

    stream->printf("(g1_object_list ");

    for (int i=0; i<r->size(); i++)
    {
      g1_object_class *v=r->value(i);

      if (v)
        stream->printf(" %s (id=%d))", v->name(), v->global_id);
      else
        stream->printf(" null");
    }
    stream->printf(")");
  }

  virtual char *name() { return "object_ref_list"; }

  virtual li_object *create(li_object *params, li_environment *env) 
  { 
    li_g1_ref_list *r=new li_g1_ref_list;

    while (params)
    {          
      g1_object_class *o=li_g1_ref::get(li_eval(li_car(params,env), env),env)->value();
      r->add(o);
    }

    return r;
  }

  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env)
  {

    li_g1_ref_list *r=li_g1_ref_list::get(o,env);
    r->compact();

    int t=r->size();
    fp->write_16(t);
    for (int i=0; i<t; i++)      
      ((g1_saver_class *)fp)->write_global_id(r->get_id(i));
  }

  virtual li_object *load_object(i4_loader_class *fp,  li_type_number *type_remap, 
                                 li_environment *env)
  {
    li_g1_ref_list *r=new li_g1_ref_list;
    int t=fp->read_16();
    for (int i=0; i<t; i++)
      r->add(((g1_loader_class *)fp)->read_global_id());

    return r;
  }   
};



class li_g1_initer : public i4_init_class
{
public:
  int init_type() { return I4_INIT_TYPE_LISP_FUNCTIONS; }
  void init()
  {
    li_g1_ref_type_number=li_add_type(new li_g1_ref_function);
    g1_null_ref=new li_g1_ref(g1_global_id.invalid_id());

    li_g1_ref_list_type_number=li_add_type(new li_g1_ref_list_function);
  }
  
  void uninit()
  {
    g1_null_ref=0;
  }
  
} li_g1_initer_instance;

