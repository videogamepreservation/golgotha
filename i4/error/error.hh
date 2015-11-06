/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __I4_ERROR_HPP_
#define __I4_ERROR_HPP_
#include "dll_export.hh"

/* This is stub error and warning handler.
   Currently error calls exit() and warn continues with a fprintf.
   I expect this will be changed to pop up a dialog with an OK or something
   of the like. */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

typedef int (*i4_error_function_type)(const char *str);

void i4_set_error_function(i4_error_function_type fun);
i4_error_function_type i4_get_error_function();

void i4_set_warning_function(i4_error_function_type fun);
i4_error_function_type i4_get_warning_function();

int i4_default_warning(const char *st);
int i4_default_error(const char *st);


int i4_error_file_line(char *file, int line, const char *format, ...);
int i4_warning_file_line(char *file, int line, const char *format, ...);

class i4_file_class;
extern i4_file_class *i4_debug;     // stream you can print debug messages to


// this are so we can get the line and file an error occured on
typedef int (*i4_error_pointer_type)(const char *format ...);
i4_error_pointer_type i4_get_error_function_pointer(const char *file, int line);
i4_error_pointer_type i4_get_warning_function_pointer(const char *file, int line);
extern const char *i4_error_file_on;
extern int i4_error_line_on;



#ifdef DEBUG
#define I4_LF __LINE__, __FILE__
#define I4_LF_ARGS int I4_LINE, char *I4_FILE
#define I4_DEBUG
#else
#define I4_LF 
#define I4_LF_ARGS
#define I4_LINE 0
#define I4_FILE ""
#endif

#define I4_LF_ARG (char *I4_FILE, int I4_LINE)



// i4_error and i4_warning looks like i4_error(char *fmt, ...) to the user
#define i4_error (i4_get_error_function_pointer(__FILE__,__LINE__))
#define i4_warning (i4_get_warning_function_pointer(__FILE__,__LINE__))


#ifdef I4_DEBUG

#define I4_ASSERT(x,y) ((x)?0:i4_error("%s:%d - %s\n",__FILE__,__LINE__,y))
#define I4_TEST(x,y)   ((x)?0:i4_warning("%s:%d - %s\n",__FILE__,__LINE__,y))
#define H()            i4_warning("Reached %s:%d\n",__FILE__,__LINE__)

#else

#define I4_ASSERT(x,y) (0)
#define I4_TEST(x,y)   (0)
#define H()            (0)

#endif

#ifdef __cplusplus
}
#endif

#endif

