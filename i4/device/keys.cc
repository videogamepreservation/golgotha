/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "device/keys.hh"
#include <string.h>

static char *i4_extended_key_names[]= 
//{{{
{
  "Up",
  "Down",
  "Left",
  "Right",
  "Left Ctrl",
  "Right Ctrl",
  "Left Alt",
  "Right Alt",
  "Left Shift",
  "Right Shift",
  "Caps Lock",
  "Num Lock",
  "Home",
  "End",
  "Del",
  "F1",
  "F2",
  "F3",
  "F4",
  "F5",
  "F6",
  "F7",
  "F8",
  "F9",
  "F10",
  "F11",
  "F12",
  "F13",
  "F14",
  "F15",
  "Insert",
  "PageUp",
  "PageDown",
  "Command",
  "Pad 0",
  "Pad 1",
  "Pad 2",
  "Pad 3",
  "Pad 4",
  "Pad 5",
  "Pad 6",
  "Pad 7",
  "Pad 8",
  "Pad 9",
  "Pad =",
  "Pad /",
  "Pad *",
  "Pad -",
  "Pad +",
  "Pad Enter",
  "Pad ."
};
//}}}

static struct i4_key_alias_struct { i4_key key; char *name; } 
i4_key_alias[] =
{
  { I4_BACKSPACE,    "Backspace" },
  { I4_TAB,          "Tab"       },
  { I4_ENTER,        "Enter"     },
  { I4_ESC,          "Escape"    },
  { I4_SPACE,        "Space"     }
};
const int i4_key_aliases=sizeof(i4_key_alias)/sizeof(i4_key_alias_struct);

static struct i4_modifier_map_struct { w16 modifier; char *name; } 
i4_modifier_map[] =
{
  { I4_MODIFIER_CTRL,    "Ctrl"  },
  { I4_MODIFIER_ALT,     "Alt"   },
  { I4_MODIFIER_SHIFT,   "Shift" },
};
const int i4_modifiers = sizeof(i4_modifier_map)/sizeof(i4_modifier_map_struct);


char *i4_get_key_name(i4_key key, w16 modifiers, char *out)
//{{{
{
  char *buffer,*p;
  char sing[2];
  w32 i;

  if (key>255 && key<I4_NUM_KEYS)
    // extended key name
    buffer = i4_extended_key_names[key-256];
  else
  {
    // search standard key aliases
    i=0; 
    while (i<i4_key_aliases && key!=i4_key_alias[i].key)
      i++;

    if (i<i4_key_aliases)
      // found key alias
      buffer = i4_key_alias[i].name;
    else 
    {
      // not found, assume normal ascii key
      buffer = sing;
      buffer[0]=(key>='a' && key<='z') ? (key-'a'+'A') : key;
      buffer[1]=0;
    }
  }

  p=out;
  for (i=0; i<i4_modifiers; i++)
    if (modifiers & i4_modifier_map[i].modifier)
    {
      strcpy(p,i4_modifier_map[i].name);
      p += strlen(p);
      *p++='+';
    }
  strcpy(p,buffer);
  
  return out;
}
//}}}


i4_str *i4_key_name(i4_key key, w16 modifiers)
//{{{
{
  char buff[256];
  
  return i4_from_ascii(i4_get_key_name(key,modifiers,buff));
}
//}}}

i4_bool i4_find_key(const i4_str& name, i4_key &key, w16 &mod)
//{{{
{
  i4_str::iterator p(name.begin());
  char buf[256];
  w32 len=0;
  w32 i;

  // clear values
  mod = key = 0;
  int combo_key=0;

  // parse key names & modifiers
  buf[0]=0;
  while ((*p).value() && len<sizeof(buf)-1)
  {
    while (len<sizeof(buf)-1 && (*p).value() && *p!=i4_char('+'))
    {
      buf[len] = (*p).value();
      ++len;
      ++p;
    }
    buf[len]=0;

    if ((*p).value())
    {
      combo_key=1;
      ++p;
      if (!(*p).value())
      {
        buf[len++]='+';
        buf[len]=0;
      }
      else
        len = 0;
    }

    // scan for modifiers
    for (i=0; i<i4_modifiers; i++)
      if (!strcmp(buf, i4_modifier_map[i].name))
        mod |= i4_modifier_map[i].modifier;
  } 
  if (len>=sizeof(buf)-1 || len==0)
    // no key name can be THAT long or THAT short
    return i4_F;
  if (len>1)
  {
    // found a named key

    // search common key aliases
    i=0;
    while (i<i4_key_aliases && strcmp(buf, i4_key_alias[i].name))
      i++;
    
    if (i<i4_key_aliases)
      key = i4_key_alias[i].key;
    else
    {
      // search extended key names
      i=256;
      while (i<I4_NUM_KEYS && strcmp(buf,i4_extended_key_names[i-256]))
        i++;

      if (i<I4_NUM_KEYS)
        key = i;
      else if (combo_key || mod==0)
        return i4_F;
      else 
        return i4_T;
    }
  }
  else
    // use ascii value for key (uppercase)
    key=(buf[0]>='a' && buf[0]<='z') ? (buf[0]-'a'+'A') : buf[0];

  return i4_T;
}
//}}}


