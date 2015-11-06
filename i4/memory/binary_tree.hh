/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_BINARY_TREE_HH
#define I4_BINARY_TREE_HH

/*
pretty simple, uses linear_allocator for quick allocation
two methods, ::insert & ::find so far, you need to define the > & <
  operators for the type if they don't exsist.

example ussage:
-------------------

i4_binary_tree<int> tree;
tree.insert(6);
tree.insert(10);
if (tree.find(12))  x=5;
 
or for user defined structs :

struct complex
{
  float a;
  float b;

  i4_bool operator>(const command_matchup c) const { return (i4_bool)(a>c.b); }
  i4_bool operator<(const command_matchup b) const { return (i4_bool)(a<c.b); }
};

i4_binary_tree<complex> tree2;
tree2.insert(complex(3.1, 4.5));
tree2.find(complex(3.1,0));
*/

#include "memory/lalloc.hh"

template <class T>
class i4_binary_tree
{
  void recursive_delete(T * node)
  {
    if (node->right)
      recursive_delete(node->right);
    if (node->left)
      recursive_delete(node->left);
    delete node;
  }

public:
  T *root;

  i4_binary_tree()   { root=0; }


  // insert return a previous instance if already in tree
  T *insert(T *x)
  {   
    x->left=x->right=0;

    if (!root)
      root=x;
    else
    {
      T *p=root;
              
      while (1)
      {
        int cmp=x->compare(p);
        if (cmp<0)
        {
          if (!p->left)
          {
            p->left=x;
            return x;
          }
          else p=p->left;
        }
        else if (cmp>0)
        {
          if (!p->right)
          {
            p->right=x;
            return x;
          }
          else p=p->right;
        }
        else return p;
      }
    }

    return x;
  }

  T *find(const T *x)
  {
    T *p=root;
    while (1)
    {
      if (!p) return 0;

      int cmp=x->compare(p);
      if (cmp<0)
        p=p->left;
      else if (cmp>0)
        p=p->right;
      else return p;
    }
  }

  void delete_tree()
  {
    if (root)
      recursive_delete(root);
  }

};

#endif
