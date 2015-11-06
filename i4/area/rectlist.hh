/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef __RECTLIST_HPP_
#define __RECTLIST_HPP_

#include "arch.hh"
#include "memory/lalloc.hh"
#include "isllist.hh"

/*

  A rect list manages a list of disjoint rectangles.  This is used for clipping, dirty_rectangles,
  and a few other misc. things.  The operations should be straitforward.

  The rectlist is actually just a point to a list of area nodes.  Rectlist should be passed as
  a reference.

  ussage example :

  // create a list containing the rectangle [0,0-320,200]
  i4_rectlist l(0,0,320,200);      

  // list now contains the area above minus the area in the rectangle [5,5,320-5,200-5]
  l.remove_area(5,5,320-5,200-5);  

  // list now conains the areas [100,0,105,5] & [100,200-5,105,200]
  l. intersect_area(100,0,105,200); 

 */


class i4_rect_list_class
{
public:
  class area
  {
    public :
    typedef i4_linear_allocator area_allocator;

    static area_allocator *area_alloc;

    i4_coord x1,y1,x2,y2;
    area *next;

#ifndef __MAC__
    area(i4_coord X1, i4_coord Y1,i4_coord X2,i4_coord Y2) 
    { 
      x1=X1; y1=Y1; 
      x2=X2; y2=Y2; 
    }
    
#ifndef i4_NEW_CHECK    
    void *operator new(size_t size)
    { 
      return area_alloc->alloc(); 
    }
    
    void operator delete(void *ptr)
    { 
      area_alloc->free((area *)ptr); 
    }
    #endif
#endif
  };  
  
#ifdef __MAC__
  area *new_area(i4_coord X1, i4_coord Y1, i4_coord X2, i4_coord Y2) 
    { 
    area *r = (area *)area::area_alloc->alloc(); 

    r->x1=X1; r->y1=Y1; 
    r->x2=X2; r->y2=Y2; 

    return r;
  }
  
  void delete_area(area *ptr)
    { 
    area::area_alloc->free(ptr); 
  }
  #else
  area *new_area(i4_coord X1, i4_coord Y1, i4_coord X2, i4_coord Y2) 
    {
    return new area(X1,Y1,X2,Y2);
  }
    void delete_area(area *ptr) { delete ptr; }
#endif
private:


public:
  typedef i4_isl_list<area>::iterator area_iter;
  i4_isl_list<area> list;

  i4_rect_list_class() {}
  i4_rect_list_class(i4_rect_list_class *copy_from, i4_coord xoff, i4_coord yoff);

  // initial size of rect area
  i4_rect_list_class(i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2)       
  { list.insert(*(new_area(x1,y1,x2,y2))); }

  // exchanges the contents of two rect_list : used to replace the display's clip list
  void swap(i4_rect_list_class *other);                    

  i4_bool empty() { return list.empty(); }
  void delete_list()
    {
    area *a;

    while (!empty())
    {
      a = &*list.begin();
      list.erase();
      delete_area(a);
    }
  }
  
  // subtracts this area from rect rects
  void remove_area(i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2);    

  // combines this area with rects in list
  void add_area(i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2,
                                                  i4_bool combine=i4_F);

  // reduces area list to that which intersects this area
  void intersect_area(i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2); 

  // reduces area list to that which intersects this area list
  void intersect_list(i4_rect_list_class *other);                                 

  // return i4_T if area is totally clipped away
  // this can be used to skip expensive drawing operations
  i4_bool clipped_away(i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2); 
                                                                            
  // debuggin purposes only
  void inspect(int print=0);                                                

  
  ~i4_rect_list_class() { delete_list(); }  
} ;


#endif
//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
