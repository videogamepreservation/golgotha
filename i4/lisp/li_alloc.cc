/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "init/init.hh"
#include "memory/malloc.hh"
#include "lisp/li_types.hh"
#include "lisp/lisp.hh"
#include "main/main.hh"
#include "time/profile.hh"
#include "threads/threads.hh"
#include <setjmp.h>

static i4_critical_section_class syms_lock;
static i4_critical_section_class cell_lock;
static volatile int threads_need_gc=0;

li_object_pointer *li_object_pointer_list=0;
i4_profile_class pf_li_gc("li_gc");


li_object *li_not(li_object *o, li_environment *env)
{
  li_object *v=li_eval(li_car(o,env),env);
  if (!v || v==li_nil)
    return li_true_sym;
  else return li_nil;
}

li_object *li_progn(li_object *o, li_environment *env)
{
  li_object *ret=li_nil;
  while (o)
  {
    ret=li_eval(li_car(o,env),env);
    o=li_cdr(o,env);
  }
  return ret;
}

li_object *li_if(li_object *o, li_environment *env)
{
  li_object *v=li_eval(li_car(o,env), env);

  if (v && v!=li_nil)
    return li_eval(li_second(o,env),env);
  
  o=li_cdr(li_cdr(o,env),env);
  if (o)
    return li_eval(li_car(o,env), env);
  else return li_nil;
}

li_object *li_equal(li_object *o, li_environment *env)
{
  li_object *o1=li_eval(li_first(o,env),env);
  li_object *o2=li_eval(li_second(o,env),env);

  if (o1->type()==o2->type())
    if (li_get_type(o1->type())->equal(o1, o2))
      return li_true_sym;
  
  return li_nil;
}



li_object_pointer::li_object_pointer(li_object *obj)
{
  o=obj;
  next=li_object_pointer_list;
  li_object_pointer_list=this;
}

li_object_pointer::~li_object_pointer()
{
  if (this==li_object_pointer_list)
    li_object_pointer_list=next;
  else
  {
    li_object_pointer *last=0, *p;
    for (p=li_object_pointer_list; p && p!=this;)
    {
      last=p;
      p=p->next;
    }
    if (p!=this) 
      li_error(0, "couldn't find object pointer to unlink");
    last->next=next;
  }
}


// global symbols
li_symbol *li_nil=0, 
  *li_true_sym=0, 
  *li_quote=0, 
  *li_backquote=0,
  *li_comma=0,
  *li_function_symbol=0;

static li_gc_object_marker_class *gc_helpers=0;

li_gc_object_marker_class::li_gc_object_marker_class()
{
  next=gc_helpers;
  gc_helpers=this;
}

li_gc_object_marker_class::~li_gc_object_marker_class()
{
  if (gc_helpers==this)
    gc_helpers=gc_helpers->next;
  else
  {
    li_gc_object_marker_class *last=0, *p;
    for (p=gc_helpers; p!=this;)
    {
      last=p;
      p=p->next;        
    }
    if (!p) 
      li_error(0,"gc_object marker not in list");
    last->next=p->next;
  }
}

void li_mark_symbols(int set);


li_symbol *li_root=0;

extern li_symbol *li_root;



li_symbol *li_find_symbol(const char *name)     // if symbol doesn't exsist, it is created
{
  syms_lock.lock();
  if (li_root)
  {
    li_symbol *p=li_root;
    while (1)
    {
      int cmp=strcmp(name,p->name()->value());
      if (cmp<0)
      {
        if (p->left())
          p=p->left();
        else
        {
          syms_lock.unlock();
          return 0;
        }
      } else if (cmp>0)
      {
        if (p->right())
          p=p->right();
        else
        {
          syms_lock.unlock();
          return 0;
        }
      } else 
      {
        syms_lock.unlock();
        return p;
      }
    }
  }

  syms_lock.unlock();
  return 0;
}

li_symbol *li_get_symbol(const char *name)     // if symbol doesn't exsist, it is created
{
  syms_lock.lock();
  if (!li_root)
  {
    li_root=new li_symbol(new li_string(name));
    syms_lock.unlock();
    return li_root;
  }
  else
  {
    li_symbol *p=li_root;
    while (1)
    {
      int cmp=strcmp(name,p->name()->value());
      if (cmp<0)
      {
        if (p->left())
          p=p->left();
        else
        {
          p->set_left(new li_symbol(new li_string(name)));
          syms_lock.unlock();
          return p->left();
        }
      } else if (cmp>0)
      {
        if (p->right())
          p=p->right();
        else
        {
          p->set_right(new li_symbol(new li_string(name)));
          syms_lock.unlock();
          return p->right();
        }
      } else
      {
        syms_lock.unlock();
        return p;
      }
    }
  }

  syms_lock.unlock();
  return 0;
}

li_symbol *li_get_symbol(char *name, li_symbol *&cache_to)
{
  if (cache_to) return cache_to;
  cache_to=li_get_symbol(name);
  return cache_to;
}

