/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "software/r1_software.hh"
#include "software/r1_software_globals.hh"

#define R1_S_ABS(x)   ((x)<0)?(-(x)):(x)

void r1_software_class::draw_line(sw32 _x0, sw32 _y0, sw32 _x1, sw32 _y1, w16 color)
{
  sw32 x0,y0,x1,y1,dx,dy; 
  
  register w16  *sp;  
  register sw32 error,count;
  register sw32 change;
  
  dx = R1_S_ABS(_x1 - _x0)+1;
  dy = R1_S_ABS(_y1 - _y0)+1;    

  if (dx>dy)
  {
    if (_x1<_x0)
    {
      y0 = _y1;
      x0 = _x1;
      y1 = _y0;
      x1 = _x0;      
    }
    else
    {
      y0 = _y0;
      x0 = _x0;
      y1 = _y1;
      x1 = _x1;
    }    
    sp = r1_software_render_buffer_ptr + x0 + (y0*r1_software_render_buffer_wpl);
    if (dy==0)
    {      
      while (dx)
      {        
        *sp = color;
        sp++;        
        dx--;
      }
      return;
    }
    
    if (y0>y1)
    {
      change = -r1_software_render_buffer_wpl;
    }
    else
    {
      change = r1_software_render_buffer_wpl;
    }

    error = 0;
    count = dx;

    while(count)
    {    
      *sp = color;
      sp++;     
      error += dy;
      if (error>dx)
      {
        sp += change;        
        error -= dx;
      }
      count--;      
    }
  }
  else
  {
    if (_y1<_y0)
    {
      y0 = _y1;
      x0 = _x1;
      y1 = _y0;
      x1 = _x0;
    }
    else
    {
      y0 = _y0;
      x0 = _x0;
      y1 = _y1;
      x1 = _x1;
    }
        
    sp = r1_software_render_buffer_ptr + x0 + (y0*r1_software_render_buffer_wpl);
    if (dx==0)
    {
      while (dy)
      {        
        *sp = color;
        sp += r1_software_render_buffer_wpl;
        dy--;
      }
      return;
    }
    if (x0>x1) change = -1;
    else change = 1;

    error = 0;
    count = dy;

    while(count)
    {                  
      *sp = color;
      sp += r1_software_render_buffer_wpl;
      error += dx;
      if (error>dy)
      {
        sp += change;        
        error -= dy;
      }
      count--;      
    }
  } 
}

void r1_software_class::draw_line(sw32 _x0, sw32 _y0, sw32 _x1, sw32 _y1, w16 s_color, w16 e_color)
{
  /*
  sw32 x0,y0,x1,y1,dx,dy,dr,dg,db; 
  sw32 r_int_add = 0, g_int_add = 0, b_int_add = 0;
  sw32 r_error   = 0, g_error   = 0, b_error   = 0;
    
  register w16  *sp;  
  register w16  color;
  register sw32 error,count;
  register sw32 change;
  
  dr = (e_color & fmt.red_mask)   - (s_color & fmt.red_mask);
  dg = (e_color & fmt.green_mask) - (s_color & fmt.green_mask);
  db = (e_color & fmt.blue_mask)  - (s_color & fmt.blue_mask);

  dx = R1_S_ABS(_x1 - _x0)+1;
  dy = R1_S_ABS(_y1 - _y0)+1;    

  if (dx>dy)
  {
    if (dr > dx)
    {
      r_int_add = dr / dx;
      dr = dr % dx;
    }
    
    if (dg > dx)
    {
      g_int_add = dg / dx;
      dg = dg % dx;
    }
    
    if (db > dx)
    {
      b_int_add = db / dx;
      db = db % dx;
    }
    
    color = start_color;

    if (_x1<_x0)
    {
      y0 = _y1;
      x0 = _x1;
      y1 = _y0;
      x1 = _x0;      
    }
    else
    {
      y0 = _y0;
      x0 = _x0;
      y1 = _y1;
      x1 = _x1;
    }    
    sp = r1_software_render_buffer_ptr + x0 + (y0*r1_software_render_buffer_wpl);
    if (dy==0)
    {      
      color += r_int_add;
      color += g_int_add;
      color += b_int_add;

      r_error += dr;
      if (r_error > dx)
      {
        r_error -= dx;
        color += 0;
      }

      while (dx)
      {        
        *sp = color;
        sp++;        
        dx--;
      }
      return;
    }
    
    if (y0>y1)
    {
      change = -r1_software_render_buffer_wpl;
    }
    else
    {
      change = r1_software_render_buffer_wpl;
    }

    error = 0;
    count = dx;

    while(count)
    {    
      *sp = color;
      sp++;     
      error += dy;
      if (error>dx)
      {
        sp += change;        
        error -= dx;
      }
      count--;      
    }
  }
  else
  {
    if (_y1<_y0)
    {
      y0 = _y1;
      x0 = _x1;
      y1 = _y0;
      x1 = _x0;
    }
    else
    {
      y0 = _y0;
      x0 = _x0;
      y1 = _y1;
      x1 = _x1;
    }
        
    sp = r1_software_render_buffer_ptr + x0 + (y0*r1_software_render_buffer_wpl);
    if (dx==0)
    {
      while (dy)
      {        
        *sp = color;
        sp += r1_software_render_buffer_wpl;
        dy--;
      }
      return;
    }
    if (x0>x1) change = -1;
    else change = 1;

    error = 0;
    count = dy;

    while(count)
    {                  
      *sp = color;
      sp += r1_software_render_buffer_wpl;
      error += dx;
      if (error>dy)
      {
        sp += change;        
        error -= dy;
      }
      count--;      
    }
  } 
  */
}
