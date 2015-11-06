/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

__inline w32 get_clock()
{
	w32 res;
	__asm	{
		push eax
		push edx
		__emit 0x0F
		__emit 0x31
		mov res,eax
		pop edx
		pop eax
	}
	return res;
}
