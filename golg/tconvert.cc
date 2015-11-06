/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#error this file is not USED
#include "tconvert.hh"
#include "error/alert.hh"
#include "loaders/load.hh"
#include "math/point.hh"
#include "image/depth.hh"
#include "image/image8.hh"
#include "image/image32.hh"
#include "string/string.hh"
#include "error/alert.hh"
#include "g1_limits.hh"

#ifndef G1_RETAIL
#include "memory/growarry.hh"
#include "quantize/histogram.hh"
#include "quantize/median.hh"
#include "loaders/fli_load.hh"
#include "loaders/bmp_write.hh"
#include "image/image2.hh"
#include "image/color.hh"
#endif

enum { MAX_MIPS=8 } ;


#ifndef G1_RETAIL
static i4_bool target_needs_update(const i4_const_str &source, const i4_const_str &target)
{
  i4_file_status_struct stat_source, stat_target;
  if (!i4_file_man.get_status(source,stat_source))
    return i4_T;

  if (!i4_file_man.get_status(target,stat_target))  
    return i4_T;

  return (i4_bool) (stat_source.last_modified>stat_target.last_modified);
}
#endif


w32 g1_convert_pal_filename_to_id(const i4_const_str &name)
{
  i4_const_str::iterator pal_end=name.begin();  
  w32 pal_file_id;

  pal_file_id=                    pal_end.get().value();  ++pal_end; 
  pal_file_id= (pal_file_id<<8) | pal_end.get().value();  ++pal_end; 
  pal_file_id= (pal_file_id<<8) | pal_end.get().value();  ++pal_end;
  pal_file_id= (pal_file_id<<8) | pal_end.get().value();  ++pal_end;
  ++pal_end;

  return pal_file_id;
}

#ifndef G1_RETAIL

static i4_image_class *load_if_valid(i4_const_str &fname, 
                                     i4_grow_array<w32> &used,
                                     i4_bool new_only,
                                     w32 &match_id)
{
  i4_str *path, *filename, *extension;
  i4_image_class *ret=0;

  i4_file_man.split_path(fname, path, filename, extension);
  if (path) delete path;

  if (extension && *extension==i4gets("tga_ext"))
  {
    w32 id=g1_convert_pal_filename_to_id(*filename);
    i4_bool ok=i4_T;

    if (new_only)
    {
      for (w32 j=0; j<used.size(); j++)
        if (used[j]==id)
          ok=i4_F;
    }
    else 
      if (match_id != id)
        ok=i4_F;
      

    if (ok)
    {
      i4_alert(i4gets("best_pal"),100,filename);

      match_id=id;
      i4_str *open_name=i4gets("default_tga").sprintf(100, filename, &i4gets("tga_ext"));
      i4_file_class *fp=i4_file_man.open(*open_name);
      delete open_name;

      if (fp)
      {
        ret=i4_load_image(fp);    
        if (ret)
          used.add(id);

        delete fp;
      }
    }
  }
  if (filename) delete filename;
  if (extension) delete extension;

  return ret;  
}


static i4_str *get_bmp_name(const i4_const_str &name)
{
  i4_str *path, *filename, *ext;
  i4_file_man.split_path(name, path,filename,ext);

  i4_const_str::iterator pal_end=filename->begin();  
  for (w32 j=0; j<5; j++)
    ++pal_end;
  i4_str *pal_name=new i4_str(filename->begin(),pal_end,5);

  if (path) delete path;
  delete filename;
  if (ext) delete ext;

  i4_str *source=i4gets("pal_source_name").sprintf(100,pal_name);
  return source;
}

