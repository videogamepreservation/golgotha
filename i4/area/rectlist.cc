/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "area/rectlist.hh"
#include "error/error.hh"
#include "memory/malloc.hh"
#include "error/error.hh"
#include "init/init.hh"

#include <stdlib.h>

#ifndef max
#define max(x,y) (((x)>(y)) ? (x) : (y))
#define min(x,y) (((x)>(y)) ? (y) : (x))
#endif

i4_rect_list_class::area::area_allocator *i4_rect_list_class::area::area_alloc=0;


// this class is used to ensure area_alloc is created after the memory manager is installed
class area_allocator_initter : public i4_init_class
{
  public :
  virtual void init()
  {
    i4_rect_list_class::area::area_alloc=
      new i4_linear_allocator(sizeof(i4_rect_list_class::area),0,300,"areas");
  }
  virtual void uninit()
  {
    delete i4_rect_list_class::area::area_alloc;
  }
} ;

static area_allocator_initter instance;

void i4_rect_list_class::swap(i4_rect_list_class *other)
{
  other->list.swap(list);
}


void i4_rect_list_class::remove_area(i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2)
{
  i4_isl_list<area>::iterator a=list.begin(),last=list.end(),del;

  for (;a!=list.end();)
  {
    if (!(x2<a->x1 || y2<a->y1 || x1>a->x2 || y1>a->y2))
    {      
      if (x2>=a->x2 && x1<=a->x1) // does it take a x slice off? (across)
      {
	if (y2>=a->y2 && y1<=a->y1)
	{
          del=a;
          ++a;

	  if (last!=list.end())
            list.erase_after(last);
	  else 
            list.erase();

	  delete_area(&*del);
	} 
	else if (y2>=a->y2)
        {
	  a->y2=y1-1;
          last=a;
          ++a;
        }
	else if (y1<=a->y1)
        {
	  a->y1=y2+1;
          last=a;
          ++a;
        }
	else
	{
	  list.insert(*(new_area(a->x1,a->y1,a->x2,y1-1)));
	  a->y1=y2+1;
          last=a;
          ++a;
	}
      }       
      else if (y2>=a->y2 && y1<=a->y1)  // does it take a y slice off (down)
      {
	if (x2>=a->x2)
        {
	  a->x2=x1-1;
          last=a;
          ++a;
        }
	else if (x1<=a->x1)
        {
	 a->x1=x2+1;
         last=a;
         ++a;
        }
	else 
	{
	  list.insert(*(new_area(a->x1,a->y1,x1-1,a->y2)));
	  a->x1=x2+1;
          last=a;
          ++a;
	}
      }      
      else   // otherwise it just takes a little chunk off
      {
	i4_coord ax1,ay1,ax2,ay2;
        del=a;
        ++a;
        
	if (last!=list.end())
          list.erase_after(last);
	else 
          list.erase();

	if (x2>=del->x2)      { ax1=del->x1; ax2=x1-1; }
	else if (x1<=del->x1) { ax1=x2+1; ax2=del->x2; }
	else                  { ax1=del->x1; ax2=x1-1; } 
	if (y2>=del->y2)      { ay1=y1; ay2=del->y2; }
	else if (y1<=del->y1) { ay1=del->y1; ay2=y2; }
	else                  { ay1=y1; ay2=y2; }
	{
	  list.insert(*(new_area(ax1,ay1,ax2,ay2)));
          if (a==list.end())
            a=list.begin();
          else if (last==list.end())
            last=list.begin();
	}
	
	
	if (x2>=del->x2 || x1<=del->x1)  { ax1=del->x1; ax2=del->x2; }
	else                             { ax1=x2+1; ax2=del->x2; } 

	if (y2>=del->y2)
	{ if (ax1==del->x1) { ay1=del->y1; ay2=y1-1; }
	else { ay1=y1; ay2=del->y2;   } }
	else if (y1<=del->y1) { if (ax1==del->x1) { ay1=y2+1; ay2=del->y2; }
	else  { ay1=del->y1; ay2=y2; } }
	else { if (ax1==del->x1) { ay1=del->y1; ay2=y1-1; }
	else { ay1=y1; ay2=y2; } }
	
	list.insert(*(new_area(ax1,ay1,ax2,ay2)));
        if (a==list.end())
          a=list.begin();
        else if (last==list.end())
          last=list.begin();

	if (x1>del->x1 && x2<del->x2)
	{
	  if (y1>del->y1 && y2<del->y2)
	  {
	    list.insert(*(new_area(del->x1,del->y1,del->x2,y1-1)));
	    list.insert(*(new_area(del->x1,y2+1,del->x2,del->y2)));
	  } else if (y1<=del->y1)
	    list.insert(*(new_area(del->x1,y2+1,del->x2,del->y2)));
	  else 
	    list.insert(*(new_area(del->x1,del->y1,del->x2,y1-1)));
	} else if (y1>del->y1 && y2<del->y2)
	  list.insert(*(new_area(del->x1,y2+1,del->x2,del->y2)));
	
	delete_area(&*del);
      }
    } else
    {
      last=a;
      ++a;
    }
  }
}


