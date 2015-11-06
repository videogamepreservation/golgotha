/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

//{{{ Ternary Tree
//  
//  Maps strings into user definable data (e.g. symbol tables) using the ternary tree
//  method, a combination of binary tree and trie.  To use, either use the
//  i4_base_ternary_tree class or instatiate a template class from the template
//  i4_ternary_tree.  the template just performs casting, so should produce minimal code,
//  but improves static type-checking.  in either case, the data stored should be 
//  castable to and from a 'void*'.  larger structures should be allocated and then the
//  pointer to the structure can be stored in the tree.
//
//    i4_base_ternary_tree tree;            // tree of string/void* pairs
//    i4_ternary_tree<CMyData *> tree;      // tree of string/MyData* pairs
//
//  To add key/value pairs:
//    
//    CMyData *value;
//    tree.add("token", value);
//    //  add a single key/value pair, return i4_F if no collisions found
//    //  if there was a collision, replace the value for the key, and return i4_T
//  
//    CMyData *value, *old_value;
//    tree.add("token", value, &old_value);
//    //  add a key/value pair, returns i4_F if no collisions found
//    //  if there was a collision, replace the value, store the old value, and return i4_T
//
//  To search for and retrieve key/value pairs:
//
//    i4_bool found = tree.find("token");
//    //  searches for the key, returns whether the key was found
//
//    CMyData *value;
//    i4_bool found = tree.find("token", &value);
//    //  searches for the key, stores value and returns i4_T on success, i4_F otherwise
//
//  To remove key/value pairs
//
//    tree.remove("token");
//    //  removes the key/value from the tree, returns i4_T on success, i4_F if not found
//  
//    CMyData *old_value;
//    tree.remove("token", &old_value);
//    //  removes the key/value from the tree, stores old value and returns i4_T on success
//  
//  To iterate through the key/value pairs,
//
//    char key_buffer[1024];
//    i4_base_ternary_tree::iterator i(tree, key_buffer, sizeof(key_buffer));
//    //  for string/void* iterator
//    i4_ternary_tree<CMyData *>::iterator i(tree, key_buffer, sizeof(key_buffer));
//    //  for typed iterator for CMyData*
//
//    for (; !i.end(); i++)      // iterate through the tree, alphabetically
//    {
//      char *key = i.key();     // get the current string key (returns the original key_buffer)
//                               // and gets destroyed upon the next iteration
//
//      CMyData *value = *i;     // if i is a iterator of the templatized type
//      CMyData *value = *i.get();
//
//      void *value = i.get();   // if i is a iterator of the base ternary tree
//
//      i++;                     // various ways to iterate to the next key/value pair
//      ++i;
//      i.next();
//    }
//
//    
//  $Id: ttree.hh,v 1.3 1998/06/17 18:05:28 oliy Exp $
//}}}

#ifndef TTREE_HH
#define TTREE_HH

#include "memory/lalloc.hh"
#include "memory/array.hh"

typedef char CHAR;

class i4_base_ternary_tree
{
protected:
  class node
  //{{{
  {
  public:
    CHAR ch;
    node *lo, *eq, *hi;
    
    void* data() const { return (void*)eq; }
    void set_data(void *new_data) { eq = (node*)new_data; }
  };
  //}}}
  i4_linear_allocator_template<node> heap;
  node *root;

  i4_bool unlink(node **n, const CHAR *s, void **old_data=0);
  node *find_node(const CHAR *s) const;
public:
  class iterator
  //{{{
  {
  protected:
    i4_array<node *> node_stack;
    i4_array<int>    index_stack;

    node *current;
    CHAR *buff;
    int pos, maxlen;

    void push(node *n)
    //{{{
    {
      if (!n)
        return;

      node_stack.add(n);
      index_stack.add(0);
    }
    //}}}

    void pop()
    //{{{
    {
      node_stack.remove(node_stack.size()-1);
      index_stack.remove(index_stack.size()-1);
    }
    //}}}
  public:
    iterator(const i4_base_ternary_tree &tree, CHAR *buff, int maxlen)
      : node_stack(40, 40), index_stack(40,40), buff(buff), current(0), pos(0), maxlen(maxlen)
    //{{{
    {
      // reserve last character for terminator
      maxlen--;
      buff[maxlen]=0;

      // start from the root and iterate
      push(tree.root);
      next();
    }
    //}}}

    i4_bool end() const { return node_stack.size()==0; }

    const CHAR *key() const { return buff; }
    void *get() const { return current->data(); }

    void next();
    
    iterator& operator++()    { next(); return *this; }
    iterator& operator++(int) { next(); return *this; }
  };
  //}}}
  friend class i4_base_ternary_tree::iterator;

  i4_base_ternary_tree() : root(0), heap(1024,1024,"ternary tree heap") {}

  i4_bool add(const CHAR *s, void *new_data, i4_bool replace=i4_T, void**old_data=0);
  i4_bool remove(const CHAR *s, void **old_data=0) { return unlink(&root, s, old_data); }
  i4_bool find(const CHAR *s, void **data=0) const
  //{{{
  {
    node *n = find_node(s);

    if (n && data)
      *data = n->data();
      
    return n!=0;
  }
  //}}}
};

template<class T>
class i4_ternary_tree : public i4_base_ternary_tree
{
public:
  class iterator : public i4_base_ternary_tree::iterator
  {
  public:
    iterator(const i4_ternary_tree &tree, CHAR *buff, int maxlen) 
      : i4_base_ternary_tree::iterator(tree,buff,maxlen) {}

    T get() const { return (T)i4_base_ternary_tree::iterator::get(); }
    T operator*() const { return get(); }
  };

  i4_bool add(CHAR *s, T new_data, i4_bool replace=i4_T, T* old_data=0)
  { return i4_base_ternary_tree::add(s, (void*)new_data, replace, (void**)old_data); }
  i4_bool remove(CHAR *s, T* old_data=0)
  { return i4_base_ternary_tree::remove(s, (void**)old_data); }
  i4_bool find(CHAR *s, T* old_data=0) const
  { return i4_base_ternary_tree::find(s, (void**)old_data); }
};

#endif

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
