/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef LI_SYMBOLS_HH
#define LI_SYMBOLS_HH


#include "lisp/li_types.hh"

// global symbols
extern li_symbol *li_nil, 
  *li_true_sym, 
  *li_quote, 
  *li_backquote,
  *li_comma,
  *li_function_symbol;

li_symbol *li_get_symbol(char *name);     // if symbol doesn't exsist, it is created

// if cache_to is 0, then the symbol is found and stored there, otherwise cache_to is returned
li_symbol *li_get_symbol(char *name, li_symbol *&cache_to);

void li_mark_symbols(int set);

#endif
