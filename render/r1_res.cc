/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include <stdio.h>
#include "r1_res.hh"
#include "string/string.hh"
#include "file/file.hh"
#include "app/registry.hh"

int r1_max_texture_size=256;

static char compressed[60], decompressed[60], cache_file[80];
static i4_const_str comp(""), decomp(""), cache("");


static void get(char *var, char *buffer, int max_buffer, char *def)
{
  if (i4_get_registry(I4_REGISTRY_USER,
                      "SOFTWARE\\Crack dot Com\\render\\1.0",
                      var, buffer, max_buffer))
    return ;
  
  char *c=getenv(var);
  if (c)
  {
    strncpy(buffer, c, max_buffer);
    return ;
  }
  
  strcpy(buffer, def);

}

class r1_resource_manager_class : public i4_init_class
{
public:
  i4_string_manager_class r1_strings;



  void init() 
  {
    get("G_DECOMPRESSED", decompressed, 60, "g_decompressed");
    get("G_COMPRESSED", compressed, 60, "g_compressed");

    i4_mkdir(compressed);
    i4_mkdir(decompressed);

    sprintf(cache_file, "%s/tex_cache.dat", decompressed);

    comp=compressed; 
    decomp=decompressed; 
    cache=cache_file;
    
    r1_strings.load("render.res"); 

    i4_const_str::iterator tmax=r1_gets("max_texture_size").begin();
    r1_max_texture_size = tmax.read_number();
    
    char max_size[40];
    get("MAX_TEXTURE_SIZE", max_size, 40, "256");

    sscanf(max_size, "%d", &r1_max_texture_size);
    if (r1_max_texture_size<16) r1_max_texture_size=16;

//    i4_warning("Setting max texture size to %d", r1_max_texture_size);  
  }

} r1_resource_man;


const i4_const_str &r1_gets(char *str, i4_bool barf_on_error)
{
  const i4_const_str *s=&r1_resource_man.r1_strings.get(str);

  if (barf_on_error && s->null())
    i4_error("Render (render.res) resource missing %s", str);

  return *s; 
}


const i4_const_str &r1_get_decompressed_dir()
{
  return decomp;
}


const i4_const_str &r1_get_compressed_dir()
{
  return comp;
}


const i4_const_str &r1_get_cache_file()
{
  return cache;
}

