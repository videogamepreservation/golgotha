/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

// *.mip file structure
//32 bytes - null terminated string, texture name (no paths)
//1st sw32 - number of mip levels
//followed by that many offsets
//seek into the file at each offset to find the corresponding mip structure
//the mip structure is 2 sw32's (width and height) followed by width*height*2
//bytes of pixel data (16-bit color, 565 RGB)

#include <stdio.h>
#include "arch.hh"
#include "mip.hh"
#include "palette/pal.hh"
#include "mip_average.hh"
#include "loaders/jpg_load.hh"
#include "image/image.hh"
#include "loaders/jpg_write.hh"
#include "r1_res.hh"
#include "file/ram_file.hh"
#include "image/context.hh"

//no compression, 24 bpp
unsigned char tga_header[12] = {0,0,2,0,0,0,0,0,0,0,0,0};

#define TGA_HEADER_SIZE (12)



i4_image_class *adjust_to_power_of_two(i4_image_class *src_texture)
{
  sw32 new_width,new_height;
  
  sw32 old_width  = src_texture->width();
  sw32 old_height = src_texture->height();

  I4_ASSERT(src_texture->pal->source.pixel_depth==I4_32BIT, "");

  //find closest power of 2
  for (new_width  = 1; (new_width  < old_width)  && (new_width < r1_max_texture_size);  new_width  *= 2);
  for (new_height = 1; (new_height < old_height) && (new_height < r1_max_texture_size); new_height *= 2);  

  //obtain minimum aspect ratio of 8x1
  if (new_width > new_height)
  {    
    for ( ; new_height < (new_width>>3); new_height *= 2);
  }
  else
  if (new_height > new_width)
  {
    for ( ; new_width < (new_height>>3); new_width *= 2);  
  }

  //do we need to allocated an adjusted texture?
  if (new_width==src_texture->width() && new_height==src_texture->height())
    return 0;

  //yes  
  i4_pixel_format fmt;
  fmt.default_format();

  const i4_pal *pal = i4_pal_man.register_pal(&fmt);

  i4_image_class *new_texture = i4_create_image(new_width,new_height,pal);

  w32 *old_tex = (w32 *)src_texture->data;
  w32 *new_tex = (w32 *)new_texture->data;
  w32 *dst     = new_tex;
  
  double width_ratio  = (double)old_width  / (double)new_width;
  double height_ratio = (double)old_height / (double)new_height;
   
  //now scale the old to fit the new
  sw32 i,j;

  for (j=0; j<new_height; j++)
  for (i=0; i<new_width;  i++, dst++)
  {    
    *dst = old_tex[(sw32)((double)j * height_ratio)*old_width + (sw32)((double)i * width_ratio)];
  }

  return new_texture;
}

sw32 num_mips(sw32 width, sw32 height)
{
  sw32 num_mip_levels = 0;
  
  sw32 mipwidth;
  sw32 mipheight;

  while (1)
  {
    mipwidth  = width  / (1<<num_mip_levels);
    mipheight = height / (1<<num_mip_levels);
    
    num_mip_levels++;

    if (mipwidth==0 || mipheight==0 || (mipwidth < 8 && mipheight < 8))
    {
      //dont do this last one
      num_mip_levels--;
      break;  
    }        

    if (num_mip_levels==8)
      break;
  }
  
  return num_mip_levels;
}

