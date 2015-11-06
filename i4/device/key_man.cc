/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "device/key_man.hh"
#include "device/event.hh"
#include "file/file.hh"
#include "error/alert.hh"
#include "device/kernel.hh"
#include "memory/growheap.hh"

i4_key_man_class i4_key_man;

i4_key_man_class::i4_key_man_class()
{
  loaded=0;
  active_list=0;
  context_list=0;
  command_list=0;
  char_heap=0;
  memset(keys, 0, sizeof(keys));
}


// end commands no longer appropriate for for the current modifiers
void i4_key_man_class::end_actives(int matches_key, i4_time_class &time)
{  
  key_item *last=0;
  for (key_item *j=active_list; j; )
  {
    if (j->modifier_flags!=key_modifiers_pressed || j->key==matches_key)
    {
      key_item *q=j;
      j=j->next_active;

      if (last)
        last->next_active=q->next_active;
      else
        active_list=active_list->next_active;
      
      i4_end_command_event_class kcmd( (*command_list)[q->command_id], q->command_id, time);
      send_event_to_agents(&kcmd, FLAG_END_COMMAND);

      q->command_active=0;
    }
    else 
    {
      last=j;
      j=j->next_active;
    }
  }
}


void i4_key_man_class::get_modifiers(int k_mod)
{
  if (k_mod)                     // turn left & right into same thing
  {
    if (k_mod & I4_MODIFIER_SHIFT)
      k_mod=I4_MODIFIER_SHIFT;
    if (k_mod & I4_MODIFIER_CTRL)
      k_mod=I4_MODIFIER_CTRL;
    if (k_mod & I4_MODIFIER_ALT)
      k_mod=I4_MODIFIER_ALT;

    // don't uses these modifiers
    if (k_mod & (I4_MODIFIER_WINDOWS | I4_MODIFIER_CAPS | I4_MODIFIER_NUMLOCK))
      k_mod &= ~(I4_MODIFIER_WINDOWS | I4_MODIFIER_CAPS | I4_MODIFIER_NUMLOCK);
  }

  if (context_list)
    k_mod &= ~((*context_list)[context_id].modifiers_taken);


  key_modifiers_pressed=k_mod;
}


void i4_key_man_class::add_active(i4_key_man_class::key_item *i, i4_time_class &time)
{
  if (!i->command_active)
  {
    i->command_active=1;
    i->next_active=active_list;
    active_list=i;

    i4_do_command_event_class kcmd( (*command_list)[i->command_id], i->command_id, time);
    send_event_to_agents(&kcmd, FLAG_DO_COMMAND);
  }
}


void i4_key_man_class::receive_event(i4_event *ev)
{
  if (!loaded) return ;

  if (ev->type()==i4_event::KEY_PRESS)
  {
    CAST_PTR(kev, i4_key_press_event_class, ev);
    int old_modifiers=key_modifiers_pressed;

    get_modifiers(kev->modifiers);

    if (old_modifiers!=key_modifiers_pressed)
    {
      for (key_item *i=active_list; i; i=i->next_active)
      {
        for (key_item *j=keys[i->key]; j; j=j->next)
        {
          if (j!=i && j->modifier_flags==key_modifiers_pressed && 
              (j->context_mask&(1<<context_id)))
            add_active(j, kev->time);

        }
      }

      end_actives(-1, kev->time);
    }

    for (key_item *i=keys[kev->key_code]; i; i=i->next)
      if (key_modifiers_pressed == i->modifier_flags && (i->context_mask& (1<<context_id)))
        add_active(i, kev->time);

  } else if (ev->type()==i4_event::KEY_RELEASE)
  {
    CAST_PTR(kev, i4_key_press_event_class, ev);

    int old_modifiers=key_modifiers_pressed;
    get_modifiers(kev->modifiers);

    if (old_modifiers!=key_modifiers_pressed)
    {
      for (key_item *i=active_list; i; i=i->next_active)
      {
        for (key_item *j=keys[i->key]; j; j=j->next)
        {
          if (j!=i && j->modifier_flags==key_modifiers_pressed && 
              (j->context_mask & (1<<context_id)))
            add_active(j, kev->time);

        }
      }

      end_actives(-1, kev->time);
    }


    end_actives(kev->key_code, kev->time);    
  }
}

static i4_bool is_white(char *s)
{
  if (*s==' ' || *s=='\n' || *s=='\r' || *s=='\t') 
    return i4_T;
  else return i4_F;
}

