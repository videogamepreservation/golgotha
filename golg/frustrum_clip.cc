/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "frustrum_clip.hh"
#include "poly/poly.hh"
#include "math/num_type.hh"
#include "clip.hh"

#define F_NEXT(i,j) ( ((i+1)==(j))?(0):(i+1) )
#define F_PREV(i,j) ( ((i)==(0))?(j-1):(i-1) )

#include "arch.hh"
#include "math/num_type.hh"
#include "frustrum_clip.hh"
#include "poly/poly.hh"

i4_polygon_class *g1_full_clip(i4_polygon_class *src, 
                               i4_polygon_class *dest,
                               w32 center_x, w32 center_y)
{
  w32 ORCODE  = 0;
  w32 ANDCODE = 0xffffffff;
  w32 i,j,c0,c1;
  w32 bitmask;
  i4_float ooz,dx,dy,dz,ds,dt,dr,dg,db,da,t;
  i4_polygon_class *temp;
  i4_vertex_class *v,clippoint;    
  
  dest->t_verts=0;
  
  for (i=0; i<src->t_verts; i++)
  {    
    v = &src->vert[i];
    v->outcode = g1_calc_clip_code(v->v, 0.01f, 0, i4_F);

    ORCODE  |= v->outcode;
    ANDCODE &= v->outcode;      
    if (!v->outcode)
    {
      ooz = r1_ooz(v->v.z);
      v->w  = ooz;
      v->px = ((v->v.x * ooz) * center_x) + center_x;
      v->py = ((v->v.y * ooz) * center_y) + center_y;        
    }
  }
  
  //all verts are outside one of the view planes
  //return a poly with 0 vertices
  if (ANDCODE)
  {
    dest->t_verts=0;
    return dest;
  }
  
  if (!ORCODE)
  {
    return src;
  }
  
  for (bitmask=16;bitmask;bitmask=bitmask>>1)
  {
    if ((bitmask&ORCODE)==0) continue;
    
    for (i=0;i<src->t_verts;i++)
    {			
      
      j  = F_NEXT(i,src->t_verts);
      
      c0 = bitmask & src->vert[i].outcode;
      c1 = bitmask & src->vert[j].outcode;
      
      //if c0 is not outside of this plane,
      //add it
      if (c0==0) {
        dest->add_vert(&src->vert[i]);
      }
      
      //if they are on the same
      //side, move to the next vert
      if (c0==c1) continue;
      
      //otherwise, generate a clipped
      //point
      
      dx = src->vert[j].v.x - src->vert[i].v.x;
      dy = src->vert[j].v.y - src->vert[i].v.y;
      dz = src->vert[j].v.z - src->vert[i].v.z;
      ds = src->vert[j].s - src->vert[i].s;
      dt = src->vert[j].t - src->vert[i].t;
      dr = src->vert[j].r - src->vert[i].r;
      dg = src->vert[j].g - src->vert[i].g;
      db = src->vert[j].b - src->vert[i].b;
      da = src->vert[j].a - src->vert[i].a;
      
      

      switch (bitmask) {
        case 1:  t = (-src->vert[i].v.x + src->vert[i].v.z) / ( dx - dz);                  
                 clippoint.v.y = src->vert[i].v.y + (t * dy);
                 clippoint.v.z = src->vert[i].v.z + (t * dz);
                 
                 clippoint.v.x = clippoint.v.z;
                 break;

        case 2:  t = ( src->vert[i].v.x + src->vert[i].v.z) / (-dx - dz);                  
                 clippoint.v.y = src->vert[i].v.y + (t * dy);
                 clippoint.v.z = src->vert[i].v.z + (t * dz);
                 
                 clippoint.v.x = -clippoint.v.z;
                 break;
        
        case 4:  t = (-src->vert[i].v.y + src->vert[i].v.z) / ( dy - dz);
                 clippoint.v.x = src->vert[i].v.x + (t * dx);                 
                 clippoint.v.z = src->vert[i].v.z + (t * dz);
                 
                 clippoint.v.y = clippoint.v.z;
                 break;
        
        case 8:  t = ( src->vert[i].v.y + src->vert[i].v.z) / (-dy - dz);
                 clippoint.v.x = src->vert[i].v.x + (t * dx);                 
                 clippoint.v.z = src->vert[i].v.z + (t * dz);
                 
                 clippoint.v.y = -clippoint.v.z; 
                 break;

        case 16: t = (0.01 - src->vert[i].v.z) / (dz);		                
                 clippoint.v.x = src->vert[i].v.x + (t * dx);
                 clippoint.v.y = src->vert[i].v.y + (t * dy);
                 
                 clippoint.v.z = 0.01;
                 break;
      }
      
      clippoint.s = src->vert[i].s + (t * ds);
      clippoint.t = src->vert[i].t + (t * dt);
      clippoint.r = src->vert[i].r + (t * dr);
      clippoint.g = src->vert[i].g + (t * dg);
      clippoint.b = src->vert[i].b + (t * db);
      clippoint.a = src->vert[i].a + (t * da);
      
      // no far clip
      clippoint.outcode = g1_calc_clip_code(clippoint.v, 0.01f, 0, i4_F);

      if (!clippoint.outcode)
      {
        ooz = r1_ooz(clippoint.v.z);        
        clippoint.px = ((clippoint.v.x * ooz) * center_x) + center_x;
        clippoint.py = ((clippoint.v.y * ooz) * center_y) + center_y;          
        clippoint.w  = ooz;
      }  
      ORCODE |= clippoint.outcode;
      
      dest->add_vert(&clippoint);      
    }
    
    temp = src;
    src = dest;
    dest = temp;
    dest->t_verts = 0;
  }
  return src;
}

