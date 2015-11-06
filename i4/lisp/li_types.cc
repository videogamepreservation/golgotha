/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "memory/array.hh"
#include "lisp/li_types.hh"
#include "file/file.hh"
#include "lisp/lisp.hh"
#include "loaders/dir_save.hh"
#include "loaders/dir_load.hh"
#include <stdio.h>


li_string::li_string(char *name)
  : li_object(LI_STRING)
{ 
  int l=strlen(name)+1; 
  _name=(char *)i4_malloc(l,"");
  memcpy(_name, name, l);
}

li_string::li_string(int len)
  : li_object(LI_STRING)
{
  _name=(char *)i4_malloc(len,"");
}

li_string::li_string(const i4_const_str &str)
  : li_object(LI_STRING)
{
  int len=str.length()+1;
  _name=(char *)i4_malloc(len,"");
  i4_os_string(str, _name, len);
}



void li_save_type(i4_file_class *fp, li_type_number type)
{
  fp->write_16(type);
}

li_type_number  li_load_type(i4_file_class *fp, li_type_number *type_remap)
{
  I4_ASSERT(type_remap, "call li_load_type_info before li_load_type");

  return type_remap[fp->read_16()];
}


void li_save_object(i4_saver_class *fp, li_object *o, li_environment *env)
{
  if (!o)
    fp->write_16(0);
  else
  {
    li_save_type(fp, o->type());

    int h;
    if (o->type()>LI_TYPE)
      h=fp->mark_size();

    li_get_type(o->type())->save_object(fp, o, env);

    if (o->type()>LI_TYPE)
      fp->end_mark_size(h);
  }
}


li_object *li_load_object(i4_loader_class *fp, li_type_number *type_remap, li_environment *env)
{
  li_type_number old_type=fp->read_16();  
  li_type_number type=type_remap[old_type];
  
  if (old_type==0)
    return 0;

  w32 skip=0;
  if (old_type>LI_TYPE)
    skip=fp->read_32();
  else if (type==0)
    i4_error("huh?");   // shouldn't happen (please, please)

  if (type)
    return li_get_type(type)->load_object(fp, type_remap, env);
  else if (type>0 && type<=LI_TYPE)
  {
    li_error(env, "type not found, but should be");
    return 0;
  }
  else
  {
    fp->seek(fp->tell() + skip);
    return 0;
  }
}

class li_invalid_type_function : public li_type_function_table
{
  virtual void mark(li_object   *o, int set) { i4_error("marking invalid object"); }
  virtual void free(li_object   *o) { i4_error("freeing invalid object"); }
  virtual int equal(li_object  *o1, li_object *o2)  
  {  
    i4_error("comparing  invalid object"); 
    return 0;
  }

  virtual void print(li_object  *o, i4_file_class *stream)   
  { i4_error("printing invalid object"); }
  virtual char *name() { i4_error("getting name for invalid object"); return 0;}

  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env) 
  { li_error(env, "saving invalid object"); }

  virtual li_object *load_object(i4_loader_class *fp, li_type_number *type_remap, li_environment *env)
  { 
    li_error(env, "loading invalid object"); 
    return 0;
  }

};


void li_symbol::free()
{
  i4_free(data);
}



class li_symbol_type_function : public li_type_function_table
{
  virtual void mark(li_object *o, int set)
  { 
    li_symbol *s=(li_symbol *)o;
    s->mark(set);

    if (s->value())
    {
      if (set!=s->value()->is_marked())
        li_get_type(s->value()->unmarked_type())->mark(s->value(), set);    
    }

    li_object *fun=s->fun();
    if (fun)
    {
      if (set!=fun->is_marked())
        li_get_type(fun->unmarked_type())->mark(fun, set);    
    }

    li_object *name=s->name();
    if (set!=name->is_marked())
      li_get_type(name->unmarked_type())->mark(name, set);    
  }

  virtual void free(li_object   *o)
  {
    li_symbol::get(o,0)->free();
  }
 