void get_header_info(mipheader_t &header, i4_image_class *src_texture, 
                     char *texture_name, w32 chroma_color)
{
  sw32 i,j;
  sw32 width  = src_texture->width();
  sw32 height = src_texture->height();
  
  i4_bool has_chroma = i4_F;
  i4_bool has_alpha  = i4_F;  

  w32 rt=0,gt=0,bt=0, t=0;
  
  if (src_texture->get_pal()->source.pixel_depth==I4_32BIT)
  {
    w32 *base_mip = (w32 *)src_texture->data;

    sw32 im_size = width * height;
    
    for (i=0; i<im_size; i++, base_mip++)
    {
      w32 c = *base_mip;

      if (c & 0xFF000000)
        has_alpha = i4_T;

      if (c==chroma_color)
        has_chroma = i4_T;
      else
      {
        rt += ((c>>16) & 0xFF);
        gt += ((c>>8)  & 0xFF);
        bt += ((c>>0)  & 0xFF);
        t++;
      }
    }
  }
  else
  {
    i4_draw_context_class context(0,0,width-1,height-1);

    for (int y=0; y<height; y++)
    {    
      for (int x=0; x<width; x++)
      {
        i4_color c=src_texture->get_pixel(x,y, context);
    
        if (c & 0xFF000000)
          has_alpha = i4_T;

        if (c==chroma_color)
          has_chroma = i4_T;
        else        
        {
          rt += ((c>>16) & 0xFF);
          gt += ((c>>8)  & 0xFF);
          bt += ((c>>0)  & 0xFF);
          t++;
        }
      }    
    }
  }

  memset(&header,0,sizeof(mipheader_t));

  header.average_color = ((rt/t)<<16) | ((gt/t)<<8) | (bt/t);

  char *tname = r1_remove_paths(texture_name);
  if (strlen(tname)>31) tname[31]=0;

  strcpy(header.tname, tname);  

  header.base_width  = src_texture->width();
  header.base_height = src_texture->height();
  
  //wtf?
  //header.num_mip_levels = num_mips(src_texture->width(), src_texture->height());
  header.num_mip_levels = 1;
  
  header.offsets[0] = r1_mip_header_disk_size();
  for (i=1; i<R1_MAX_MIP_LEVELS; i++)
    header.offsets[i] = 0xFFFFFFFF;

  header.flags = 0;  
  
  if (has_alpha)
    header.flags = R1_MIP_IS_ALPHATEXTURE;
  else
  if (has_chroma)
    header.flags = R1_MIP_IS_TRANSPARENT;
  else
  if ((src_texture->width() >= 16) && (src_texture->height() >= 16))
    header.flags = R1_MIP_IS_JPG_COMPRESSED;  
}

i4_bool r1_write_tga_mips(i4_image_class *src_texture, char *dst_file, 
                          char *texture_name, w32 chroma_color)
{ 
  sw32 i,j;
  mipheader_t header;

  i4_file_class *f = i4_open(i4_const_str(dst_file), I4_WRITE);

  if (!f) return i4_F;

  i4_image_class *adjusted_texture = adjust_to_power_of_two(src_texture);
  i4_image_class *use_texture = adjusted_texture ? adjusted_texture : src_texture;

  get_header_info(header, use_texture, texture_name, chroma_color);
  header.write(f);

  if (!(header.flags & R1_MIP_IS_JPG_COMPRESSED))
  {
    w8 rgb[4];
    i4_color c;

    sw32 width  = use_texture->width();
    sw32 height = use_texture->height();

    f->write_32(width);
    f->write_32(height);
    
    i4_draw_context_class context(0,0, width-1, height-1);

    if (header.flags & R1_MIP_IS_ALPHATEXTURE)
    {
      for (j=0; j<height; j++)
      {
        for (i=0; i<width; i++)
        {
          //assumes that i4_color is a 32-bit ARGB
          c = use_texture->get_pixel(i,j,context);
          
          rgb[0] = (c>>24) & 0xFF;
          rgb[1] = (c>>16) & 0xFF;
          rgb[2] = (c>>8)  & 0xFF;
          rgb[3] = c & 0xFF;

          f->write(rgb,4);
        }
      }
    }
    else
    {
      for (j=0; j<height; j++)
      {
        for (i=0; i<width; i++)
        {
          c = use_texture->get_pixel(i,j,context);
          
          rgb[0] = (c>>16) & 0xFF;
          rgb[1] = (c>>8)  & 0xFF;
          rgb[2] = c & 0xFF;

          f->write(rgb,3);
        }
      }
    }
  }
  else
  {
    i4_str *fmt=r1_gets("encoding_jpg").sprintf(200, dst_file);
    i4_status_class *status=i4_create_status(*fmt);
    delete fmt;
    i4_write_jpeg(use_texture, f, 90, status);
    if (status)
      delete status;
  }

  if (use_texture!=src_texture)
    delete use_texture;

  delete f;
  return i4_T;
}

//#undef LINUX

/*
DECOMPRESSION ROUTINES
*/


w8  *temp_mip_24=0;
w16 *temp_mip_16=0;

sw32 last_mip_24_size = 0;
sw32 last_mip_16_size = 0;

i4_bool decompress_to_square = i4_F;

void r1_setup_decompression(i4_pixel_format *reg_fmt,
                            i4_pixel_format *chroma_fmt,
                            i4_pixel_format *alpha_fmt,
                            w32 chroma_color, i4_bool square_textures)
{
  setup_pixel_formats(*reg_fmt,*chroma_fmt,*alpha_fmt,chroma_color);
  last_mip_24_size = 0;
  last_mip_16_size = 0;
  temp_mip_24 = 0;
  temp_mip_16 = 0;
  decompress_to_square = square_textures;
}