static void skip_white(char *&s)
{
  while (*s && is_white(s)) s++;
}

static i4_bool i4_go_key_start(char *&s)
{
  while (*s && *s!='(')
  {
    if (*s=='#') 
    {
      while (*s && (*s!='\n' && *s!='\r')) 
        s++;
    } else s++;
  }
  
  if (*s)
  {
    while (*s && *s!=' ') s++;
    skip_white(s);
    return i4_T;
  }
  else return i4_F;  
}

static char get_char(char *&s)
{
  if (*s=='\\')
  {
    s+=2;
    if (s[-1]=='n') return '\n';
    if (s[-1]=='r') return '\r';
    if (s[-1]=='t') return '\t';
    if (s[-1]=='b') return '\b';    
    if (s[-1]=='\\') return '\\';    
  }
  else 
  {
    s++;
    return s[-1];
  }
  return '\\';
}

static void i4_read_str(char *&s, char *buf)
{
  skip_white(s);
  if (s[0]=='"')
  {
    s++;
    while (*s && *s!='"')      
      *(buf++)=get_char(s);
    *buf=0;
    s++;
  }
  else
  {
    *(buf++)=*(s++);
    while (*s && !is_white(s) && *s!=')') 
      *(buf++)=get_char(s);
    *buf=0;
  }
}

int i4_key_man_class::acquire_modifiers_for_contexts(int context_mask, int mod, char *key_name)
{
  int c=context_mask, i=0, total=0, skip_this_key=0;
  while (c)
  {
    if (c&1)
    {
      if (((*context_list)[i].modifiers_used & mod))
      { 
        i4_alert(i4gets("modifier_in_use"),200, key_name);
        return 0;
      }
      else
      {
        (*context_list)[i].modifiers_used |= mod;
        (*context_list)[i].modifiers_taken |= mod;
      }
    }

    c>>=1;
    i++;
  }
 
  return 1;
}

int i4_key_man_class::use_modifiers_for_contexts(int context_mask, int mod, char *key_name)
{
 int c=context_mask, i=0;

  while (c)
  {
    if (c&1)
    {
      if (((*context_list)[i].modifiers_taken & mod))
      { 
        i4_alert(i4gets("modifier_in_use"),200, key_name);
        return 0;
      }
      else
        (*context_list)[i].modifiers_used |= mod;
    }

    c>>=1;
    i++;
  }

  return 1;
}

i4_bool i4_key_man_class::load(const i4_const_str &filename)
{
  check_init();
  i4_file_class *fp=i4_open(filename);
  if (!fp) return i4_F;

  int size=fp->size();
  void *mem=i4_malloc(size+1,"");
  fp->read(mem,size);
  delete fp;
  
  char *c=(char *)mem;
  c[size]=0;

  char tmp[256];
  
  int x=0;
  while (i4_go_key_start(c))
  {
    w16 mod;
    i4_key key;
    char key_name[256],cmd[256];
    int skip_key=0;

    x++;
    i4_read_str(c,key_name);    
    if (!i4_find_key(i4_const_str(key_name), key, mod))
    {
      i4_alert(i4gets("no_key"),100, key_name, &filename);
      skip_key=1;
    }

    i4_read_str(c,cmd);
    int id=get_command_id(cmd);

    int context_mask=0;
    do
    {
      i4_read_str(c,tmp);
      if (tmp[0] && tmp[0]!=')')
        context_mask|=(1<<get_context_id(tmp));
    } while (tmp[0]!=')' && tmp[0]);

    if (key==0 && mod!=0 && !skip_key)   // they want a CTRL-ALT-SHIFT type key
    {
      if (acquire_modifiers_for_contexts(context_mask, mod, key_name))
      {

        if (mod & I4_MODIFIER_CTRL)      
        {
          keys[I4_CTRL_L]=new key_item(context_mask, id, 0, I4_CTRL_L, keys[I4_CTRL_L]);
          keys[I4_CTRL_R]=new key_item(context_mask, id, 0, I4_CTRL_R, keys[I4_CTRL_R]);
        }

        if (mod & I4_MODIFIER_ALT)      
        {
          keys[I4_ALT_L]=new key_item(context_mask, id, 0, I4_ALT_L, keys[I4_ALT_L]);
          keys[I4_ALT_R]=new key_item(context_mask, id, 0, I4_ALT_R, keys[I4_ALT_R]);
        }

        if (mod & I4_MODIFIER_SHIFT)      
        {
          keys[I4_SHIFT_L]=new key_item(context_mask, id, 0, I4_SHIFT_L, keys[I4_SHIFT_L]);
          keys[I4_SHIFT_R]=new key_item(context_mask, id, 0, I4_SHIFT_R, keys[I4_SHIFT_R]);
        }

        if (mod & I4_MODIFIER_WINDOWS)
          keys[I4_COMMAND]=new key_item(context_mask, id, 0, I4_COMMAND, keys[I4_COMMAND]);

        if (mod & I4_MODIFIER_CAPS)
          keys[I4_CAPS]=new key_item(context_mask, id, 0, I4_CAPS, keys[I4_CAPS]);

        if (mod & I4_MODIFIER_NUMLOCK)
          keys[I4_NUM_LOCK]=new key_item(context_mask, id, 0, I4_NUM_LOCK, keys[I4_NUM_LOCK]); 
      }
    }
    else if (!skip_key)
    {
      if (use_modifiers_for_contexts(context_mask, mod, key_name))
      {
        // make sure the key isn't already assigned
        for (key_item *i=keys[key]; i; i=i->next)
        {        
          if (i->modifier_flags==mod && (i->context_mask & context_mask))
            i4_error("attempting to assign command %s but key %s (%d) already command %s",
                     (*command_list)[i->command_id], key_name, key, (*command_list)[id]);

        }

        keys[key]=new key_item(context_mask, id, mod, key, keys[key]);    
      }
    }
  }

  i4_free(mem);
  loaded=i4_T;
  return i4_T;
}


