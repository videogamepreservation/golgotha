/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

// String Classes
//
//   Strings are hidden behind the two classes :
//
//       i4_const_str   (constant strings.. cannot be changed)
//   and i4_str         (modifiable strings)
//
//
//   i4_strs can only be created from i4_const_strs or other i4_strs and i4_const_strs can
//   only be created through the i4_string_manager.  All strings gotten from i4_string_manager
//   are loaded from an external file.  This ensures that all strings are stored external and
//   UNICODE or wide-chars can be added in without any code changes outside this module.
//   Because for most applications only one string manager is needed I have created a global
//   one called i4_string_man.  
//
//   To see if a string is available from the string manager the call
//
//   const i4_const_str *s=&i4_string_man.get(str);
//
//   is made.  If s->null()==i4_T then the string was not loaded.
//   Since this is a common operation the short-named function i4gets is provided 
//   (see end of this file).
//
//
//   example :
//
//   resource.res
//   ---------------
//   hello "Hello World"
//
//   ---------------
//
//
//   i4gets("hello") ->  i4_const_str("Hello World")
//
//
//   So, you might ask, how do I create a string quick and dirty for debugging purposes?
//
//   You have to do it the hard way for now.......

#ifndef __STRING_HPP_
#define __STRING_HPP_

#include "arch.hh"
#include "init/init.hh"
#include "error/error.hh"
#include "memory/malloc.hh"

#include <stdlib.h>
#include <string.h>

class i4_char
{
protected:
  w8 ch;
public:
  i4_char(w8 ch) : ch(ch) {}
  w16 value() const { return ch; }
  i4_bool is_space() const { return (i4_bool)(ch==' ' || ch=='\r' || ch=='\n' || ch=='\t'); }
  i4_bool is_slash() const { return (i4_bool)(ch=='/'); }
  i4_bool is_backslash() const { return (i4_bool)(ch=='\\'); }
  i4_bool is_period() const { return (i4_bool)(ch=='.'); }

  i4_char to_lower() const 
  { return (ch>='A' && ch<='Z') ? i4_char(ch-'A'+'a') : i4_char(ch); }

  i4_char to_upper() const 
  { return (ch>='a' && ch<='z') ? i4_char(ch-'a'+'A') : i4_char(ch); }

  i4_bool operator==(const i4_char &b) const { return (ch == b.ch); }
  i4_bool operator!=(const i4_char &b) const { return (ch != b.ch); }
  i4_bool operator>(const i4_char &b) const { return  (ch > b.ch); }
  i4_bool operator<(const i4_char &b) const { return  (ch < b.ch); }
  i4_bool operator>=(const i4_char &b) const { return (ch >= b.ch); }
  i4_bool operator<=(const i4_char &b) const { return (ch <= b.ch); }

  // this should only be called if you know the string you are looking at has ascii chars in it
  w8 ascii_value() const { return ch; }
};


class i4_str;
class i4_string_manager_class;
class i4_file_class;


class i4_const_str
{
protected:
  typedef char char_type;

  char_type *ptr;
  w16   len;

  i4_const_str() : ptr(0), len(0) {}

  friend class i4_string_manager_class;

public:
  i4_const_str(const char *ptr) : ptr((char *)ptr) 
  { 
    if (ptr) 
      len=strlen(ptr); 
    else 
      len=0;
  }

  i4_const_str(char *ptr, w16 len) : ptr(ptr), len(len) {}

  i4_const_str(const i4_const_str &str) : ptr(str.ptr), len(str.len) {}


  class iterator
  {
    friend class i4_const_str;
    friend class i4_str;
  protected:
    char_type *node;

  public:
    iterator() : node(0) {}
    iterator(const iterator &p) : node(p.node) {}
    iterator(char_type *p) : node(p) {}

    int operator==(const iterator &p) const { return (node == p.node); }
    int operator!=(const iterator &p) const { return (node != p.node); }
    