void g1_create_best_palettes()
{
  i4_str **files, **dirs;
  w32 tfiles, tdirs, i,j;
  i4_image_class *im;

  i4_grow_array<w32> calced(128,"calced pals",64);

  if (i4_file_man.get_directory(i4gets("default_tga_dir"), files, tfiles, dirs, tdirs))
  {
    for (i=0; i<tfiles; i++)
    {
      i4_histogram_class hist;

      w32 id;
      im=load_if_valid(*files[i], calced, i4_T, id);
      if (im)
      {
        j=0;
      
        while (im)
        {
          hist.add_image_colors(im, 1);
          delete im;
          im=0;
          while (j<tfiles && !im)
          {
            if (j!=i)
              im=load_if_valid(*files[j], calced, i4_F, id);
            j++;

          }          
        }

        i4_pal_handle_class pal=i4_median_cut(&hist, 1);

        i4_unmatched_image8 *i8=new i4_unmatched_image8(256,1,pal);
        i4_unmatched_image8::iterator it=i8->create_iterator(0,0);
        for (w32 x=0; x<256; x++) 
        {
          i8->iterator_store(it, x);
          ++it;
        }
        
        i4_file_man.mkdir(i4gets("texture_dir"));
        i4_file_man.mkdir(i4gets("pal_dir"));

        i4_str *target=get_bmp_name(*files[i]);
        i4_file_class *out=i4_file_man.open(*target,i4_file_manager_class::WRITE);
        delete target;

        if (out)
        {
          i4_write_bmp(i8, out);
          delete out;
        }

        delete i8;

      }

    }

    for (i=0; i<tfiles; i++)
      delete files[i];

    for (i=0; i<tdirs; i++)
      delete dirs[i];    

    i4_free(files);
    i4_free(dirs);
  }
 
}


static void null_pal_handle(i4_pal_handle_class &pal)
{
  w32 data[256];
  for (w32 i=0; i<256; i++)
    data[i]=(i<<16)|(i<<8)|i;

  i4_pixel_format fmt;
  fmt.pixel_depth=I4_8BIT;
  fmt.lookup=data;

  pal=i4_pal_man.register_pal(&fmt);  
}

static void save_pal(const i4_const_str &source,
                     const i4_const_str &target,
                     i4_pal_handle_class &pal)

{
  i4_file_class *fp=i4_file_man.open(source);
  if (!fp)
    null_pal_handle(pal);
  else
  {
    i4_image_class *im=i4_load_image(fp);    
    if (!im)
    {
      delete fp;
      null_pal_handle(pal);
      return;
    }
    i4_histogram_class *hist=new i4_histogram_class;
    hist->add_image_colors(im, 1);
    pal=i4_median_cut(hist, 1);

    i4_unmatched_image8 *i8=new i4_unmatched_image8(256,1,pal);
    i4_unmatched_image8::iterator i=i8->create_iterator(0,0);
    for (w32 x=0; x<256; x++) 
    {
      i8->iterator_store(i, x);
      ++i;
    }

    i4_file_man.mkdir(i4gets("texture_dir"));
    i4_file_man.mkdir(i4gets("pal_dir"));

    i4_file_class *out=i4_file_man.open(target,i4_file_manager_class::WRITE);
    if (!out)
    {
      delete hist;
      delete im;
      delete fp; 
      null_pal_handle(pal);
      return;
    }

    i4_write_bmp(i8, out);
    delete out;
    delete fp;
    delete i8;

    delete hist;
    delete im;
  }

}

#else

void g1_create_best_palettes() { ; }

#endif


