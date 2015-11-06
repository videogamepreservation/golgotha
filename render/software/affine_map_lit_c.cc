/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "software/r1_software_globals.hh"
#include "software/inline_fpu.hh"

w32 *texture_affine_lit_starter()
{
  w32 returnval;
  _asm
  {
    mov eax,OFFSET dumb_addr
dumb_addr:
    mov returnval,eax
  }
  return (w32 *)returnval;
}

void texture_scanline_affine_lit(w16 *start_pixel,
                                 sw32 start_x,
                                 void *_left,//perspective_span *left,
                                 sw32 width)
{
  //temporary dwords for lighting calculations
  w16 texel;  
  w32 t1,t2;
  w32 l_lookup;

  start_pixel = (w16 *)((w8 *)start_pixel + start_x);

  affine_span *left = (affine_span *)_left;
    
  left_s = left->s + cur_grads.s_adjust;
  left_t = left->t + cur_grads.t_adjust;
  left_l = left->l;

  temp_dsdx = qftoi(cur_grads.dsdx);
  temp_dtdx = qftoi(cur_grads.dtdx);

  width_global = width;

  while (width_global)
  {
    texel = *(r1_software_texture_ptr + (left_s>>16) + ((left_t>>16)<<r1_software_twidth_log2));
                
    l_lookup = left_l & (NUM_LIGHT_SHADES<<8);

    //lookup low bits
    t1 = ((w32 *)(0xDEADBEEF))[l_lookup + (texel & 0xFF)];

    //lookup high bits
    t2 = ((w32 *)(0xDEADBEEF)+ctable_size)[l_lookup + (texel>>8)];

    *start_pixel = (w16)(t1+t2);

    start_pixel++;

    left_s += temp_dsdx;
    left_t += temp_dtdx;
    left_l += dldx_fixed;

    width_global--;
  }
}

w32 *texture_affine_lit_sentinel()
{
  w32 returnval;
  _asm
  {
    mov eax,OFFSET dumb_addr
dumb_addr:
    mov returnval,eax
  }
  return (w32 *)returnval;
}

void insert_color_modify_address_low(w32 *address);
void insert_color_modify_address_high(w32 *address);
extern w32 color_modify_list[];
extern sw32 num_color_modifies;

void setup_color_modify_affine_lit()
{
  w32 *stop = texture_affine_lit_sentinel();

  w32 *search = texture_affine_lit_starter();
  //start searching for 0xDEADBEEF
  while (search < stop)
  {
    //casting craziness
    search = (w32 *)((w8 *)search + 1);
    if (*search==0xDEADBEEF)
    {
      insert_color_modify_address_low(search);
    }
    else
    if (*search==(0xDEADBEEF + ctable_size_bytes))
    {
      insert_color_modify_address_high(search);
    }
  }
}