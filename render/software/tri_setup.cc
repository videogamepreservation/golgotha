/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "arch.hh"
#include "software/r1_software.hh"
#include "software/r1_software_globals.hh"
#include "software/span_buffer.hh"
#include "software/inline_fpu.hh"
#include "software/mappers.hh"

//all calls to qftoi() here assume it will truncate, so be sure that start_trunc() gets
//called before these functions execute

inline void standard_draw_tri(s_vert *v0,s_vert *v1, s_vert *v2, w8 poly_type)
{
  if (!tri_draw_functions[poly_type]) return;
  if (!texture_scanline_functions[poly_type]) return;

  //find top, middle, and bottom verts    
  s_vert *min_y=0,*mid_y=0,*max_y=0;

  sw32 midedgecompare,botedgecompare;  

  //sort the vertices by increasing y. special case for 3 verts
  //is faster than a bubble sort or qsort, etc
  if (v0->iy < v1->iy)
  {
    if (v0->iy < v2->iy)
    {
      min_y = v0;
      
      if (v1->iy < v2->iy)
      {
        mid_y = v1;
        max_y = v2;
        midedgecompare=1;
        botedgecompare=2;
      }
      else
      {
        mid_y = v2;
        max_y = v1;
        midedgecompare=2;
        botedgecompare=1;
      }
    }
    else
    {
      min_y = v2;
      mid_y = v0;
      max_y = v1;
      midedgecompare=0;
      botedgecompare=1;
    }
  }
  else
  {
    if (v1->iy < v2->iy)
    {
      min_y = v1;

      if (v0->iy < v2->iy)
      {
        mid_y = v0;
        max_y = v2;
        midedgecompare=3;
        botedgecompare=2;
      }
      else
      {
        mid_y = v2;
        max_y = v0;
        midedgecompare=2;
        botedgecompare=3;
      }
    }
    else
    {
      min_y = v2;
      mid_y = v1;
      max_y = v0;
      midedgecompare=1;
      botedgecompare=0;
    }
  }
  
  //setup the edges
  tri_edge top_to_middle,
           top_to_bottom,
           mid_to_bottom;
     
  top_to_middle.dy = mid_y->iy - min_y->iy;
  if (top_to_middle.dy)
  {
    double dxdy              = (mid_y->px - min_y->px) / (mid_y->py - min_y->py);
    
    top_to_middle.dxdy       = dxdy;
    top_to_middle.dxdy_fixed = qftoi(dxdy * 65536.f);
    
    //starting point x for when we begin rasterizing
    top_to_middle.px         = qftoi( (min_y->px + ((float)min_y->iy - min_y->py)*dxdy) * 65536.f );
  }
  
  top_to_bottom.dy = max_y->iy - min_y->iy;
  if (top_to_bottom.dy)
  {
    double dxdy              = (max_y->px - min_y->px) / (max_y->py - min_y->py);
    
    top_to_bottom.dxdy       = dxdy;
    top_to_bottom.dxdy_fixed = qftoi(dxdy * 65536.f);
    
    //starting point x for when we begin rasterizing
    top_to_bottom.px         = qftoi( (min_y->px + ((float)min_y->iy - min_y->py)*dxdy) * 65536.f );
  }
  
  mid_to_bottom.dy = max_y->iy - mid_y->iy;
  if (mid_to_bottom.dy)
  {
    double dxdy              = (max_y->px - mid_y->px) / (max_y->py - mid_y->py);
    
    mid_to_bottom.dxdy       = dxdy;
    mid_to_bottom.dxdy_fixed = qftoi(dxdy * 65536.f);
    
    //starting point x for when we begin rasterizing
    mid_to_bottom.px         = qftoi( (mid_y->px + ((float)mid_y->iy - mid_y->py)*dxdy) * 65536.f );
  }
    
  //set cur_scanline_texture_func to the appropriate type
  cur_scanline_texture_func = texture_scanline_functions[poly_type];

  //set the fpu in single precision 1st

  start_single();

  //call the appropriate rasterizer
  tri_draw_functions[poly_type](top_to_middle,top_to_bottom,mid_to_bottom,min_y->iy,(botedgecompare < midedgecompare));

  stop_single();
}

