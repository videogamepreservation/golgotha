/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "software/r1_software.hh"
#include "software/r1_software_globals.hh"

w32  *color_modify_list_low[128];
sw32 num_color_modifies_low=0;

w32  *color_modify_list_high[128];
sw32 num_color_modifies_high=0;

void insert_color_modify_address_low(w32 *address)
{
  if (num_color_modifies_low>=128)
  {
    i4_error("out of color modify address space low");
  }
  
  color_modify_list_low[num_color_modifies_low] = address;
  num_color_modifies_low++;  
}

void insert_color_modify_address_high(w32 *address)
{
  if (num_color_modifies_high>=128)
  {
    i4_error("out of color modify address space high");
  }
  
  color_modify_list_high[num_color_modifies_high] = address;
  num_color_modifies_high++;  
}

void update_color_modify_addresses(w32 *color_table_ptr)
{
  sw32 i;
  for (i=0;i<num_color_modifies_low;i++)
  {
    (*color_modify_list_low[i]) = (w32)color_table_ptr;
  }
  
  for (i=0;i<num_color_modifies_high;i++)
  {
    (*color_modify_list_high[i]) = ((w32)color_table_ptr) + ctable_size_bytes;
  }
}

const int MAX_SOFTWARE_COLOR_TABLES = 10;
software_color_table software_color_tables[MAX_SOFTWARE_COLOR_TABLES];
sw32 num_software_color_tables=0;

inline w16 light_pixel(w32 c, i4_pixel_format *s, double x,
                       double r_factor, double g_factor, double b_factor,
                       double &r_error, double &g_error, double &b_error)
{
  double r = (double)((c & s->red_mask)   >> s->red_shift);
  double g = (double)((c & s->green_mask) >> s->green_shift);
  double b = (double)((c & s->blue_mask)  >> s->blue_shift);

  double f_nr = r * x * r_factor;
  double f_ng = g * x * g_factor;
  double f_nb = b * x * b_factor;

  w32 nr,ng,nb;

  nr = i4_f_to_i(f_nr+0.5);
  
  ng = i4_f_to_i(f_ng+0.5);
  
  nb = i4_f_to_i(f_nb+0.5);

  return (s->red_mask & (nr << s->red_shift)) | (s->green_mask & (ng << s->green_shift)) | (s->blue_mask & (nb << s->blue_shift));
}

void generate_color_table(software_color_table *dest_table, i4_pixel_format *fmt,
                          double r_factor, double g_factor, double b_factor)
{
  sw32 x,y;

  double ytemp;
  
  double r_error,g_error,b_error;
  
  w32 lo;
  w32 hi;
  
  for(x=255; x>=0; x--)
  {
    r_error = 0;
    g_error = 0;
    b_error = 0;

    for(y=NUM_LIGHT_SHADES; y>=0; y--)
    {
      ytemp = (double)y / (double)NUM_LIGHT_SHADES;

      lo = light_pixel(x, fmt, ytemp, r_factor, g_factor, b_factor, r_error, g_error, b_error);

      dest_table->low_lookups()[y*256 + x] = lo;
    }
  }

  for (x=255; x>=0; x--)
  {
    r_error = 0;
    g_error = 0;
    b_error = 0;

    for(y=NUM_LIGHT_SHADES; y>=0; y--)
    {
      ytemp = (double)y / (double)NUM_LIGHT_SHADES;

      hi = light_pixel(((w32)x) << 8, fmt, ytemp, r_factor, g_factor, b_factor, r_error, g_error, b_error);

      dest_table->high_lookups()[y*256 + x] = hi;
    }
  }
}

void setup_alpha_table(i4_pixel_format *fmt)
{
  sw32 i;

  for (i=0; i<4096;i++)
  {
    w16 r = ((i & 0x0F00) >> 4);
    w16 g = ((i & 0x00F0) >> 0);
    w16 b = ((i & 0x000F) << 4);
    
    r = (r >> (8-fmt->red_bits))   << fmt->red_shift;
    g = (g >> (8-fmt->green_bits)) << fmt->green_shift;
    b = (b >> (8-fmt->blue_bits))  << fmt->blue_shift;

    alpha_table[i] = r | g | b;                               
  }    
}

r1_color_tint_handle r1_software_class::register_color_tint(i4_float r, i4_float g, i4_float b)
{
  if (num_color_tints>=MAX_SOFTWARE_COLOR_TABLES) return 0;
  
  generate_color_table(&software_color_tables[num_software_color_tables],&fmt,r,g,b);
  num_software_color_tables++;
  
  return num_software_color_tables-1;
}

void r1_software_class::set_color_tint(r1_color_tint_handle c)
{
  color_tint_on = i4_T;
  if ((c != cur_color_tint) && (c < num_software_color_tables))
  {
    cur_color_tint = c;
    
    //dont actually modify the code while you're buffering up spans, nothing is
    //actually being drawn - instead, the cur_color_tint variable is used to remember
    //what tint to use for all polys inserted into the buffer for this duration
    //of time, and then set_color_tint() is called again at the end of the frame for
    //each polygon, with use_spans set to false, so that it actually modifies the code
    if (!use_spans)
      update_color_modify_addresses(software_color_tables[c].table);
  }
}

void setup_color_modify_affine_lit();
void setup_color_modify_perspective_lit();

void setup_color_modify_affine_lit_amd3d();
void setup_color_modify_perspective_lit_amd3d();

void r1_software_class::init_color_tints()
{
  setup_alpha_table(&fmt);

#ifdef USE_AMD3D
  setup_color_modify_affine_lit_amd3d();
  setup_color_modify_perspective_lit_amd3d();
#endif

  setup_color_modify_affine_lit();
  setup_color_modify_perspective_lit();

  w16 tr = (((fmt.red_mask)   >> (fmt.red_shift))-1)   << fmt.red_shift;
  w16 tg = (((fmt.green_mask) >> (fmt.green_shift))-1) << fmt.green_shift;
  w16 tb = (((fmt.blue_mask)  >> (fmt.blue_shift))-1)  << fmt.blue_shift;
  
  pre_blend_and  = (tr | tg | tb);
  post_blend_and = pre_blend_and >> 1;

  pre_blend_and  = pre_blend_and  | (pre_blend_and  << 16);
  post_blend_and = post_blend_and | (post_blend_and << 16);
  
  red_clight_mul   = ((1<<fmt.red_bits)-1);
  green_clight_mul = ((1<<fmt.green_bits)-1);
  blue_clight_mul  = ((1<<fmt.blue_bits)-1);

  num_software_color_tables = 1;

  r1_software_class_instance.use_spans = i4_F;

  generate_color_table(&software_color_tables[0],&fmt,1.0,1.0,1.0);  
  
  set_color_tint(0);    
}

