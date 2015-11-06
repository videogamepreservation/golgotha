/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "main/main.hh"
#include "lisp/lisp.hh"
#include "file/file.hh"
#include "lisp/li_types.hh"
#include "lisp/li_class.hh"
#include "loaders/dir_load.hh"
#include "loaders/dir_save.hh"
#include "lisp/li_init.hh"
#include "lisp/li_load.hh"

static li_type_edit_class *li_class_editor=0;

void li_set_class_editor(li_type_edit_class *editor)
{
  li_class_editor=editor;
}


////////////////////////////// li_class_type members ///////////////////////////////////////////////

li_class *li_this;

class li_class_type : public li_type_function_table
{
public:

  struct var
  {
    li_object  *default_value;
    li_object  *property_list;
    li_symbol  *sym;
    int        original_order;

    void init()
    {
      sym=0;
      default_value=0;
      property_list=0;
      original_order=0;
    }
  };

  static int var_compare(const var *a, const var *b);

  i4_fixed_array<var> vars;

  int old_tvars;
  sw16 *value_remap;     // used during loading of a li_class

  li_class_type *derived_from;
  li_symbol *sym;
  var *get_var(li_symbol *sym);

  int type;

  static li_class_type *get(li_type_function_table *o, li_environment *env)
  { 
    li_class_type *c=(li_class_type *)o;
#ifdef LI_TYPE_CHECK
    if (c!=li_get_type(c->type))
      li_error(env, "function table does not point to a class");
#endif      
    return c;
  }

  li_object *create(li_object *params, li_environment *env);

  void mark(int set);
  void mark(li_object   *o, int set);
  void free(li_object   *o);
  void print(li_object  *o, i4_file_class *stream);
  char *name();

  li_class_type(li_symbol *sym, li_class_type *derived_from)  
    : sym(sym), derived_from(derived_from)
  {
    value_remap=0;
  }


  int get_var_offset(li_symbol *sym, int die_on_error)
  {
    w32 r=vars.size();

    if (!r) return 0;
    int l=0,m;
    li_symbol *s1,*s2;

    while (l<r)
    {
      m = (l+r)/2;
      s1=vars[m].sym;

      if (s1==sym) 
        return m;

      if (sym<s1)
        r = m;
      else
        l = m+1;
    }

    if (l==r || vars[l].sym!=sym)
      if (die_on_error)
        li_error(0, "var not in class %O", sym);
      else return -1;

    return l;
  }

  ~li_class_type()
  {
    vars.uninit();
  }

  // these load and save type information
  virtual void save(i4_saver_class *fp, li_environment *env);
  virtual void load(i4_loader_class *fp, li_type_number *type_remap,
                    li_environment *env);
                    
  virtual void load_done();

  // load & save type instance information
  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env);
  virtual li_object *load_object(i4_loader_class *fp, li_type_number *type_remap,
                                 li_environment *env);

};


struct sym_var
{
  li_class_type::var *var;
  li_object     *value;
};



char *li_class_type::name() 
{ 
  if (sym)
    return sym->name()->value();
  else
    return "anonymous-class"; 
}


void li_class_type::mark(int set)
{
  for (int i=0; i<vars.size(); i++)
  {
    if (vars[i].default_value)
      if (vars[i].default_value->is_marked()!=set)
        li_get_type(vars[i].default_value->unmarked_type())->mark(vars[i].default_value, set);

    if (vars[i].property_list)
      if (vars[i].property_list->is_marked()!=set)
        li_get_type(vars[i].property_list->unmarked_type())->mark(vars[i].property_list, set);
  }     
}

void li_class::mark(int set)
{
  if (!set)
    li_object::mark(set);

  li_class_type *t=get_type();
  for (int i=0; i<t->vars.size(); i++)
  {   
    int type=t->vars[i].default_value->unmarked_type();

    li_object *o=object_value(i);
    // int's and floats are stored directly and don't need marking
    if (type!=LI_INT && type!=LI_FLOAT && o->is_marked()!=set)
      li_get_type(o->unmarked_type())->mark(o, set);
  }      

  if (set)
    li_object::mark(set);
}

