/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "mip_average.hh"
#include "palette/pal.hh"
#include "mip.hh"
#include <string.h>

void setup_pixel_formats(i4_pixel_format &reg_fmt,
                         i4_pixel_format &chroma_fmt,
                         i4_pixel_format &alpha_fmt,
                         w32 chroma_color)
{
  //regular format setup
  mip_b_shift = 8-reg_fmt.blue_bits;  
  mip_b_and   = reg_fmt.blue_mask;

  mip_g_shift = reg_fmt.blue_bits - (8-reg_fmt.green_bits);
  mip_g_and   = reg_fmt.green_mask;

  mip_r_shift = reg_fmt.green_bits + reg_fmt.blue_bits - (8-reg_fmt.red_bits);
  mip_r_and   = reg_fmt.red_mask;

  //chroma format setup
  mip_c_b_shift = 8-chroma_fmt.blue_bits;  
  mip_c_b_and   = chroma_fmt.blue_mask;

  mip_c_g_shift = chroma_fmt.blue_bits - (8-chroma_fmt.green_bits);
  mip_c_g_and   = chroma_fmt.green_mask;

  mip_c_r_shift = chroma_fmt.green_bits + chroma_fmt.blue_bits - (8-chroma_fmt.red_bits);
  mip_c_r_and   = chroma_fmt.red_mask;

  mip_c_a_shift = chroma_fmt.green_bits + chroma_fmt.blue_bits + chroma_fmt.red_bits - (8-chroma_fmt.alpha_bits);
  mip_c_a_and   = chroma_fmt.alpha_mask;

  //alpha format setup
  mip_a_b_shift = 8-alpha_fmt.blue_bits;  
  mip_a_b_and   = alpha_fmt.blue_mask;

  mip_a_g_shift = alpha_fmt.blue_bits - (8-alpha_fmt.green_bits);
  mip_a_g_and   = alpha_fmt.green_mask;

  mip_a_r_shift = alpha_fmt.green_bits + alpha_fmt.blue_bits - (8-alpha_fmt.red_bits);
  mip_a_r_and   = alpha_fmt.red_mask;
       
  mip_a_a_shift = alpha_fmt.green_bits + alpha_fmt.blue_bits + alpha_fmt.red_bits - (8-alpha_fmt.alpha_bits);
  mip_a_a_and   = alpha_fmt.alpha_mask;

  //chroma key color setup
  mip_chroma_red   = (chroma_color & 0xFF0000) >> 16;
  mip_chroma_green = (chroma_color & 0x00FF00) >> 8;
  mip_chroma_blue  = (chroma_color & 0x0000FF);
}


void r1_dos_paths(char *src)
{
  while (*src)
  {
    if (*src=='/')
      *src='\\';
    
    src++;
  }
}

void r1_unix_paths(char *src)
{  
  while (*src)
  {
    if (*src=='\\')
      *src='/';
    
    src++;
  }   
}

char *r1_remove_paths(char *src)
{
  char *ret = src;

  while (*src)
  {
    if (*src=='/')
      ret=src+1;

    src++;    
  }

  return ret;
}

void r1_remove_extention(char *src)
{
  while (*src)
  {
    if (*src=='.')
    {
      *src=0;
      return;
    }
    src++;
  }
}

void concatenate_path(char *src, char *path)
{
  char temp[256];

  char *last_path = r1_remove_paths(src);
  strcpy(temp,last_path);

  *last_path=0;

  strcat(src,path);
  strcat(src,temp);
}

void chroma_process(w8 *&p, float &r, float &g, float &b, sw8 &num_regular, sw8 &num_chroma)
{
  if (p[0]==mip_chroma_red && p[1]==mip_chroma_green && p[2]==mip_chroma_blue)
  {    
    //IS the chroma color. dont average
    num_chroma++;
  }
  else
  {
    //not the chroma color. average as a regular pixel
    r += (float)(p[0]);
    g += (float)(p[1]);
    b += (float)(p[2]);
    
    num_regular++;
  }
}

