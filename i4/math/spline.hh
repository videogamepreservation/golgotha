/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_BSPLINE_HH
#define I4_BSPLINE_HH

#include "arch.hh"
#include "math/num_type.hh"

class i4_file_class;
class i4_linear_allocator;

class i4_spline_class
{
  w32 t_points;


  static i4_linear_allocator *p_alloc;
  static w32 point_refs;

  void cleanup();

public:
  class point
  {
  public:
    i4_float x,y,z;
    w32 frame;         // frame in time this point exsist, golgotha runs at 15hz

    i4_bool selected;
    point *next;
  };

protected:
  point *points;
public:
  void move(i4_float x_add, i4_float y_add, i4_float z_add);

  point *begin() { return points; }
  i4_spline_class *next;

  w32 total() { return t_points; }

  point *add_control_point(i4_float x, i4_float y, i4_float z, w32 frame);
  point *get_control_point(w32 p);

  point *get_control_point_previous_to_frame(w32 frame);

  i4_bool get_point(w32 frame, i4_float &x, i4_float &y, i4_float &z);

  // inserts a new control point after ever selected node
  void insert_control_points();

  void delete_selected();
  w32 last_frame();
  i4_spline_class();
  ~i4_spline_class();

  void save(i4_file_class *fp);
  void load(i4_file_class *fp);
};


#endif