inline void setup_spans_for_poly(s_vert *v, sw32 t_verts, span_tri_info *new_tri, i4_bool reverse=0)
{
  new_tri->color_tint = r1_software_class_instance.cur_color_tint;

  tri_gradients &grads = new_tri->grads;

  sw32 i,j,dy;
  sw32 one;

  //reverse is used to handle backfacing polys
  if (reverse)
    one = -1;
  else
    one = 1;

  for (i=0; i<t_verts; i++)
  {
    j = i + one;
    
    if (j==t_verts)
      j=0;
    else
    if (j==-1)
      j=t_verts-1;
        
    sw32 &min_y = v[i].iy;
    sw32 &max_y = v[j].iy;
    
    dy = max_y - min_y;
    if (dy==0) continue; //edge is perfectly horizontal? ignore it

    //allocate a new edge
    span_edge *new_edge = new_span_edge();
    if (!new_edge) return;    

    if (dy<0)
    {
      //its a trailing edge (dy and min_y / max_y should be swapped/negated)

      new_edge->tri_1 = new_tri;
      new_edge->flags = 0; //0 means its trailing
                
      double dxdy    = (v[i].px - v[j].px) / (v[i].py - v[j].py);
      new_edge->dxdy = qftoi(65536.f * dxdy);
      new_edge->x    = qftoi( (v[j].px + ((float)v[j].iy - v[j].py)*dxdy) * 65536.f );
      
      add_start_edge(new_edge,max_y); //this edge will be added to the active edge table when processing
                                      //scanline min_y

      add_remove_edge(new_edge,min_y-1); //this edge will be removed from the active edge table when done
                                         //processing scanline max_y-1
    }
    else
    {
      //its a leading edge
      
      new_edge->tri_1 = new_tri;
      new_edge->flags = LEADING_1; //LEADING_1 means leading, obviously
                
      double dxdy    = (v[j].px - v[i].px) / (v[j].py - v[i].py);
      new_edge->dxdy = qftoi(65536.f * dxdy);
      new_edge->x    = qftoi( (v[i].px + ((float)v[i].iy - v[i].py)*dxdy) * 65536.f );
      
      add_start_edge(new_edge,min_y); //this edge will be added to the active edge table when processing
                                      //scanline min_y

      add_remove_edge(new_edge,max_y-1); //this edge will be removed from the active edge table when done
                                         //processing scanline max_y-1
    }
  }
}