enum
//{{{ Internal modifier key translation flags
{
  I4_SHIFT_L_FLAG = 1,
  I4_SHIFT_R_FLAG = 2,
  I4_CAPS_FLAG = 4,
  I4_SHIFT_FLAGS = 7,
  I4_NUM_LOCK_FLAG = 8,
  I4_CTRL_L_FLAG = 16,
  I4_CTRL_R_FLAG = 32,
  I4_CTRL_FLAGS = 48
};
//}}}


w16 i4_key_translate(i4_key raw_key, i4_bool press, w16 &state)
//{{{
{
  switch (raw_key)
  {
    case I4_SHIFT_L:
      if (press)
        state |= I4_MODIFIER_SHIFT_L;
      else
        state &= ~I4_MODIFIER_SHIFT_L;
      break;
    case I4_SHIFT_R:
      if (press)
        state |= I4_MODIFIER_SHIFT_R;
      else
        state &= ~I4_MODIFIER_SHIFT_R;
      break;

    case I4_CAPS:
      if (press)
        state ^= I4_MODIFIER_CAPS;
      break;

    case I4_NUM_LOCK:
      if (press)
        state ^= I4_MODIFIER_NUMLOCK;
      break;

    case I4_CTRL_L:
      if (press)
        state |= I4_MODIFIER_CTRL_L;
      else
        state &= ~I4_MODIFIER_CTRL_L;
      break;

    case I4_CTRL_R:
      if (press)
        state |= I4_MODIFIER_CTRL_R;
      else
        state &= ~I4_MODIFIER_CTRL_R; 
      break;

    case '1':  return (state & I4_MODIFIER_SHIFT)? '!' : '1';
    case '2':  return (state & I4_MODIFIER_SHIFT)? '@' : '2';
    case '3':  return (state & I4_MODIFIER_SHIFT)? '#' : '3';
    case '4':  return (state & I4_MODIFIER_SHIFT)? '$' : '4';
    case '5':  return (state & I4_MODIFIER_SHIFT)? '%' : '5';
    case '6':  return (state & I4_MODIFIER_SHIFT)? '^' : '6';
    case '7':  return (state & I4_MODIFIER_SHIFT)? '&' : '7';
    case '8':  return (state & I4_MODIFIER_SHIFT)? '*' : '8';
    case '9':  return (state & I4_MODIFIER_SHIFT)? '(' : '9';
    case '0':  return (state & I4_MODIFIER_SHIFT)? ')' : '0';
    case '-':  return (state & I4_MODIFIER_SHIFT)? '_' : '-';
    case '=':  return (state & I4_MODIFIER_SHIFT)? '+' : '=';
    case '[':  return (state & I4_MODIFIER_SHIFT)? '{' : '[';
    case ']':  return (state & I4_MODIFIER_SHIFT)? '}' : ']'; 
    case '\\': return (state & I4_MODIFIER_SHIFT)? '|' : '\\'; 
    case ';':  return (state & I4_MODIFIER_SHIFT)? ':' : ';'; 
    case '\'': return (state & I4_MODIFIER_SHIFT)? '"' : '\''; 
    case ',':  return (state & I4_MODIFIER_SHIFT)? '<' : ','; 
    case '.':  return (state & I4_MODIFIER_SHIFT)? '>' : '.'; 
    case '/':  return (state & I4_MODIFIER_SHIFT)? '?' : '/'; 
    case '`':  return (state & I4_MODIFIER_SHIFT)? '~' : '`'; 

    case I4_KP0:
    case I4_KP1:
    case I4_KP2:
    case I4_KP3:
    case I4_KP4:
    case I4_KP5:
    case I4_KP6:
    case I4_KP7:
    case I4_KP8:
    case I4_KP9:
      if (((state & I4_MODIFIER_NUMLOCK)!=0) ^ 
          ((state & I4_MODIFIER_SHIFT)!=0))
        return raw_key - I4_KP0 + '0';
      else
        return raw_key;
      
      
    default:
      if (raw_key>='A' && raw_key<='Z')
        if (state & I4_MODIFIER_CTRL)
          return raw_key - 'A' + 1;
        else if (!(state & I4_MODIFIER_SHIFT))
          return raw_key - 'A' + 'a';
      
  }
  return raw_key;
}
//}}}


//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
