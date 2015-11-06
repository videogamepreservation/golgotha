/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "memory/bmanage.hh"
#include "error/error.hh"
#include "file/file.hh"
#include <string.h>
#include <stdlib.h>

//#define i4_MEM_CLEAR 1

#ifdef i4_MEM_CHECK
static int i4_show_libc_mallocs=0;

static char *str_alloc(char *s) 
{
  if (i4_show_libc_mallocs)
    i4_warning("str_alloc : '%s'", s);

  int l=strlen(s)+1;
  char *t=(char *)::malloc(l);
  if (!t)
    i4_error("could not allocate memory for string on libc heap");
  memcpy(t,s,l);
  return t;
}

static void str_free(char *s)
{
  if (!s || s[0]==0)
    i4_error("trying to free libc str (0)");

  if (i4_show_libc_mallocs)
    i4_warning("str_free : '%s'", s);

  ::free(s);
}

#endif

int i4_block_manager_class::valid_ptr(void *ptr)
{
  void *next=(void *)(*(((long *)ptr)-1));
  if (next && next<ptr)                        // small allocation
  {
    small_block *s=(small_block *)next;
    if (s->size<=0) return 0;

    small_block *c=sblocks[s->size];
    while (c && c!=s) c=c->next;
    if (!c) return 0;
    return 1;
  }

  memory_node *o=(memory_node *)(((char *)ptr)-sizeof(memory_node));
  memory_node *f=sfirst;
  while (f && f!=o) f=f->next;
  if (f) return 1;
  else return 0;
}


void i4_block_manager_class::inspect()
{
  memory_node *f=sfirst;
  for (;f;f=f->next);               // scan through static big list

  int i,bit=1;
  for (i=0;i<JM_SMALL_SIZE;i++)
  {
    for (small_block *s=sblocks[i];s;s=s->next)
    {
      char *addr=((char *)(s+1));
      bit = 1;
      for (int j=0;j<32;j++)
      {
        if (s->alloc_list&bit)
        {
          void *next=(void *)(*(((long *)addr)));
          if ((long)next!=(long)s)
          {
            i4_warning("inspect : bad pointer\n");
            return;
          }
        }
        bit=bit<<1;
        addr+=s->size+4;
      }
    }
  }
}

static char *check_allocation(char *s)
{
  char tmp[200];
  strcpy(tmp,s);
  ::free(s);
  s=(char *)::malloc(strlen(tmp)+1);
  strcpy(s,tmp);
  return s;
}

void i4_block_manager_class::report(i4_file_class *fp)
{
  fp->printf("************** Block size = %d ***************\n",block_size);
  int i=0;
  memory_node *f=sfirst;
  int f_total=0, a_total=0;

  for (;f;f=f->next,i++)
  {    
    fp->printf("%4d   %p (%d) %4d      ",i,f,((char *)f-(char *)sfirst),f->size);
    if (f->size>0)
      a_total+=f->size;
    else
      f_total+=-f->size;

#ifdef i4_MEM_CHECK
    if (f->size>0)    
      fp->printf("%s",check_allocation(f->name));
    else 
      fp->printf("FREE");
#endif
    fp->printf("\n");
  }    
  for (i=0;i<JM_SMALL_SIZE;i++)
  {
    for (small_block *s=sblocks[i];s;s=s->next)
    {      
      fp->printf("*** Small Block size = %d ***\n",i);      
      unsigned long bit=1;
      char *addr=((char *)(s+1));
      for (int j=0;j<32;j++)
      {
	fp->printf("%p   ",addr);
	if (s->alloc_list&bit)
	{
#ifdef i4_MEM_CHECK
	  fp->printf("%s\n",check_allocation(s->name[j]));
#else
	  fp->printf("allocated\n");
#endif	  
	} else fp->printf("FREE\n");
	bit=bit<<1;
	addr+=s->size+4;
      }
    }
  }

  fp->printf("**************** Block summary : %d free, %d allocated\n", f_total, a_total);

}

long i4_block_manager_class::pointer_size(void *ptr)
{
  long ret;
  void *next=(void *)(*(((long *)ptr)-1));
  if (next>ptr)
    ret=((memory_node *)(((char *)ptr)-sizeof(memory_node)))->size;
  else
    ret=((small_block *)next)->size;  

  return ret;
}


long i4_block_manager_class::largest_free_block()
{
  long l=0;
  memory_node *f;
  for (f=sfirst;f;f=f->next)
    if (-f->size>l)
      l=-f->size;

  return l;
}

long i4_block_manager_class::available()
{
  long size=0;
  memory_node *f;
  for (f=sfirst;f;f=f->next)
    if (f->size<0) size-=f->size;

  return size;
}

long i4_block_manager_class::allocated()
{
  long size=0;
  memory_node *f;
  for (f=sfirst;f;f=f->next)
    if (f->size>0) size+=f->size;

  return size;
}