void sprite_setup_affine_unlit_alpha(s_vert *v,sw32 t_verts)
{
  if (t_verts != 4)
  {
    i4_warning("software::probably shouldnt be calling sprite_setup_affine_unlit_alpha w/out 4 vertices");
  }

  //just draw a sprite. calculate some necessary info first, though

  //alias the vertex list
  s_vert  *v0 = &v[0],
          *v1 = &v[1],
          *v2 = &v[2];
  
  tri_area_struct *t = triangle_info; //areas for each tri have already been calculated and stored here
  
  i4_bool reverse; //used to determine if the sprite is backfacing

  float &dx1x0 = t->dx1x0;
  float &dx2x0 = t->dx2x0;

  float &dy1y0 = t->dy1y0;
  float &dy2y0 = t->dy2y0;

  float &denom_gradx = t->area;

  if (denom_gradx < 5)
  {
    if (denom_gradx > -5 || !r1_software_class_instance.allow_backfaces)
    {
      return;
    }
    reverse = i4_T;
  }
  else
    reverse = i4_F;

  //setup an alias
  tri_gradients *grads;

  //setup new spanned triangle
  span_tri_info *new_tri = 0;

  if (r1_software_class_instance.use_spans)
  {
    new_tri = new_span_tri();      
    if (!new_tri) return;      
    grads = &new_tri->grads;
  }
  else
    grads = &cur_grads;

  //calculate these "magic" multipliers (based on triangle area, obviously)
  double oodgx = 1.0 / (double)denom_gradx;
  double oodgy = -oodgx;
  
  //sprites dont have ooz deltas, their ooz is constant. duh. clear them.
  grads->doozdx  = 0;
  grads->doozdy  = 0;
  grads->oozat00 = v0->ooz;
    
  //scale these multipliers so our s and t gradients are pre-shifted when we
  //store them to integer registers (ie they'll already be 16:16 fixed)
  oodgx *= 65536.0;
  oodgy *= 65536.0;

  //calculate s and t deltas
  double ds1s0 = (v1->s - v0->s);
  double ds2s0 = (v2->s - v0->s);

  double dt1t0 = (v1->t - v0->t);
  double dt2t0 = (v2->t - v0->t);
  
  //calculate s gradient for affine tri
  grads->dsdx  = (((ds2s0)*(dy1y0) - (ds1s0)*(dy2y0)) * oodgx);
  grads->dsdy  = (((ds2s0)*(dx1x0) - (ds1s0)*(dx2x0)) * oodgy);
  grads->sat00 = v0->s*65536.0 - (v0->px * grads->dsdx) - (v0->py * grads->dsdy);
    
  //determine s adjustment (needed to ensure we dont step outside texture boundary)
  grads->s_adjust = 0;
    
  if (grads->dsdx < 0)
    grads->s_adjust = -(sw32)1;
  else
    if (grads->dsdy < 0)
      grads->s_adjust = -(sw32)1;
    
  //calculate t gradient for affine tri
  grads->dtdx  = (((dt2t0)*(dy1y0) - (dt1t0)*(dy2y0)) * oodgx);
  grads->dtdy  = (((dt2t0)*(dx1x0) - (dt1t0)*(dx2x0)) * oodgy);
  grads->tat00 = v0->t*65536.0 - (v0->px * grads->dtdx) - (v0->py * grads->dtdy);     

  //determine t adjustment
  grads->t_adjust = 0;

  if (grads->dtdx < 0)
    grads->t_adjust = -(sw32)1;
  else
    if (grads->dtdy < 0)
      grads->t_adjust = -(sw32)1;
    
  if (r1_software_class_instance.use_spans)
  {
    //add it to spanlist if we're using spans
    //set up the remaining span tri information
      
    new_tri->texture        = r1_software_texture_ptr;
    new_tri->twidth_log2    = r1_software_twidth_log2;
    new_tri->texture_width  = r1_software_texture_width;
    new_tri->texture_height = r1_software_texture_height;
    new_tri->span_list_head = 0;    
    new_tri->type           = small_poly_type;
      
    setup_spans_for_poly(v,4,new_tri,reverse);
  }
  else
  {
    //no spans, gonna draw immediately, setup pertinent global vars (s_frac_add, t_frac_add, and s_t_carry)
    
    //about to draw, setup pertinent global vars (temp_dsdx, temp_dtdx, dsdx_frac, dtdx_frac, s_t_carry)
    temp_dsdx = qftoi(cur_grads.dsdx);
    temp_dtdx = qftoi(cur_grads.dtdx);

    dsdx_frac = (temp_dsdx<<16);
    dtdx_frac = (temp_dtdx<<16);
    
    s_t_carry[1] = (temp_dsdx>>16) + ((temp_dtdx>>16)<<r1_software_twidth_log2); //integral add when t doesnt carry
    s_t_carry[0] = s_t_carry[1] + r1_software_texture_width;           //integral add when t carrys

    //draw the sprite as 2 triangles (blech)
    standard_draw_tri(v0,v1,v2,small_poly_type);
    v1++; v2++;
    standard_draw_tri(v0,v1,v2,small_poly_type);
  }
}

