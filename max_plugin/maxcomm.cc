/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "maxcomm.hh"


int m1_mail_slot_class::create(const i4_const_str &name)
//{{{
{
  char buf[256];
  slot = CreateMailslot(i4_os_string(name,buf,sizeof(buf)),    // mail slot name
                        0,                     // maximum message size (0 == NO_LIMIT) 
                        MAILSLOT_WAIT_FOREVER, // read timeout status 
                        0);                    // security attributes 

  if (slot == INVALID_HANDLE_VALUE)
    return 0;

  return 1;
}
//}}}


int m1_mail_slot_class::open(const i4_const_str &name)
//{{{
{
  char buf[256];
  slot = CreateFile(i4_os_string(name,buf,sizeof(buf)),    // mail slot name
                    GENERIC_WRITE,         // needs GENERIC_WRITE to send to slot
                    FILE_SHARE_READ,       // required for mail slots
                    0,                     // secutiry attributes
                    OPEN_EXISTING,         // find named slot
                    FILE_ATTRIBUTE_NORMAL,
                    0);                    // template file to grab attributes

  if (slot == INVALID_HANDLE_VALUE)
    return 0;

  return 1;
}
//}}}


int m1_mail_slot_class::read_ready()
//{{{
{
  w32 maxsize = 0;
  w32 size, number;
  int result;

  result = GetMailslotInfo(slot,     // mail slot handle
                           &maxsize, // 
                           &size, 
                           &number, 
                           0);

  if (!result)
    return 0;

  return (number>0);
}
//}}}


w32 m1_mail_slot_class::write(char *buff, w32 size)
//{{{
{
  w32 r;
  int result;

  result = WriteFile(slot, 
                     buff, 
                     size, 
                     &r, 
                     0);
  
  if (!result)
  {
    slot = INVALID_HANDLE_VALUE;
    return 0;
  }

  return r;
}
//}}}


w32 m1_mail_slot_class::read(char *buff, w32 maxsize)
//{{{
{
  w32 size, number, r;
  int result;

  result = GetMailslotInfo(slot, &maxsize, &size, &number, 0);
  
  if (!result)
    return 0;

  result = ReadFile(slot, buff, size, &r, 0);
  
  if (!result)
    return 0;

  return r;
}
//}}}


//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
