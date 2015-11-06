/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef DEBUG_WIN_HH
#define DEBUG_WIN_HH

#include "window/style.hh"

#ifdef DEBUG
// debugging enabled

#define g1_debug   g1_debug_printf
#define g1_debugxy g1_debug_printfxy

int g1_debug_close(i4_graphical_style_class *style);
int g1_debug_open(i4_graphical_style_class *style, 
		  i4_parent_window_class *parent, 
		  const i4_const_str &title,
		  int status_lines,
		  i4_event_reaction_class *closed);
int g1_debug_printf(char *s, ...);
int g1_debug_printfxy(int x, int y, char *s, ...);
#else
// no more!

#define g1_debug               if (0)
#define g1_debugxy             if (0)
#define g1_debug_close(style)  (0)
#define g1_debug_open(a,b,c,d,e) (0) 
#define g1_debug_printf        if (0)
#define g1_debug_printfxy      if (0)
#endif

#endif


