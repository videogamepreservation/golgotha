/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "r1_clip.hh"
#include "r1_vert.hh"
#include "r1_api.hh"
#include "image/context.hh"

#define FPDIV_CORRECTION 0.0001

int *r1_clip(r1_clip_vert_array *v_array,
             int *src, int t_src,             // indexes into vertex array for initial verts
             int *dst1, int *dst2,            // destination index arrays
             int &t_dst,
             float center_x, float center_y,
             i4_bool clip_code_and_project_done)
{
  w32 ORCODE  = 0;
  w32 ANDCODE = 0xffffffff;
  w32 i,j,c0,c1;
  w32 bitmask;
  i4_float ooz,dx,dy,dz,ds,dt,dr,dg,db,da,t;

  
  t_dst=0;

  r1_vert *vbuf=v_array->buf, *v;
  int *sv=src;

  if (clip_code_and_project_done)
  {
    for (i=0; i<t_src; i++, sv++)
    {    
      v=vbuf+*sv;
      ORCODE  |= v->outcode;
      ANDCODE &= v->outcode;
    }
  }
  else
  {
    for (i=0; i<t_src; i++, sv++)
    {    
      v=vbuf+*sv;
      v->outcode = r1_calc_outcode(v);

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
  }

  //all verts are outside one of the view planes return a poly with 0 vertices
  if (ANDCODE)
    return 0;
  
  if (!ORCODE)      // all verts are inside, return that no clipping occurred
  {
    t_dst=t_src;
    return src;
  }

  
  int *dst=dst1;  

  for (bitmask=16; bitmask; bitmask=bitmask>>1)
  {
    if (bitmask & ORCODE)
    {
      t_dst=0;

      for (i=0; i<t_src; i++)
      {			
        j=i+1;
        if (j==t_src) j=0;

        r1_vert *vi=vbuf + src[i], *vj=vbuf + src[j], *cp;
      
        c0 = bitmask & vi->outcode;
        c1 = bitmask & vj->outcode;
      
        //if c0 is not outside of this plane, add it
        if (c0==0) 
          dst[t_dst++]=src[i];
      
        //if they are on the same
        //side, move to the next vert
        if (c0==c1) continue;
      
        //otherwise, generate a clipped point


        dst[t_dst++]=v_array->total;
        cp=v_array->add();

        dx = vj->v.x - vi->v.x;
        dy = vj->v.y - vi->v.y;
        dz = vj->v.z - vi->v.z;
        ds = vj->s - vi->s;
        dt = vj->t - vi->t;
        dr = vj->r - vi->r;
        dg = vj->g - vi->g;
        db = vj->b - vi->b;
        da = vj->a - vi->a;
      
      

        switch (bitmask | (ORCODE&128)) {
          case 1:  
            t = (-vi->v.x + vi->v.z) / ( dx - dz);                  
            cp->v.y = vi->v.y + (t * dy);
            cp->v.z = vi->v.z + (t * dz);                 
            cp->v.x = cp->v.z;
            break;

          case 2: 
            t = ( vi->v.x + vi->v.z) / (-dx - dz);                  
            cp->v.y = vi->v.y + (t * dy);
            cp->v.z = vi->v.z + (t * dz);
                 
            cp->v.x = -cp->v.z;
            break;
        
          case 4:  
            t = (-vi->v.y + vi->v.z) / ( dy - dz);
            cp->v.x = vi->v.x + (t * dx);                 
            cp->v.z = vi->v.z + (t * dz);
                 
            cp->v.y = cp->v.z;
            break;
        
          case 8: 
            t = ( vi->v.y + vi->v.z) / (-dy - dz);
            cp->v.x = vi->v.x + (t * dx);                 
            cp->v.z = vi->v.z + (t * dz);
                 
            cp->v.y = -cp->v.z; 
            break;

          case 16: 
            t = (r1_near_clip_z - vi->v.z) / (dz);		                
            cp->v.x = vi->v.x + (t * dx);
            cp->v.y = vi->v.y + (t * dy);
                 
            cp->v.z = r1_near_clip_z;
            break;

          case 1|128:
            t = (1.0 - vi->v.x) / dx;
            cp->v.y = vi->v.y + (t * dy);
            cp->v.z = vi->v.z + (t * dz);
            cp->v.x = 1.0;
            break;
            
          case 2|128:
            t = (vi->v.x + 1.0) / -dx;
            cp->v.y = vi->v.y + (t * dy);
            cp->v.z = vi->v.z + (t * dz);
                   
            cp->v.x = -1.0;
            break;
        
          case 4|128:
            t = (1.0 - vi->v.y) / dy;
            cp->v.x = vi->v.x + (t * dx);                 
            cp->v.z = vi->v.z + (t * dz);
                   
            cp->v.y = 1.0;
            break;
        
          case 8|128:
            t = (vi->v.y + 1.0) / -dy;
            cp->v.x = vi->v.x + (t * dx);                 
            cp->v.z = vi->v.z + (t * dz);
                  
            cp->v.y = -1.0; 
            break;
        }
      
        cp->s = vi->s + (t * ds);
        cp->t = vi->t + (t * dt);
        cp->r = vi->r + (t * dr);
        cp->g = vi->g + (t * dg);
        cp->b = vi->b + (t * db);
        cp->a = vi->a + (t * da);
      
        // no far clip
        cp->outcode = r1_calc_outcode(cp);

        if (!cp->outcode)
        {
          ooz = r1_ooz(cp->v.z);
          cp->px = ((cp->v.x * ooz) * center_x) + center_x;
          cp->py = ((cp->v.y * ooz) * center_y) + center_y;          
          cp->w  = ooz;
        }  
        ORCODE |= cp->outcode;      
      }
    
      if (dst==dst1)
      {
        dst=dst2;
        src=dst1;
      }
      else
      {
        dst=dst1;
        src=dst2;
      }
      
      t_src=t_dst;
    }
  }
  return src;
}


void r1_clip_render_lines(int t_lines, r1_vert *verts, 
                          float center_x, float center_y,
                          r1_render_api_class *api)
{
  r1_vert v[2];

  w8 bitmask;

  i4_float dz,dy,dx,dr,dg,db,t;

  sw32 i;

  for (i=0; i<t_lines+1; i++)
    verts[i].outcode   = r1_calc_outcode(&verts[i]);

  for (i=0;i<t_lines;i++)
  {    
    //both outside? continue
    
    if (verts[i].outcode & verts[i+1].outcode) continue;
    
    v[0] = verts[i];
    v[1] = verts[i+1];

    w8 outcode = v[0].outcode | v[1].outcode;

    if (!outcode)
    {
      api->render_lines(1,v);
      continue;
    }

    //gotta clip
    for (bitmask=32; bitmask && (v[0].outcode & v[1].outcode)==0; bitmask = bitmask >> 1)
    {      
      if (bitmask & v[0].outcode)
      {     
        r1_vert clip_point;

        dx = v[1].v.x - v[0].v.x;
        dy = v[1].v.y - v[0].v.y;
        dz = v[1].v.z - v[0].v.z;
        dr = v[1].r - v[0].r;
        dg = v[1].g - v[0].g;
        db = v[1].b - v[0].b;
  
        switch (bitmask | (v[0].outcode&128)) 
        {
          case 1:
            t = (-v[0].v.x + v[0].v.z) / ( dx - dz);                  
            clip_point.v.y = v[0].v.y + (t * dy);
            clip_point.v.z = v[0].v.z + (t * dz);
                   
            clip_point.v.x = clip_point.v.z;
            break;

          case 2:
            t = ( v[0].v.x + v[0].v.z) / (-dx - dz);                  
            clip_point.v.y = v[0].v.y + (t * dy);
            clip_point.v.z = v[0].v.z + (t * dz);
                   
            clip_point.v.x = -clip_point.v.z;
            break;
        
          case 4:
            t = (-v[0].v.y + v[0].v.z) / ( dy - dz);
            clip_point.v.x = v[0].v.x + (t * dx);                 
            clip_point.v.z = v[0].v.z + (t * dz);
                   
            clip_point.v.y = clip_point.v.z;
            break;
        
          case 8:
            t = ( v[0].v.y + v[0].v.z) / (-dy - dz);
            clip_point.v.x = v[0].v.x + (t * dx);                 
            clip_point.v.z = v[0].v.z + (t * dz);
                  
            clip_point.v.y = -clip_point.v.z; 
            break;

          case 16: 
            t = (r1_near_clip_z - v[0].v.z) / (dz);		                
            clip_point.v.x = v[0].v.x + (t * dx);
            clip_point.v.y = v[0].v.y + (t * dy);
                 
            clip_point.v.z = r1_near_clip_z;
            break;


          case 32: 
            t = -(r1_far_clip_z - v[0].v.z) / (dz);		                
            clip_point.v.x = v[0].v.x + (t * dx);
            clip_point.v.y = v[0].v.y + (t * dy);
                 
            clip_point.v.z = r1_far_clip_z;
            break;

          case 1|128:
            t = (1.0 - v[0].v.x) / dx;
            clip_point.v.y = v[0].v.y + (t * dy);
            clip_point.v.z = v[0].v.z + (t * dz);
            clip_point.v.x = 1.0;
            break;
            
          case 2|128:
            t = (1.0 + v[0].v.x) / -dx;
            clip_point.v.y = v[0].v.y + (t * dy);
            clip_point.v.z = v[0].v.z + (t * dz);
            clip_point.v.x = -1.0;
            break;
        
          case 4|128:
            t = (1.0 - v[0].v.y) / dy;
            clip_point.v.x = v[0].v.x + (t * dx);                 
            clip_point.v.z = v[0].v.z + (t * dz);
                   
            clip_point.v.y = 1.0;
            break;
        
          case 8|128:
            t = ( 1.0 + v[0].v.y) / -dy;
            clip_point.v.x = v[0].v.x + (t * dx);                 
            clip_point.v.z = v[0].v.z + (t * dz);
            clip_point.v.y = -1.0; 
            break;
        }
        clip_point.r = v[0].r + (t * dr);
        clip_point.g = v[0].g + (t * dg);
        clip_point.b = v[0].b + (t * db);        
                
        clip_point.outcode = r1_calc_outcode(&clip_point);

        if (!clip_point.outcode)
        {
          i4_float ooz  = r1_ooz(clip_point.v.z);

          clip_point.px = ((clip_point.v.x * ooz) * center_x) + center_x;
          clip_point.py = ((clip_point.v.y * ooz) * center_y) + center_y;
          clip_point.w  = ooz;
        }
        
        v[0] = clip_point;
      }
      
      if (bitmask & v[1].outcode)
      {     
        r1_vert clip_point;

        clip_point = v[0];
        v[0]       = v[1];
        v[1]       = clip_point;

        dx = v[1].v.x - v[0].v.x;
        dy = v[1].v.y - v[0].v.y;
        dz = v[1].v.z - v[0].v.z;
        dr = v[1].r - v[0].r;
        dg = v[1].g - v[0].g;
        db = v[1].b - v[0].b;
  
        switch (bitmask | (v[1].outcode&128)) 
        {
          case 1: 
            t = (-v[0].v.x + v[0].v.z) / ( dx - dz);                  
            clip_point.v.y = v[0].v.y + (t * dy);
            clip_point.v.z = v[0].v.z + (t * dz);
            
            clip_point.v.x = clip_point.v.z;
            break;

          case 2: 
            t = ( v[0].v.x + v[0].v.z) / (-dx - dz);                  
            clip_point.v.y = v[0].v.y + (t * dy);
            clip_point.v.z = v[0].v.z + (t * dz);
            
            clip_point.v.x = -clip_point.v.z;
            break;
            
          case 4: 
            t = (-v[0].v.y + v[0].v.z) / ( dy - dz);
            clip_point.v.x = v[0].v.x + (t * dx);                 
            clip_point.v.z = v[0].v.z + (t * dz);
            
            clip_point.v.y = clip_point.v.z;
            break;
            
          case 8:
            t = ( v[0].v.y + v[0].v.z) / (-dy - dz);
            clip_point.v.x = v[0].v.x + (t * dx);                 
            clip_point.v.z = v[0].v.z + (t * dz);
            
            clip_point.v.y = -clip_point.v.z; 
            break;
            
          case 16:
            t = (r1_near_clip_z - v[0].v.z) / (dz);		                
            clip_point.v.x = v[0].v.x + (t * dx);
            clip_point.v.y = v[0].v.y + (t * dy);
            
            clip_point.v.z = r1_near_clip_z;
            break;

          case 32:
            t = -(r1_far_clip_z - v[0].v.z) / (dz);		                
            clip_point.v.x = v[0].v.x + (t * dx);
            clip_point.v.y = v[0].v.y + (t * dy);
            
            clip_point.v.z = r1_far_clip_z;
            break;

          case 1|128:
            t = (1.0 - v[0].v.x) / dx;
            clip_point.v.y = v[0].v.y + (t * dy);
            clip_point.v.z = v[0].v.z + (t * dz);
            clip_point.v.x = 1.0;
            break;
            
          case 2|128:
            t = (1.0 + v[0].v.x) / -dx;
            clip_point.v.y = v[0].v.y + (t * dy);
            clip_point.v.z = v[0].v.z + (t * dz);
            clip_point.v.x = -1.0;
            break;
        
          case 4|128:
            t = (1.0 - v[0].v.y) / dy;
            clip_point.v.x = v[0].v.x + (t * dx);                 
            clip_point.v.z = v[0].v.z + (t * dz);
                   
            clip_point.v.y = 1.0;
            break;
        
          case 8|128:
            t = ( 1.0 + v[0].v.y) / -dy;
            clip_point.v.x = v[0].v.x + (t * dx);                 
            clip_point.v.z = v[0].v.z + (t * dz);
            clip_point.v.y = -1.0; 
            break;
        }
        clip_point.r = v[0].r + (t * dr);
        clip_point.g = v[0].g + (t * dg);
        clip_point.b = v[0].b + (t * db);        
                
        clip_point.outcode = r1_calc_outcode(&clip_point);

        if (!clip_point.outcode)
        {
          i4_float ooz = r1_ooz(clip_point.v.z);
          clip_point.px = ((clip_point.v.x * ooz) * center_x) + center_x;
          clip_point.py = ((clip_point.v.y * ooz) * center_y) + center_y;          
          clip_point.w  = ooz;
        }
        
        v[0] = v[1];
        v[1] = clip_point;
      }      
    }

    if (!v[0].outcode && !v[1].outcode)
      api->render_lines(1,v);
  }
}


void r1_clip_clear_area(int x1, int y1, int x2, int y2, w32 color, float z, 
                        i4_draw_context_class &context,
                        r1_render_api_class *api)
{
  api->set_alpha_mode(R1_ALPHA_DISABLED);
  api->set_constant_color(color);
  api->disable_texture();

  for (i4_rect_list_class::area_iter c=context.clip.list.begin();c!=context.clip.list.end();++c)
  {
    int lx1,ly1,lx2,ly2;

    if (x1<c->x1) lx1=c->x1; else lx1=x1;
    if (y1<c->y1) ly1=c->y1; else ly1=y1;
    if (x2>c->x2) lx2=c->x2; else lx2=x2;
    if (y2>c->y2) ly2=c->y2; else ly2=y2;
    if (!(lx1>lx2 || ly1>ly2))
      api->clear_area(lx1, ly1, lx2, ly2, color, z);
  } 
}


void r1_clip_render_textured_rect(float x1, float y1, float x2, float y2, float z,  float a,
                                  int win_width, int win_height,
                                  r1_texture_handle handle,
                                  int frame,
                                  r1_render_api_class *api,
                                  float s1, float t1,
                                  float s2, float t2)
{
  float ooz=r1_ooz(z);

  api->use_texture(handle, i4_f_to_i( x2-x1+1), frame);

  r1_vert v[4];
  
  // should be counter-clockwise
  v[0].px  = x2;
  v[0].py  = y1;
  v[0].v.z = z;
  v[0].w   = ooz;
  v[0].s   = s2;
  v[0].t   = t1;
      
  v[1].px  = x1;
  v[1].py  = y1;
  v[1].v.z = z;
  v[1].w   = ooz;      
  v[1].s   = s1;
  v[1].t   = t1;

  v[2].px  = x1;
  v[2].py  = y2;
  v[2].v.z = z;
  v[2].w   = ooz;      
  v[2].s   = s1;
  v[2].t   = t2;

  v[3].px  = x2;
  v[3].py  = y2;
  v[3].v.z = z;
  v[3].w   = ooz;      
  v[3].s   = s2;
  v[3].t   = t2;

  v[0].r=v[1].r=v[2].r=v[3].r = 1;
  v[0].g=v[1].g=v[2].g=v[3].g = 1;
  v[0].b=v[1].b=v[2].b=v[3].b = 1;

  v[0].a=v[1].a=v[2].a=v[3].a = a;
                                  

  int clip_x1=0, clip_y1=0, clip_x2=win_width, clip_y2=win_height;

  //left x check
  if (v[1].px < clip_x1)
  {
    i4_float diff = (clip_x1 - v[1].px) / (i4_float)(x2-x1+1);
    v[1].s = s1+(s2-s1)*diff;
    v[2].s = s1+(s2-s1)*diff;
    v[1].px = clip_x1;
    v[2].px = clip_x1;
  }
  else if (v[1].px > clip_x2)
    return;

  //right x check
  if (v[0].px > clip_x2)
  {
    i4_float diff = (v[0].px - clip_x2) / (i4_float)(x2-x1+1);
    v[0].s = s2 - diff*(s2-s1);        
    v[3].s = s2 - diff*(s2-s1);
    v[0].px = clip_x2;
    v[3].px = clip_x2;
  }
  else if (v[0].px < clip_x1)
    return;


  //top y check
  if (v[0].py < clip_y1)
  {
    i4_float diff = (clip_y1 - v[0].py) / (i4_float)(y2-y1+1);
    v[0].t  = t1+(t2-t1)*diff;
    v[1].t  = t1+(t2-t1)*diff;
    v[0].py = clip_y1;
    v[1].py = clip_y1;
  }
  else if (v[0].py > clip_y2)
    return;

  //bottom y check
  if (v[2].py > clip_y2)
  {
    i4_float diff = (v[2].py - clip_y2) / (i4_float)(y2-y1+1);
    v[2].t  = t2-diff*(t2-t1);
    v[3].t  = t2-diff*(t2-t1);        
    v[2].py = clip_y2;
    v[3].py = clip_y2;
  }
  else if (v[2].py < clip_y1)
    return;

  api->render_sprite(v);
}


