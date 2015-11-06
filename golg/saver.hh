/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_SAVER_HH
#define G1_SAVER_HH

#include "file/file.hh"
#include "memory/growarry.hh"
#include "path.hh"
#include "reference.hh"
#include "loaders/dir_load.hh"
#include "loaders/dir_save.hh"
#include "lisp/li_types.hh"
#include "global_id.hh"

class g1_path_saver_class;
class g1_object_class;


struct g1_saver_section_type
{
  w16 section_id;
  w32 section_offset;
  g1_saver_section_type(w16 section_id, w32 section_offset)
    : section_offset(section_offset), section_id(section_id) {}
};


class g1_saver_class : public i4_saver_class
{
  g1_object_class **ref_list;
  w32 t_refs;
  g1_global_id_manager_class::remapper *remap;

public: 
  g1_saver_class(i4_file_class *out, i4_bool close_on_delete=i4_T);
  ~g1_saver_class();

  void set_helpers(g1_object_class **reference_list, w32 total_references);
  virtual i4_bool write_reference(const g1_reference_class &ref);
  i4_bool write_global_id(w32 id);
};

class g1_loader_class : public i4_loader_class
{
  g1_object_class **ref_list;
  w32 t_refs;
  g1_reference_class *first_ref;
  w32 *id_remap;

public: 
  li_type_number *li_remap;    // used to load lisp objects

  i4_bool references_were_loaded() { return first_ref!=0; }

  g1_loader_class(i4_file_class *in, i4_bool close_on_delete=i4_T);

  void set_remap(w32 total_refs);
  void end_remap();

  void set_helpers(g1_object_class **reference_list, w32 total_references);

  void read_reference(g1_reference_class &ref);
  w32 read_global_id();

  // called by level loader after all objects have been loaded
  void convert_references();

  ~g1_loader_class();
};

// returns NULL if file is corrupted
g1_loader_class *g1_open_save_file(i4_file_class *in,
                                   i4_bool close_on_delete_or_fail=i4_T);

#endif