  virtual void print(li_object  *o, i4_file_class *stream)   
  {  
    li_symbol *s=li_symbol::get(o,0);
    char *name=s->name()->value();    
    stream->write(name, strlen(name));
  }
  virtual char *name() { return "symbol"; }

  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env)
  {
    li_symbol *s=li_symbol::get(o,env);
    char *name=s->name()->value();
    int name_len=strlen(name)+1;

    fp->write_16(name_len);
    fp->write(name, name_len);
  }

  virtual li_object *load_object(i4_loader_class *fp, li_type_number *type_remap,
                                 li_environment *env)
  { 
    char buf[200];
    int len=fp->read_16();
    if (len>200)
      li_error(env, "symbol name too long");
    fp->read(buf, len);
    return li_get_symbol(buf);
  }
};



char *li_get_type_name(li_type_number type)
{
  return li_get_type(type)->name();
}

li_string::li_string(i4_file_class *fp) : li_object(LI_STRING)
{
  int l=fp->read_32();
  _name=(char *)i4_malloc(l,"");
  fp->read(_name, l);
}

class li_string_type_function : public li_type_function_table
{ 
  virtual void free(li_object   *o) 
  { 
    i4_free(li_string::get(o,0)->value());
  }

  virtual void print(li_object  *o, i4_file_class *stream)   
  { 
    stream->printf("\"%s\"", li_string::get(o,0)->value());
  }

  virtual int equal(li_object  *o1, li_object *o2)  
  {   
    return (strcmp(li_string::get(o1,0)->value(), li_string::get(o2,0)->value())==0);
  }

  virtual char *name() { return "string"; }

  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env)
  {
    char *s=li_string::get(o,env)->value();
    int l=strlen(s)+1;
    fp->write_32(l);
    fp->write(s,l);
  }

  virtual li_object *load_object(i4_loader_class *fp, li_type_number *type_remap, li_environment *env)
  {
    return new li_string(fp);
  }

};



class li_int_type_function : public li_type_function_table
{
  virtual int equal(li_object  *o1, li_object *o2)
  { 
    return li_int::get(o1,0)->value()==li_int::get(o2, 0)->value(); 
  }

  virtual void print(li_object  *o, i4_file_class *stream)   
  { 
    stream->printf("%d", li_int::get(o,0)->value());
  }

  virtual char *name() { return "int"; }

  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env)
  {
    fp->write_32(li_int::get(o,0)->value());
  }

  virtual li_object *load_object(i4_loader_class *fp,  li_type_number *type_remap,
                                 li_environment *env)
  {
    return new li_int(fp->read_32());
  }
    

};


class li_type_type_function : public li_type_function_table
{
  virtual int equal(li_object  *o1, li_object *o2)  
  { 
    return li_int::get(o1,0)->value()==li_int::get(o2,0)->value(); 
  }

  virtual void print(li_object  *o, i4_file_class *stream)   
  { 
    stream->printf("type-%s", li_get_type(li_type::get(o,0)->value())->name());
  }

  virtual char *name() { return "type"; }

  virtual void save_object(i4_saver_class *fp, li_object *o,
                           li_environment *env)
  {
    li_save_type(fp, li_type::get(o,env)->value());
  }

  virtual li_object *load_object(i4_loader_class *fp, li_type_number *type_remap,
                                 li_environment *env)
  {
    int new_type=li_load_type(fp, type_remap);
    if (new_type)
      return new li_type(new_type);
    else
      return 0;
  }

};



class li_float_type_function : public li_type_function_table
{
  virtual int equal(li_object  *o1, li_object *o2)  
  { return li_float::get(o1,0)->value()==li_float::get(o2,0)->value(); }

  virtual void print(li_object  *o, i4_file_class *stream)   
  { 
    char buf[200], dec=0;
    sprintf(buf, "%f", li_float::get(o,0)->value());
    
    for (char *c=buf; *c; c++)
      if (*c=='.') dec=1;
    
    if (dec)
    {
      while (buf[strlen(buf)-1]=='0')
        buf[strlen(buf)-1]=0;
    
      if (buf[strlen(buf)-1]=='.')
        buf[strlen(buf)-1]=0;
    }


    stream->write(buf,strlen(buf));
  }

  virtual char *name() { return "float"; }

  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env)
  {
    fp->write_float(li_float::get(o,env)->value());
  }

  virtual li_object *load_object(i4_loader_class *fp, li_type_number *type_remap,
                                 li_environment *env)
  {
    return new li_float(fp->read_float());
  }

};