i4_polygon_class *g1_fast_full_clip(i4_polygon_class *src, 
                                    i4_polygon_class *dest,
                                    w32 center_x, w32 center_y)
{
  w32 ORCODE  = 0;    
  w32 i,j,c0,c1;
  w32 bitmask;
  i4_float ooz,dx,dy,dz,ds,dt,dr,da,t;
  i4_polygon_class *temp;
  i4_vertex_class *v,clippoint;    
  
  dest->t_verts=0;
  
  for (i=0; i<src->t_verts; i++)
  {      
    v = &src->vert[i];
    v->outcode = g1_calc_clip_code(v->v, 0.01f, 0, i4_F);

    ORCODE |= v->outcode;    
  }
  
  for (bitmask=16;bitmask;bitmask=bitmask>>1)
  {
    if ((bitmask&ORCODE)==0) continue;
    
    for (i=0;i<src->t_verts;i++)
    {			
      
      j  = F_NEXT(i,src->t_verts);
      
      c0 = bitmask & src->vert[i].outcode;
      c1 = bitmask & src->vert[j].outcode;
      
      //if c0 is not outside of this plane,
      //add it
      if (c0==0) {
        dest->add_vert(&src->vert[i]);
      }
      
      //if they are on the same
      //side, move to the next vert
      if (c0==c1) continue;
      
      //otherwise, generate a clipped
      //point
      
      dx = src->vert[j].v.x - src->vert[i].v.x;
      dy = src->vert[j].v.y - src->vert[i].v.y;
      dz = src->vert[j].v.z - src->vert[i].v.z;
      ds = src->vert[j].s - src->vert[i].s;
      dt = src->vert[j].t - src->vert[i].t;
      dr = src->vert[j].r - src->vert[i].r;
      da = src->vert[j].a - src->vert[i].a;

      switch (bitmask) {
        case 1:  t = (-src->vert[i].v.x + src->vert[i].v.z) / ( dx - dz);                  
                 clippoint.v.y = src->vert[i].v.y + (t * dy);
                 clippoint.v.z = src->vert[i].v.z + (t * dz);
                 
                 clippoint.v.x = clippoint.v.z;
                 break;

        case 2:  t = ( src->vert[i].v.x + src->vert[i].v.z) / (-dx - dz);                  
                 clippoint.v.y = src->vert[i].v.y + (t * dy);
                 clippoint.v.z = src->vert[i].v.z + (t * dz);
                 
                 clippoint.v.x = -clippoint.v.z;
                 break;
        
        case 4:  t = (-src->vert[i].v.y + src->vert[i].v.z) / ( dy - dz);
                 clippoint.v.x = src->vert[i].v.x + (t * dx);                 
                 clippoint.v.z = src->vert[i].v.z + (t * dz);
                 
                 clippoint.v.y = clippoint.v.z;
                 break;
        
        case 8:  t = ( src->vert[i].v.y + src->vert[i].v.z) / (-dy - dz);
                 clippoint.v.x = src->vert[i].v.x + (t * dx);
                 clippoint.v.y = src->vert[i].v.y + (t * dy);
                 clippoint.v.z = src->vert[i].v.z + (t * dz);
                 
                 clippoint.v.y = -clippoint.v.z; 
                 break;

        case 16: t = (0.01 - src->vert[i].v.z) / (dz);		                
                 clippoint.v.x = src->vert[i].v.x + (t * dx);
                 clippoint.v.y = src->vert[i].v.y + (t * dy);
                 
                 clippoint.v.z = 0.01;
                 break;
      }
      
      clippoint.s = src->vert[i].s + (t * ds);
      clippoint.t = src->vert[i].t + (t * dt);
      clippoint.r = src->vert[i].r + (t * dr);
      
      clippoint.outcode =  g1_calc_clip_code(clippoint.v, 0.01f, 0, i4_F);

      if (!clippoint.outcode)
      {
        ooz = r1_ooz(clippoint.v.z);
        clippoint.px = ((clippoint.v.x * ooz) * center_x) + center_x;
        clippoint.py = ((clippoint.v.y * ooz) * center_y) + center_y;              
        clippoint.w  = ooz;
      }
      
      ORCODE |= clippoint.outcode;
      
      dest->add_vert(&clippoint);      
    }
    
    temp = src;
    src = dest;
    dest = temp;
    dest->t_verts = 0;
  }
  return src;
}

