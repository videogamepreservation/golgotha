/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __CHECKSUM_HPP_
#define __CHECKSUM_HPP_

#include "arch.hh"

// these routines calculate the accumulative checksum on a buffer

w32 i4_check_sum32(void *buf, w32 buf_len);
w16 i4_check_sum16(void *buf, w32 buf_len);

#endif
