/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


class i4_gui_status_class : public i4_status_class
{
public:
  virtual void update(float percent) = 0;
  virtual ~i4_status_class() { ; }
};

// this is operating system dependant
i4_status_class *i4_create_status(const i4_const_str &description);
                      
