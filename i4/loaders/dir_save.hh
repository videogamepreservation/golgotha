/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef I4_DIR_SAVE_HH
#define I4_DIR_SAVE_HH


#include "file/file.hh"
#include "memory/array.hh"

struct i4_saver_section_type
{
  w32 section_id;
  w32 section_offset;
  i4_saver_section_type(w32  section_id, w32 section_offset)
    : section_offset(section_offset), section_id(section_id) {}
};


class i4_saver_class : public i4_file_class
{
protected:
  enum { DIRECTORY_CREATE, DATA_WRITE, DATA_LOAD } state;

  w32 current_offset, last_version_start, current_skip;

  i4_file_class *out;
  i4_bool close_on_delete;

  i4_array<w16>                    skips;
  i4_array<i4_saver_section_type>  sections;
  i4_array<w32>                    sizes;
  int marker_on;

public: 
  i4_saver_class(i4_file_class *out, i4_bool close_on_delete=i4_T);

  int mark_size();
  void end_mark_size(int marker_number);

  void mark_section(w32 section_id);
  void mark_section(char *section_name);  // calls above with checksum of name

  void start_version(w16 version);
  void end_version();

  i4_bool begin_data_write();          // returns false if error occured
 
  virtual w32 read (void *buffer, w32 size);
  virtual w32 write(const void *buffer, w32 size);

  virtual w32 seek (w32 offset) { return out->seek(offset); }
  virtual w32 size () { return out->size(); }
  virtual w32 tell ();

  virtual ~i4_saver_class();
};


#endif
