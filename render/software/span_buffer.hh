/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef SPAN_BUFFER_HH
#define SPAN_BUFFER_HH

#include "time/profile.hh"

union span_entry;
struct span_tri_info;

union span_entry //needs to be 8 byte aligned
{
  struct
  {
  //dont reorder these, the position of the x and y are required for a special amd3d mmx data load
  sw32  x;
  sw32  y;
  sw32  width;  
  
  w32   next_tri_span; //index into the global span list of the next span  
  
  float ooz; //this ooz will be the ooz at the spans start
             //since this is non-intersecting span sorting,
             //this is all we need
  w16   *scanline_ptr;
  } s;
  double alignment_dummy;
}; //should be 16 bytes

struct span_tri_info //needs to be 8 byte aligned
{
  tri_gradients grads;
  
  w16           *texture;
  
  union
  {
    sw32        texture_width;
    w32         color;
  };
  
  sw32          texture_height; 
  
  float         cur_span_start_ooz;
  
  span_tri_info *next_stack;
  span_tri_info *last_stack;
  
  sw16          cur_span_start_x;  //used when creating spans
  w16           span_list_head;  
  w8            twidth_log2;
  w8            has_leader;  
  w8            type;
  w8            color_tint;
}; //including the tri_gradients structure, this should be 104 bytes

enum {MAX_TRIS = 20000};

extern span_tri_info global_tri_list[]; //sized of max_tris
extern int num_global_tris;

inline span_tri_info *new_span_tri()
{
  if (num_global_tris < MAX_TRIS)
  {
    num_global_tris++;
    span_tri_info *t = &global_tri_list[num_global_tris-1];
    t->span_list_head=0;
    t->next_stack=0;
    t->last_stack=0;
    t->has_leader=0;
    return t;
  }
#ifdef DEBUG
  i4_warning("out of span_tri_info memory");
#endif
  return 0;
}

enum {MAX_SPANS = 30000};

extern span_entry global_span_list[]; //sized to max_spans
extern int num_global_spans;

enum {MAX_VERTICAL_RESOLUTION = 768};
enum {MAX_NUM_EDGES           = 10000};

struct span_edge //needs to be 4 byte aligned
{  
  sw32 x;
  sw32 dxdy;
   
  span_tri_info *tri_1;

  span_edge *next_remove; //in setup and when generating spans, this is the next remove edge (ends on the same scanline as this)
  span_edge *next_active; //when generating the spans, this is the next active span in the active span list
  union
  {
    span_edge *last_active;//when generating the spans, this is the previous active span in the active span list  
    span_edge *next; //in setup, this is the next new edge (starts on the same scanline as this)
  };
  
  w8 flags;
  //flags & LEADING_1 means its a leading edge
  //lower 7 bits define the type (see mappers.hh)

  w8 pad1;
  w8 pad2;
  w8 pad3; //pad it to 28 bytes
};

extern span_edge *new_edges[];
extern span_edge *remove_edges[];
extern span_edge global_edges[];
extern span_edge *active_list;

extern int num_global_edges;

inline span_edge *new_span_edge()
{
  if (num_global_edges < MAX_NUM_EDGES)
  {
    num_global_edges++;
    span_edge *e = &global_edges[num_global_edges-1];
    e->next_active = 0;
    e->last_active = 0;    
    return e;
  }
#ifdef DEBUG
  //i4_warning("out of span_edge memory");
#endif
  return 0;
}

extern i4_profile_class pf_software_add_start_edge;
extern sw32 shared_edges;
extern sw32 total_edges;

inline void add_start_edge(span_edge *e,sw32 scanline)
{
  pf_software_add_start_edge.start();

  span_edge *compare_here = new_edges[scanline];
  span_edge *last_compare = 0;

  while (compare_here && e->x > compare_here->x)
  {
    last_compare = compare_here;
    compare_here = compare_here->next;
  }
  
  e->next = compare_here;
  
  //total_edges++;

  if (last_compare)
  {
    last_compare->next = e;
    /*
    if (compare_here && (compare_here->x == e->x) && (compare_here->dxdy==e->dxdy))
    {
      shared_edges++;
    }
    */
  }
  else
    new_edges[scanline] = e;
  
  pf_software_add_start_edge.stop();
}

inline void add_remove_edge(span_edge *e,sw32 scanline)
{
  e->next_remove = remove_edges[scanline];
  remove_edges[scanline] = e;
}

void init_span_buffer();
void clear_spans();
void flush_spans();

#endif