void r1_end_decompression()
{
  if (temp_mip_24)
  {   
    i4_free(temp_mip_24);
    temp_mip_24 = 0;
  }
  
  if (temp_mip_16)
  {
    i4_free(temp_mip_16);
    temp_mip_16 = 0;
  }
  
  last_mip_24_size = 0;
  last_mip_16_size = 0;  
}

void process_intern_raw_decomp(i4_file_class *dst_file,
                               i4_file_class *dst_lowmip_file,
                               i4_file_class *src_file,
                               mipheader_t &new_mipheader,
                               mipheader_t *&old_mipheader,
                               sw32 &start_here)
{
  sw32 i,j,k,width,height;
  sw32 num_to_copy = new_mipheader.num_mip_levels;
  sw32 base_width  = old_mipheader->base_width;

  w8 base_pixel_size = 3;
  
  if (old_mipheader->flags & R1_MIP_IS_TRANSPARENT)
    base_pixel_size = 3;

  if (old_mipheader->flags & R1_MIP_IS_ALPHATEXTURE)
    base_pixel_size = 4;
  
  //this base mip better exist or else we must generate it  
  if (old_mipheader->offsets[start_here] == 0xFFFFFFFF)
  {
    //have to generate the new base mip, starting with mip #0. sucks.
    src_file->seek(old_mipheader->offsets[0]+8);    
    
    src_file->read(temp_mip_24,old_mipheader->base_width*old_mipheader->base_height*base_pixel_size);

    for (k=1; k < start_here; k++)
    {            
      width  = old_mipheader->base_width  / (1<<k);
      height = old_mipheader->base_height / (1<<k);

      make_next_mip(temp_mip_24,0,width,height,base_width,old_mipheader->flags);
    }
    //when thats done, temp_mip_24 has the proper starting new mip level
  }

  for (k=0; k<num_to_copy; k++)
  {    
    width  = old_mipheader->base_width  / (1<<(k+start_here));
    height = old_mipheader->base_height / (1<<(k+start_here));

    dst_file->write_32(width);
    dst_file->write_32(height);

    if (old_mipheader->offsets[k+start_here] != 0xFFFFFFFF)
    {        
      //+8 to skip the width and height
      src_file->seek(old_mipheader->offsets[k+start_here]+8);      

      //just read copy and convert
      src_file->read(temp_mip_24,width*height*base_pixel_size);
      
      mip_24_to_16(temp_mip_24,temp_mip_16,width,height,base_width,old_mipheader->flags);            
    }
    else
    {
      //assumes that temp_mip has the last mip level 
      //already in it. stores the next mip level in temp_mip
      
      make_next_mip(temp_mip_24,temp_mip_16,width,height,base_width,old_mipheader->flags);
    }
    
    dst_file->write(temp_mip_16,width*height*2);
    
    if (dst_lowmip_file && k==num_to_copy-1)
    {
      dst_lowmip_file->write_32(width);
      dst_lowmip_file->write_32(height);
      dst_lowmip_file->write(temp_mip_16,width*height*2);
    }
  }      
}