void li_recursive_mark(li_symbol *p, int set)
{
  if (p)
  {
    li_get_type(LI_SYMBOL)->mark(p, set);
    li_recursive_mark(p->left(), set);
    li_recursive_mark(p->right(), set);
  }
}

void li_mark_symbols(int set)
{
  li_recursive_mark(li_root, set);    
}



void li_mark_symbol_tree(li_symbol *s, int set)
{
  if (s)
  {
    if (set!=s->is_marked())
      li_get_type(LI_SYMBOL)->mark(s, set);

    li_mark_symbol_tree(s->left(), set);
    li_mark_symbol_tree(s->right(), set);
  }
}

void li_mark_memory_region(li_list **start, li_list **end,
                           li_list *c1, li_list *c2, int set)
{
  if (set)
  {
    for (li_list **s=start; s!=end; s++)          
      if ( ((long)(*s)&7)==0 &&  *s>=c1 && *s<c2 && (*s)->type() && !(*s)->is_marked())
        li_get_type( (*s)->unmarked_type() )->mark(*s,1);
  }
  else
    for (li_list **s=start; s!=end; s++)
      if (((long)(*s)&7)==0 && *s>=c1 && *s<c2 && (*s)->is_marked())
        li_get_type( (*s)->unmarked_type() )->mark(*s,0);
  
}

li_object *li_setf(li_object *o, li_environment *env)
{
  li_symbol *s=li_symbol::get(li_car(o,env),env);  o=li_cdr(o,env);
  li_object *value=li_eval(li_car(o,env), env);
  li_set_value(s, value, env); 
  return value;
}

li_object *li_quote_fun(li_object *o, li_environment *env)
{
  return li_car(o,env);
}

li_object *li_new(li_object *o, li_environment *env)
{
  int type=li_type::get(li_eval(li_car(o,env)),env)->value();
  return li_get_type(type)->create(li_cdr(o,env), env);
}

int li_max_cells=20*1024;

li_object *li_ptr(li_object *o, li_environment *env)
{
  return (li_object *)(li_get_int(li_eval(li_car(o,env), env),env));
}


class li_memory_manager_class : public i4_init_class
{
public:
  li_list *cells, *cstart;
  li_list *first_free;

  void get_stack_range(li_object *&start, li_object *&end)
  {
    void *current_stack_object;
    li_object *current_stack=(li_object *)(&current_stack_object);

    li_list **stack_start=((li_list **)i4_stack_base);

    if ((long)stack_start<(long)current_stack) 
    { 
      start=(li_object *)stack_start; 
      end=current_stack; 
    }
    else
    { 
      end=(li_object *)stack_start; 
      start=current_stack; 
    }
  }

  i4_bool valid_object(li_object *o)
  {
    if ((li_list *)o>=cstart && ((li_list *)o)<cstart+li_max_cells && li_valid_type(o->type()))
      return i4_T;
    else
    {
      if (i4_stack_base!=0)
      {
        li_object *s,*e;
        get_stack_range(s,e);
        
        if (o>=s && o<e)
          return i4_T;
      }

      return i4_F;
    }
  }

  int init_type() { return I4_INIT_TYPE_LISP_MEMORY; }

  void mark_stacks(int mark)
  {
    int id=i4_get_first_thread_id();
    do
    {
      void *base, *top;
      i4_get_thread_stack(id, base,top);
      if (base<top)
        li_mark_memory_region((li_list **)base,(li_list **)top,                               
                              cells, cells+li_max_cells, mark);
      else
        li_mark_memory_region((li_list **)top,(li_list **)base,
                              cells, cells+li_max_cells, mark);
    } while (i4_get_next_thread_id(id, id));
  }