static void load_palette(const i4_const_str &name, 
                         const i4_const_str &original_name,
                         w32 *buffer)
{  



  w32 pal_file_id=g1_convert_pal_filename_to_id(name);

  i4_str *target=i4gets("pal_target_name").sprintf(100,
                                                   (pal_file_id>>24)&0xff,
                                                   (pal_file_id>>16)&0xff,
                                                   (pal_file_id>>8)&0xff,
                                                   (pal_file_id)&0xff
                                                   );

#ifndef G1_RETAIL
  i4_str *source=get_bmp_name(name);
  if (target_needs_update(*source,*target))
  {
    i4_image_class *im=i4_load_image(*source);
    i4_pal_handle_class pal;

    if (!im)
    {
      i4_alert(i4gets("pal_file_missing"),120,source,&name);
      save_pal(original_name, *source, pal);
    }
    else
    {
      pal=im->get_pal();
      delete im;
    }

    
    if (pal.source_type()!=I4_8BIT)
      i4_error("palette file is not 8 bit");


    i4_file_class *fp=i4_file_man.open(*target,i4_file_manager_class::WRITE);
    if (!fp)
      i4_error("unable to open palette output file");
    
    w32 *pdata=pal.pal->source.lookup;
    for (w32 i=0; i<256; i++)
      fp->write_32(pdata[i]);
    delete fp;
  }

  delete source;
#endif
  
  i4_file_class *fp=i4_open(*target);
  if (!fp)
  {
    i4_alert(i4gets("file_missing"),100,target);
    i4_error("bye");
  }

  for (w32  i=0; i<256; i++)
    buffer[i]=fp->read_32();
  
  delete fp;
}


#ifndef G1_RETAIL

// this function will average 4 rgb packed colors and return the new rgb color
static inline i4_color average_4_32s(i4_color c1,
                                     i4_color c2,
                                     i4_color c3,
                                     i4_color c4)
{
  enum { r_mask=0xff0000,
         g_mask=0xff00,
         b_mask=0xff,
         r_shift=16,
         g_shift=8,
         b_shift=0 };


  w32 red=(((c1&r_mask)+
            (c2&r_mask)+
            (c3&r_mask)+
            (c4&r_mask))>>r_shift)/4;

  w32 green=(((c1&g_mask)+
              (c2&g_mask)+
              (c3&g_mask)+
              (c4&g_mask))>>g_shift)/4;

  w32 blue=(((c1&b_mask)+
             (c2&b_mask)+
             (c3&b_mask)+
             (c4&b_mask))>>b_shift)/4;

  return (red<<r_shift)|(green<<g_shift)|(blue<<b_shift);  
}

// this will create a half sized image where every pixel is the averge of two of it's parents
// pixels
void mip_scale(i4_image32 *im, i4_image2 *holy, i4_image32 *&ret_32, i4_image2 *&ret_2)
{
  i4_image32 *small=new i4_image32(im->width()/2,
                                   im->height()/2,
                                   im->get_pal());
  i4_image2 *small_holy=0;
  i4_image2::iterator hsource1, hsource2, hdest;
  i4_image32::iterator source1, source2, dest;
  
  if (holy)
    small_holy = new i4_image2(holy->width()/2,
                               holy->height()/2,
                               holy->get_pal());
  

  w32 xcounter,ycounter,
    end_skip=(im->width()&1); // in case the image width is odd we need to skip a byte
  
  source1=im->create_iterator(0,0);
  source2=im->create_iterator(0,1);
  dest=small->create_iterator(0,0);

  if (holy)
  {
    hsource1=holy->create_iterator(0,0);
    hsource2=holy->create_iterator(0,1);
    hdest=small_holy->create_iterator(0,0);
  }


  i4_color c1,c2,c3,c4, r,g,b;
  sw32 total_pixels;

  for (ycounter=small->height(); ycounter; ycounter--)
  {
    for (xcounter=small->width(); xcounter; xcounter--)
    {
      c1=im->iterator_get(source1);
      ++source1;
      c2=im->iterator_get(source1);
      ++source1;

      c3=im->iterator_get(source2);
      ++source2;
      c4=im->iterator_get(source2);
      ++source2;


      if (holy)
      {
        total_pixels=0;
        r=g=b=0;

        if (holy->iterator_get(hsource1)) 
        { total_pixels++; r+=(c1&0xff0000)>>16; g+=(c1&0xff00)>>8; b+=(c1&0xff)>>0; }
        ++hsource1;

        if (holy->iterator_get(hsource1))
        { total_pixels++; r+=(c2&0xff0000)>>16; g+=(c2&0xff00)>>8; b+=(c2&0xff)>>0; }
        ++hsource1;

        if (holy->iterator_get(hsource2))
        { total_pixels++; r+=(c3&0xff0000)>>16; g+=(c3&0xff00)>>8; b+=(c3&0xff)>>0; }
        ++hsource2;

        if (holy->iterator_get(hsource2))  
        { total_pixels++; r+=(c4&0xff0000)>>16; g+=(c4&0xff00)>>8; b+=(c4&0xff)>>0; }
        ++hsource2;

        if (total_pixels > (rand()%4))
        {
          small->iterator_store(dest, i4_rgb_to_32bit(r/total_pixels, 
                                                      g/total_pixels,
                                                      b/total_pixels));

          small_holy->iterator_store(hdest, 1);                   
        }
        else
        {
          small->iterator_store(dest, 0);
          small_holy->iterator_store(hdest, 0);
        }

        ++hdest;
        ++dest;
      }
      else
      {
        small->iterator_store(dest,average_4_32s(c1,c2,c3,c4));
        ++dest;
      }    
    }
    if (holy)
    {
      hsource1+=end_skip+im->width();
      hsource2+=end_skip+im->width();
    }

    source1+=end_skip+im->width();
    source2+=end_skip+im->width();
    
  }

  ret_32=small;
  ret_2=small_holy;

}