void poly_setup_affine_lit(s_vert *v,sw32 t_verts)
{
  //step through the vertices and extract the tris for drawing (or span buffering)

  i4_bool do_light = (small_poly_type==SPAN_TRI_AFFINE_LIT);

  //alias the vertex list
  s_vert  *v0 = &v[0],
          *v1 = &v[1],
          *v2 = &v[2];

  //temporary list of verts for each tri,
  //needed for the call to setup_spans_for_poly (which doesnt take tris, but rather polys)
  s_vert span_verts[3];
  span_verts[0] = *v0;
  
  tri_area_struct *t = triangle_info; //areas for each tri have already been calculated and stored here
  
  i4_bool reverse; //used to determine if the tri is backfacing
  sw32    i;

  for (i=1; i<t_verts-1; i++,t++)
  {      
    float &dx1x0 = t->dx1x0;
    float &dx2x0 = t->dx2x0;

    float &dy1y0 = t->dy1y0;
    float &dy2y0 = t->dy2y0;

    float &denom_gradx = t->area;

    i4_bool supersmall = i4_F;

    if (denom_gradx > 0)
    {
      if (denom_gradx < 5)
        supersmall = i4_T;
      
      reverse = i4_F;
    }
    else
    if (denom_gradx < 0 && r1_software_class_instance.allow_backfaces)
    {
      if (denom_gradx > -5)
        supersmall = i4_T;

      reverse = i4_T;
    }
    else
    {
      v1++;
      v2++;
      continue;
    }

    //setup an alias
    tri_gradients *grads;

    //setup new spanned triangle
    span_tri_info *new_tri = 0;

    if (r1_software_class_instance.use_spans)
    {
      new_tri = new_span_tri();      
      if (!new_tri) return;      
      grads = &new_tri->grads;
    }
    else
      grads = &cur_grads;

    if (supersmall)
    {
      //this could be optimized further but.. too complicated. we just want
      //SOMETHING to be drawn, so calculate the ooz gradient so that it sorts
      //correctly, set s t and l gradients to 0

      //calculate these "magic" multipliers (based on triangle area, obviously)
      double oodgx = 1.0 / (double)denom_gradx;
      double oodgy = -oodgx;
  
      //calculate ooz deltas
      double dooz1ooz0 = (v1->ooz - v0->ooz);  
      double dooz2ooz0 = (v2->ooz - v0->ooz); 
  
      //calculate ooz gradient for affine tri (ie no doozdxspan calculated)
      grads->doozdx  = ((dooz2ooz0)*(dy1y0) - (dooz1ooz0)*(dy2y0)) * oodgx;
      grads->doozdy  = ((dooz2ooz0)*(dx1x0) - (dooz1ooz0)*(dx2x0)) * oodgy;
      grads->oozat00 = v0->ooz - (v0->px * grads->doozdx) - (v0->py * grads->doozdy);
    
      //calculate s gradient for affine tri
      grads->dsdx  = 0;
      grads->dsdy  = 0;
      grads->sat00 = v0->s*65536.0;
      grads->s_adjust = 0;
  
      //calculate t gradient for affine tri
      grads->dtdx  = 0;
      grads->dtdy  = 0;
      grads->tat00 = v0->t*65536.0;
      grads->t_adjust = 0;

      if (do_light)
      {
        grads->dldx  = 0;
        grads->dldy  = 0;
        grads->lat00 = v0->l*256.0;
      }
    }
    else
    {
      //calculate these "magic" multipliers (based on triangle area, obviously)
      double oodgx = 1.0 / (double)denom_gradx;
      double oodgy = -oodgx;
  
      //calculate ooz deltas
      double dooz1ooz0 = (v1->ooz - v0->ooz);  
      double dooz2ooz0 = (v2->ooz - v0->ooz); 
  
      //calculate ooz gradient for affine tri (ie no doozdxspan calculated)
      grads->doozdx  = ((dooz2ooz0)*(dy1y0) - (dooz1ooz0)*(dy2y0)) * oodgx;
      grads->doozdy  = ((dooz2ooz0)*(dx1x0) - (dooz1ooz0)*(dx2x0)) * oodgy;
      grads->oozat00 = v0->ooz - (v0->px * grads->doozdx) - (v0->py * grads->doozdy);
    
      //calculate s and t deltas
      double ds1s0 = (v1->s - v0->s);
      double ds2s0 = (v2->s - v0->s);

      double dt1t0 = (v1->t - v0->t);
      double dt2t0 = (v2->t - v0->t);
    
      //scale these multipliers so our s and t gradients are pre-shifted when we
      //store them to integer registers (ie they'll already be 16:16 fixed)
      oodgx *= 65536.0;
      oodgy *= 65536.0;

      //calculate s gradient for affine tri
      grads->dsdx  = (((ds2s0)*(dy1y0) - (ds1s0)*(dy2y0)) * oodgx);
      grads->dsdy  = (((ds2s0)*(dx1x0) - (ds1s0)*(dx2x0)) * oodgy);
      grads->sat00 = v0->s*65536.0 - (v0->px * grads->dsdx) - (v0->py * grads->dsdy);
    
      //determine s adjustment (needed to ensure we dont step outside texture boundary)
      grads->s_adjust = 0;
    
      if (grads->dsdx < 0)
        grads->s_adjust = -(sw32)1;
      else
        if (grads->dsdy < 0)
          grads->s_adjust = -(sw32)1;
    
      //calculate t gradient for affine tri
      grads->dtdx     = (((dt2t0)*(dy1y0) - (dt1t0)*(dy2y0)) * oodgx);
      grads->dtdy     = (((dt2t0)*(dx1x0) - (dt1t0)*(dx2x0)) * oodgy);
      grads->tat00    = v0->t*65536.0 - (v0->px * grads->dtdx) - (v0->py * grads->dtdy);     

      //determine t adjustment
      grads->t_adjust = 0;

      if (grads->dtdx < 0)
        grads->t_adjust = -(sw32)1;
      else
        if (grads->dtdy < 0)
          grads->t_adjust = -(sw32)1;
    
      if (do_light)
      {
        //scale the multipliers so the l gradient is preshifted to
        //fixed point 28:4
        oodgx *= (1.0/256.0);
        oodgy *= (1.0/256.0);

        //l gradient calculation
        double dl1l0 = (v1->l - v0->l);
        double dl2l0 = (v2->l - v0->l);

        grads->dldx  = (((dl2l0)*(dy1y0) - (dl1l0)*(dy2y0)) * oodgx);
        grads->dldy  = (((dl2l0)*(dx1x0) - (dl1l0)*(dx2x0)) * oodgy);
        grads->lat00 = ((v0->l*256.0) - (v0->px * grads->dldx) - (v0->py * grads->dldy));
      }
    }

    if (r1_software_class_instance.use_spans)
    {
      //add it to spanlist if we're using spans
      //set up the remaining span tri information
      
      new_tri->texture        = r1_software_texture_ptr;
      new_tri->twidth_log2    = r1_software_twidth_log2;
      new_tri->texture_width  = r1_software_texture_width;
      new_tri->texture_height = r1_software_texture_height;
      new_tri->span_list_head = 0;    
      new_tri->type           = small_poly_type;
      
      //copy the current 2 verts into the span_vert list
      span_verts[1] = *v1;
      span_verts[2] = *v2;
      setup_spans_for_poly(span_verts,3,new_tri,reverse);
    }
    else
    {
      //about to draw, setup pertinent global vars (temp_dsdx, temp_dtdx, dsdx_frac, dtdx_frac, s_t_carry)
      temp_dsdx = qftoi(cur_grads.dsdx);
      temp_dtdx = qftoi(cur_grads.dtdx);

      dsdx_frac = (temp_dsdx<<16);
      dtdx_frac = (temp_dtdx<<16);
    
      s_t_carry[1] = (temp_dsdx>>16) + ((temp_dtdx>>16)<<r1_software_twidth_log2); //integral add when t doesnt carry
      s_t_carry[0] = s_t_carry[1] + r1_software_texture_width;           //integral add when t carrys

      if (do_light)
        dldx_fixed = qftoi(cur_grads.dldx);

      standard_draw_tri(v0,v1,v2,small_poly_type);
    }

    //advance the vertex pointers
    v1++;
    v2++;
  }
}

