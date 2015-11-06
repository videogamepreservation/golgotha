/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "arch.hh"
#include "math/num_type.hh"
#include "time/profile.hh"
#include "software/r1_software.hh"
#include "software/r1_software_globals.hh"
#include "software/mappers.hh"
#include "software/span_buffer.hh"
#include "software/inline_fpu.hh"

#include "software/amd3d/amd3d.h"

#ifndef USE_AMD3D

i4_bool amd3d_build_triangle_span_lists() {}

#else

extern span_tri_info tri_stack;
extern span_edge active_list_head;
extern span_edge active_list_tail;

extern span_edge *compare_here;
extern span_edge *temp_edge,*t1;

inline void amd3d_sort_in_new_edges(span_edge *first)
{
  compare_here = &active_list_head;

  while (first)
  {    
    while (first->x > compare_here->next_active->x)
    {      
      compare_here = compare_here->next_active;      
    }        
    
    //store first->next. remember, ->next and ->last_active are a union
    // (to save space)

    temp_edge = first->next;

    first->last_active = compare_here;
    first->next_active = compare_here->next_active;

    compare_here->next_active->last_active = first;
    compare_here->next_active = first;    
    
    compare_here = first;
    
    #ifdef DEBUG
	if (!first->next_active)
	{
	sw32 a=0;
	}
    #endif

	//first = first->next,see comment above
    first = temp_edge;
  }
}

extern span_entry *next_new_span;
extern span_tri_info *cur_stack;
extern span_tri_info *temp_stack;
extern span_tri_info *e_tri;
extern i4_bool everything_transparent;
extern sw32 span_buff_x, span_buff_y,span_length;
extern w16  *cur_scanline;

extern span_edge **new_edge_list;
extern span_edge **remove_edge_list;

extern float fx_fy[2];

extern float e_ooz;
extern float ooz_compare;

extern i4_bool on_top;

inline w32 greater_than_compare(float *arg1, float *arg2)
{
  _asm
  {
    mov edx,dword ptr [arg1]    
    mov edi,dword ptr [arg2]

    movd mm0,dword ptr [edx]
    movd mm1,dword ptr [edi]
    
    pfcmpgt (m0, m1)
    movd eax, mm0
  }
}

inline void amd3d_depth_at_pixel(tri_gradients *grads, float *result)
{
  _asm
  {
    mov eax,dword ptr [result]
    movq mm0, qword ptr [fx_fy]

    mov edi,dword ptr [grads]
    movq mm1, qword ptr [edi]tri_gradients.doozdx

    pfmul (m0, m1)
    movq mm2, dword ptr [edi]tri_gradients.oozat00

    //low  bits of mm0 - fx * grads.doozdx
    //high bits of mm0 - fy * grads.doozdy
    
    //low bits of mm2  - grads.oozat00
    
    pfacc (m0, m1) //m1 can be anything really
    //low bits of mm0 - fx * grads.doozdx + fy * grads.doozdy

    pfadd (m0, m2)
    //low bits of mm0 - grads.oozat00 + fx * grads.doozdx + fy*grads.doozdy
    
    movd dword ptr [eax], mm0
  }
}

inline void fy_float(sw32 y)
{
  _asm
  {
    lea edi, dword ptr [y]
    
    pi2fd (m0, _edi)

    movd  dword ptr [fx_fy+4],mm0
  }
}

inline void fx_float(sw32 x)
{
  _asm
  {
    lea edi, dword ptr [x]
    
    pi2fd (m0, _edi)
    
    movd  dword ptr [fx_fy],mm0
  }
}