void i4_key_man_class::uninit()
{
  if (!command_list) return;

  i4_time_class now;
  while (active_list)
    end_actives(active_list->key, now);

  int i;
  for (i=0; i<I4_NUM_KEYS; i++)
  {
    while (keys[i])
    {
      key_item *ki=keys[i];
      keys[i]=keys[i]->next;
      delete ki;
    }
  }

  delete command_list;  command_list=0;
  delete context_list;  context_list=0;
  delete char_heap;     char_heap=0;

  i4_kernel.unrequest_events(this, 
                             i4_device_class::FLAG_KEY_PRESS | i4_device_class::FLAG_KEY_RELEASE);
}

char *i4_key_man_class::alloc_str(char *s)
{
  int l=strlen(s)+1;
  char *t=(char *)char_heap->malloc(l,"");
  memcpy(t,s,l);
  return t;
}

void i4_key_man_class::check_init()
{
  if (!context_list)
  {
    context_list = new i4_array<context>(32,32);
    command_list = new i4_array<char *>(32,32);
    char_heap = new i4_grow_heap_class(2048, 2048);

    i4_kernel.request_events(this, 
                             i4_device_class::FLAG_KEY_PRESS | i4_device_class::FLAG_KEY_RELEASE);
  }
}

int i4_key_man_class::get_context_id(char *context_name)
{
  check_init();
  int s=context_list->size();
  for (int i=0; i<s; i++)
    if (strcmp( (*context_list)[i].name, context_name)==0)
      return i;

  if (context_list->size()==32)
    i4_error("max contexts exceed with %s", context_name);

  context *c=context_list->add();
  c->name=alloc_str(context_name);
  c->modifiers_taken=0;
  c->modifiers_used=0;

  return s;
}

int i4_key_man_class::get_command_id(char *command)
{
  check_init();
  int s=command_list->size();
  for (int i=0; i<s; i++)
    if (strcmp( (*command_list)[i], command)==0)
      return i;

  command_list->add(alloc_str(command));
  return s;
}


i4_bool i4_key_man_class::get_key_for_command(int command_id, i4_key &key, w16 &mod)
{
  for (int i=0; i<I4_NUM_KEYS; i++)
  {
    for (key_item *k=keys[i]; k; k=k->next)
    {
      if ((k->context_mask & (1<<context_id)) && (k->command_id == command_id))
      {
        key=i;
        mod=k->modifier_flags;
        return i4_T;
      }
    }
  }

  return i4_F;
}



void i4_key_matchup_class::add(char *command, int remap)   
{ 
  matchup.insert(new command_matchup(i4_key_man.get_command_id(command), remap)); 
}

int i4_key_matchup_class::remap(int command_id) 
{
  command_matchup f=command_matchup(command_id,0);
  command_matchup *m=matchup.find(&f);
  if (m)
    return m->remap_id;
  else return -1;
}
