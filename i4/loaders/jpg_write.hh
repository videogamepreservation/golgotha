/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

class i4_file_class;
class i4_image_class;
class i4_status_class;

int i4_write_jpeg(i4_image_class *im,
                    i4_file_class *fp,
                    int quality,           // 0..100
                    i4_status_class *status=0);