//only clips x,y,z
i4_polygon_class *g1_geometric_clip(i4_polygon_class *src, 
                                    i4_polygon_class *dest,
                                    w32 center_x, w32 center_y)
{
  w32 ORCODE  = 0;
  w32 ANDCODE = 0xffffffff;
  w32 i,j,c0,c1;
  w32 bitmask;
  i4_float ooz,dx,dy,dz,t;
  i4_polygon_class *temp;
  i4_vertex_class *v,clippoint;    
  
  dest->t_verts=0;
  
  for (i=0; i<src->t_verts; i++)
  {    
    v = &src->vert[i];
    v->outcode = g1_calc_clip_code(v->v, 0.01f, 0, i4_F);

    ORCODE  |= v->outcode;
    ANDCODE &= v->outcode;      
    if (!v->outcode)
    {
      ooz = r1_ooz(v->v.z);
      v->px = ((v->v.x * ooz) * center_x) + center_x;
      v->py = ((v->v.y * ooz) * center_y) + center_y;
      v->w  = ooz;
    }
  }
  
  //all verts are outside one of the view planes
  //return a poly with 0 vertices
  if (ANDCODE)
  {
    dest->t_verts=0;
    return dest;
  }
  
  if (!ORCODE)
  {
    return src;
  }
  
  for (bitmask=16;bitmask;bitmask=bitmask>>1)
  {
    if ((bitmask&ORCODE)==0) continue;
    
    for (i=0;i<src->t_verts;i++)
    {			
      
      j  = F_NEXT(i,src->t_verts);
      
      c0 = bitmask & src->vert[i].outcode;
      c1 = bitmask & src->vert[j].outcode;
      
      //if c0 is not outside of this plane,
      //add it
      if (c0==0) {
        dest->add_vert(&src->vert[i]);
      }
      
      //if they are on the same
      //side, move to the next vert
      if (c0==c1) continue;
      
      //otherwise, generate a clipped
      //point
      
      dx = src->vert[j].v.x - src->vert[i].v.x;
      dy = src->vert[j].v.y - src->vert[i].v.y;
      dz = src->vert[j].v.z - src->vert[i].v.z;
      
      switch (bitmask) {
        case 1:  t = (-src->vert[i].v.x + src->vert[i].v.z) / ( dx - dz);                  
                 clippoint.v.y = src->vert[i].v.y + (t * dy);
                 clippoint.v.z = src->vert[i].v.z + (t * dz);
                 
                 clippoint.v.x = clippoint.v.z;
                 break;

        case 2:  t = ( src->vert[i].v.x + src->vert[i].v.z) / (-dx - dz);                  
                 clippoint.v.y = src->vert[i].v.y + (t * dy);
                 clippoint.v.z = src->vert[i].v.z + (t * dz);
                 
                 clippoint.v.x = -clippoint.v.z;
                 break;
        
        case 4:  t = (-src->vert[i].v.y + src->vert[i].v.z) / ( dy - dz);
                 clippoint.v.x = src->vert[i].v.x + (t * dx);                 
                 clippoint.v.z = src->vert[i].v.z + (t * dz);
                 
                 clippoint.v.y = clippoint.v.z;
                 break;
        
        case 8:  t = ( src->vert[i].v.y + src->vert[i].v.z) / (-dy - dz);
                 clippoint.v.x = src->vert[i].v.x + (t * dx);
                 clippoint.v.y = src->vert[i].v.y + (t * dy);
                 clippoint.v.z = src->vert[i].v.z + (t * dz);
                 
                 clippoint.v.y = -clippoint.v.z; 
                 break;

        case 16: t = (0.01 - src->vert[i].v.z) / (dz);		                
                 clippoint.v.x = src->vert[i].v.x + (t * dx);
                 clippoint.v.y = src->vert[i].v.y + (t * dy);
                 
                 clippoint.v.z = 0.01;
                 break;
      }
            
      clippoint.outcode = g1_calc_clip_code(v->v, 0.01f, 0, i4_F);

      if (!clippoint.outcode)
      {
        ooz = r1_ooz(clippoint.v.z);
        clippoint.px = ((clippoint.v.x * ooz) * center_x) + center_x;
        clippoint.py = ((clippoint.v.y * ooz) * center_y) + center_y;              
        clippoint.w  = ooz;
      }
      
      ORCODE |= clippoint.outcode;
      
      dest->add_vert(&clippoint);      
    }
    
    temp = src;
    src = dest;
    dest = temp;
    dest->t_verts = 0;
  }
  return src;
}

