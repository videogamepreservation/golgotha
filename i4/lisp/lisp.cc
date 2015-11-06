/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "error/error.hh"
#include "main/main.hh"
#include "init/init.hh"
#include "file/file.hh"
#include "lisp/li_types.hh"
#include "lisp/lisp.hh"
#include "status/status.hh"
#include "threads/threads.hh"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

char li_last_file[150];
int li_last_line=0;

// returns the length of the list
int        li_length(li_object *o, li_environment *env) 
{
  if (o->type()!=LI_LIST)
    return 0;
  else
  {
    int t=0;
    while (o)
    {
      t++;
      o=li_cdr(o, env);
    }
    return t;
  }
    
}

i4_bool li_is_number(li_object *o) 
{ 
  return (i4_bool)(o->type()==LI_INT || o->type()==LI_FLOAT); 
}


float li_get_float(li_object *o, li_environment *env)  // will convert int to float
{
  if (o->type()==LI_INT)
    return li_int::get(o, env)->value();
  else
    return li_float::get(o, env)->value();
}

int li_get_int(li_object *o, li_environment *env)    // will convert float to int
{
  if (o->type()==LI_FLOAT)
    return (int)li_float::get(o, env)->value();
  else
    return li_int::get(o, env)->value();
}

char *li_get_string(li_object *o, li_environment *env)
{
  return li_string::get(o, env)->value();
}


void li_skip_c_comment(char *&s)
{
  s+=2;
  while (*s && (*s!='*' || *(s+1)!='/'))
  {
    if (*s=='/' && *(s+1)=='*')
      li_skip_c_comment(s);
    else s++;
  }
  if (*s) s+=2;
}


int li_read_token(char *&s, char *buffer)
{
  // skip space
  while (*s==' ' || *s=='\t' || *s=='\n' || *s=='\r' || *s==26)
  {
    if (*s=='\n')
      li_last_line++;
    s++;
  }
  
  if (*s==';')  // comment
  {
    while (*s && *s!='\n' && *s!=26)
    {
      if (*s=='\n')
        li_last_line++;
      s++;
    }
    
    return li_read_token(s,buffer);
  } else if  (*s=='/' && *(s+1)=='*')   // c style comment
  {
    li_skip_c_comment(s);
    return li_read_token(s,buffer);    
  }
  else if (*s==0)
    return 0;
  else if (*s==')' || *s=='(' || *s=='\'' || *s=='`' || *s==',' || *s==26)
  {
    *(buffer++)=*(s++);
    *buffer=0;
  } else if (*s=='"')    // string
  {
    *(buffer++)=*(s++);          // don't read off the string because it
    // may be to long to fit in the token buffer
    // so just read the '"' so the compiler knows to scan the rest.
    *buffer=0;
  } else if (*s=='#')
  {
    *(buffer++)=*(s++);      
    if (*s!='\'')
      *(buffer++)=*(s++);      
    *buffer=0;
  } else
  {
    while (*s && *s!=')' && *s!='(' && *s!=' ' && 
           *s!='\n' && *s!='\r' && *s!='\t' && *s!=';' && *s!=26)
      *(buffer++)=*(s++);      
    *buffer=0;
  }
  return 1;    
}

int li_streq(char *s1, char *s2)
{
  return strcmp(s1,s2)==0;
}


long li_str_token_len(char *st)
{
  long x=1;
  while (*st && (*st!='"' || st[1]=='"'))
  {
    if (*st=='\\' || *st=='"') st++;    
    st++; x++;
  }
  return x;
}

static i4_critical_section_class token_buf_lock;
enum {MAX_LISP_TOKEN_LEN=512};
static char li_token[MAX_LISP_TOKEN_LEN];  // assume all tokens will be < 512 characters