void average_4x4_chroma(w16 *&dst, w8 *dst_24, w8 *&p0, w8 *p1, w8 *p2, w8 *p3)
{
  sw8 num_chroma  = 0;
  sw8 num_regular = 0;

  float r=0;
  float g=0;
  float b=0;

  chroma_process(p0,r,g,b,num_regular,num_chroma);
  chroma_process(p1,r,g,b,num_regular,num_chroma);
  chroma_process(p2,r,g,b,num_regular,num_chroma);
  chroma_process(p3,r,g,b,num_regular,num_chroma);

  //return the chroma color
  if (num_chroma > num_regular)
  {
    //alpha component and rgb component of 16-bit pixel should be entirely 0'd
    dst_24[0] = mip_chroma_red;
    dst_24[1] = mip_chroma_green;
    dst_24[2] = mip_chroma_blue;

    *dst  = 0;
    
    return;
  }

  float ooreg = 1.f / num_regular;
  
  r *= ooreg; if (r<0) r=0; if (r>255) r = 255;
  g *= ooreg; if (g<0) g=0; if (g>255) g = 255;
  b *= ooreg; if (b<0) b=0; if (b>255) b = 255;

  dst_24[0] = (w8)r;
  dst_24[1] = (w8)g;
  dst_24[2] = (w8)b;

  *dst = (( (w16)dst_24[0] << mip_c_r_shift) & mip_c_r_and) |
         (( (w16)dst_24[1] << mip_c_g_shift) & mip_c_g_and) |
         (( (w16)dst_24[2] >> mip_c_b_shift) & mip_c_b_and) |          
         (( 0xFFFFFFFF << mip_c_a_shift) & mip_c_a_and);  
}

void average_4x4_chroma(w8 *dst_24, w8 *&p0, w8 *p1, w8 *p2, w8 *p3)
{
  sw8 num_chroma  = 0;
  sw8 num_regular = 0;

  float r=0;
  float g=0;
  float b=0;

  chroma_process(p0,r,g,b,num_regular,num_chroma);
  chroma_process(p1,r,g,b,num_regular,num_chroma);
  chroma_process(p2,r,g,b,num_regular,num_chroma);
  chroma_process(p3,r,g,b,num_regular,num_chroma);

  //return the chroma color
  if (num_chroma > num_regular)
  {
    //alpha component and rgb component of 16-bit pixel should be entirely 0'd
    dst_24[0] = mip_chroma_red;
    dst_24[1] = mip_chroma_green;
    dst_24[2] = mip_chroma_blue;
    return;
  }

  float ooreg = 1.f / num_regular;
  
  r *= ooreg; if (r<0) r=0; if (r>255) r = 255;
  g *= ooreg; if (g<0) g=0; if (g>255) g = 255;
  b *= ooreg; if (b<0) b=0; if (b>255) b = 255;

  dst_24[0] = (w8)r;
  dst_24[1] = (w8)g;
  dst_24[2] = (w8)b;
}

void average_4x4_alpha(w8 *dst_24, w8 *&p0, w8 *p1, w8 *p2, w8 *p3)
{
  sw32 ia,ir,ig,ib;
  
  ia = p0[0];
  ir = p0[1];
  ig = p0[2];
  ib = p0[3];
  
  ia += (p1[0]);
  ir += (p1[1]);
  ig += (p1[2]);
  ib += (p1[3]);
  
  ia += (p2[0]);
  ir += (p2[1]);
  ig += (p2[2]);
  ib += (p2[3]);
  
  ia += (p3[0]);
  ir += (p3[1]);  
  ig += (p3[2]);
  ib += (p3[3]);

  ia = ia >> 2;
  ir = ir >> 2;
  ig = ig >> 2;
  ib = ib >> 2;
  
  dst_24[0] = (w8)ia;
  dst_24[1] = (w8)ir;
  dst_24[2] = (w8)ig;
  dst_24[3] = (w8)ib;
}

