/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef REF_MAN_HH
#define REF_MAN_HH

#include "arch.hh"

class g1_object_class;
class g1_saver_class;
class g1_loader_class;


// this structure is used to hold a linked list of objects needing notification
// that an object is being deleted.  This is so if a missle is tracking a tank
// and the tank blows up before the missle gets there, the missle can be notified
// the object is being deleted and it can decide to track something else, detonate,
// or go in a strait line.

// references save notifiee and ref as indexes into an object array
// because these objects may not exsist on load, they are kept as indexes until
// the whole level is loaded, so you cannot use a reference directly after load()
// but by the time your think function is called your reference will be valid

class g1_reference_class
{
protected:
  friend class g1_remove_manager_class;
  friend class g1_saver_class;
  friend class g1_loader_class;
  g1_object_class *ref;

  void remove_ref();

  g1_reference_class() : ref(0), next(0) {}
  g1_reference_class(g1_object_class *object_being_referenced)
  { reference_object(object_being_referenced); }

  void reference_object(g1_object_class *object_being_referenced);
public:
  g1_reference_class *next;
  
  void save(g1_saver_class *fp);
  void load(g1_loader_class *fp);

  i4_bool valid() const { return ref!=0; }

  i4_bool operator==(const g1_reference_class &a) const { return ref == a.ref; }
  i4_bool operator!=(const g1_reference_class &a) const { return ref != a.ref; }

  ~g1_reference_class();
};

template <class T>
class g1_typed_reference_class : public g1_reference_class
{
public:
  T *get() const { return (T *)ref; }
  T& operator*() const { return *((T *)ref); }
  T* operator->() const { return (T *)ref; }

  g1_typed_reference_class<T>() : g1_reference_class() {}
  g1_typed_reference_class<T>(const g1_typed_reference_class<T> &r) : g1_reference_class(r.ref) {}

  g1_typed_reference_class<T>& operator=(const g1_typed_reference_class<T> &r) 
  { 
    g1_reference_class::reference_object(r.ref); 
    return *this; 
  }

  g1_typed_reference_class<T>& operator=(const T *ref) 
  { 
    g1_reference_class::reference_object((g1_object_class *)ref); 
    return *this; 
  }
};

#endif