  // gc : Garbage Collection
  // scans & marks all cells referenced by 
  // symbols, main stack, thread stacks, global_pointers, & helper objects
  int gc()
  {
    int t_free=0;

    if (i4_get_thread_id()!=i4_get_main_thread_id())
    {
      // if this is called from a thread stop and let main program do gc()
      threads_need_gc=1;
      while (threads_need_gc)
        i4_thread_yield();

      cell_lock.lock();
      for (int i=0; i<li_max_cells; i++)
      {
        if (cells[i]._type==LI_INVALID_TYPE)
          t_free++;
      }
      cell_lock.unlock();
    }
    else
    {
      li_object_pointer *pl;
      int i;

      if (!i4_stack_base)
        i4_error("gc:: need to call li_gc_init() from main prog");
      
      pf_li_gc.start();

      cell_lock.lock();

      i4_suspend_other_threads();
      mark_stacks(1);

      li_mark_symbols(1);

      if (li_root)     // if the system has shut down, don't mark type's objects
      {
        for (i=1; i<li_max_types(); i++)
        {
          li_type_function_table *t=li_get_type(i);
          if (t)
            t->mark(1);
        }
      }

      li_gc_object_marker_class *helpers;
      for (helpers=gc_helpers; helpers; helpers=helpers->next)
        helpers->mark_objects(1);

      for (pl=li_object_pointer_list; pl; pl=pl->next)
        if (pl->o && !pl->o->is_marked())
          li_get_type(pl->o->type())->mark(pl->o, 1);

      first_free=0;
      for (i=0; i<li_max_cells; i++)
      {
        if (!cells[i].is_marked())
        {
          if (cells[i].type()!=LI_INVALID_TYPE)
          {
            li_get_type(cells[i].type())->free(cells+i);
            cells[i].mark_free();
            cells[i]._type=LI_INVALID_TYPE;
          }


          // add to free_list
          cells[i].set_next_free(first_free);
          first_free=cells+i;
          t_free++;
        }
      }


      // unmark the stacks
      mark_stacks(0);

      // unmark symbols
      li_mark_symbols(0);

      if (li_root)
      {
        for (i=1; i<li_max_types(); i++)
        {
          li_type_function_table *t=li_get_type(i);
          if (t)
            t->mark(0);
        }
      }


      for (helpers=gc_helpers; helpers; helpers=helpers->next)
        helpers->mark_objects(0);

      for (pl=li_object_pointer_list; pl; pl=pl->next)
        if (pl->o && pl->o->is_marked())
          li_get_type(pl->o->unmarked_type())->mark(pl->o, 0);

      cell_lock.unlock();
      threads_need_gc=0;
      i4_resume_other_threads();
      pf_li_gc.stop();
    }

    return t_free;
  }

  li_list *alloc_list()
  {
    if (!first_free)
    {
      if (!gc())
        i4_error("li_alloc : out of li_list");      
    }
    
    cell_lock.lock();
    li_list *ret=first_free;
    first_free=first_free->get_next_free();
    cell_lock.unlock();


    return ret;
  }

  void free_list(li_list *l)
  {
    cell_lock.lock();
    int i=l-cells;

    // add to free_list
    cells[i]._type=LI_INVALID_TYPE; 
    cells[i].set_next_free(first_free);
    first_free=cells+i;

    cell_lock.unlock();
  }


  void init()
  {
    if (sizeof(li_list)!=8 || sizeof(li_int)!=8)
      li_error(0, "this code assumes lisp objects are size 8");


    cells=(li_list *)i4_malloc(li_max_cells * sizeof(li_list),"");
    cstart=cells;

    if (((long)cells)&7)  // pointer needs to alligned to 8 byte boundary
    {
      cells=((li_list *)(((long)cells&(~7))+8));
      li_max_cells--;
    }

    for (int i=0; i<li_max_cells-1; i++)
    {
      cells[i].mark_free();
      cells[i].set_next_free(cells+i+1);
    }

    cells[li_max_cells-1].set_next_free(0);
    cells[li_max_cells-1].mark_free();

    first_free=cells;

    li_nil=li_get_symbol("nil");        li_set_value(li_nil, li_nil);

    li_true_sym=li_get_symbol("T");     li_set_value(li_true_sym, li_true_sym);
    li_quote=li_get_symbol("'");
    li_backquote==li_get_symbol("`");
    li_comma=li_get_symbol(",");
    li_function_symbol=li_get_symbol("#");

    li_add_function("not", li_not);
    li_add_function("progn", li_progn);
    li_add_function("equal", li_equal);
    li_add_function("if", li_if);
    li_add_function("load", li_load);
    li_add_function("setf", li_setf);
    li_add_function("print", li_print);
    li_add_function(li_quote, li_quote_fun);
    li_add_function("new", li_new);
    li_add_function("read-eval", li_read_eval);
    li_add_function("ptr", li_ptr);
  }


  void uninit()
  {
    li_root=0;

    // clear all pointer references
    for (li_object_pointer *pl=li_object_pointer_list; pl; pl=pl->next)
      pl->o=0;

    int t_free=gc();


    if (t_free!=li_max_cells)
    {
      i4_warning("li_cleanup : possibly %d items still referenced", 
                 li_max_cells-t_free);

      for (int i=0; i<li_max_cells; i++)
        cells[i].mark_free();
    }

    // delete all types
    for (int t=0; t<li_max_types(); t++)
      if (t==0 || li_valid_type(t))
        li_remove_type(t);

    li_cleanup_types();

    i4_free(cstart);
  }

} li_mem_man;


void *li_cell8_alloc()
{
  if (threads_need_gc && i4_get_thread_id()==i4_get_main_thread_id())
   li_gc();
    
  return li_mem_man.alloc_list();
}


void li_cell8_free(void *ptr)
{
  li_mem_man.free_list((li_list *)ptr);
}

int li_gc()
{
  jmp_buf env;       // save all registers on the stack
  setjmp(env);


  return li_mem_man.gc();
}


i4_bool li_valid_object(li_object *o)
{
  return li_mem_man.valid_object(o);
}
