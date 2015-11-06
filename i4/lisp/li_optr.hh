/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef LI_OPTR_HH
#define LI_OPTR_HH

class li_object;

// anything referenced by a li_object_pointer will be marked as in use furing gc
class li_object_pointer
{
public:
  li_object *o;
  li_object_pointer *next;
  
  li_object_pointer(li_object *o=0);
  ~li_object_pointer();

  li_object& operator*() const { return *o; }
  li_object* operator->() const { return o; }
  li_object *get() { return o; }

  li_object_pointer &operator=(const li_object_pointer &other) { o=other.o;  return *this; }
  li_object_pointer &operator=(li_object *object) { o=object; return *this;}    
};


#endif