    iterator& operator++() {     node++;  return *this; }
    iterator& operator++(int) { node++; return *this; }
    iterator& operator--()  {     node--;  return *this; }
    iterator operator+(const sw32 other)    {  return iterator(node+other); }
    iterator operator+=(const sw32 len) {   node+=len;   return *this;}

    
    i4_char get() const { return i4_char(*node); }

    i4_char operator* () const { return get(); }
    i4_str *read_string();
    w32     read_ascii(char *buffer, w32 buffer_size);  // returns bytes read
    sw32    read_number();
    double  read_float();
  };
  

  const iterator end()   const { return ptr+len; }
  const iterator begin() const { return ptr; }

  // number of characters in string
  w32 length() const { return len; }

  // ascii length includes null-terminator and be eventaully be longer than length()+1
  // when kanji support is added and escape characters are used
  w32 ascii_length() const { return len+1; } 

  sw32 ptr_diff(const iterator first, const iterator last) const
  { return last.node-first.node; }

  i4_bool operator== (const i4_const_str &other) const 
  { 
    if (len!=other.len)
      return 0;
    else return ::strncmp(other.ptr,ptr,len)==0; 
  }

  i4_bool operator!= (const i4_const_str &other) const { return !(other==*this); }
  int strncmp(const i4_const_str &other, w32 max_cmp) const
  {
    return ::strncmp(ptr,other.ptr,max_cmp); 
  }
  
  i4_bool null() const { return (i4_bool)(ptr==0); }

  // max_length is the maximum expanded length sprintf will create
  // sprintf uses it's own internal string as the format, and returns the
  // result of the sprintf operation as a new i4_str
  // example : i4_str *new_str=i4gets("format_str").sprintf(100,some_number);
  i4_str *sprintf(w32 max_length, ...) const;
  i4_str *vsprintf(w32 max_length, va_list &ap) const;

  iterator strstr(const i4_const_str &needle_to_find) const;
};


class i4_str : public i4_const_str
{
  friend i4_str *i4_from_ascii(const char *buf);
protected:
  w16 buf_len;

  i4_str(w16 _buf_len) { alloc(_buf_len); }

  void alloc(w16 _buf_len);  
  void init_from_string(const i4_const_str &str, w16 _buf_len);

public:
  class iterator : public i4_const_str::iterator
  {
  protected:
  public:
    iterator(const iterator &p) : i4_const_str::iterator(p) {}
    iterator(char_type *p) : i4_const_str::iterator((char_type *)p) {}
    void set(i4_char ch) { *((char *)node)=(char_type)ch.value(); }

  };
  
  i4_str(const i4_str &str) : i4_const_str(0) { init_from_string(str, (w16)str.length()); }
  i4_str(const i4_const_str &str) : i4_const_str(0) { init_from_string(str, (w16)str.length()); }
  i4_str(const i4_const_str &str, w16 _len) : i4_const_str(0) { init_from_string(str, _len); }

  // copies from start to end-1 characters (does not include end)
  i4_str(const i4_const_str::iterator start, const i4_const_str::iterator end, w16 buf_len);
  
  void insert(i4_str::iterator p, const i4_const_str &other);   // insert other before p
  void insert(i4_str::iterator p, const i4_char ch);            // insert ch before p

  void remove(i4_str::iterator start, i4_str::iterator last);

  void to_upper();   // converts all the chars in this string to upper case
  void to_lower();   // converts all the chars in this string to lower case

  const iterator end()   const { return ptr+len; }
  const iterator begin() const { return ptr; }

  void set_length(int l) { len=l; }

  ~i4_str();
};


class i4_linear_allocator;
class i4_grow_heap_class;
class i4_string_manager_class : public i4_init_class
{  

private:
  i4_grow_heap_class *string_heap;

  char  *alloc_str(char *string);
  void add_node(char *token, char *string);
  void add_array_node(char *token, char **array, w32 total);