li_object *li_locked_get_expression(char *&s, li_environment *env)
{

  li_object *ret=0;

  if (!li_read_token(s,li_token))
    return 0;
  if (li_streq(li_token,"nil"))
    return li_nil;
  else if (li_token[0]=='T' && !li_token[1])
    return li_true_sym;
  else if (li_token[0]=='\'')                    // short hand for quote function
    return new li_list(li_quote, new li_list(li_locked_get_expression(s, env), 0));    
  else if (li_token[0]=='`')                    // short hand for backquote function
    return new li_list(li_backquote, new li_list(li_locked_get_expression(s, env),0));
  else if (li_token[0]==',')              // short hand for comma function
    return new li_list(li_comma, new li_list(li_locked_get_expression(s, env), 0));
  else if (li_token[0]=='(')                     // make a list of everything in ()
  {
    li_list *first=NULL,*cur=NULL,*last=NULL;   

    int done=0;
    do
    {
      char *tmp=s;
      if (!li_read_token(tmp,li_token))           // check for the end of the list
        li_error(env, "unexpected end of program");
      if (li_token[0]==')') 
      {
        done=1;
        li_read_token(s,li_token);                // read off the ')'
      }
      else
      {     
        if (li_token[0]=='.' && !li_token[1])
        {
          if (!first)
            li_error(env, "token '.' not allowed here : %s\n",s);	      
          else 
          {
            li_read_token(s,li_token);              // skip the '.'
            last->set_next(li_locked_get_expression(s, env));          // link the last cdr to 
            last=NULL;
          }
        } else if (!last && first)
          li_error(env, "illegal end of dotted list\n");
        else
        {	
          li_list *p=new li_list(li_locked_get_expression(s, env), 0);
          if (last)
            last->set_next(p);
          else
            first=p;
          last=p;
        }
      } 
    } while (!done);

    if (!first)
      return li_nil;
    else return first;

  } else if (li_token[0]==')')
    li_error(env, "mismatched ) at %s",s);
  else if (isdigit(li_token[0]) || (li_token[0]=='-' && isdigit(li_token[1])))
  {
    int i=0,per=0,hex=0,x;
    
    if (li_token[0]=='0' && li_token[1]=='x')     // hex number
    {
      hex=1;
      i+=2;
    }
        
    for (; li_token[i] && (isdigit(li_token[i]) || li_token[i]=='.' || li_token[i]=='-'); i++)
      if (li_token[i]=='.')
        per=1;

    if (per)
    {
      float y;
      sscanf(li_token,"%f",&y);      
      return new li_float(y);
    }
    else if (hex)
    {
      sscanf(li_token,"%x",&x);
      return new li_int(x);
    }
    else
    {
      sscanf(li_token,"%d",&x);
      return new li_int(x);
    }
  } else if (li_token[0]=='"')
  {
    li_string *r=new li_string(li_str_token_len(s));

    char *start=r->value();

    for (;*s && (*s!='"' || s[1]=='"');s++,start++)
    {
      if (*s=='\\')
      {
        s++;
        if (*s=='n') *start='\n';
        if (*s=='r') *start='\r';
        if (*s=='t') *start='\t';
        if (*s=='\\') *start='\\';
      } else *start=*s;
      if (*s=='"') s++;
    }
    *start=0;
    s++;

    return r;
  } else if (li_token[0]=='#')
  {
    if (li_token[1]=='\\')
    {
      li_read_token(s,li_token);                   // read character name
      if (li_streq(li_token,"newline"))
        ret=new li_character('\n');
      else if (li_streq(li_token,"space"))
        ret=new li_character(' ');       
      else 
        ret=new li_character(li_token[0]);       
    }
    else if (li_token[1]==0)                           // short hand for function
      return new li_list(li_function_symbol, new li_list(li_locked_get_expression(s, env), 0));
    else
    {
      li_error(env, "Unknown #\\ notation : %s\n",li_token);
      exit(0);
    }
  } else 
    return li_get_symbol(li_token);

  return ret;
}

// because we can only allow one thread to use the token buffer at a time
// so we don't have to allocate it on the stack (because it's fairly recursive)
// I lock access to the token buffer per thread
li_object *li_get_expression(char *&s, li_environment *env)
{
  token_buf_lock.lock();
  li_object *ret=li_locked_get_expression(s, env);
  token_buf_lock.unlock();
  return ret;
}


void lip(li_object *o)
{
  if (!o)
  {
    i4_debug->printf("(null object)\n");
    return ;
  }

  if (!li_valid_object(o))
  {
    i4_debug->printf("(invalid object)\n");
    return ;
  }

  li_get_type(o->type())->print(o, i4_debug);
  i4_debug->printf("\n");
}

li_object *li_print(li_object *o, li_environment *env)
{
  li_object *ret=0;
  while (o)
  {
    ret=li_eval(li_car(o,env),env);
    lip(ret);
    o=li_cdr(o,env);
  }
  return ret;
}

