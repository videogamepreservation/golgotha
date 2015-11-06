/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

// new_type.cc : adds a new type "vector" into the li_ system
// mainly a new type is responsible for
//   - printing
//   - loading & saving
//   - marking for garbage collection if it contains references to other li_object's
//   - freeing memory when gc says you are no longer referenced

#include "main/main.hh"
#include "init/init.hh"
#include "lisp/lisp.hh"
#include "math/vector.hh"
#include "file/file.hh"
#include "loaders/dir_save.hh"
#include "loaders/dir_load.hh"

// this is the global type number assigned by the li system for li_vector
li_type_number li_vector_type;

class li_vector : public li_object
{
  i4_3d_vector *vector;
public:
  li_vector(const i4_3d_vector &v)
    : li_object(li_vector_type)
  {
    vector=new i4_3d_vector(v.x, v.y, v.z);
  }

  // used by load_object
  li_vector(i4_file_class *fp)
    : li_object(li_vector_type)
  {
    float x=fp->read_float();
    float y=fp->read_float();
    float z=fp->read_float();
    vector=new i4_3d_vector(x,y,z);
  }

  // free memory assocaited with vector during gc()
  void free() { delete vector; }

  // print the object in a english form
  void print(i4_file_class *fp) 
  { 
    fp->printf("<vector %f %f %f>", vector->x, vector->y, vector->z);
  }

  // write to disk
  void save(i4_file_class *fp, li_environment *env)
  {
    fp->write_float(vector->x);
    fp->write_float(vector->y);
    fp->write_float(vector->z);
  }

  
  i4_3d_vector value() { return *vector; }
  static li_vector *get(li_object *o, li_environment *env) 
  {  check_type(o, li_vector_type, env); return ((li_vector *)o); }
};

// if you want to add a new type into the system, implement one of these
// and call li_add_type
class li_vector_function_table : public li_type_function_table
{ 
public:
  // free data associated with an instance of this type
  virtual void free(li_object   *o) { li_vector::get(o,0)->free(); }

  // compare 2 objects
  virtual int equal(li_object  *o1, li_object *o2) 
  { 
    i4_3d_vector v1=li_vector::get(o1,0)->value();
    i4_3d_vector v2=li_vector::get(o2,0)->value();

    return v1.x==v2.x && v1.y==v2.y && v1.z==v2.z;
  }

  // print the object
  virtual void print(li_object  *o, i4_file_class *stream) 
  {
    li_vector::get(o,0)->print(stream);
  }

  virtual char *name() { return "vector"; }

  // create a new object
  virtual li_object *create(li_object *params, li_environment *env)
  {
    if (params)  // with parameters?
    {
      float x=li_get_float(li_eval(li_first(params,env), env),env);
      float y=li_get_float(li_eval(li_second(params,env), env),env);
      float z=li_get_float(li_eval(li_third(params,env), env),env);

      return new li_vector(i4_3d_vector(x,y,z));
    }
    else
      return new li_vector(i4_3d_vector(0,0,0));      // default vector
  }

  // write to disk
  virtual void save_object(i4_saver_class *fp, li_object *o, li_environment *env) 
  { li_vector::get(o,env)->save(fp,env); }

  // load from disk
  virtual li_object *load_object(i4_loader_class *fp, li_type_number *type_remap, li_environment *env) 
  {
    return new li_vector(fp);
  }
};



void i4_main(w32 argc, i4_const_str *argv)
{
  i4_init();

  // add the type into the system
  li_vector_type=li_add_type(new li_vector_function_table, 0,0);


  li_load("new_type.scm");

  i4_uninit();
}