void poly_setup_perspective_lit(s_vert *v,sw32 t_verts)
{
  i4_bool do_light = (big_poly_type==SPAN_TRI_PERSPECTIVE_LIT);

  i4_bool reverse;
  s_vert *v0,*v1,*v2;
  v0 = &v[0];
  v1 = &v[1];
  v2 = &v[2];

  sw32 i;
  s_vert v_2[3];

  if (!v0->st_projected)
  {
    v0->soz = v0->s * v0->ooz;
    v0->toz = v0->t * v0->ooz;
    v0->st_projected = i4_T;
  }

  v_2[0] = *v0;
    
  tri_area_struct *t = triangle_info;

  //by default this will be a "big" polygon (thats why the perspective
  //setup was called in the 1st place)
  //BUT if one of the triangles is super tiny (<5 pixels), it will be drawn affine
  //this_poly_type will override big_poly_type

  for (i=1; i<t_verts-1; i++,t++)
  {      
    float &dx1x0 = t->dx1x0;
    float &dx2x0 = t->dx2x0;

    float &dy1y0 = t->dy1y0;
    float &dy2y0 = t->dy2y0;

    float &denom_gradx = t->area;
     
    i4_bool supersmall = i4_F;

    if (denom_gradx > 0)
    {
      if (denom_gradx < 5)
        supersmall = i4_T;
      
      reverse = i4_F;
    }
    else
    if (denom_gradx < 0 && r1_software_class_instance.allow_backfaces)
    {
      if (denom_gradx > -5)
        supersmall = i4_T;

      reverse = i4_T;
    }
    else
    {
      v1++;
      v2++;
      continue;
    }
    
    //setup new spanned triangle
    span_tri_info *new_tri=0;
    
    //setup an alias
    tri_gradients *grads;

    if (r1_software_class_instance.use_spans)
    {
      new_tri = new_span_tri();
      if (!new_tri) return;
      grads = &new_tri->grads;
    }
    else
      grads = &cur_grads;
    
    if (supersmall)    
    {
      double oodgx = 1.0 / (double) denom_gradx;
      double oodgy = -oodgx;
  
      double dooz1ooz0 = (v1->ooz - v0->ooz);  
      double dooz2ooz0 = (v2->ooz - v0->ooz); 
  
      grads->doozdx  = ((dooz2ooz0)*(dy1y0) - (dooz1ooz0)*(dy2y0)) * oodgx;
      grads->doozdy  = ((dooz2ooz0)*(dx1x0) - (dooz1ooz0)*(dx2x0)) * oodgy;
      grads->oozat00 = (double)v0->ooz - ((double)v0->px * grads->doozdx) - ((double)v0->py * grads->doozdy);

      //soz gradient calculation
      grads->dsdx     = 0;
      grads->dsdy     = 0;
      grads->sat00    = v0->s*65536.0;
      grads->s_adjust = 0;

      //toz gradient calculation
      grads->dtdx     = 0;
      grads->dtdy     = 0;
      grads->tat00    = v0->t*65536.0;
      grads->t_adjust = 0;

      if (do_light)
      {
        //do light gradient setup
        grads->dldx  = 0;
        grads->dldy  = 0;
        grads->lat00 = v0->l*256.0;
      }
    }
    else
    {
      double oodgx = 1.0 / (double) denom_gradx;
      double oodgy = -oodgx;
  
      double dooz1ooz0 = (v1->ooz - v0->ooz);  
      double dooz2ooz0 = (v2->ooz - v0->ooz); 
  
      grads->doozdx     = ((dooz2ooz0)*(dy1y0) - (dooz1ooz0)*(dy2y0)) * oodgx;  
      grads->doozdxspan = grads->doozdx * 16;  
      grads->doozdy     = ((dooz2ooz0)*(dx1x0) - (dooz1ooz0)*(dx2x0)) * oodgy;
      grads->oozat00    = (double)v0->ooz - ((double)v0->px * grads->doozdx) - ((double)v0->py * grads->doozdy);

      if (!v1->st_projected)
      {
        v1->soz = v1->s * v1->ooz;
        v1->toz = v1->t * v1->ooz;
        v1->st_projected = i4_T;
      }

      if (!v2->st_projected)
      {
        v2->soz = v2->s * v2->ooz;
        v2->toz = v2->t * v2->ooz;
        v2->st_projected = i4_T;
      }

      //do the s and t gradient setup
      double dsoz1soz0 = (v1->soz - v0->soz);
      double dsoz2soz0 = (v2->soz - v0->soz);

      double dtoz1toz0 = (v1->toz - v0->toz);
      double dtoz2toz0 = (v2->toz - v0->toz);  
    
      oodgx *= 65536.0;
      oodgy *= 65536.0;

      //soz gradient calculation
      grads->dsozdx     = (((dsoz2soz0)*(dy1y0) - (dsoz1soz0)*(dy2y0)) * oodgx);
      grads->dsozdxspan = grads->dsozdx * 16;
      grads->dsozdy     = (((dsoz2soz0)*(dx1x0) - (dsoz1soz0)*(dx2x0)) * oodgy);
      grads->sozat00    = (v0->soz*65536.0) - (v0->px * grads->dsozdx) - (v0->py * grads->dsozdy);
      grads->s_adjust   = 0;

      //toz gradient calculation
      grads->dtozdx     = (((dtoz2toz0)*(dy1y0) - (dtoz1toz0)*(dy2y0)) * oodgx);
      grads->dtozdxspan = grads->dtozdx * 16;
      grads->dtozdy     = (((dtoz2toz0)*(dx1x0) - (dtoz1toz0)*(dx2x0)) * oodgy);
      grads->tozat00    = (v0->toz*65536.0) - (v0->px * grads->dtozdx) - (v0->py * grads->dtozdy);
      grads->t_adjust   = 0;

      if (do_light)
      {
        //do light gradient setup
        double dl1l0 = (v1->l - v0->l);
        double dl2l0 = (v2->l - v0->l);

        oodgx *= (1.0/256.0);
        oodgy *= (1.0/256.0);

        grads->dldx  = (((dl2l0)*(dy1y0) - (dl1l0)*(dy2y0)) * oodgx);    
        grads->dldy  = (((dl2l0)*(dx1x0) - (dl1l0)*(dx2x0)) * oodgy);
        grads->lat00 = (v0->l*256.0) - (v0->px * grads->dldx) - (v0->py * grads->dldy);
      }
            
      i4_float dsdx_indicator = grads->dsozdx*v0->ooz - v0->soz*grads->doozdx*65536.0;
      if (dsdx_indicator < 0)
        grads->s_adjust = (sw32)-1;
      else
      {
        i4_float dsdy_indicator = grads->dsozdy*v0->ooz - v0->soz*grads->doozdy*65536.0;
        if (dsdy_indicator<0)
          grads->s_adjust = (sw32)-1;
      }

      i4_float dtdx_indicator = grads->dtozdx*v0->ooz - v0->toz*grads->doozdx*65536.0;
      if (dtdx_indicator < 0)
        grads->t_adjust = (sw32)-1;
      else
      {
        i4_float dtdy_indicator = grads->dtozdy*v0->ooz - v0->toz*grads->doozdy*65536.0;
        if (dtdy_indicator<0)
          grads->t_adjust = (sw32)-1;
      }
    }

    if (r1_software_class_instance.use_spans)
    {
      //set up the remaining tri info    
      new_tri->texture        = r1_software_texture_ptr;
      new_tri->twidth_log2    = r1_software_twidth_log2;
      new_tri->texture_width  = r1_software_texture_width;
      new_tri->texture_height = r1_software_texture_height;
      new_tri->span_list_head = 0;
      
      if (supersmall)
        new_tri->type = small_poly_type;
      else
        new_tri->type = big_poly_type;

      v_2[1] = *v1;
      v_2[2] = *v2;
      setup_spans_for_poly(v_2,3,new_tri,reverse);
    }
    else
    {
      //about to draw, setup pertinent global vars (temp_dsdx, temp_dtdx, dsdx_frac, dtdx_frac, s_t_carry)

      if (do_light)
        dldx_fixed = qftoi(cur_grads.dldx);

      if (supersmall)
      {
        //setup for affine rasterization
        temp_dsdx = qftoi(cur_grads.dsdx);
        temp_dtdx = qftoi(cur_grads.dtdx);

        dsdx_frac = (temp_dsdx<<16);
        dtdx_frac = (temp_dtdx<<16);
    
        s_t_carry[1] = (temp_dsdx>>16) + ((temp_dtdx>>16)<<r1_software_twidth_log2); //integral add when t doesnt carry
        s_t_carry[0] = s_t_carry[1] + r1_software_texture_width;           //integral add when t carrys

        standard_draw_tri(v0,v1,v2,small_poly_type);
      }
      else
      {
        //setup for perspective_correct rasterization
        
        s_mask = ((r1_software_texture_width -1)<<16) | 0xFFFF;
        t_mask = ((r1_software_texture_height-1)<<16) | 0xFFFF;
        
        standard_draw_tri(v0,v1,v2,big_poly_type);
      }
    }

    //advance the vertex pointers
    v1++;
    v2++;
  }  
}

