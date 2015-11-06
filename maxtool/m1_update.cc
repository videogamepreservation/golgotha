/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "r1_api.hh"
#include "lisp/li_init.hh"
#include "status/status.hh"
#include "m1_info.hh"
#include "memory/array.hh"
#include "r1_res.hh"
#include "tupdate.hh"
#include "error/alert.hh"
#include "file/file.hh"

w32 m1_get_file_id(const i4_const_str &fname)
{
  int x;
  char st[30], *s;
  s=st;
  
  i4_const_str::iterator l=fname.begin();
  while (l!=fname.end() && l.get().ascii_value()!='.')
  {
    *(s++)=l.get().ascii_value();
    ++l;
  }

  *s=0;

  if (sscanf(st,"%x",&x))
    return x;
  else return 0;   
}


struct m1_texture
{
  w32 id;
  w32 last_modified;
};

int m1_find_texture(i4_array<m1_texture> &t, int id)
{
  for (int i=0; i<t.size(); i++)
    if (t[i].id==id)
      return i;
  return -1;
}

int m1_texture_compare(const m1_texture *a, const m1_texture *b)
{
  if (a->id < b->id)
    return -1;
  else
  if (a->id > b->id)
    return 1;
  else
    return 0;
}

inline char *remove_paths(char *src)
{
  char *ret = src;

  while (*src)
  {
    if (*src=='/' || *src=='\\')
      ret=src+1;

    src++;    
  }

  return ret;
}

void m1_copy_update(i4_bool all)
{
  int i;

  i4_status_class *stat=i4_create_status(i4gets("updating_textures"));  

  i4_array<i4_str *> tlist(64,64);
  m1_info.get_texture_list(tlist,all);


  int t=tlist.size();  
  i4_directory_struct ds;
  i4_get_directory(i4gets("default_tga_dir"), ds, i4_T);
  
  i4_array <m1_texture> network_tga_list(128,128);  

  for (i=0; i<ds.tfiles; i++)
  {
    if (stat)
      stat->update(i/(float)ds.tfiles);

    m1_texture new_entry;    
    
    new_entry.id            = r1_get_texture_id(*ds.files[i]);
    new_entry.last_modified = ds.file_status[i].last_modified;

    network_tga_list.add(new_entry);
  }

  if (stat)
    delete stat;

  stat = i4_create_status(i4gets("updating_textures"), i4_T);  

  i4_directory_struct ds2;
  i4_get_directory(r1_get_compressed_dir(), ds2, i4_T);

  i4_array <m1_texture> local_gtx_list(128,128);
  
  m1_texture temp_tex;


  for (i=0; i<ds2.tfiles; i++)
  {    
    if (stat && !stat->update(i/(float)ds2.tfiles))
    {
      if (stat)
        delete stat;
      return;
    }

    temp_tex.id            = m1_get_file_id(*ds2.files[i]);
    temp_tex.last_modified = ds2.file_status[i].last_modified;
    local_gtx_list.add(temp_tex);
  }

  if (stat)
    delete stat;
  stat = i4_create_status(i4gets("updating_textures"), i4_T);

  network_tga_list.sort(m1_texture_compare);
  local_gtx_list.sort(m1_texture_compare);    
  
  for (i=0; i<t; i++)
  {    
    if (stat && !stat->update(i/(float)t))
    {
      if (stat)
        delete stat;
      return;
    }

    if (tlist[i]->null()) continue;

    temp_tex.id = r1_get_texture_id(*tlist[i]);    

    w32 network_index = network_tga_list.binary_search(&temp_tex,m1_texture_compare);
    if (network_index != -1)
    {     
      w32 local_index = local_gtx_list.binary_search(&temp_tex,m1_texture_compare);

      if (local_index==-1 || 
          (local_gtx_list[local_index].last_modified<network_tga_list[network_index].last_modified)
         )
      {
        m1_info.texture_list_changed();
        
        m1_update_texture(*tlist[i], i4_T,
                          stat, 
                          i/(float)t, (i+1)/(float)t);
      }
    }
    else
    {
      char s1[256];
      char s2[256];
      char tga_dir[256];

      i4_os_string(*tlist[i],s1,256);
      i4_os_string(i4gets("default_tga_dir"),tga_dir,256);

      char *filename = remove_paths(s1);

      sprintf(s2,"Texture Missing: %s\\%s",tga_dir,filename);

      i4_str *n = i4_from_ascii(s2);
      i4_alert(*n,256);
      delete n;
    }
       
    delete tlist[i];        
  }
      
  if (stat)
    delete stat;
}

li_object *m1_update_all_textures(li_object *o, li_environment *env)
{
  m1_copy_update(i4_T);
  return 0;
}
li_automatic_add_function(m1_update_all_textures, "update_all_textures");

li_object *m1_update_textures(li_object *o, li_environment *env)
{
  m1_info.texture_list_changed();
  return 0;
}
li_automatic_add_function(m1_update_textures, "update_textures");