static i4_image32 *convert_8_to_32(i4_image_class *image8)
{
  i4_pal_handle_class pal;
  i4_unmatched_image8 *im8=(i4_unmatched_image8 *)image8;

  i4_image32 *new32=new i4_image32(im8->width(),
                                   im8->height(),
                                   pal);
  i4_image32::iterator i32=new32->create_iterator(0,0);
  i4_unmatched_image8::iterator i8=im8->create_iterator(0,0);

  w32 *pal_data=image8->get_pal().pal->source.lookup;

  w32 i,t=im8->width()*im8->height();
  for (i=0; i<t; i++)
  {    
    new32->iterator_store(i32, pal_data[ im8->iterator_get(i8) ]);
    ++i32;
    ++i8;
  }
  
  return new32;
}


void clear_holes(i4_unmatched_image8 *im8,
                 i4_image2 *im2)
{  
  i4_unmatched_image8::iterator i8=im8->create_iterator(0,0);
  i4_image2::iterator i2=im2->create_iterator(0,0);

  w32 t=im8->width() * im8->height(), i;

  for (i=0; i<t; i++)
  {
    if (!im2->iterator_get(i2))
      im8->iterator_store(i8, 0);
    ++i8;
    ++i2;
  }
}

i4_image2 *get_holes(i4_unmatched_image8 *im8)
{  
  w32 pal_d[2]= {0, 0xffffffff};
  i4_pixel_format fmt;
  fmt.pixel_depth=I4_2BIT;
  fmt.lookup=pal_d;

  i4_pal_handle_class pal=i4_pal_man.register_pal(&fmt);

  w32 t=im8->width() * im8->height(), i;

  i4_image2 *im2=new i4_image2(im8->width(), im8->height(), pal);

  i4_unmatched_image8::iterator i8=im8->create_iterator(0,0);
  i4_image2::iterator i2=im2->create_iterator(0,0);

  w32 *pal_data=im8->get_pal().pal->source.lookup;

  w32 index,red,green,blue,rgbcolor;
  i4_bool transparent;
  
  w32 r_mask  = 0x00ff0000;
  w32 g_mask  = 0x0000ff00;
  w32 b_mask  = 0x000000ff;
  w32 r_shift = 16;
  w32 g_shift = 8;
  w32 b_shift = 0;         

  for (i=0; i<t; i++)
  {
    index = im8->iterator_get(i8);
    
    rgbcolor = pal_data[index];

    red   = (rgbcolor & r_mask) >> r_shift;
    green = (rgbcolor & g_mask) >> g_shift;
    blue  = (rgbcolor & b_mask) >> b_shift;
    
    transparent = (red==254 && green==2 && blue==166);

    if (!transparent)
      im2->iterator_store(i2, 1);
    else
      im2->iterator_store(i2, 0);
    ++i8;
    ++i2;
  }
  return im2;
}