li_list *li_make_list(li_object *first, ...)
{
  va_list ap;
  va_start(ap, first);
  
  li_list *ret=new li_list(first,0), *last;
  last=ret;
  
  while (1)
  {
    li_object *o=va_arg(ap, li_object *);
    if (o)
    {
      li_list *next=new li_list(o,0);
      last->set_next(next);
      last=next;
    }
    else
    {
      va_end(ap);
      return ret;
    }
  }
}



li_object *li_get_fun(li_symbol *sym, li_environment *env) 
{ 
  if (env)
    return env->fun(sym);
  else return sym->fun();
}

li_object *li_get_fun(char *sym, li_environment *env)
{
  return li_get_fun(li_get_symbol(sym),env);
}


li_object *li_eval(li_object *expression, li_environment *env)
{
  if (!expression)
    return li_nil;
   
  int type=expression->type();
  switch (type)
  {    
    case LI_SYMBOL : 
    {
      li_object *v=li_get_value(li_symbol::get(expression,env), env);
      if (!v)
        li_error(env, "Symbol '%O' has no value", expression);
      return v;

    } break;

    case LI_LIST :
    {
      li_list *o=li_list::get(expression,env);
      li_symbol *sym=li_symbol::get(o->data(),env);
      return li_call(sym, o->next(), env);
    } break;

    default :
      return expression; 
      break;
  }

  return 0;
}


li_object *li_load(i4_file_class *fp, li_environment *env, i4_status_class *status)
{
  li_object *ret=0;
  li_last_line=0;


  int l=fp->size();

  char *buf=(char *)i4_malloc(l+1,"");
  buf[l]=0;
  fp->read(buf,l);

  char *s=buf;
  

  li_object *exp;
  do
  {
    if (status)
      status->update((s-buf)/(float)l);

    exp=li_get_expression(s, env);
    if (exp)
      ret=li_eval(exp, env);    
  } while (exp);

  i4_free(buf);
  return ret;
}

li_object *li_load(li_object *name, li_environment *env)
{
  return li_load(name, env, 0);
}

li_object *li_load(li_object *name, li_environment *env, i4_status_class *status)
{
  li_object *ret=0;

  char old_file[256];
  strcpy(old_file, li_last_file);
  int old_line=li_last_line;
  
  li_gc();

  while (name)
  {
    char *s=li_string::get(li_eval(li_car(name,env),env),env)->value();
    strcpy(li_last_file, s);

    i4_file_class *fp=i4_open(i4_const_str(s));
    if (fp)
    {
      ret=li_load(fp, env, status);
      delete fp;
    }
    else
      i4_warning("li_load : file missing %s", s);

    name=li_cdr(name,env);
  }
  
  strcpy(li_last_file, old_file);
  li_last_line=old_line;
  

  return ret;
}

li_object *li_read_eval(li_object *o, li_environment *env)
{
  char line[1000], *c=line;
  int t=0;
  i4_debug->printf("eval>");
  do
  {
    if (i4_debug->read(c,1)!=1)
      return 0;
    t++;
    c++;
  } while (c[-1]!='\n' && t<998);
  
  *c=0;
  c=line;
  li_object *ret=li_eval(li_get_expression(c, env), env);
  lip(ret);
  return ret;
}

li_object *li_load(char *filename, li_environment *env, i4_status_class *status)
{
  return li_load(new li_list(new li_string(filename), 0), env, status);
}

void li_add_function(li_symbol *sym, 
                     li_function_type fun,
                     li_environment *env)
{
  li_function *f=new li_function(fun);

  if (env)
    env->set_fun(sym, f);
  else
    sym->set_fun(f);
}


void li_add_function(char *sym_name, li_function_type fun, li_environment *env)
{
  li_add_function(li_get_symbol(sym_name), fun, env);
}

i4_bool li_get_bool(li_object *o, li_environment *env)
{ 
  if (!o) return i4_F;

  li_symbol *s=li_symbol::get(o,env);

  if (o==li_nil)
    return i4_F;
  else if (o==li_true_sym)
    return i4_T;
  else
    li_error(env, "expecting T or nil, got %O", o);

  return 0;
}

static inline int fmt_char(char c)
{
  if ((c>='a' && c<='z') || (c>='A' && c<='Z'))
    return 1;
  return 0;
}

static w8 li_recursive_error=0;

