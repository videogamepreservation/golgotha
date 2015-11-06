/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "network/net_prot.hh"

i4_net_protocol *i4_net_protocol::first=0;

i4_net_protocol::i4_net_protocol()
{
  next=first;
  first=this;
}


i4_net_protocol *i4_get_typed_protocol(i4_protocol_type type)
{
  for (i4_net_protocol *p=i4_get_first_protocol(); p; p=p->next)
    if (p->type()==type)
      return p;
         

  return 0;
}
