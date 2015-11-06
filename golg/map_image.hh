/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

// creates an image of the projected map closest to the dimentions specified
// the aspect ratio of the map is 1x1

class i4_image_class;
class i4_rect_list_class;
class g1_path_object_class;

void g1_calc_map_area(int max_width, int max_height, int &x1, int &y1, int &x2, int &y2);
i4_image_class *g1_create_map_image(int max_width, int max_height, i4_bool interlace=i4_F);
void g1_draw_takeover_spots(i4_image_class *im);
void g1_draw_paths(i4_image_class *im, i4_rect_list_class *list);
void g1_draw_strategy_border(i4_image_class *im);