void process_intern_jpg_decomp(i4_file_class *dst_file,
                               i4_file_class *dst_lowmip_file,
                               i4_file_class *src_file,
                               mipheader_t &new_mipheader,
                               mipheader_t *&old_mipheader,
                               sw32 &start_here)
{
  sw32 i,j,k,width,height;
  sw32 num_to_copy = new_mipheader.num_mip_levels;
  sw32 base_width  = old_mipheader->base_width;
  sw32 jpg_width,jpg_height;

  //this base mip better exist or else we must generate it  
  if (old_mipheader->offsets[start_here] == 0xFFFFFFFF)
  {
    //have to generate the new base mip, starting with mip #0. sucks.
    src_file->seek(old_mipheader->offsets[0]);

    i4_jpg_loader_instance.special_load24(src_file,temp_mip_24,&jpg_width,&jpg_height);

    if (jpg_width != old_mipheader->base_width || jpg_height != old_mipheader->base_height)
    {
      i4_error("jpg size matchup error");
    }

    for (k=1; k < start_here; k++)
    {            
      width  = old_mipheader->base_width  / (1<<k);
      height = old_mipheader->base_height / (1<<k);
                 
      make_next_mip(temp_mip_24,0,width,height,base_width,old_mipheader->flags);
    }
    //when thats done, temp_mip has the proper starting new mip level
  }  

  for (k=0; k<num_to_copy; k++)
  {    
    width  = old_mipheader->base_width  / (1<<(k+start_here));
    height = old_mipheader->base_height / (1<<(k+start_here));

    dst_file->write_32(width);
    dst_file->write_32(height);
    
    if (old_mipheader->offsets[k+start_here] != 0xFFFFFFFF)
    {      
      src_file->seek(old_mipheader->offsets[k+start_here]);

      i4_jpg_loader_instance.special_load24(src_file,temp_mip_24,&jpg_width,&jpg_height);

      if (jpg_width != width || jpg_height != height)
      {
        i4_error("jpg size matchup error");
      }
      
      //just read, convert, and copy
      mip_24_to_16(temp_mip_24,temp_mip_16,width,height,base_width,old_mipheader->flags);      
    }
    else
    {
      //assumes that temp_mip has the last mip level
      //already in it. stores the next mip level in temp_mip
      
      make_next_mip(temp_mip_24,temp_mip_16,width,height,base_width,old_mipheader->flags);
    }
    
    dst_file->write(temp_mip_16,width*height*2);
    
    if (dst_lowmip_file && k==num_to_copy-1)
    {
      dst_lowmip_file->write_32(width);
      dst_lowmip_file->write_32(height);
      dst_lowmip_file->write(temp_mip_16,width*height*2);
    }
  }
}