class li_character_type_function : public li_type_function_table
{
  virtual int equal(li_object  *o1, li_object *o2)  
  { 
    return li_character::get(o1,0)->value()==li_character::get(o2,0)->value(); 
  }

  virtual void print(li_object  *o, i4_file_class *stream)   
  { 
    stream->printf("#%c",li_character::get(o,0)->value());
  }

  virtual char *name() { return "character"; }

  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env)
  {
    fp->write_16(li_character::get(o,env)->value());
  }

  virtual li_object *load_object(i4_loader_class *fp, li_type_number *type_remap,
                                 li_environment *env)
  {
    return new li_character(fp->read_16());
  }

};



class li_list_type_function : public li_type_function_table
{
  virtual void mark(li_object   *o, int set) 
  { 
    if (o->is_marked() && set)
      return ;

    li_list *l=(li_list *)o;
    if (l->data())
    {
      for (li_list *p=l; p;)
      {
        p->mark(set);
        if (p->data())
        {
          if (set!=p->data()->is_marked())
            li_get_type(p->data()->unmarked_type())->mark(p->data(), set);

          if (p->next() && (set!=p->next()->is_marked()))
          {
            if (p->next()->unmarked_type()==LI_LIST)
              p=(li_list *)p->next();
            else
            {
              li_get_type(p->next()->unmarked_type())->mark(p->next(), set);
              p=0;
            }
          } else p=0;
        }
        else p=0;

      }
    }
  }
  
  virtual void free(li_object   *o) 
  { 
    li_list *l=(li_list *)o;
    l->cleanup();

  }

  virtual int equal(li_object  *o1, li_object *o2)  
  { 
    if (o1==o2) return 1; 
    li_list *p1=li_list::get(o1,0), *p2=li_list::get(o2,0);

    for (;p1;)
    {
      if (!o2) return 0;

      if (p1->data()->type() != p2->data()->type()) return 0;

      if (li_get_type(p1->data()->type())->equal(p1->data(), p2->data())==0)  return 0;
       
      if (p1->next()->type()==LI_LIST)
      {
        if (p2->next()->type()!=LI_LIST)   return 0;
        p1=(li_list *)p1->next();
        p2=(li_list *)p2->next();
      }
      else if (p1->next()->type()!=p2->next()->type()) return 0;
      else return li_get_type(p1->next()->type())->equal(p1->next(), p2->next());
    }

    if (!p2) return 1;
    else return 0;
  }


  virtual void print(li_object  *o, i4_file_class *stream)   
  { 
    stream->write_8('(');
    li_list *p=li_list::get(o,0);
    o->mark(1);          // mark to prevent recursive prints

    for (; p; )
    {  
      li_get_type(p->data()->type())->print(p->data(), stream);

      if (p->next())
      {
        if (p->next()->type()!=LI_LIST)
        {
          stream->write(" . ",3);
          li_get_type(p->next()->type())->print(p->next(), stream);
          p=0;
        }
        else
        {
          p=(li_list *)p->next();
          stream->write_8(' ');
        }
      }
      else p=0;
    }

    o->mark(0);

    stream->write_8(')');
  }

  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env)
  {
    int t=0;
    int last_is_cons=0;
    li_list *l;
    for (l=li_list::get(o,env); l;)
    {
      t++;
      if (t>2000000)
        li_error(env, "list is really big : trying to save a circular structure doesn't work");

      li_object *next=l->next();
      if (next)
      {
        if (next->type()!=LI_LIST)
        {
          l=0;
          last_is_cons=0;
        }
        else l=(li_list *)next;
      }
      else l=0;
    }


    fp->write_32(t);

    if (last_is_cons)
      fp->write_8(1);
    else
      fp->write_8(0);

    for (l=li_list::get(o, env); l;)
    {
      li_object *data=l->data();

      li_save_object(fp, data, env);

      li_object *next=l->next();
      if (next)
      {
        if (next->type()==LI_LIST)
          l=(li_list *)next;
        else
        {
          li_save_object(fp, next, env);
          l=0;
        }
      } else l=0;
    }
  }

  virtual li_object *load_object(i4_loader_class *fp, li_type_number *type_remap,
                                 li_environment *env)
  {
    int t=fp->read_32();
    int last_is_cons=fp->read_8();
    li_list *last=0, *first=0;

    for (int i=0; i<t; i++)
    {
      li_object *data=li_load_object(fp, type_remap, env);
      li_list *l=new li_list(data, 0);
      if (!first)
        first=l;
      else
        last->set_next(l);
      last=l;
    }

    if (last_is_cons)
      last->set_next(li_load_object(fp,type_remap,env));

    return first;
  }


  virtual char *name() { return "list"; }
};