void average_4x4_alpha(w16 *dst, w8 *dst_24, w8 *&p0, w8 *p1, w8 *p2, w8 *p3)
{
  sw32 ia,ir,ig,ib;
  
  ia = p0[0];
  ir = p0[1];
  ig = p0[2];
  ib = p0[3];
  
  ia += (p1[0]);
  ir += (p1[1]);
  ig += (p1[2]);
  ib += (p1[3]);
  
  ia += (p2[0]);
  ir += (p2[1]);
  ig += (p2[2]);
  ib += (p2[3]);
  
  ia += (p3[0]);
  ir += (p3[1]);  
  ig += (p3[2]);
  ib += (p3[3]);

  ia = ia >> 2;
  ir = ir >> 2;
  ig = ig >> 2;
  ib = ib >> 2;
  
  dst_24[0] = (w8)ia;
  dst_24[1] = (w8)ir;
  dst_24[2] = (w8)ig;
  dst_24[3] = (w8)ib;

  *dst = (( (w16)dst_24[1] << mip_a_r_shift) & mip_a_r_and) |
         (( (w16)dst_24[2] << mip_a_g_shift) & mip_a_g_and) |
         (( (w16)dst_24[3] >> mip_a_b_shift) & mip_a_b_and) |          
         (( (w16)dst_24[0] << mip_a_a_shift) & mip_a_a_and);
}


void average_4x4_normal(w16 *dst, w8 *dst_24, w8 *p0, w32 bpl)
{  
#ifndef WIN32
  
  sw32 ir,ig,ib;
  
  ir = p0[0];
  ig = p0[1];
  ib = p0[2];
  
  ir += (p0[0+3]);
  ig += (p0[1+3]);
  ib += (p0[2+3]);
  
  ir += (p0[0+bpl]);
  ig += (p0[1+bpl]);
  ib += (p0[2+bpl]);
  
  ir += (p0[0+bpl+3]);
  ig += (p0[1+bpl+3]);
  ib += (p0[2+bpl+3]);

  ir = ir >> 2;
  ig = ig >> 2;
  ib = ib >> 2;

  dst_24[0] = (w8)ir;
  dst_24[1] = (w8)ig;
  dst_24[2] = (w8)ib;

  *dst = (( (w16)p0[0] << mip_r_shift) & mip_r_and) |
         (( (w16)p0[1] << mip_g_shift) & mip_g_and) |
         (( (w16)p0[2] >> mip_b_shift) & mip_b_and);
#else
  _asm
  {
  mov esi,p0
  mov edi,dst
  
  mov ecx,bpl

  mov eax,0 //red
  mov ebx,0 //green
  mov edx,0 //blue

  mov al,byte ptr [esi]  
  add al,byte ptr [esi+3]
  adc ah,0
  add al,byte ptr [esi+ecx]
  adc ah,0
  add al,byte ptr [esi+ecx+3]
  adc ah,0

  mov bl,byte ptr [esi+1]  
  add bl,byte ptr [esi+4]
  adc bh,0
  add bl,byte ptr [esi+ecx+1]
  adc bh,0
  add bl,byte ptr [esi+ecx+4]
  adc bh,0

  mov dl,byte ptr [esi+2]
  add dl,byte ptr [esi+5]
  adc dh,0
  add dl,byte ptr [esi+ecx+2]
  adc dh,0
  add dl,byte ptr [esi+ecx+5]
  adc dh,0
      
  shr eax,2
  shr ebx,2
  shr edx,2

  mov esi,dst_24

  mov byte ptr [esi],al
  mov byte ptr [esi+1],bl
  mov byte ptr [esi+2],dl
  
  mov cl,mip_r_shift
  shl eax,cl
  and eax,mip_r_and

  mov cl,mip_g_shift
  shl ebx,cl
  and ebx,mip_g_and

  mov cl,mip_b_shift
  shr edx,cl
  and edx,mip_b_and

  or  eax,ebx
  or  eax,edx

  mov word ptr [edi],ax
  }
#endif
} 