void i4_rect_list_class::add_area(i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2, 
                                  i4_bool combine)
{
  if (x1>x2 || y1>y2)
    return ;

  if (list.begin()==list.end())
    list.insert(*(new_area(x1,y1,x2,y2)));
  else
  {
    i4_isl_list<area>::iterator a=list.begin(),last=list.end();

    for (;a!=list.end();)
    {
      // check to see if this new rectangle completly encloses the check rectangle
      if (x1<=a->x1 && y1<=a->y1 && x2>=a->x2 && y2>=a->y2)
      {
        if (last==list.end())
          list.erase();
        else list.erase_after(last);
        i4_isl_list<area>::iterator q=a;
        ++a;
        delete_area(&*q);
      } else if (!(x2<a->x1 || y2<a->y1 || x1>a->x2 || y1>a->y2))  // intersects another area?
      {	  
	if (x1<a->x1) 
	  add_area(x1,(i4_coord) max(y1,a->y1),(sw16)(a->x1-1),(i4_coord) min(y2,a->y2));
	if (x2>a->x2)
	  add_area(a->x2+1,(i4_coord) max(y1,a->y1),x2,(i4_coord) min(y2,a->y2));
	if (y1<a->y1)
	  add_area(x1,y1,x2,a->y1-1);
	if (y2>a->y2)
	  add_area(x1,a->y2+1,x2,y2);

	return ;
      } 
      else if (combine && a->x2+1==x1 && a->y1==y1 && a->y2==y2)  // combines to the right
      {
        if (last==list.end())
          list.erase();
        else list.erase_after(last);

        a->x2=x2;

        add_area(a->x1, a->y1, a->x2, a->y2);
        delete_area(&*a);
        return;
      }
      else if (combine && a->x1-1==x2 && a->y1==y1 && a->y2==y2)  // combines to the left
      {
        if (last==list.end())
          list.erase();
        else list.erase_after(last);

        a->x1=x1;

        add_area(a->x1, a->y1, a->x2, a->y2);
        delete_area(&*a);
        return;
      }
      else if (combine && a->y1-1==y2 && a->x1==x1 && a->x2==x2)  // combines above
      {
        if (last==list.end())
          list.erase();
        else list.erase_after(last);

        a->y1=y1;

        add_area(a->x1, a->y1, a->x2, a->y2);
        delete_area(&*a);
        return;
      }
      else if (combine && a->y2+1==y1 && a->x1==x1 && a->x2==x2)  // combines below
      {
        if (last==list.end())
          list.erase();
        else list.erase_after(last);

        a->y2=y2;

        add_area(a->x1, a->y1, a->x2, a->y2);
        delete_area(&*a);
        return;
      }
      else
      {        
	last=a;
	++a;
      }
    }
    list.insert(*(new_area(x1,y1,x2,y2)));
  }
}


void i4_rect_list_class::intersect_area(i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2) // reduces area list to that which intersects this area
{
  i4_isl_list<area>::iterator a=list.begin(),last=list.end(),next;
  for (;a!=list.end();a=next)
  {
    next=a;
    ++next;

    if (!(x2<a->x1 || y2<a->y1 || x1>a->x2 || y1>a->y2))
    {
      if (x1>a->x1) a->x1=x1;
      if (x2<a->x2) a->x2=x2;
      if (y1>a->y1) a->y1=y1;
      if (y2<a->y2) a->y2=y2;  
      last=a;
    } else 
    {
      if (a==list.begin())        
        list.erase();
      else 
        list.erase_after(last);
      delete_area(&*a);
    }
  }
}


// return i4_T if area is totally clipped away
// this can be used to skip expensive drawing operations
i4_bool i4_rect_list_class::clipped_away(i4_coord x1, i4_coord y1, i4_coord x2, i4_coord y2)
{
  i4_rect_list_class area_left(x1,y1,x2,y2);

  i4_isl_list<area>::iterator a=list.begin();
  for (;a!=list.end();++a)  
    area_left.remove_area(a->x1, a->y1, a->x2, a->y2);
  
  return area_left.empty();
}


i4_rect_list_class::i4_rect_list_class(i4_rect_list_class *copy_from, i4_coord xoff, i4_coord yoff)
{
  i4_isl_list<area>::iterator a=copy_from->list.begin(),last,q;
  if (a!=copy_from->list.end())
  {
    last=new_area(a->x1+xoff,a->y1+yoff,a->x2+xoff,a->y2+yoff);
    list.insert(*last);

    ++a;
    while (a!=copy_from->list.end())
    {
      q=new_area(a->x1+xoff,a->y1+yoff,a->x2+xoff,a->y2+yoff);
      list.insert_after(last,*q);
      last=q;
      ++a;
    }
  }

}

void i4_rect_list_class::intersect_list(i4_rect_list_class *other)                                 // reduces area list to that which intersects this area list
{
  i4_rect_list_class intersection;
  i4_isl_list<area>::iterator a,b;

  for (b=other->list.begin();b!=other->list.end();++b)
  {
    for (a=list.begin();a!=list.end();++a)
    {
      if (!(b->x2<a->x1 || b->y2<a->y1 || b->x1>a->x2 || b->y1>a->y2))
      {
	i4_coord x1,y1,x2,y2;
	if (b->x1>a->x1) x1=b->x1; else x1=a->x1;
	if (b->x2<a->x2) x2=b->x2; else x2=a->x2;
	if (b->y1>a->y1) y1=b->y1; else y1=a->y1;
	if (b->y2<a->y2) y2=b->y2; else y2=a->y2;

	intersection.list.insert(*(new_area(x1,y1,x2,y2)));
      }  
    }
  }
  intersection.list.swap(list);
}


void i4_rect_list_class::inspect(int print)
{
  for (i4_isl_list<area>::iterator a=list.begin(); a!=list.end(); ++a)
  {
    if (print)
      i4_warning("(%d %d %d %d  (%d x %d)", a->x1, a->y1, a->x2, a->y2, 
                 a->x2-a->x1+1, a->y2-a->y1+1);
  }

}

