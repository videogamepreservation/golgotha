/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __KEYS_HPP_
#define __KEYS_HPP_

#include "arch.hh"
#include "string/string.hh"

typedef w16 i4_key;

enum i4_key_enum
//{{{
{
  I4_NO_KEY        =0,
  I4_BACKSPACE     =8,
  I4_TAB           =9,
  I4_ENTER        =13,
  I4_ESC          =27,
  I4_SPACE        =32,
  I4_UP           =256,
  I4_DOWN,
  I4_LEFT,
  I4_RIGHT,
  I4_CTRL_L,
  I4_CTRL_R,
  I4_ALT_L,
  I4_ALT_R,
  I4_SHIFT_L,
  I4_SHIFT_R,
  I4_CAPS,
  I4_NUM_LOCK,
  I4_HOME,
  I4_END,
  I4_DEL,
  I4_F1,
  I4_F2,
  I4_F3,
  I4_F4,
  I4_F5,
  I4_F6,
  I4_F7,
  I4_F8,
  I4_F9,
  I4_F10,
  I4_F11,
  I4_F12,
  I4_F13,
  I4_F14,
  I4_F15,
  I4_INSERT,
  I4_PAGEUP,
  I4_PAGEDOWN,
  I4_COMMAND,
  I4_KP0,
  I4_KP1,
  I4_KP2,
  I4_KP3,
  I4_KP4,
  I4_KP5,
  I4_KP6,
  I4_KP7,
  I4_KP8,
  I4_KP9,
  I4_KPEQUAL,
  I4_KPSLASH,
  I4_KPSTAR,
  I4_KPMINUS,
  I4_KPPLUS,
  I4_KPENTER,
  I4_KPPERIOD,
  I4_NUM_KEYS
};
//}}}

enum i4_key_modifer_state
{
  I4_MODIFIER_SHIFT_L=1,
  I4_MODIFIER_SHIFT_R=2,
  I4_MODIFIER_SHIFT=1+2,

  I4_MODIFIER_CTRL_L=4,
  I4_MODIFIER_CTRL_R=8,
  I4_MODIFIER_CTRL  =4+8,

  I4_MODIFIER_ALT_L=16,
  I4_MODIFIER_ALT_R=32,
  I4_MODIFIER_ALT  =16+32,

  I4_MODIFIER_WINDOWS=64,
  I4_MODIFIER_CAPS=128,
  I4_MODIFIER_NUMLOCK=256
};

i4_str *i4_key_name(i4_key key, w16 state);
// returns a ASCII string describing a key, i.e. "Up Arrow"

char *i4_get_key_name(i4_key key, i4_key state, char *buffer);
// fills buffer with key name

i4_bool i4_find_key(const i4_str& name, i4_key &key, w16 &mod);

w16 i4_key_translate(i4_key raw_key, i4_bool press, w16 &state);
// translates keypresses & shift states to standard input keys

#endif

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