void average_4x4_normal(w8 *dst_24, w8 *p0, w32 bpl)
{  
#ifndef WIN32
  sw32 ir,ig,ib;
  
  ir = (p0[0]);
  ig = (p0[1]);
  ib = (p0[2]);
  
  ir += (p0[0+3]);
  ig += (p0[1+3]);
  ib += (p0[2+3]);
  
  ir += (p0[0+bpl]);
  ig += (p0[1+bpl]);
  ib += (p0[2+bpl]);
  
  ir += (p0[0+bpl+3]);
  ig += (p0[1+bpl+3]);
  ib += (p0[2+bpl+3]);  

  ir = ir >> 2; if (ir<0) ir = 0; else if (ir>255) ir = 255;
  ig = ig >> 2; if (ig<0) ig = 0; else if (ig>255) ig = 255;
  ib = ib >> 2; if (ib<0) ib = 0; else if (ib>255) ib = 255;
  
  dst_24[0] = (w8)ir;
  dst_24[1] = (w8)ig;
  dst_24[2] = (w8)ib;  
#else  
  _asm
  {
  mov esi,p0
  mov edx,bpl
  mov edi,dst_24

  mov eax,0 //red
  mov ebx,0 //green
  mov ecx,0 //blue

  mov al,byte ptr [esi]  
  add al,byte ptr [esi+3]
  adc ah,0
  add al,byte ptr [esi+edx]
  adc ah,0
  add al,byte ptr [esi+edx+3]
  adc ah,0

  mov bl,byte ptr [esi+1]  
  add bl,byte ptr [esi+4]
  adc bh,0
  add bl,byte ptr [esi+edx+1]
  adc bh,0
  add bl,byte ptr [esi+edx+4]
  adc bh,0

  mov cl,byte ptr [esi+2]
  add cl,byte ptr [esi+5]
  adc ch,0
  add cl,byte ptr [esi+edx+2]
  adc ch,0
  add cl,byte ptr [esi+edx+5]
  adc ch,0
      
  shr eax,2
  shr ebx,2
  shr ecx,2
  
  mov byte ptr [edi],al
  mov byte ptr [edi+1],bl
  mov byte ptr [edi+2],cl
  }
#endif
} 


void mip_24_to_16(w8 *mip24, w16 *mip16, sw32 &width, sw32 &height, sw32 &base_width, w8 &flags)
{
  pf_mip_24_to_16.start();

  sw32 i,j,total_pixels;
  
  w8  *src = mip24;
  w16 *dst = mip16;
    
  sw32 base_bpl = base_width*3;
  sw32 src_add  = (base_width - width)*3;
  
  if (flags & R1_MIP_IS_ALPHATEXTURE)
  {
    base_bpl = base_width*4;
    src_add  = (base_width - width)*4;
  }  

  if (flags & R1_MIP_IS_TRANSPARENT)
  {
    for (j=0;j<height;j++)
    {
      for (i=0;i<width;i++)
      {        
        chroma_remap_pixel(dst,src);
      
        dst += 1;
        src += 3;      
      }
  
      src += src_add;
    }
  }
  else
  if (flags & R1_MIP_IS_ALPHATEXTURE)
  {
    for (j=0;j<height;j++)
    {
      for (i=0;i<width;i++)
      {        
        alpha_remap_pixel(dst,src);
      
        dst += 1;
        src += 4;      
      }
  
      src += src_add;
    }
  }
  else
  {
    for (j=0; j<height; j++)
    {
      src = mip24 + j*base_bpl;

      for (i=0; i<width;  i++)
      {
        remap_pixel(dst,src);
      
        dst += 1;
        src += 3;
      }
      
      //src += src_add;
    }
  }
  pf_mip_24_to_16.stop();
}