class li_function_type_function : public li_type_function_table
{
  virtual void print(li_object  *o, i4_file_class *stream)
  { 
    stream->printf("#(compiled function @ 0x%x)", (long)(li_function::get(o,0)->value()));
  }

  virtual char *name() { return "function"; }

  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env)
  {
    fp->write_16(li_type::get(o,env)->value());
  }

  virtual li_object *load_object(i4_loader_class *fp, li_type_number *type_remap, li_environment *env)
  {
    int t=type_remap[fp->read_16()];
    if (t)
      return new li_type(t);
    else
      return 0;
  }

};


li_symbol *&li_environment::current_function()
{
  return data->current_function;
}

li_object *&li_environment::current_arguments()
{
  return data->current_args;
}

void li_environment::print_call_stack(i4_file_class *fp)
{
  li_symbol *s=current_function();
  li_object *o=current_arguments();

  if (s && o)
    li_printf(fp, "%O %O", s,o);
  else if (s)
    li_printf(fp, "%O %O", s);

  if (data->next)
    data->next->print_call_stack(fp);
}


li_object *li_environment::value(li_symbol *s)
{
  for (value_data *p=data->value_list; p; p=p->next)
    if (p->symbol==s)
      return p->value;

  if (data->next)
    return data->next->value(s);

  return s->value();
}


li_object *li_environment::fun(li_symbol *s)
{
  for (fun_data *p=data->fun_list; p; p=p->next)
    if (p->symbol==s)
      return p->fun;

  if (data->next)
    return data->next->value(s);

  return s->fun();
}

void li_environment::set_value(li_symbol *s, li_object *value)
{
  if (data->local_namespace)
  {
    for (value_data *p=data->value_list; p; p=p->next)
      if (p->symbol==s)
        p->value=value;
  
    value_data *v=new value_data;
    v->symbol=s;
    v->value=value;
    v->next=data->value_list;
    data->value_list=v;
  }
  else if (data->next)
    data->next->set_value(s,value);
  else
    s->set_value(value);
}


void li_environment::set_fun(li_symbol *s, li_object *fun)
{
  if (data->local_namespace)
  {
    for (fun_data *p=data->fun_list; p; p=p->next)
      if (p->symbol==s)
        p->fun=fun;
  
    fun_data *f=new fun_data;
    f->symbol=s;
    f->fun=fun;
    f->next=data->fun_list;
    data->fun_list=f;
  }
  else if (data->next)
    data->next->set_fun(s, fun);
  else 
    s->set_fun(fun);
}


void li_environment::mark(int set)
{
  li_object::mark(set);

  for (value_data *v=data->value_list; v; v=v->next)
    if (set!=v->value->is_marked())
      li_get_type(v->value->unmarked_type())->mark(v->value,set);

  for (fun_data *f=data->fun_list; f; f=f->next)
    if (set!=f->fun->is_marked())
      li_get_type(f->fun->unmarked_type())->mark(f->fun,set);

  if (data->next && data->next->is_marked()!=set)
    li_get_type(LI_ENVIROMENT)->mark(data->next, set);
}

void li_environment::free()
{
  for (value_data *v=data->value_list; v; )
  {   
    value_data *last=v;
    v=v->next;
    delete last;
  }

  for (fun_data *f=data->fun_list; f; )
  {   
    fun_data *last=f;
    f=f->next;
    delete last;
  }

  delete data;
}