li_class_type::var *li_class_type::get_var(li_symbol *sym)
{
  for (int i=0; i<vars.size(); i++)
    if (vars[i].sym==sym) 
      return &vars[i];

  if (derived_from)
    return derived_from->get_var(sym);

  return 0;
}

void li_class_type::mark(li_object   *o, int set)   
{  
  ((li_class *)o)->mark(set);
}

void li_class_type::free(li_object   *o) 
{ 
  li_class::get(o,0)->free(); 
}

void li_class_type::print(li_object  *o, i4_file_class *stream) 
{ 
  li_class::get(o,0)->print(stream); 
}

li_object *li_class_type::create(li_object *params, li_environment *env)
{
  return new li_class(type, params, env);
}


// these load and save type information
void li_class_type::save(i4_saver_class *fp, li_environment *env)
{
  fp->write_32(vars.size());
  for (int i=0; i<vars.size(); i++)
    li_save_object(fp,vars[i].sym, env);
}


void li_class_type::load_done()
{
  if (value_remap)
  {
    i4_free(value_remap);
    value_remap=0;
  }
}

void li_class_type::load(i4_loader_class *fp, li_type_number *type_remap,
                         li_environment *env)
{
  old_tvars=fp->read_32();
  if (old_tvars)
  {
    value_remap=(sw16 *)i4_malloc(sizeof(sw16) * old_tvars, ""); 
    for (int j=0; j<old_tvars; j++)
      value_remap[j]=-1;

    for (int i=0; i<old_tvars; i++)
    {
      li_symbol *old_sym=li_symbol::get(li_load_object(fp, type_remap,env), env);
      for (int j=0; j<vars.size(); j++)
        if (old_sym==vars[j].sym)
          value_remap[i]=j;
    } 
  }
}



void li_class::save(i4_saver_class *fp, li_environment *env)
{
  li_class_type *ct=get_type();

  int t_vars=ct->vars.size();
  for (int i=0; i<t_vars; i++)
  {
    li_object *def=ct->vars[i].default_value;
    li_object *v=value(i);

    if (li_get_type(def->type())->equal(def, v))
      li_save_object(fp, 0, env);
    else
      li_save_object(fp, value(i), env);
  }
}

  // load & save type instance information
void li_class_type::save_object(i4_saver_class *fp, li_object *o, li_environment *env)
{
  li_class::get(o,env)->save(fp, env);
}

void li_class::load(i4_loader_class *fp, li_type_number *type_remap, li_environment *env)
{
  li_class_type *ct=get_type();
  int old_tvars=ct->old_tvars;
  sw16 *value_remap=ct->value_remap;

  for (int i=0; i<old_tvars; i++)
  {
    li_object *o=li_load_object(fp, type_remap, env);
    int remap=value_remap[i];
    if (remap!=-1)
    {
      li_object *def=ct->vars[remap].default_value;
         
      // if type has changed use default value      
      if ( (def && o) && o->type()==def->type())
        set_value(remap, o);
    }
  }
}

li_object *li_class_type::load_object(i4_loader_class *fp, li_type_number *type_remap, 
                                      li_environment *env)
{
  li_class *c=new li_class(type);
  c->load(fp, type_remap, env);
  return c;
}


//////////////////////////////////// li_class members /////////////////////////////////

li_class::li_class(li_type_number class_type,
                   li_object *params,
                   li_environment *env)
  : li_object(class_type)
{
  li_class_type *ct=get_type();
  int t_vars=ct->vars.size();

  values=(void **)i4_malloc(sizeof(void *) * t_vars, "");


  int i;
  for (i=0; i<t_vars; i++)
    set_value(i, ct->vars[i].default_value);



  i=0;
  while (params)
  {
    li_object *val=li_eval(li_car(params,env));
    

    for (int j=0; j<t_vars; j++)
      if (ct->vars[j].original_order==i)
      {
        set_value(j, val);
        j=t_vars;
      }

    params=li_cdr(params,env);
    i++;
  }
  

}