i4_bool amd3d_build_triangle_span_lists()
{
  register span_edge *e;

  new_edge_list    = new_edges;
  remove_edge_list = remove_edges;
  cur_scanline     = r1_software_render_buffer_ptr;
  next_new_span    = &global_span_list[num_global_spans];

  for (span_buff_y = 0; span_buff_y < r1_software_render_buffer_height; 
       span_buff_y++,cur_scanline += r1_software_render_buffer_wpl)
  {
    if (*new_edge_list)
    {
      amd3d_sort_in_new_edges(*new_edge_list);
    }
    new_edge_list++;
        
    tri_stack.has_leader         = 0;
    tri_stack.cur_span_start_x   = 0;
    tri_stack.cur_span_start_ooz = -99999.f;
    tri_stack.next_stack = tri_stack.last_stack = &tri_stack;
    
    fy_float(span_buff_y); //fy = (float)y;

    //go through the x sorted active spans for this scanline
    for (e=active_list_head.next_active; e != &active_list_tail; e=e->next_active)
    {      
      //round to nearest x
      span_buff_x  = (e->x+0xFFFF) >> 16;
      
      fx_float(span_buff_x); //fx = (float)x;

      e_tri = e->tri_1;

      if (e->flags & LEADING_1)
      {         
        if ((e_tri->has_leader++)==0)
        {
          //e_ooz = e_tri->grads.oozat00 + (fx*e_tri->grads.doozdx) + (fy*e_tri->grads.doozdy);
          amd3d_depth_at_pixel(&e_tri->grads, &e_ooz);

          //leading edge. stack manipulation stuff
          cur_stack = tri_stack.next_stack;
          
          everything_transparent = i4_T;
          on_top                 = i4_T;
          
          //go down the stack until we find where this guy will go.          
          while (cur_stack != &tri_stack)
          {
            //ooz_compare = e_ooz - (cur_stack->grads.oozat00     +
                                   //cur_stack->grads.doozdx * fx +
                                   //cur_stack->grads.doozdy * fy);

            amd3d_depth_at_pixel(&cur_stack->grads, &ooz_compare);
            
            //ooz_compare = e_ooz - ooz_compare;
            //if (ooz_compare > 0)
            //  break;
            
            if (greater_than_compare(&e_ooz, &ooz_compare))
              break;
            
            if ((*(int *)&e_ooz) == (*(int *)&ooz_compare))
            {
              //edges are of equal depth. compare the gradient.
              //if (e_tri->grads.doozdx > cur_stack->grads.doozdx)
              //  break;

              if (greater_than_compare(&e_tri->grads.doozdx, &cur_stack->grads.doozdx))
                break;
            }
            
            on_top = i4_F;

            //this will tell us if everything on top of (in front of) this surface is see-thru-able
            if (cur_stack->type < SPAN_TRI_SEE_THRU)
              everything_transparent = i4_F;
            
            cur_stack = cur_stack->next_stack;                                                            
          }
          
          e_tri->next_stack = cur_stack;
          e_tri->last_stack = cur_stack->last_stack;          
  
          cur_stack->last_stack->next_stack = e_tri;
          cur_stack->last_stack             = e_tri;
  
          e_tri->cur_span_start_x = span_buff_x;
          *(int *)&e_tri->cur_span_start_ooz = *(int *)&e_ooz; //for some reason the compiler did this with fld / fst
   
          //if this is a solid span, and everything above it is transparent, gotta draw whatever is below it
          //if this is a solid span, and its on top, gotta draw whatever is below it
          
          if (e_tri->type<SPAN_TRI_SEE_THRU && (on_top || everything_transparent))
          {
            cur_stack = e_tri->next_stack;

            while (cur_stack != &tri_stack)
            {
              //we've gotta create spans for everything below this in the stack.
              span_length = span_buff_x - cur_stack->cur_span_start_x;
              if (span_length>0)
              {
                //make a span
                next_new_span->s.x            = cur_stack->cur_span_start_x;
                next_new_span->s.y            = span_buff_y;
                next_new_span->s.width        = span_length;
                next_new_span->s.ooz          = cur_stack->cur_span_start_ooz;
                next_new_span->s.scanline_ptr = cur_scanline;
                
                //add to its triangles list of spans
                next_new_span->s.next_tri_span = cur_stack->span_list_head;
                cur_stack->span_list_head = num_global_spans;
                
                num_global_spans++;
                next_new_span++;
                if (num_global_spans>=MAX_SPANS) return i4_F;
              }
              if (cur_stack->type<SPAN_TRI_SEE_THRU) break;
              cur_stack = cur_stack->next_stack;
            }
          }
        }
      }
      else //is a trailer
      if ((e_tri->has_leader--)==1)
      {
        //trailing edge
        //is this the end of a span for the tri currently on top of the stack?
        if (tri_stack.next_stack==e_tri)
        {
          cur_stack = e_tri;
          
          span_length = span_buff_x - cur_stack->cur_span_start_x;
          if (span_length>0)
          {
            //make a span            
            next_new_span->s.x            = cur_stack->cur_span_start_x;
            next_new_span->s.y            = span_buff_y;
            next_new_span->s.width        = span_length;
            next_new_span->s.ooz          = cur_stack->cur_span_start_ooz;
            next_new_span->s.scanline_ptr = cur_scanline;
                
            //add to its triangles list of spans
            next_new_span->s.next_tri_span = cur_stack->span_list_head;
            cur_stack->span_list_head = num_global_spans;                
                
            num_global_spans++;
            next_new_span++;
            if (num_global_spans>=MAX_SPANS) return i4_F;
          }
                    
          //setup the start of the next span on the stack
          //IF this one wasnt transparent. otherwise,
          //we want the span start info for that span to be
          //maintained (unaltered)
          if (e_tri->type<SPAN_TRI_SEE_THRU)
          {
            do
            {
              cur_stack = cur_stack->next_stack;
              
              if (cur_stack==&tri_stack)
                break;
              
              cur_stack->cur_span_start_x  = span_buff_x;
              
              //calculate the ooz value for this span (needed for depth comparisons as well as actually
              //writing out the span)

              amd3d_depth_at_pixel(&cur_stack->grads, &cur_stack->cur_span_start_ooz);
              // = cur_stack->grads.oozat00 + (cur_stack->grads.doozdy * fy) + (cur_stack->grads.doozdx * fx);

            } while (cur_stack->type > SPAN_TRI_SEE_THRU);
          }
        }
        else
        {
          //this span ends below whats on top of the stack.
          //but if everything above this span is transparent, we need to draw it
          temp_stack = e_tri->last_stack;
          
          while (temp_stack->type>SPAN_TRI_SEE_THRU)
            temp_stack = temp_stack->last_stack;
          
          //did we make it past everything in the stack?
          if (temp_stack==&tri_stack)
          {
            //if so, that means everything above us is transparent and this span
            //will be seen, so we gotta draw it
          
            span_length = span_buff_x - e_tri->cur_span_start_x;
            if (span_length>0)
            {                          
              //make a span
              next_new_span->s.x            = e_tri->cur_span_start_x;
              next_new_span->s.y            = span_buff_y;
              next_new_span->s.width        = span_length;
              next_new_span->s.ooz          = e_tri->cur_span_start_ooz;
              next_new_span->s.scanline_ptr = cur_scanline;
                
              //add to its triangles list of spans
              next_new_span->s.next_tri_span = e_tri->span_list_head;
              e_tri->span_list_head = num_global_spans;
                
              num_global_spans++;
              next_new_span++;
              if (num_global_spans>=MAX_SPANS) return i4_F;
            }

            //setup the start of the next span in
            //IF this one wasnt transparent. otherwise,
            //we want the span start info for that span to be
            //maintained (unaltered)
            temp_stack = e_tri;

            if (e_tri->type<SPAN_TRI_SEE_THRU)
            {
              do
              {
                temp_stack = temp_stack->next_stack;
                if (temp_stack == &tri_stack)
                  break;
                
                temp_stack->cur_span_start_x = span_buff_x;
                
                //calculate the ooz value for this span (needed for depth comparisons as well as actually
                //writing out the span)            
                //temp_stack->cur_span_start_ooz = temp_stack->grads.oozat00 +
                //                                (temp_stack->grads.doozdy * fy) +
                //                                (temp_stack->grads.doozdx * fx);

                amd3d_depth_at_pixel(&temp_stack->grads, &temp_stack->cur_span_start_ooz);

              } while (temp_stack->type > SPAN_TRI_SEE_THRU);
            }
          }                    
        }
                
        e_tri->last_stack->next_stack = e_tri->next_stack;
        e_tri->next_stack->last_stack = e_tri->last_stack;                
      }      
    }
        
    //done with this scanline.
    //dont forget that IF we want to draw the background, we need to make one more span going from
    //tri_stack.cur_span_start_x to the rightmost pixel on the screen (if that area is "open")

    //remove the ones that need removing
    temp_edge = *remove_edge_list;
    while (temp_edge)
    {      
      temp_edge->last_active->next_active = temp_edge->next_active;
      temp_edge->next_active->last_active = temp_edge->last_active;

      temp_edge = temp_edge->next_remove;
    }
    remove_edge_list++;

    //step the active edges
    temp_edge = active_list_head.next_active;
    while (temp_edge != &active_list_tail)
    {
      temp_edge->x += temp_edge->dxdy;
      
      //make sure the edges remain sorted            
      compare_here = temp_edge;
      if (temp_edge->x < compare_here->last_active->x)
      { 
        t1 = temp_edge->next_active;

        while (temp_edge->x < compare_here->last_active->x)
          compare_here = compare_here->last_active;
     
        //get temp_edge out of its current spot
        temp_edge->last_active->next_active = temp_edge->next_active;
        temp_edge->next_active->last_active = temp_edge->last_active;

        //insert it between compare_here and its predecessor      
        compare_here->last_active->next_active = temp_edge;
        
        temp_edge->last_active = compare_here->last_active;
        compare_here->last_active = temp_edge;

        temp_edge->next_active = compare_here;
        
        temp_edge = t1;
      }
      else
      {
        temp_edge = temp_edge->next_active;
      }
    }    
  }  
  return i4_T;
}

#endif