void li_environment::print(i4_file_class *s)
{
  s->printf("#env-(syms=");

  for (value_data *v=data->value_list; v; v=v->next)
  {
    s->write_8('(');
    li_get_type(v->symbol->type())->print(v->symbol, s);
    s->write_8(' ');
    li_get_type(v->value->type())->print(v->value,  s);    
    s->write_8(')');
  }

  s->printf("funs=");
  for (fun_data *f=data->fun_list; f; f=f->next)
  {
    s->write_8('(');
    li_get_type(f->symbol->type())->print(f->symbol, s);
    s->write_8(' ');
    li_get_type(f->fun->type())->print(f->fun,  s);    
    s->write_8(')');
  }
  s->write_8(')');

}



class li_environment_type_function : public li_type_function_table
{
public:
  virtual void mark(li_object   *o, int set)   { ((li_environment *)o)->mark(set); }
  virtual void free(li_object   *o) {  li_environment::get(o,0)->free();  }
  virtual void print(li_object  *o, i4_file_class *s) { li_environment::get(o,0)->print(s); }
  virtual char *name() { return "environment"; }

  
  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env) 
  { li_error(env, "cannot be saved"); }

  virtual li_object *load_object(i4_loader_class *fp, li_type_number *type_remap, li_environment *env) 
  { 
    li_error(env, "cannot be loaded"); 
    return 0;
  }

};



class li_type_manager_class : public i4_init_class
{

public: 
  i4_array<li_type_function_table *> table;

  int add(li_type_function_table *type_functions,
          li_environment *env=0,
          int anon=0)

  {
    li_type_number old_type=0, new_type=table.size();    

    if (!anon)
    {
      li_symbol *sym=li_get_symbol(type_functions->name());
      if (sym->value() && sym->value()->type()==LI_TYPE)  
      {
        old_type=li_type::get(sym->value(), env)->value();
        i4_warning("attempt to reassign type %s ignored", type_functions->name());
        delete type_functions;
        return old_type;
      }
      
      li_set_value(sym, new li_type(new_type), env);
    }

    table.add(type_functions);

    
    return new_type;
  }

  li_type_manager_class() : table(0,32) {}

  void remove(int type_num)
  {
    delete table[type_num];
    table[type_num]=0;
  }

  li_type_function_table *get(int num)
  {
    return table[num];
  }

  int init_type() { return I4_INIT_TYPE_LISP_BASE_TYPES; }
  void init()
  {
    li_invalid_type_function *invalid=new li_invalid_type_function;  
    for (int i=0; i<LI_LAST_TYPE; i++)
      add(invalid,0,1);
    
    table[LI_SYMBOL]=new li_symbol_type_function;
    table[LI_STRING]=new li_string_type_function;
    table[LI_INT]=new li_int_type_function;
    table[LI_FLOAT]=new li_float_type_function;
    table[LI_LIST]=new li_list_type_function;
    
    table[LI_CHARACTER]=new li_character_type_function;
    table[LI_FUNCTION]=new li_function_type_function;
    table[LI_ENVIROMENT]=new li_environment_type_function;
    table[LI_TYPE]=new li_type_type_function;
  }

  int find(char *name)
  {
    for (int i=1; i<table.size(); i++)
      if (strcmp(table[i]->name(), name)==0)
        return i;

    return 0;
  }

};

static li_type_manager_class li_type_man;

int li_add_type(li_type_function_table *type_functions,   // return type number for type
                li_environment *env,
                int anon)

{
  return li_type_man.add(type_functions, env, anon);
}

void li_remove_type(int type_num)
{
  li_type_man.remove(type_num);
}

void li_cleanup_types()
{
  li_type_man.table.uninit();
}

li_type_function_table *li_get_type(li_type_number type_num)
{
  return li_type_man.get(type_num);
}



li_type_number li_find_type(char *name, li_environment *env)
{
  li_symbol *s=li_find_symbol(name);
  if (s)
    return li_type::get(li_get_value(s, env),env)->value();
  else
    return 0;
}

li_type_number li_find_type(char *name, li_environment *env, li_type_number &cache_to)
{
  if (cache_to)
    return cache_to;
  else
  {
    cache_to=li_type::get(li_get_value(li_get_symbol(name), env), env)->value();
    return cache_to;
  }
}



i4_bool li_valid_type(li_type_number type_number)
{
  return type_number>=0 && type_number<li_type_man.table.size() && 
    li_type_man.table[type_number]!=0;
}

int li_max_types()
{
  return li_type_man.table.size();
}
