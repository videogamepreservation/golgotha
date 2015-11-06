/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "ttree.hh"

void i4_base_ternary_tree::iterator::next()
//{{{
{
  current=0;
  while (!current)
  {
    if (end())
      return;

    // get current stack context
    node *n = node_stack[node_stack.size()-1];
    int i = index_stack[index_stack.size()-1]++;

    switch(i)
    {
      case 0:
        // traverse lo branch
        push(n->lo);
        break;
      case 1:
        // add character to current word
        if (pos<maxlen) buff[pos] = n->ch;
        pos++;

        if (n->ch)
          // traverse remaining word
          push(n->eq);
        else
          // found next valid word
          current = n;
        break;
      case 2:
        // remove character
        pos--;

        // traverse hi branch;
        push(n->hi);
        break;
      case 3:
        // go up stack
        pop();
        break;
    }
  }
}
//}}}

i4_bool i4_base_ternary_tree::unlink(node **o, const CHAR *s, void **old_data)
//{{{
//  recursively traverses the tree to remove a key/value pair
{
  node *n = *o;

  if (!n)
    return 0;

  i4_bool ret;
  i4_bool found=i4_F;

  if (*s < n->ch)
    ret = unlink(&n->lo, s, old_data);
  else if (*s > n->ch)
    ret = unlink(&n->hi, s, old_data);
  else if (*s)
    ret = unlink(&n->eq, s+1, old_data);
  else
  {
    ret = found = i4_T;
    // store old data
    if (old_data) *old_data = n->data();
  }

  if (found || !n->eq)
  {
    // node is either the terminator to be removed, or a node that now has no continuation,
    //   so we should delete it, like deleting out of a binary tree

    if (!n->lo && !n->hi)
      // no children, just axe me
      *o=0;
    else if (!n->lo || !n->hi)
      // one child, just bump child up
      *o = n->lo? n->lo : n->hi;
    else
    {
      // two children, find succesor to replace me

      // find the successor
      node **psplice, *splice;
      psplice = &n->hi;
      while ((splice = *psplice)->lo)
        psplice = &splice->lo;

      // unlink successor from its parent
      *psplice = splice->hi;

      // splice successor in
      splice->lo = n->lo;
      splice->hi = n->hi;
      *o = splice;
    }

    heap.free(n);
  }

  return ret;
}
//}}}

i4_base_ternary_tree::node *i4_base_ternary_tree::find_node(const CHAR *s) const
//{{{
{
  node *n=root;

  while (n)
  {
    if      (*s < n->ch) n = n->lo;
    else if (*s > n->ch) n = n->hi;
    else if (*s)         { n = n->eq; s++; }
    else                 return n;
  }
  return 0;
}
//}}}

i4_bool i4_base_ternary_tree::add(const CHAR *s, void *new_data, i4_bool replace, void **old_data)
//{{{
{
  i4_bool collide = i4_T;
  node **p=&root, *n;

  while (1)
  {
    n = *p;

    if (!n)
    {
      // create new node
      n = heap.alloc();
      n->ch = *s;
      n->lo = n->hi = n->eq = 0;
      collide = i4_F;
      *p = n;
    }

    if      (*s < n->ch) p = &n->lo;
    else if (*s > n->ch) p = &n->hi;
    else if (*s)         { p = &n->eq; s++; }
    else
    {
      if (old_data)
        *old_data = n->data();                    // store old data
      if (!collide || replace) 
        n->set_data(new_data);                    // store new data
      return collide;
    }
  }
}
//}}}

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
