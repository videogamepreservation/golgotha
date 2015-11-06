/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

class i4_file_class;        // defined in i4/file/file.hh


class li_environment;

// typical printf format, with %O == li_object *
void li_printf(i4_file_class *fp, char *fmt, ...);

// typical printf format, with %O == li_object, calls i4_error with resulting string
void li_error(li_environment *env, char *fmt, ...);                