void make_next_mip(w8 *mip24, w16 *mip16, sw32 &width, sw32 &height, sw32 &base_width, w8 &flags)
{
  pf_make_next_mip.start();

  sw32 i,j;
  
  w8  *src    = mip24;
  w16 *dst    = mip16;
  w8  *dst_24 = mip24;

  sw32 base_bpl   = base_width*3;
  sw32 dst_24_add = (base_width - width)*3;
  sw32 src_add    = (base_width - width*2)*3 + base_bpl;
  
  if (flags & R1_MIP_IS_ALPHATEXTURE)
  {
    base_bpl   = base_width*4;
    dst_24_add = (base_width - width)*4;
    src_add    = (base_width - width*2)*4 + base_bpl;
  }  

  if (dst)
  {
    if (flags & R1_MIP_IS_TRANSPARENT)
    {
      for (j=0; j<height; j++)
      {
        for (i=0; i<width;  i++)
        {
          average_4x4_chroma(dst,dst_24,src,src+3,src+base_bpl,src+base_bpl+3);
       
          dst    += 1;
          dst_24 += 3;
          src    += 6;
        }
        
        dst_24 += dst_24_add;
        src    += src_add;        
      }
    }
    else
    if (flags & R1_MIP_IS_ALPHATEXTURE)
    {
      for (j=0; j<height; j++)
      {
        for (i=0; i<width;  i++)
        {
          average_4x4_alpha(dst,dst_24,src,src+4,src+base_bpl,src+base_bpl+4);
       
          dst    += 1;
          dst_24 += 4;
          src    += 8;
        }
        
        dst_24 += dst_24_add;
        src    += src_add;        
      }
    }
    else
    {
      for (j=0; j<height; j++)
      {
        for (i=0; i<width;  i++)
        {
          average_4x4_normal(dst,dst_24,src,base_bpl);
        
          dst    += 1;
          dst_24 += 3;
          src    += 6;
        }
               
        dst_24 += dst_24_add;
        src    += src_add;        
      }
    }
  }
  else
  {
    if (flags & R1_MIP_IS_TRANSPARENT)
    {
      for (j=0; j<height; j++)
      {        
        for (i=0; i<width;  i++)
        {
          average_4x4_chroma(dst_24,src,src+3,src+base_bpl,src+base_bpl+3);
          
          dst_24 += 3;
          src    += 6;
        }        
        
        dst_24 += dst_24_add;
        src    += src_add;
      }
    }
    else
    if (flags & R1_MIP_IS_ALPHATEXTURE)
    {
      for (j=0; j<height; j++)
      {
        for (i=0; i<width;  i++)
        {
          average_4x4_alpha(dst_24,src,src+4,src+base_bpl,src+base_bpl+4);

          dst_24 += 4;
          src    += 8;
        }
        
        dst_24 += dst_24_add;
        src    += src_add;        
      }
    }
    else
    {
      for (j=0; j<height; j++)
      {        
        for (i=0; i<width;  i++)
        {
          average_4x4_normal(dst_24,src,base_bpl);
          
          dst_24 += 3;
          src    += 6;
        }       
        
        dst_24 += dst_24_add;
        src    += src_add;
      }
    }
  }
  
  pf_make_next_mip.stop();
}

void make_square(w8 *mip24, w8 *square_mip24, w8 base_pixel_size, sw32 base_width, 
                 sw32 base_height, sw32 new_dimention)
{
  double width_ratio  = (double)base_width  / (double)new_dimention;
  double height_ratio = (double)base_height / (double)new_dimention;
   
  //now scale the old to fit the new
  sw32 i,j,k;

  w8 *dst = square_mip24;

  for (j=0; j<new_dimention; j++)
  for (i=0; i<new_dimention; i++)
  {    
    w8 *src = &mip24[(i4_f_to_i( ((double)j * height_ratio)*base_width ) + 
                      i4_f_to_i( ((double)i * width_ratio))) * base_pixel_size ];

    for (k=0; k<base_pixel_size; k++)
    {
      *dst = *src;
      src++; 
      dst++;
    }
  }

}


void generate_mip_offsets(sw32 base_width,sw32 base_height,
                          sw32 num_to_copy, 
                          sw32 *offsets, sw32 pixel_size)
{  
  sw32 cur_offset = r1_mip_header_disk_size();

  //start copying w/this mip
  sw32 i;
  for (i=0;i<num_to_copy;i++)
  {
    offsets[i] = cur_offset;

    sw32 width  = base_width  / (1<<i);
    sw32 height = base_height / (1<<i);
    
    cur_offset += ((width*height*pixel_size) + sizeof(sw32)*2);
  }
}