  friend class i4_string_manager_saver_class;

  class node
  {
  public:
    typedef i4_linear_allocator node_allocator;
    static node_allocator *nodes;
    static w32 nodes_ref;           // number of string managers using 'nodes'

    char *str_token;
    i4_const_str value;
    node *left,*right;

#ifndef i4_NEW_CHECK
    void *operator new(size_t size);
    void operator delete(void *ptr);
#endif

    node(char *token, const i4_const_str &value) : str_token(token), value(value)
    {
      left=0;
      right=0;
    }
    
    ~node();
  };
  node *root;

#ifdef __MAC__
  node *new_node(char *token, const i4_const_str &value) 
  { 
    node *p = (node *)node::nodes->alloc();
    p->str_token = token;
    p->value = value;
    p->left = 0;
    p->right = 0;

    return p;
  }
  
  void delete_node(node *n)
  {
    if (n->left)
      delete_node(n->left);

    if (n->right)
      delete_node(n->right);

    node::nodes->free(n);
  }
#else
  node *new_node(char *token, const i4_const_str &value) { return new node(token,value); }
  void delete_node(node *n) { delete n; }
#endif

  class array_node
  {
  public:
    typedef i4_linear_allocator node_allocator;
    static node_allocator *nodes;

    char *str_token;
    char **value;
    array_node *left,*right;



#ifndef i4_NEW_CHECK
    void *operator new(size_t size);
    void operator delete(void *ptr);
#endif

    array_node(char *token, char **value) : str_token(token),value(value)
    {
      left=0;
      right=0;
    }

    ~array_node();
  } *array_root;
  
#ifdef __MAC__
  array_node *new_array_node(char *token, char **value) 
  { 
    array_node *p = (array_node *)array_node::nodes->alloc();
    p->str_token = token;
    p->value = value;
    p->left = 0;
    p->right = 0;

    return p;
  }
  
  void delete_array_node(array_node *n)
  {
    if (n->left)
      delete_array_node(n->left);

    if (n->right)
      delete_array_node(n->right);

    array_node::nodes->free(n);
  }
#else
  array_node *new_array_node(char *token, char **value) 
  { 
    return new array_node(token,value); 
  }
  void delete_array_node(array_node *n) { delete n; }
#endif

  void get_token(char *&s, char *&buf, w32 &line_on, char *error_prefix);
  void expand_macro(char *&s, char *&buf, w32 &line_on, char *error_prefix);
  void read_array(char *&s, 
                  char **array, 
                  w32 &total,
                  w32 &line_on, 
                  char *error_prefix, 
                  char *token_buf);

  void get_char(char *&s, char *&buf, w32 &line_on, char *error_prefix);

  void show_node(node *who);
  void show_nodes();

public:
  i4_string_manager_class();
  ~i4_string_manager_class();
  
  int init_type() { return I4_INIT_TYPE_STRING_MANAGER; }
  void init();
  void uninit();

  i4_bool       load(char *filename);
  i4_bool       load(const i4_const_str &filename);
  i4_bool       load_buffer(void *internal_buffer, char *error_prefix);

  const i4_const_str &get(const char *internal_name);
  const i4_const_str &get(const i4_const_str &internal_name);

  i4_const_str *get_array(const char *internal_name);
  i4_const_str *get_array(const i4_const_str &internal_name);
};

// "the" main string manager for a program
extern i4_string_manager_class i4_string_man;

const i4_const_str &i4gets(char *str, i4_bool barf_on_error=i4_T);
int i4getn(char *str, i4_bool barf_on_error=i4_T);

// converts an i4_const_str to an 8 bit ascii string (where possible)
extern char *i4_os_string(const i4_const_str &name, char *buffer, int buflen);

// converts from an ascii string to an i4_str
extern i4_str *i4_from_ascii(const char *buf);

#endif

