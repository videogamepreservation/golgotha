/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

//{{{ Numeric Interval template
//
//  Allows the manipulation of numeric closed intervals, or the union of
//  all numbers that fall between pairs of numbers, inclusively.
//
//Usage:
//  Instantiate the template on a numeric type
//  Add intervals using 'add_interval', which also returns whether the specified
//    interval was not contained
//  Test for containment using 'contains'
//  Reset the interval with 'clear'
//  
//  To allow for printing of the interval list, define 'DEBUG_INTERVAL' and
//    use 'print' in the debugger.
//
//$Id: interval.hh,v 1.4 1996/09/03 05:12:19 oliy Exp $
//}}}

#include "arch.hh"
#include "math/num_type.hh"

template <class T>
class i4_interval_template
{
protected:
  class interval_node;
  typedef interval_node *interval_link;
  class interval_node
  //{{{
  {
    friend class i4_interval_template<T>;
  protected:
    interval_link left, right;
  public:
    T min, max;

    interval_node() : left(0), right(0) {}
    interval_node(T _min, T _max) 
      : min(_min), max(_max), left(0), right(0) {}

    ~interval_node() { delete left; delete right; }
  };
  //}}}
  interval_link root;

  interval_link *find_interval(T val, interval_link *pp)
  //{{{
  {
    interval_link p;

    while ((p = *pp)!=0)
    {
      if (val<p->min)
        pp = &p->left;
      else if (val>p->max)
        pp = &p->right;
      else
        return pp;
    }
    return pp;
  }
  //}}}

  i4_bool recurse_combine(interval_link *pp,
                          T min, 
                          T max, 
                          interval_link replace)
  //{{{
  {
    interval_link p = *pp;

    if (!p)
    {
      if (!replace)
      {
        *pp = new interval_node(min,max);
        return 1;
      }
      else
        return 0;
    }

    if (max<p->min)
      // interval totally to the left of me
      recurse_combine(&p->left,min,max,replace);
    else if (min>p->max)
      // interval totally to the right of me
      recurse_combine(&p->right,min,max,replace);
    else
    {
      // interval touches me
      T local_min, local_max;

      if (min<=p->min)
        // can possibly combine more to the left
        recurse_combine(&p->left,min,max,p);
      if (max>=p->max)
        // can possibly combine more to the right
        recurse_combine(&p->right,min,max,p);

      local_min = (min < p->min)? min : p->min;
      local_max = (max > p->max)? max : p->max;

      if (replace)
      {
        // combine me into my ancestor
        replace->min = (local_min < replace->min)? local_min : replace->min;
        replace->max = (local_max > replace->max)? local_max : replace->max;

        delete p;
        *pp = 0;
      }
      else
      {
        // combine interval with me
        p->min = local_min;
        p->max = local_max;
      }
    }
    return 0;
  }
  //}}}

  //{{{ Debugging Tools

#ifdef DEBUG_INTERVAL
  void print_tree(interval_link tree)
  //{{{
  {
    if (!tree)
      return;

    print_tree(tree->left);
    printf("[%f %f]\n",tree->min,tree->max);
    print_tree(tree->right);
  }
  //}}}
#endif
  
#ifdef DEBUG_INTERVAL
public:
  void print()
  //{{{
  {
    print_tree(root);
  }
  //}}}
protected:
#endif
  
  //}}}
  
public:
  i4_interval_template() : root(0) {}
  
  i4_bool contains(T val)
  //{{{
  {
    return (*find_interval(val,&root) != 0);
  }
  //}}}

  i4_bool add_interval(T min, T max)
  //{{{
  {
    return recurse_combine(&root,min,max,0);
  }
  //}}}

  void clear()
  //{{{
  {
    if (root)
      delete root;
    root = 0;
  }
  //}}}
};

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
