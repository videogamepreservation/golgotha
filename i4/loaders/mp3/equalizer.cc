/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


#include "loaders/mp3/mpg123.hh"

real equalizer[2][32];
real equalizer_sum[2][32];
int equalizer_cnt;

void do_equalizer(real *bandPtr,int channel) 
{
	int i;

	if(flags.equalizer & 0x1) {
		for(i=0;i<32;i++)
			bandPtr[i] *= equalizer[channel][i];
	}

	if(flags.equalizer & 0x2) {
		equalizer_cnt++;
		for(i=0;i<32;i++)
			equalizer_sum[channel][i] += bandPtr[i];
	}
}

