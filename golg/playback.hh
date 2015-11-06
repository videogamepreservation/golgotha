/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef PLAYBACK_HH
#define PLAYBACK_HH

enum
{
  G1_COMMAND_END=0,
  G1_COMMAND_TURN,
  G1_COMMAND_ACCEL,
  G1_COMMAND_STRAFE,
  G1_COMMAND_LOOK,
  G1_COMMAND_DEPLOY,
  G1_COMMAND_BUILD,
  G1_COMMAND_TARGET,
  G1_COMMAND_FIRE0,
  G1_COMMAND_FIRE1,
  G1_COMMAND_FIRE2,
  G1_COMMAND_CONTINUE,
  G1_NUM_COMMANDS
};

#endif