void li_class::print(i4_file_class *fp)
{        
  fp->write("#inst-",6);

  li_class_type *c=get_type();

  char *name=c->name();
  fp->write(name,strlen(name));

  fp->write_8('<');

  for (int i=0; i<c->vars.size(); i++)
  {
    li_symbol *sym=c->vars[i].sym;

    fp->write(" (",2);
    li_get_type(LI_SYMBOL)->print(sym, fp);
    fp->write_8(' ');

    li_object *v=value(i);
    li_get_type(v->type())->print(v, fp);

    fp->write_8(')');
  }

  fp->write_8('>');

}

void li_class::free()
{
  i4_free(values);
}




int li_class::member_offset(char *sym) const
{
  return get_type()->get_var_offset(li_get_symbol(sym), 0);
}

int li_class::member_offset(li_symbol *sym) const
{
  return get_type()->get_var_offset(sym, 0);
}


int li_class::get_offset(li_class_member &c, li_type_number _type) const
{
  li_class_type *ct=get_type();

  if (!c.sym)
    c.sym=li_get_symbol(c.name);
  
  c.class_type=type();
  c.offset=ct->get_var_offset(c.sym, 1);

  if (c.offset==-1)
    li_error(0, "class %s does not have a member %s", ct->name(), c.name);

#ifdef LI_TYPE_CHECK
  if (ct->vars[c.offset].default_value->type()!=_type)
    li_error(0, "class member %O is wrong type (%s should be %s)", 
             c.sym,
             li_get_type(_type)->name(),
             li_get_type(ct->vars[c.offset].default_value->type())->name());  
#endif


  return c.offset;
}



int li_class::get_offset(li_class_member &c) const
{
  li_class_type *ct=get_type();

  if (!c.sym)
    c.sym=li_get_symbol(c.name);
  
  c.class_type=type();
  c.offset=ct->get_var_offset(c.sym, 0);

  return c.offset;
}



#ifdef LI_TYPE_CHECK
li_class *li_class::get(li_object *o, li_environment *env)
{ 
  check_type(o, ((li_class_type *)li_get_type(o->type()))->type, env);   
  return ((li_class *)o); 
}
#endif


li_object *li_class::value(int member)
{
  switch (get_type()->vars[member].default_value->type())
  {
    case LI_INT : return new li_int(int_value(member)); break;
    case LI_FLOAT : return new li_float(float_value(member)); break;
    default : return object_value(member); break;
  }
}

li_object *li_class::value(char *member_name)
{
  return value(member_offset(member_name));
}


void li_class::set_value(int member, li_object *value)
{    
  li_class_type *ct=get_type();
  li_object *def_value=ct->vars[member].default_value;

  int t=def_value->type();
  switch (t) 
  {
    case LI_INT : int_value(member) = li_int::get(value,0)->value(); break;
    case LI_FLOAT : float_value(member) = li_float::get(value,0)->value(); break;
    default : object_value(member)=value;
  }
}



///////////////////////////////////// li_def_class ///////////////////////////////////////////

