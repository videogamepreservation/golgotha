/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_MP3_LOAD_HH
#define I4_MP3_LOAD_HH


class i4_file_class;
class i4_sound_info;
class i4_status_class;


i4_bool i4_load_mp3(i4_file_class *in,  // open file at beginging of mp3 data (after header)
                    i4_file_class *out,    // open file ready to write raw data
                    i4_sound_info &output_format,  // defined in i4/loaders/wav_load.hh
                    i4_status_class *status);   // tells user what's going on
                       


#endif
