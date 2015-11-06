/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_GLOBAL_ID_HH
#define G1_GLOBAL_ID_HH

#include "arch.hh"
#include "g1_limits.hh"

class g1_object_class;

class g1_global_id_reset_notifier
{
public:
  g1_global_id_reset_notifier *next;
  static g1_global_id_reset_notifier *first;
  g1_global_id_reset_notifier();
  virtual void reset() = 0;
  ~g1_global_id_reset_notifier();
};

class g1_global_id_manager_class
{
public:
  enum 
  { 
    ID_INCREMENT=(1<<(G1_MAX_OBJECTS_BITS+1)),
    ID_MASK=ID_INCREMENT-1,
  };
private:

  w32 obj_id[G1_MAX_OBJECTS];
  g1_object_class *obj[G1_MAX_OBJECTS];
  w16 first_free;
public:
  g1_global_id_manager_class();

  void init();
  void claim_freespace();
  void free_objects();

  i4_bool check_id(w32 id) const { return id==obj_id[id&ID_MASK]; }

  i4_bool preassigned(w32 id) const;
  void assign(w32 id, g1_object_class *for_who);
  w32 alloc(g1_object_class *for_who);
  void free(w32 id);
  g1_object_class *get(w32 id) { return obj[id&ID_MASK]; }
  g1_object_class *checked_get(w32 id) { return check_id(id) ? get(id) : 0; }

  w32 invalid_id() { return 0; }

  class remapper
  {
  protected:
    friend g1_global_id_manager_class;
    g1_global_id_manager_class *gid;
    w32 *map;
    // remapping functions
    remapper(g1_global_id_manager_class *gid);
    ~remapper();
  public:
    w32 &operator[](w32 id) { return map[id&ID_MASK]; }
  };
  remapper* alloc_remapping() { return new remapper(this); }
  void free_remapping(remapper* mapper) { delete mapper; }

  void debug(w32 flag_pass=0xffffffff);
};

extern g1_global_id_manager_class g1_global_id;
class g1_loader_class;
class g1_saver_class;

// a convient way to access objects through global id's
class g1_id_ref
{
public:
  w32 id;
  g1_object_class *get() const { return g1_global_id.checked_get(id); }
  i4_bool valid() { return g1_global_id.check_id(id); }
  g1_id_ref() { id=0; }
  g1_id_ref(w32 id) : id(id) {}
  g1_id_ref(g1_object_class *o);
  g1_object_class *operator->() const { return get(); }
  g1_id_ref& operator=(const g1_id_ref &r) { id=r.id;  return *this; }
  g1_id_ref& operator=(w32 _id) { id=_id;  return *this; }
  g1_id_ref& operator=(g1_object_class *o);
  void save(g1_saver_class *fp);
  void load(g1_loader_class *fp);
};

#endif
