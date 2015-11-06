/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef MK_TABLE_HH
#define MK_TABLE_HH

#include "error.hh"

template <class T>
class table
{
protected:
  T *entry;
  int used,entries,grow;
public:

  int size() const { return used; }
  T& operator[](int i) const 
  {
    assert(i>=0 && i<used, "table::bad array reference");
    return entry[i]; 
  }    

  table(int entries, int grow = 0) : entries(entries), grow(grow), entry(0), used(0)
  {
    if (entries>0)
      entry = (T*)malloc(sizeof(T)*entries);
    else
      entry = (T*)malloc(sizeof(T)*grow);

    assert(entry, "table::can't allocate entries");
  }
  
  ~table() 
  {
    clear(); 
    free(entry);
    entries = 0;
  }
  
  int add(T item,int ref = -1)
  {
    assert(item, "table::bad item add");

    if (ref<0)
      ref += used+1;

    assert(ref>=0 && ref<=used,"table::bad item referenced");

    if (used>=entries)
    {
      if (grow)
      {
        entries += grow;

        T* new_entry = (T*)realloc(entry, sizeof(T *)*entries);
        
        assert(new_entry, "table::out of memory");

        entry = new_entry;
      }
      else
        assert(0, "table::out of entries");
    }

    for (int i=used; i>ref; i--)
      entry[i] = entry[i-1];
    entry[ref] = item;
    used++;

    return ref;
  }
  
  int add_table(const table& tab,int ref = -1)
  {
    if (ref<0)
      ref += used+1;

    assert(ref>=0 && ref<=used,"table::bad item referenced");

    if (used+tab.size() >= entries)
    {
      if (grow)
      {
        if (used+tab.size() >= entries+grow)
          entries = used+tab.size();
        else
          entries += grow;

        T* new_entry = (T*)realloc(entry, sizeof(T *)*entries);
        
        assert(new_entry, "table::out of memory");

        entry = new_entry;
      }
      else
        assert(0, "table::out of entries");
    }

    int i;

    for (i=used-1; i>ref; i--)
      entry[i+tab.size()] = entry[i];
    for (i=0; i<tab.size(); i++)
      entry[ref+i] = tab.entry[i];

    used+=tab.size();

    return ref;
  }
  
  void remove(int ref)
  {
    assert(ref>=0 && ref<used, "table::bad item deletion");

    used--;
    for (int i=ref; i<used; i++)
      entry[i] = entry[i+1];
  }
  
  void clear()
  {
    used = 0;
  }
};

class name_table : public table<char *>
{
public:
  name_table(int entries = 20, int grow = 100) : table<char*>(entries,grow) {}

  name_table(name_table &other) : table<char*>(20,200) 
  {
    add_table(other);
  }

  name_table& operator=(char *item) { clear(); add(item);  return *this; }
  name_table& operator=(const name_table &tab) { clear(); add_table(tab);  return *this; }
  name_table& operator+(char *item) { add(item);  return *this; }
  name_table& operator+(const name_table &tab) { add_table(tab);  return *this; }
  name_table& operator+=(char *item) { add(item);  return *this; }
  name_table& operator+=(const name_table &tab) { add_table(tab);  return *this; }
  name_table& clear() { table<char*>::clear(); return *this; }
  int find(char *item)
  {
    for (int i=0; i<size(); i++) 
      if (strcmp(item, entry[i])==0)
        return i;
    return -1;
  }

};

typedef name_table list;

#endif