void i4_block_manager_class::init(void *block, long Block_size)
{
  block_size=Block_size;
  addr=block;
  /* 
     I'm padding each block, because I'm comparing pointers against size
     in jfree to determine weither a pointer is too a small object or a large alloc
     and it must always be true that the address of the pointer is > JM_SMALL_SIZE 
     All systems I know start pointer address pretty high, but this is a porting consern.     
  */
  
  slast=sfirst=(memory_node *)(((char *)block)+JM_SMALL_SIZE);   
  sfirst->size=-(block_size-(sw32)sizeof(memory_node)-(sw32)JM_SMALL_SIZE);
  sfirst->next=NULL;
  memset(sblocks,0,sizeof(sblocks));

}

void *i4_block_manager_class::alloc(long size, char *name)
{
  if (size<JM_SMALL_SIZE)
  {
    small_block *s=sblocks[size];
    for (;s && s->alloc_list==0xffffffff;s=s->next);
    if (!s)
    {
      s=(small_block *)i4_block_manager_class::alloc((size+4)*32+sizeof(small_block),"small_block");
      if (!s) return NULL;   // not enough room for another small block
      s->alloc_list=1;
      s->next=sblocks[size];
      sblocks[size]=s;
      s->size=size;
#ifdef i4_MEM_CHECK
      s->name[0]=str_alloc(name);
#endif      
      long *addr=(long *)(((char *)s)+sizeof(small_block));
      *addr=(long)s;
      return (void *)(addr+1);  // return first block
    } else
    {
      int bit=1,i=0;
      char *addr=((char *)s)+sizeof(small_block);
      while (1)        // we already know there is a bit free
      {
	if ((s->alloc_list&bit)==0)
	{
	  s->alloc_list|=bit;
#ifdef i4_MEM_CHECK
	  s->name[i]=str_alloc(name);
#endif      	 
	  *((long *)addr)=(long)s;

	  return (void *)(addr+4);
	}
	i++;
	bit=bit<<1;
	addr+=size+4;
      }      
    }                
  }


  memory_node *s=sfirst;
  if (!s) return NULL;
  for (;s && -s->size<size;s=s->next);  
  if (!s) return NULL;
  s->size=-s->size;

  if (s->size-size>sizeof(memory_node)+4)  // is there enough space to split the block?
  {    
    memory_node *p=(memory_node *)((char *)s+sizeof(memory_node)+size);
    if (s==slast)
      slast=p;
    p->size=-(s->size-size-(sw32)sizeof(memory_node));
#ifdef i4_MEM_CLEAR
    memset( ((memory_node *)p)+1,0,-p->size);
#endif
    p->next=s->next;
    s->next=p;
    s->size=size;
  }
#ifdef i4_MEM_CHECK
  s->name=str_alloc(name);
#endif
  return (void *)(((char *)s)+sizeof(memory_node));
}


int i4_show_frees=0;

/************************** FREE **********************************/
/*    should be called to free a pointer in the static heap       */
/*    i.e. begining of the heap                                   */
/******************************************************************/
void i4_block_manager_class::free(void *ptr)
{
  // see if this was a small_block allocation
  void *next=(void *)(*(((long *)ptr)-1));
  if (next && next<ptr)  // small allocation
  {
    small_block *s=(small_block *)next;
    if (s->size<=0)
    {
      i4_warning("i4_free : bad pointer\n");
      return ;
    }
#ifdef i4_MEM_CLEAR
    memset(ptr,0,s->size);
#endif

    int field=(((char *)ptr)-((char *)s)-sizeof(small_block))/(s->size+4);
#ifdef i4_MEM_CHECK
    if (i4_show_frees)
      i4_warning("small free : %s",s->name[field]);
    str_free(s->name[field]);
#endif
    s->alloc_list&=(0xffffffff-(1<<field));
    if (s->alloc_list==0)
    {
      small_block *l=NULL;
      small_block *n=sblocks[s->size];
      for (;n!=s;n=n->next) l=n;
#ifdef i4_MEM_CHECK
      if (!n) { i4_warning("Free small block error\n"); }
#endif
      if (!l)
      sblocks[s->size]=s->next;
      else l->next=s->next;
      i4_block_manager_class::free(s);
    }      
  } else
  {
    memory_node *o=(memory_node *)(((char *)ptr)-sizeof(memory_node)),*last=NULL;
#ifdef i4_MEM_CHECK
    if (i4_show_frees)
      i4_warning("big free : %s",o->name);
    str_free(o->name);
#endif
#ifdef i4_MEM_CLEAR
    memset(ptr,0,o->size);
#endif

    if (o->next && o->next->size<0)   // see if we can add into next block
    {
      if (o->next==slast)
        slast=o;
      o->size+=-o->next->size+sizeof(memory_node);
      o->next=o->next->next;
    }

    memory_node *n=sfirst;
    for (;n && n!=o;n=n->next) last=n;
#ifdef i4_MEM_CHECK
    if (!n) { i4_warning("Free big block error\n"); }
#endif
    
    if (last && last->size<0)
    {
      if (o==slast) {
        slast=last;
      }
      last->next=o->next;
      last->size-=o->size+sizeof(memory_node);	
    } else o->size=-o->size;            
  }  
}