i4_bool r1_decompress_to_local_mip(i4_file_class *orig_src_file,
                                   i4_file_class *dst_lowmip_file,
                                   char *network_file,
                                   char *local_file,
                                   mipheader_t *old_mipheader,
                                   sw32 max_mip_dimention)
{
  sw32 start_here=-1;
  sw32 num_to_copy=0;
  sw32 width,height;
  sw32 i,j,k;
  w8   *ram_file_buffer = 0;
  i4_ram_file_class *rf = 0;

  i4_file_class *src_file = orig_src_file;

  //make sure there is space allocated
  sw32 need_24_size = old_mipheader->base_width*old_mipheader->base_height*4;
  sw32 need_16_size = old_mipheader->base_width*old_mipheader->base_height*2;

  if (need_24_size > last_mip_24_size)
  {
    if (temp_mip_24)
      i4_free(temp_mip_24);

    temp_mip_24 = (w8 *)i4_malloc(need_24_size,"temp 24/32 bit decompression buffer");
    last_mip_24_size = need_24_size;
  }

  if (need_16_size > last_mip_16_size)
  {
    if (temp_mip_16)
      i4_free(temp_mip_16);

    temp_mip_16 = (w16 *)i4_malloc(need_16_size,"temp 16 bit decompression buffer");
    last_mip_16_size = need_16_size;
  }

  if (decompress_to_square && (old_mipheader->base_width != old_mipheader->base_height))
  {
    //make it a square, create a new i4_ram_file_class and point src_file to it
    mipheader_t m = *old_mipheader;
    m.base_width  = (old_mipheader->base_width < old_mipheader->base_height) ? (old_mipheader->base_width):(old_mipheader->base_height);
    m.base_height = m.base_width;
    m.num_mip_levels = 1;
    
    //declarations above
    ram_file_buffer = (w8 *)i4_malloc(need_24_size+r1_mip_header_disk_size(),"texture decompression ram file buffer");
    rf = new i4_ram_file_class(ram_file_buffer,need_24_size+r1_mip_header_disk_size());
    
    m.offsets[0] = r1_mip_header_disk_size();
    for (i=1; i<R1_MAX_MIP_LEVELS;i++)
      m.offsets[i] = 0xFFFFFFFF;

    m.write(rf);    
    rf->write_32(m.base_width);
    rf->write_32(m.base_height);

    w8 base_pixel_size = 3;

    if (old_mipheader->flags & R1_MIP_IS_TRANSPARENT)
      base_pixel_size = 3;

    if (old_mipheader->flags & R1_MIP_IS_ALPHATEXTURE)
      base_pixel_size = 4;
    
    if (old_mipheader->flags & R1_MIP_IS_JPG_COMPRESSED)
    {
      src_file->seek(old_mipheader->offsets[0]);

      sw32 jpg_width,jpg_height;
      
      i4_jpg_loader_instance.special_load24(src_file,temp_mip_24,&jpg_width,&jpg_height);
      if (jpg_width != old_mipheader->base_width || jpg_height != old_mipheader->base_height)
      {
        i4_error("jpg size matchup error");
      }
      
      m.flags &= (~R1_MIP_IS_JPG_COMPRESSED);
    }
    else
    {
      src_file->seek(old_mipheader->offsets[0]+8);
      src_file->read(temp_mip_24, old_mipheader->base_width*old_mipheader->base_height*base_pixel_size);
    }
    
    make_square(temp_mip_24, ram_file_buffer+r1_mip_header_disk_size()+8,
                base_pixel_size, 
                old_mipheader->base_width, old_mipheader->base_height,
                m.base_width);
    
    src_file = rf;
    *old_mipheader = m;
  }

  sw32 num_mip = num_mips(old_mipheader->base_width,old_mipheader->base_height);
  for (i=0; i<num_mip; i++)
  {
    width  = old_mipheader->base_width  / (1<<i);
    height = old_mipheader->base_height / (1<<i);
    
    if (width <= max_mip_dimention && height <= max_mip_dimention)
    {
      if (start_here==-1) start_here = i;
      num_to_copy++;      
    }    
  }

  //we need to copy at least 1 mip level
  //AND we need to shrink it before we store the local mip level
  if (start_here==-1)
  {    
    start_here  = old_mipheader->num_mip_levels-1;
    num_to_copy = 1;
  }

  mipheader_t new_mipheader;
  memset(&new_mipheader,0,sizeof(mipheader_t));

  strcpy(new_mipheader.tname,old_mipheader->tname);  
  new_mipheader.num_mip_levels = num_to_copy;
  new_mipheader.flags          = old_mipheader->flags;
  new_mipheader.base_width     = old_mipheader->base_width  / (1<<(start_here));
  new_mipheader.base_height    = old_mipheader->base_height / (1<<(start_here));
  new_mipheader.average_color  = old_mipheader->average_color;

  generate_mip_offsets(new_mipheader.base_width,
                       new_mipheader.base_height,
                       num_to_copy,
                       new_mipheader.offsets,
                       2);
  
  i4_file_class *dst_file;  
    
  dst_file = i4_open(i4_const_str(local_file), I4_WRITE);

  if (!dst_file)
    i4_warning("decompress_to_local_mip: dest texture null");  
  
  if (!src_file)
    i4_warning("decompress_to_local_mip: source texture null");

  new_mipheader.write(dst_file);
  
  //which way will the file be decompressed?
  if (old_mipheader->flags & R1_MIP_IS_JPG_COMPRESSED)
    process_intern_jpg_decomp(dst_file,dst_lowmip_file,src_file,
                              new_mipheader,old_mipheader,start_here);
  else 
    process_intern_raw_decomp(dst_file,dst_lowmip_file,src_file,
                              new_mipheader,old_mipheader,start_here);
    
  delete dst_file;
    
  //copy over the old info
  *old_mipheader = new_mipheader;
  
  //free up the temporary ram file
  if (rf)
  {
    delete rf;
    rf = 0;
  }

  if (ram_file_buffer)
  {
    i4_free(ram_file_buffer);
    ram_file_buffer=0;
  }

  return i4_T;
}


//regular shifting info
w32 mip_r_and; w8 mip_r_shift; //r and g shifts are to the left
w32 mip_g_and; w8 mip_g_shift; 
w32 mip_b_and; w8 mip_b_shift; //b shift is to the right

//chroma-key shifting info
w32 mip_c_r_and; w8 mip_c_r_shift; //r and g shifts are to the left
w32 mip_c_g_and; w8 mip_c_g_shift; 
w32 mip_c_b_and; w8 mip_c_b_shift; //b shift is to the right
w32 mip_c_a_and; w8 mip_c_a_shift; //a shift is to the left

//alpha texture shifting info
w32 mip_a_r_and; w8 mip_a_r_shift; //r and g shifts are to the left
w32 mip_a_g_and; w8 mip_a_g_shift; 
w32 mip_a_b_and; w8 mip_a_b_shift; //b shift is to the right
w32 mip_a_a_and; w8 mip_a_a_shift; //a shift is to the left

w8 mip_chroma_red,mip_chroma_green,mip_chroma_blue;

i4_profile_class pf_make_next_mip("make_next_mip");
i4_profile_class pf_mip_24_to_16("mip_24_to_16");
