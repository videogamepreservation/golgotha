/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_EDITOR_EDIT_ID_HH
#define G1_EDITOR_EDIT_ID_HH


class g1_create_object_message_class : public i4_user_message_event_class
{ 
public:
  w16 object_type;

  g1_create_object_message_class(w16 object_type) 
    : i4_user_message_event_class(G1_EDITOR_CREATE_OBJECT),
      object_type(object_type)
  {}
  virtual i4_event  *copy() { return new g1_create_object_message_class(object_type); }    
};


#endif