static i4_bool save_texture(i4_image32 *im, 
                            i4_image2  *holy_patern,
                            const i4_pal_handle_class &pal,
                            w32 pal_id,
                            const i4_const_str &filename)
{
  i4_image32 *mip_levels[MAX_MIPS];
  i4_image2  *holy_levels[MAX_MIPS];

  sw32 i,t_mips=0,w,h;

  i4_file_class *fp=i4_file_man.open(filename, i4_file_manager_class::WRITE);
  if (!fp) return i4_F;
  w=im->width();
  h=im->height();

  mip_levels[0]=im;
  holy_levels[0]=holy_patern;

  for (i=1; i<MAX_MIPS; i++)
  {   
    // if the texture is too small, don't create anymore mip map levels
    if (mip_levels[i-1]->width()<4 ||
        mip_levels[i-1]->height()<2)
      break;

    mip_scale(mip_levels[i-1],
              holy_levels[i-1],
              mip_levels[i],
              holy_levels[i]);
    t_mips++;
  }


  fp->write_8(G1_TEXTURE_FILE);
  fp->write_32(pal_id);
  fp->write_16(mip_levels[t_mips-1]->width());
  fp->write_16(mip_levels[t_mips-1]->height());


  for (i=t_mips-1; i>=0; i--)
  {   
    i4_unmatched_image8 *i8=mip_levels[i]->quantize(pal,1,
                                                    0,0,
                                                    mip_levels[i]->width(),
                                                    mip_levels[i]->height());
    if (holy_patern)
      clear_holes(i8, holy_levels[i]);
      

    if (i==0)
    {
      fp->write_16(0);
      fp->write_16(0);
    }
    else
    {
      fp->write_16(mip_levels[i-1]->width());
      fp->write_16(mip_levels[i-1]->height());
    }

    fp->write(i8->local_sub_data(0,0),
              i8->width() * i8->height());
             
    delete i8;
    delete mip_levels[i];
    if (holy_patern)
      delete holy_levels[i];
  } 

  delete fp;

  return i4_T;
}
#endif

g1_texture_handle convert_tga(const i4_const_str &original_name,
                              const i4_const_str &basename,
                              g1_texture_mapper_class &tmapper)
{
  i4_str *target=i4gets("text_name").sprintf(100, &basename);

#ifndef G1_RETAIL
  // see if we need to update the texture
  if (target_needs_update(original_name, *target))  
  {
    i4_alert(i4gets("updating_texture"),100,&original_name);
    i4_image_class *im=i4_load_image(original_name);
    if (!im)
    {
      i4_alert(i4gets("file_missing"), 100, &original_name);
      delete target;
      return 0;
    }
    else if (im->get_pal().source_type()!=I4_32BIT)    
    {
      i4_alert(i4gets("not_32"), 100, &original_name);      
      delete target;
      return 0;
    }
    else
    {
      w32 buf[256];
      load_palette(basename, original_name, buf);
      w32 pal_id=g1_convert_pal_filename_to_id(basename);

      i4_pixel_format fmt;
      fmt.pixel_depth = I4_8BIT;
      fmt.lookup = buf;

      i4_pal_handle_class pal=i4_pal_man.register_pal(&fmt);
      save_texture((i4_image32 *)im, 0, pal, pal_id, *target);
    }
  }  
#endif

  g1_file_handle han=g1_file_man.get_handle(*target);
  g1_texture_handle th=tmapper.register_texture(han);
  delete target;
  g1_file_man.destroy_handle(han);
  return th;
}