void li_vprintf(i4_file_class *fp,
                char *fmt,
                va_list ap)
{
  
  while (*fmt)
  {
    if (*fmt=='%')
    {
      char *fmt_end=fmt;
      while (!fmt_char(*fmt_end) && *fmt_end) fmt_end++;
      char f[10], out[500]; 
      memcpy(f, fmt, fmt_end-fmt+1);
      f[fmt_end-fmt+1]=0;
      out[0]=0;

      switch (*fmt_end)
      {
        case 'O' : 
        {
          li_object *o=va_arg(ap,li_object *);
          li_get_type(o->type())->print(o, fp);
        } break;

        case 'd' :
        case 'i' :
        case 'x' :
        case 'X' :
        case 'o' :
          ::sprintf(out,f,va_arg(ap,int));
          break;

        case 'f' :
        {
          float fl=va_arg(ap, double);
          ::sprintf(out,f,fl);
        } break;

        case 'g' :
          ::sprintf(out,f,va_arg(ap,double));
          break;

        default :
          ::sprintf(out,f,va_arg(ap,void *));
          break;
      }
      fp->write(out, strlen(out));
      fmt=fmt_end;
      if (*fmt)
        fmt++;
    }
    else
    {
      fp->write_8(*fmt);
      fmt++;
    }


  }
}


void li_printf(i4_file_class *fp,
               char *fmt,                   // typical printf format, with %o == li_object
              ...)
{  
  va_list ap;
  va_start(ap, fmt);
  li_vprintf(fp, fmt, ap);
  va_end(ap);

}

void li_error(li_environment *env,
              char *fmt,
              ...)
{
  if (!li_recursive_error)      // error shouldn't call error again!
  {
    li_recursive_error++;
    i4_file_class *fp=i4_open("li_error.txt", I4_WRITE);

    if (fp)
    {
      va_list ap;
      va_start(ap, fmt);
  
      li_vprintf(fp, fmt, ap);
      fp->printf("\nCall stack:\n");
      if (env)
        env->print_call_stack(fp);

      fp->printf("\nlast file %s:%d", li_last_file, li_last_line);
      delete fp;
      

      fp=i4_open("li_error.txt");
      if (fp)
      {
        int size=fp->size();
        char *b=(char *)i4_malloc(size+1,"");
        fp->read(b, size);
        b[size]=0;
        delete fp;

        i4_get_error_function_pointer(li_last_file, 0)(b);

        i4_free(b);
      }
    }

    li_recursive_error--;
  }  
}

li_object *li_new(char *type_name, li_object *params, li_environment *env)
{
  li_symbol *s=li_find_symbol(type_name);
  if (!s) return 0;

  li_object *v=li_get_value(s, env);
  if (!v || v->type()!=LI_TYPE) return 0;
  
  li_type_number type=li_type::get(v,env)->value();
  return li_get_type(type)->create(params, env);
}

li_object *li_new(int type, li_object *params, li_environment *env)
{
  return li_get_type(type)->create(params, env);
}

li_object *li_call(li_symbol *val, li_object *params, li_environment *env)
{
  if (val)
  {
    li_symbol *old_fun=0;
    li_object *old_args=0;
    if (env)
    {     
      old_fun=env->current_function();
      old_args=env->current_arguments();
    }
    else
      env=new li_environment(env, i4_F);

    env->current_function()=val;
    env->current_arguments()=params;
    
    
    li_object *ret=0;    
    li_object *f=li_get_fun(val, env);
    if (f)
    {
      li_function_type fun=li_function::get(f,env)->value();
      if (fun)
        ret=fun(params, env);      
    }
    else
      li_error(env, "symbol %O has no function", val);
    
    if (old_fun)
    {
      env->current_function()=old_fun;
      env->current_arguments()=old_args;
    }

    return ret;
  }

  return 0;
}

li_object *li_call(char *fun_name, li_object *params, li_environment *env)
{
  return li_call(li_get_symbol(fun_name), params, env);
}

li_object  *li_first(li_object *o, li_environment *env) { return li_car(o,env); }
li_object  *li_second(li_object *o, li_environment *env) { return li_car(li_cdr(o,env),env); }
li_object  *li_third(li_object *o, li_environment *env) { return li_car(li_cdr(li_cdr(o,env),env),env); }
li_object  *li_fourth(li_object *o, li_environment *env) 
{ return li_car(li_cdr(li_cdr(li_cdr(o,env),env),env),env); }

li_object  *li_fifth(li_object *o, li_environment *env) 
{ return li_car(li_cdr(li_cdr(li_cdr(li_cdr(o,env),env),env),env),env); }

li_object  *li_nth(li_object *o, int x, li_environment *env) 
{ while (x--) o=li_cdr(o,env); return li_car(o,env); }

