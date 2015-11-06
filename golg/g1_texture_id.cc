/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "tile.hh"
#include "checksum/checksum.hh"
#include <string.h>

r1_texture_handle g1_get_texture(char *texture_name)
{
  int l=strlen(texture_name); 
  return g1_tile_man.get(g1_tile_man.get_tile_from_checksum(i4_check_sum32(texture_name, l)))->texture;
}
