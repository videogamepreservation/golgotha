/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "error/error.hh"
#include "device/event.hh"

i4_do_command_event_class::i4_do_command_event_class(char *_command, int command_id)
  : command_id(command_id)
{
  int len=strlen(_command);
  if (len>=sizeof(command))
    i4_error("command too long");
  strcpy(command, _command);
}

i4_do_command_event_class::i4_do_command_event_class(char *_command, int command_id, i4_time_class &time)
  : command_id(command_id), time(time)
{
  int len=strlen(_command);
  if (len>=sizeof(command))
    i4_error("command too long");
  strcpy(command, _command);
}

i4_event  *i4_do_command_event_class::copy() 
{ 
  return new i4_do_command_event_class(command, command_id); 
}



i4_end_command_event_class::i4_end_command_event_class(char *_command, int command_id, i4_time_class &time)
    : command_id(command_id), time(time)
{
  int len=strlen(_command);
  if (len>=sizeof(command))
    i4_error("command too long");
  strcpy(command, _command);
}

i4_end_command_event_class::i4_end_command_event_class(char *_command, int command_id)
  : command_id(command_id)
{
  int len=strlen(_command);
  if (len>=sizeof(command))
    i4_error("command too long");
  strcpy(command, _command);
}