g1_texture_handle convert_flc(const i4_const_str &original_name,
                              const i4_const_str &basename,
                              g1_texture_mapper_class &tmapper)
{
  i4_str *target=i4gets("anim_name").sprintf(100, &basename);

#ifndef G1_RETAIL
  // see if we need to update the texture
  if (target_needs_update(original_name, *target))
  {
    i4_alert(i4gets("updating_anim"),100,&original_name);
    w32 tframes;
    //hacked by trey on 5-23-97 to load a series of .pcx's
    
    w32 buf[256];
    load_palette(basename, original_name, buf);

    i4_pixel_format fmt;
    fmt.pixel_depth = I4_8BIT;
    fmt.lookup = buf;

    i4_pal_handle_class pal=i4_pal_man.register_pal(&fmt);
    w32 pal_id=g1_convert_pal_filename_to_id(basename);
              
    i4_str *path,*bname,*extension;
    i4_str *fname;

    i4_file_man.split_path(original_name,path,bname,extension);
    
    w32 i;
    i = 1;
    i4_file_class *fp;

    while (1)
    {
      
      fname = i4gets("pcx_sequence").sprintf(100,path,bname,i);      
      
      fp = NULL;
      fp = i4_open(*fname);
      if (!fp) {
        delete fname;
        break;
      }

      i4_image_class *im = NULL;

      im=i4_load_image(fp);
      delete fp;
      
      if (im) {                
        i4_str *frame_name=i4gets("anim_frame_name").sprintf(100,&basename,i-1);
        i4_image32 *i32=convert_8_to_32((i4_unmatched_image8 *)im);
        i4_image2  *i2=get_holes((i4_unmatched_image8 *)im);

        save_texture(i32, NULL/*i2*/, pal, pal_id, *frame_name);
        delete frame_name;
        delete im;    
      }
      i++;
      delete fname;
    }
    
    tframes = i-1;
    
    if (path) delete path;
    if (bname) delete bname;
    if (extension) delete extension;

    fp=i4_file_man.open(*target,i4_file_manager_class::WRITE);
    fp->write_8(G1_ANIMATION_FILE);
    fp->write_16(tframes);
    fp->write_counted_str(basename);
    delete fp;    
  }  
#endif

  g1_file_handle han=g1_file_man.get_handle(*target);
  g1_texture_handle th=tmapper.register_texture(han);
  delete target;
  g1_file_man.destroy_handle(han);
  return th;
}


g1_texture_handle convert_name_to_texture(const i4_const_str &name,
                                          g1_texture_mapper_class &tmapper)
{
  if (name.length()==0)
  {
    i4_warning("texture has no name");
    return 0;
  }

  i4_str *path,*basename,*extension;
  i4_str *lower_name=new i4_str(name,name.length()+1);

  i4_file_man.split_path(*lower_name,path,basename,extension);
  basename->to_lower();
  lower_name->to_lower();

  if (path)
    delete path;


  if (extension)
  {
#ifndef G1_RETAIL
    i4_file_status_struct stat;
    if (!i4_file_man.get_status(*lower_name, stat))
    {
      delete lower_name;
      lower_name=i4gets("default_tga").sprintf(150, basename, extension);

      if (!i4_file_man.get_status(*lower_name, stat))
      {
        if (extension) delete extension;
        if (basename) delete basename;
        if (lower_name) delete lower_name;

        i4_alert(i4gets("file_missing"),100,&name);
        return 0;
      }
    }
#endif
    if (*extension==i4gets("flc_ext"))
    { 
      g1_texture_handle ret=convert_flc(*lower_name,*basename,tmapper);
      delete extension;
      delete basename;
      delete lower_name;
      return ret;
    }
    else if (*extension==i4gets("tga_ext"))
    {
      g1_texture_handle ret=convert_tga(*lower_name,*basename,tmapper);
      delete extension;
      delete basename;
      delete lower_name;
      return ret;
    }
  }
  else 
  {
    delete extension;
    delete basename;
    delete lower_name;
    i4_warning("no extension on texture");
    return 0;
  }
  return 0;
}