void poly_setup_solid_color(s_vert *v, sw32 t_verts)
{  
  i4_bool reverse;
  
  s_vert *v0 = &v[0],
         *v1 = &v[1],
         *v2 = &v[2];

  sw32 i;
  tri_area_struct *t = triangle_info;
        
  if (r1_software_class_instance.use_spans)
  {
    span_tri_info *new_tri = 0;

    tri_area_struct *t = triangle_info;

    float &dx1x0 = t->dx1x0;
    float &dx2x0 = t->dx2x0;

    float &dy1y0 = t->dy1y0;
    float &dy2y0 = t->dy2y0;

    float &denom_gradx = t->area;
    
    if (total_poly_area > 0)
      reverse = i4_F;
    else
    if (total_poly_area < 0 && r1_software_class_instance.allow_backfaces)
      reverse = i4_T;
    else
      return;

    new_tri = new_span_tri();
    if (!new_tri) return;
    
    tri_gradients *grads = &new_tri->grads;

    double oodgx = 1.0 / (double) denom_gradx;
    double oodgy = -oodgx;
  
    double dooz1ooz0 = (v1->ooz - v0->ooz);  
    double dooz2ooz0 = (v2->ooz - v0->ooz); 
  
    //setup the ooz gradient, its all he needs
    grads->doozdx     = ((dooz2ooz0)*(dy1y0) - (dooz1ooz0)*(dy2y0)) * oodgx;  
    grads->doozdy     = ((dooz2ooz0)*(dx1x0) - (dooz1ooz0)*(dx2x0)) * oodgy;
    grads->oozat00    = v0->ooz - (v0->px * grads->doozdx) - (v0->py * grads->doozdy);

    //set his gradients here  
    new_tri->span_list_head = 0;
    new_tri->type           = small_poly_type;
    new_tri->color          = v->color; //use the color from the first vertex
    
    setup_spans_for_poly(v,t_verts,new_tri,reverse);
  }
  else
  {
    //no solid rasterizer yet
    return;

    s_vert v_2[3];
    v_2[0] = *v0;

    for (i=1; i<t_verts-1; i++,t++)
    {      
      float &dx1x0 = t->dx1x0;
      float &dx2x0 = t->dx2x0;

      float &dy1y0 = t->dy1y0;
      float &dy2y0 = t->dy2y0;

      float &denom_gradx = t->area;
     
      if (denom_gradx > 0)
      {
        reverse = i4_F;
      }
      else
      if (denom_gradx < 0 && r1_software_class_instance.allow_backfaces)
      {
        reverse = i4_T;
      }
      else
      {
        v1++;
        v2++;
        continue;
      }
    
      tri_gradients *grads = &cur_grads;
    
      double oodgx = 1.0 / (double) denom_gradx;
      double oodgy = -oodgx;
  
      double dooz1ooz0 = (v1->ooz - v0->ooz);  
      double dooz2ooz0 = (v2->ooz - v0->ooz); 
  
      grads->doozdx     = ((dooz2ooz0)*(dy1y0) - (dooz1ooz0)*(dy2y0)) * oodgx;  
      grads->doozdxspan = grads->doozdx * 16;  
      grads->doozdy     = ((dooz2ooz0)*(dx1x0) - (dooz1ooz0)*(dx2x0)) * oodgy;
      grads->oozat00    = (double)v0->ooz - ((double)v0->px * grads->doozdx) - ((double)v0->py * grads->doozdy);

      standard_draw_tri(v0,v1,v2,small_poly_type);

      //advance the vertex pointers
      v1++;
      v2++;
    }
  }
}
