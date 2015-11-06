/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "tupdate.hh"
#include "loaders/dir_load.hh"
#include "saver_id.hh"
#include "loaders/load.hh"
#include "error/alert.hh"
#include "memory/array.hh"
#include "g1_limits.hh"
#include "status/status.hh"
#include "tex_id.hh"
#include "image/image8.hh"

#include "file/file.hh"
#include "mip.hh"
#include "r1_res.hh"
#include "palette/pal.hh"

extern int m1_max_mip_level;

static i4_str *m1_locate_texture(const i4_const_str &texture_filename)
{
  i4_file_status_struct s;
  i4_str *path, *fname, *ext;

  if (texture_filename.null())
    return 0;

  if (i4_get_status(texture_filename, s))
    return new i4_str(texture_filename);

  i4_filename_struct fn;
  i4_split_path(texture_filename, fn);

  char s_dir[128];
  i4_os_string(r1_gets("s_dir"), s_dir, 128);

  char def_name[256];
  if (fn.extension)
  {
    sprintf(def_name, "%s/%s.%s", s_dir, fn.filename, fn.extension);
    if (i4_get_status(def_name, s))
      return new i4_str(def_name);    
  }

  sprintf(def_name, "%s/%s.jpg", s_dir, fn.filename);
  if (i4_get_status(def_name, s))
    return new i4_str(def_name);    

  return 0;
}



static i4_image_class *m1_convert_8_to_32(i4_image_class *image8)
{
  i4_pixel_format fmt;
  fmt.default_format();

  const i4_pal *pal=i4_pal_man.register_pal(&fmt);
  
  i4_image_class *im8=image8;

  i4_image_class *new32=i4_create_image(im8->width(),
                                        im8->height(),
                                        pal);
  
  w32 *i32=(w32 *)new32->data;
  w8  *i8=(w8 *)im8->data;

  w32 *pal_data=image8->get_pal()->source.lookup;

  w32 i,t=im8->width()*im8->height();
  for (i=0; i<t; i++)
  {
    *i32=pal_data[*i8];    
    ++i32;
    ++i8;
  }
  
  return new32;
}

i4_bool r1_write_tga_mips(i4_image_class *src_texture,
                          char *dst_file,
                          char *texture_name, w32 chroma_color);

i4_bool m1_pack_mip_texture(const i4_const_str &src_tga,
                            const i4_const_str &out_ctext)
{
  i4_file_class *fp = i4_open(src_tga);

  i4_bool result = i4_F;

  if (!fp)
    return i4_F;

  i4_str *stat_str=r1_gets("loading_texture_x").sprintf(200,&src_tga);
  i4_status_class *status=i4_create_status(*stat_str);
  delete stat_str;
  i4_image_class *src_texture = i4_load_image(fp, status);
  if (status)
    delete status;

  delete fp;
  
  if (!src_texture)
    return i4_F;

  if (src_texture->get_pal()->source.pixel_depth==I4_8BIT)
  {
    i4_image_class *old = src_texture;    
    src_texture = m1_convert_8_to_32(src_texture);    
    delete old;
  }

  if (src_texture->get_pal()->source.pixel_depth==I4_32BIT)
  {
    /*
    //create a texture update window
    i4_str *update_texture_message = i4gets("updating_texture").sprintf(100,&src_tga);
    
    i4_status_class *status_window = i4_create_status(*update_texture_message);
    
    delete info; //stupid i4 str delete
    */

    char dst_ctext[256];
    i4_os_string(out_ctext,dst_ctext,256);

    char texname[256];
    i4_os_string(src_tga,texname,256);

    //generate the mips    
    result = r1_write_tga_mips(src_texture,dst_ctext,texname,G1_CHROMA_COLOR);
    
    //delete stat; //done making the mips, delete the update window       
  }    

  delete src_texture;

  return result;
}

