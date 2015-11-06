/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef PROCESSOR_HH
#define PROCESSOR_HH

#include "arch.hh"

struct i4_cpu_info_struct
{
  enum
  {
    NON_X86     = 0,
    UNKNOWN_X86 = 1,
    INTEL_X86   = 2,
    AMD_X86     = 3
  };

  enum
  {
    RDTSC = 1,
    MMX   = 2,
    AMD3D = 4
  };

  w32 cpu_type;
  w32 cpu_flags;
};

void i4_get_cpu_info(i4_cpu_info_struct *s);

#endif