li_object *li_def_class(li_object *fields, li_environment *env)
{
  li_symbol *sym=li_symbol::get(li_car(fields,env),env);  fields=li_cdr(fields,env);
  li_object *derived=li_eval(li_car(fields,env), env); fields=li_cdr(fields,env);
  li_class_type  *d=0;
  int derived_type=0;
  
  if (derived!=li_nil) 
  {
    derived_type=li_type::get(derived,env)->value();
    if (derived_type)
    {   
      d=(li_class_type *)li_get_type(derived_type);
      if (d->type!=derived_type)
        li_error(env, "cannot derive a class from %O, only other classes", derived);
    }
    else li_error(env, "no such type %O", derived);
  }

  li_class_type *me=new li_class_type(sym, d);

  li_object *c;
  int t_vars=0;

  // how many variables in the parent class
  if (derived_type)
    t_vars+=li_class_total_members(derived_type);  
  
  for (c=fields; c; c=li_cdr(c,env))      // count how many variables were added
    t_vars++;

  me->vars.resize(t_vars);

  t_vars=0;

  if (derived_type)
  {
    int t_from_derived_class=li_class_total_members(derived_type);
    for (int i=0; i<t_from_derived_class; i++)
    {
      me->vars[t_vars].init();
      me->vars[t_vars].original_order=t_vars;
      li_symbol *s=li_class_get_symbol(derived_type, i);
      me->vars[t_vars].sym=s;
      me->vars[t_vars].default_value=li_class_get_default(derived_type, s);
      me->vars[t_vars].property_list=li_class_get_property_list(derived_type, s);
      t_vars++;
    }
  }
    

  for (c=fields; c; c=li_cdr(c,env))
  {
    li_object *var=li_car(c,env);
    me->vars[t_vars].init();
    me->vars[t_vars].original_order=t_vars;
    
    
    me->vars[t_vars].sym=li_symbol::get(li_car(var,env),env);  var=li_cdr(var,env);
    
    if (var)
    {
      me->vars[t_vars].default_value=li_eval(li_car(var,env), env);  var=li_cdr(var,env);      

      li_symbol *s=me->vars[t_vars].sym;
      li_object *d=me->vars[t_vars].default_value;


      if (var)
        me->vars[t_vars].property_list=li_eval(li_car(var,env), env);
    }

    t_vars++;
  }
    
  me->vars.sort(li_class_type::var_compare);
  me->editor=li_class_editor;
  me->type=li_add_type(me);

  return new li_type(me->type);
}

li_object *li_class::set(char *member_name, li_object *value) // slow, but easy way to access data
{
  int off=member_offset(member_name);
  if (off==-1) 
    li_error(0, "class %o does not have member %s", member_name);
  set_value(off, value);
  return value;
}


int li_class_type::var_compare(const var *a, const var *b)
{
  if (a->sym<b->sym)
    return -1;
  else if (a->sym>b->sym)
    return 1;
  else return 0;
}


int li_class_total_members(li_type_number type)
{
  return li_class_type::get(li_get_type(type),0)->vars.size();
}

li_symbol *li_class_get_symbol(li_type_number type, int member_number)
{
  li_class_type *ct=li_class_type::get(li_get_type(type),0);
  return ct->vars[member_number].sym;
}


li_object *li_class_get_default(li_type_number type, li_symbol *sym)
{
  li_class_type *ct=li_class_type::get(li_get_type(type),0);
  return ct->vars[ct->get_var_offset(sym, 1)].default_value;
}

li_object *li_class_get_property_list(li_type_number type, li_symbol *sym)
{
  li_class_type *ct=li_class_type::get(li_get_type(type),0);
  return ct->vars[ct->get_var_offset(sym, 1)].property_list;
}

li_object *li_setm(li_object *o, li_environment *env)
{
  li_class *c=li_class::get(li_first(o,0),0);
  li_symbol *member=li_symbol::get(li_second(o,0),0);
  li_object *value=li_eval(li_third(o,0), env);
  c->set_value(c->member_offset(member), value);
  return value;
}

li_object *li_getm(li_object *o, li_environment *env)
{
  li_class *c=li_class::get(o,0);
  return c->value(c->member_offset(li_symbol::get(li_first(o,0),0)));  
}



li_automatic_add_function(li_def_class, "def_class");
li_automatic_add_function(li_setm, "setm");
li_automatic_add_function(li_getm, "getm");