i4_bool is_newer(const i4_const_str &f1,
                 const i4_const_str &f2)
{
  i4_file_status_struct f1_time, f2_time;

  if (i4_get_status(f1, f1_time))
  {
    if (i4_get_status(f2, f2_time))
      return f1_time.last_modified>f2_time.last_modified;
    else return i4_F;
  }
  else return i4_F;
}

// this function checks the times of the textures with the
// compressed mip version and updates it if it is newer
i4_bool m1_update_texture(const i4_const_str &texture_filename,
                          i4_bool force_all,
                          i4_status_class *stat,
                          float start, float end)
{
  i4_bool ret=i4_F;


  i4_file_status_struct s1,s2; 
  i4_str *tname=m1_locate_texture(texture_filename);
  if (tname)
    {
      if (!force_all)
        i4_get_status(*tname, s1);

      w32 id = r1_get_texture_id(*tname);

      i4_str *texture_file = r1_texture_id_to_filename(id, r1_get_compressed_dir());
    
      ret=i4_T;

      if (force_all)
        {
          m1_pack_mip_texture(*tname,*texture_file);
        }
      else
        {        
          memset(&s1,0,sizeof(i4_file_status_struct));
          memset(&s2,0,sizeof(i4_file_status_struct));
          i4_get_status(*tname, s1);
          i4_get_status(*texture_file, s2);
       
          if (s1.last_modified>s2.last_modified)
            {       
              m1_pack_mip_texture(*tname,*texture_file);
            }
        }


      delete texture_file;
      delete tname;
    } 



  
    return ret;
}

// this calls m1_update_texture on all the textures in an object model file
i4_bool m1_update_object_textures(const i4_const_str &object_filename, 
                                  i4_bool force_all,
                                  const i4_const_str *out_dir)
{

  i4_str *info=i4gets("updating_obj").sprintf(100, &object_filename);
  i4_status_class *stat=i4_create_status(*info);
  delete info;


  i4_bool ret=i4_F;
  i4_file_class *fp=i4_open(object_filename);
  if (fp)
  {
    i4_loader_class *gfp=i4_open_save_file(fp);
    if (gfp)
    {
      if (gfp->goto_section(G1_SECTION_MODEL_TEXTURE_NAMES))
      {
        w16 num_quads = gfp->read_16();
        
        for (int i=0; i<num_quads; i++)
        {
          i4_str *texture_name = gfp->read_counted_str(); 
          
          if (m1_update_texture(*texture_name, force_all, stat, 
                                i/(float)num_quads, (i+1)/(float)num_quads))
          {
            ret = i4_T;
          }
          
          delete texture_name;
          
          if (stat)
            stat->update((float)i / (float)num_quads);
        }
      }
      delete gfp;
    }
  }
  else i4_alert(i4gets("obj_missing"),100,&object_filename);

  if (stat)
    delete stat;

  return ret;
}

struct m1_tentry
{
  w32 id;
  w16 first_size;
  w16 w, h;
  w32 ns;
  w8 *data;
  w8 flags;
};

struct m1_tarray
{
  w32 id;
  w16 total;
  w32 *ids;
};

static int id_compare(const m1_tentry *a, const m1_tentry *b)
{
  if (a->id < b->id)
    return -1;
  else if (a->id > b->id)
    return 1;
  else return 0;
}



void m1_convert_all_in_dir(const i4_const_str &dir)
{
  i4_str *info=i4gets("converting_directory").sprintf(100,&dir);
  i4_status_class *stat=i4_create_status(*info);
  delete info;

 
  i4_directory_struct ds;
  if (i4_get_directory(dir, ds))
  {
    for (int i=0; i<ds.tfiles; i++)
    {
      i4_str *n=i4_const_str("%S/%S").sprintf(200, &dir, ds.files[i]);

      m1_update_texture(*n, i4_F, stat, i/(float)ds.tfiles, (i+1)/(float)ds.tfiles);
      delete n;
    }
  }

  if (stat)
    delete stat;  
}
