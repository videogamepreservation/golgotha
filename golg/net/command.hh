/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_NET_COMMAND_HH
#define G1_NET_COMMAND_HH




// packet type  [ should be the first byte in each pack ]
enum
{
  G1_PK_I_WANNA_JOIN=128,
  G1_PK_YOU_HAVE_JOINED,
  G1_PK_GAME_DATA
};



// sent to server

//  w16 [array of key down times in milliseconds]
//  troop build/movement commands, add on later..



// received from server
// w8 total_objects
// 




#endif
