/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "map.hh"
#include "g1_object.hh"
#include "height_info.hh"
#include "map_cell.hh"


// int g1_map_class::check_collision(i4_float x, i4_float y, 
//                                      i4_float occupancy_radius,
//                                      i4_float &dx, i4_float &dy,
//                                      sw32 _ix, sw32 _iy,
//                                      g1_object_class*& hit) const
// {
//   i4_float gx,gy,r;
//   i4_float a,b,c,d,e;

//   hit = 0;
//   i4_float min_dist=0;

//   sw32 x_left,x_right,y_top,y_bottom,ix,iy;
  
//   if (
//       (cell(_ix,_iy)->is_blocking()) || 
//       (x<0)    || (y<0)    || (x>=w)    || (y>=h) ||
//       (x+dx<0) || (y+dy<0) || (x+dx>=w) || (y+dy>=h)
//      )
//   {
//     //dx = 0; 
//     //dy = 0;
//     return -1;
//   }

//   x_left   = i4_f_to_i(x - fabs(dx)); if (x_left<0)     x_left=0;
//   x_right  = i4_f_to_i(x + fabs(dx)); if (x_right>w-1)  x_right=w-1;
//   y_top    = i4_f_to_i(y - fabs(dy)); if (y_top<0)      y_top=0;
//   y_bottom = i4_f_to_i(y + fabs(dy)); if (y_bottom>h-1) y_bottom=h-1;


//   for (ix=x_left; ix<=x_right;  ix++)
//   for (iy=y_top;  iy<=y_bottom; iy++)
//   {
//     if (ix>=0 && iy>=0 && ix<w && iy<h)
//     {
//       g1_object_chain_class *p = cell(ix,iy)->get_solid_list();
//       while (p)
//       {    
//         //if its an exact collision (exactly same x's and y's) its probably the same object,
//         //so ignore it
//         if (p->object->x!=x || p->object->y!=y)
//         {

//           gx = p->object->x - x;
//           gy = p->object->y - y;                

//           a  = dx*gx + dy*gy;
//           b  = dx*dx + dy*dy;                
//           c  = (a*a)/b;
        
//           d  = gx*gx + gy*gy;
//           r  = p->object->occupancy_radius();

//           if ((r*r)>d-c)
//           {      
//             i4_bool x_check=i4_F,y_check=i4_F;
//             //min_dist=d;
//             if (dx>0)
//             {
//               if ((p->object->x+r > x) && (p->object->x-r < (x+dx))) x_check = i4_T;
//             }
//             else
//             {
//               if ((p->object->x-r < x) && (p->object->x+r > (x+dx))) x_check = i4_T;
//             }
      
//             if (dy>0)
//             {
//               if ((p->object->y+r > y) && (p->object->y-r < (y+dy))) y_check = i4_T;
//             }
//             else
//             {
//               if ((p->object->y-r < y) && (p->object->y+r > (y+dy))) y_check = i4_T;
//             }

//             if (x_check && y_check)
//               hit =p->object;
//           }
//         }
//         p = p->next_solid();
//       }
//     }
//   }
  
//   if (hit)
//     return 1;
  
//   return 0;
// }

int g1_map_class::check_non_player_collision(g1_player_type player_num,
                                             const i4_3d_vector &point,
                                             i4_3d_vector &ray,
                                             g1_object_class*& hit) const
{
  hit = 0;

  sw32 x_left,x_right,y_top,y_bottom, ix,iy;

  if ((point.x<0) || (point.y<0) || (point.x>=w) || (point.y>=h) ||
      (point.x+ray.x<0) || (point.y+ray.y<0) || (point.x+ray.x>=w) || (point.y+ray.y>=h))
  {
    ray.x=0;
    ray.y=0;
    ray.z=0;
    return -1;
  }

  i4_3d_vector final(point);
  final += ray;
  i4_float height = terrain_height(final.x,final.y);
  if (final.z<height)
  {
    // hit the ground
    ray.z = height - point.z;
    return 1;
  }

  if (ray.x<0)
  {
    x_left   = i4_f_to_i(point.x + ray.x);
    x_right  = i4_f_to_i(point.x);
  }
  else
  {
    x_left   = i4_f_to_i(point.x);
    x_right  = i4_f_to_i(point.x + ray.x);
  }
  if (ray.y<0)
  {
    y_top    = i4_f_to_i(point.y + ray.y);
    y_bottom = i4_f_to_i(point.y);
  }
  else
  {
    y_top    = i4_f_to_i(point.y);
    y_bottom = i4_f_to_i(point.y + ray.y);
  }

  if (x_left<0)     x_left=0;    
  if (x_right>w-1)  x_right=w-1; 
  if (y_top<0)      y_top=0;     
  if (y_bottom>h-1) y_bottom=h-1;

  for (ix=x_left; ix<=x_right;  ix++)
    for (iy=y_top;  iy<=y_bottom; iy++)
    {
      g1_object_chain_class *p = cell(ix,iy)->get_solid_list();

      while (p)
      {
        // if the object is not on our team or so not dangerous (probably a building)
        if ((p->object->player_num!=player_num ||
             !p->object->get_flag(g1_object_class::DANGEROUS)) &&
            p->object->check_collision(point, ray))
          hit = p->object;
        
        p = p->next_solid();
      }
    }
  
  if (hit)
    return 1;
  
  return 0;
}


// int g1_map_class::check_cell(i4_float x, i4_float y, 
//                              i4_float occupancy_radius,
//                              sw32 ix, sw32 iy) const
// {
//   i4_float dx, dy, r;

//   if (!cell(ix,iy).get_solid_list())
//     return 1;
//   return 0;
// }


int g1_map_class::check_terrain_location(i4_float x, i4_float y, i4_float z,
                                         i4_float rad, w8 grade, w8 dir) const
{
  i4_float x_left,x_right,y_top,y_bottom;
  
  x_left   = x - rad; if (x_left<0)    return 0;
  x_right  = x + rad; if (x_right>=w)  return 0;
  y_top    = y - rad; if (y_top<0)     return 0;
  y_bottom = y + rad; if (y_bottom>=h) return 0;

  sw32 xl = i4_f_to_i(x_left);
  sw32 xr = i4_f_to_i(x_right);
  sw32 yt = i4_f_to_i(y_top);
  sw32 yb = i4_f_to_i(y_bottom);

  g1_map_cell_class *c;
//   const g1_block_map_class *block = get_block_map(grade);

//   c = &cell(xl,yt);

//   if (c->is_blocking() || block->is_blocked(xl,yt,dir)) return 0;

//   c = &cell(xr,yt);

//   if (c->is_blocking() || block->is_blocked(xr,yt,dir)) return 0;  

//   c = &cell(xl,yb);

//   if (c->is_blocking() || block->is_blocked(xl,yb,dir)) return 0;
  
//   c = &cell(xr,yb);

//   if (c->is_blocking() || block->is_blocked(xr,yb,dir)) return 0;
  
  return 1;
}
