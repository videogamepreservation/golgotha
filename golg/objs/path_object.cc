/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/path_object.hh"
#include "object_definer.hh"
#include "lisp/li_init.hh"
#include "lisp/li_class.hh"
#include "li_objref.hh"
#include "map_man.hh"
#include "map.hh"
#include "saver.hh"
#include "li_objref.hh"
#include "player.hh"
#include "g1_render.hh"
#include "objs/map_piece.hh"
#include "isllist.hh"
#include "objs/bases.hh"
#include "sound/sfx_id.hh"

extern int g1_show_list;  // defined in map_piece.cc

enum 
{
  DATA_VERSION1=1,
  DATA_VERSION,
};




static li_symbol_class_member active("active");
static li_symbol_ref on("on"), yes("yes"), no("no"), already_attached("already_attached");

static li_symbol_ref off("off"), s_add_link("add_link"), s_remove_link("remove_link");
static li_g1_ref_class_member start("start");
static li_symbol_class_member bridgeable_spot("bridgeable_spot");

i4_isl_list<g1_path_object_class> g1_path_object_list;


static li_g1_ref_list_class_member links("links"), enemy_links("enemy_links"),
  controlled_objects("controlled_objects");

static li_int_class_member warning_level("warning_level");


int g1_path_object_class::bomb_warning_level()
{
  return vars->get(warning_level);
}

g1_path_object_class::bridge_status_type g1_path_object_class::get_bridge_status()
{
  li_symbol *s=vars->get(bridgeable_spot);
  if (s==yes.get())
    return NO_BRIDGE;
  else if (s==no.get())
    return NOT_BRIDGABLE;
  else
    return HAS_BRIDGE;
}

g1_path_object_class::g1_path_object_class(g1_object_type id, g1_loader_class *fp)
  : g1_object_class(id, fp), link(8,16)
{
  int i;
  w16 ver=0,data_size;
  if (fp) fp->get_version(ver,data_size);

  switch (ver)
  {
    case DATA_VERSION:
    {
      for (i=0; i<G1_MAX_TEAMS; i++)
        last_selected_tick[i]=fp->read_32();
      
      link_index[0]=0;
      for (i=0; i<G1_MAX_TEAMS; i++)
        link_index[i+1] = fp->read_8();

      for (i=0; i<link_index[G1_MAX_TEAMS]; i++)
      {
        link_class *l = link.add();
        
        l->path.load(fp);
        l->object.load(fp);
      }
    } break;

    default:
    {
      if (fp) fp->seek(fp->tell() + data_size);

      link_index[0]=0;
      for (i=0; i<G1_MAX_TEAMS; i++)
      {
        last_selected_tick[i]=1;
        link_index[i+1]=0;
      }
    } break;
  }
  if (fp) fp->end_version(I4_LF);

  draw_params.setup("blackred");
  set_flag(SELECTABLE | TARGETABLE, 1);
}
    
void g1_path_object_class::validate()
{
  for (int a=0; a<G1_MAX_TEAMS; a++)
    for (int i=total_links((g1_team_type)a)-1; i>=0; i--)
    {
      link_class *l = &link[link_index[a]+i];
      if (!l->path.valid() || !l->object.valid())
      {
        link.remove(link_index[a] + i);
        for (int t=a; t<G1_MAX_TEAMS; t++)
          link_index[t+1]--;
      }
    }
}
    
void g1_path_object_class::save(g1_saver_class *fp)
{
  int i;

  g1_object_class::save(fp);

  fp->start_version(DATA_VERSION);

  for (i=0; i<G1_MAX_TEAMS; i++)
    fp->write_32(last_selected_tick[i]);
  
  for (i=0; i<G1_MAX_TEAMS; i++)
    fp->write_8(link_index[i+1]);

  for (i=0; i<link.size(); i++)
  {
    link[i].path.save(fp);
    link[i].object.save(fp);
  }

  fp->end_version();
}
    
i4_bool g1_path_object_class::occupy_location()
{

  int a,i;

  if (occupy_location_corners())
  {
    g1_path_object_list.insert(*this);
    return i4_T;
  }
  else return i4_F;  
}


void g1_path_object_class::unoccupy_location()
{
  if (get_flag(MAP_OCCUPIED))
  {
    g1_object_class::unoccupy_location();
    g1_path_object_list.find_and_unlink(this);
  }
}

void g1_path_object_class::draw(g1_draw_context_class *context)
{
  if (g1_show_list)
  {
    int a=0;
    for (int i=0; i<link.size(); i++)
    {
      while (i>=link_index[a+1]) a++; // determine team number
      i4_float offs = a*0.2-0.1;
      i4_color col = (a==0)? 0xffff : 0xff00ff;
      g1_object_class *o = link[i].get_object();
      if (o)
        g1_render.render_3d_line(i4_3d_point_class(x+offs,y+offs,h+0.1),
                                 i4_3d_point_class(o->x+offs, o->y+offs, o->h+0.1),
                                 col, 0, context->transform);
    }
  }

#if 0  
  if (controlled_objects()->size())
    g1_model_draw(this, draw_params, context);
  else
#endif
    g1_editor_model_draw(this, draw_params, context);

}
  

void g1_path_object_class::add_controlled_object(g1_object_class *o)
{
  li_class_context context(vars);
  li_g1_ref_list *list=controlled_objects()->clone();
  vars->set_value(controlled_objects.offset, list);
  if (list->find(o)<0)
    list->add(o);
}

void g1_path_object_class::add_link(g1_team_type team, g1_path_object_class *o)
{
  if (get_path_index(team, o)<0)
  {
    link_class *l = link.add_at(link_index[team+1]);
    l->path = o;
    l->object = o;
    for (int i=team; i<G1_MAX_TEAMS; i++)
      link_index[i+1]++;
  }
}

i4_bool g1_path_object_class::remove_link(g1_team_type team, g1_path_object_class *p)
{
  int loc = get_path_index(team, p);
  if (loc>=0)
  {
    link.remove(link_index[team] + loc);
    for (int i=team; i<G1_MAX_TEAMS; i++)
      link_index[i+1]--;
    return i4_T;
  }
  return i4_F;
}

void g1_path_object_class::request_remove()
{
  int team=0;
  for (int i=0; i<link.size(); i++)
  {
    while (i>=link_index[team+1]) team++; // determine team number

    g1_object_class *o = link[i].get_object();
    g1_map_piece_class *mp;
    
    while (mp = g1_map_piece_class::cast(o))
    {
      if (mp->next_path.get() == this)
        o = mp->prev_object.get();
      else
        o = mp->next_object.get();
      mp->unlink();
    }

    g1_path_object_class *path = link[i].get_path();


    I4_TEST(o == path, "Invalid Linked List!");

    if (path)
      path->remove_link((team==G1_ALLY)? G1_ENEMY : G1_ALLY, this);
  }
  link.clear();
  g1_object_class::request_remove();

}

li_object *g1_path_object_class::message(li_symbol *message_name,
                                         li_object *message_params, 
                                         li_environment *env)
{
  li_class_context context(vars);
  
  if (message_name==on.get() || message_name==off.get())
    active() = on.get();
  else if (message_name==s_add_link.get())
  {
    g1_object_class *o=li_g1_ref::get(message_params,env)->value();
    g1_path_object_class *path = g1_path_object_class::cast(o);

    if (path)
    {
      add_link(G1_ALLY, path);
      path->add_link(G1_ENEMY, this);
    }
  }
  else if (message_name==s_remove_link.get())
  {
    g1_object_class *o=li_g1_ref::get(message_params,env)->value();
    g1_path_object_class *path = g1_path_object_class::cast(o);
    
    if (path)
    {
      remove_link(G1_ALLY, path);
      path->remove_link(G1_ENEMY, this);
    }
  }

  return 0;
}

int g1_path_object_class::get_path_index(g1_team_type team, g1_path_object_class *o) const
{
  for (int i=link_index[team]; i<link_index[team+1]; i++)
    if (link[i].get_path() == o)
      return i-link_index[team];

  return -1;
}

int g1_path_object_class::get_path_index(g1_path_object_class *o) const
{
  for (int i=0; i<link.size(); i++)
    if (link[i].get_path() == o)
      return i;

  return -1;
}

int g1_path_object_class::get_object_index(g1_object_class *o) const
{
  for (int i=0; i<link.size(); i++)
    if (link[i].get_object() == o)
      return i;

  return -1;
}

g1_path_object_class *g1_path_object_class::get_recent_link(g1_team_type team,
                                                            g1_path_object_class *last_used)
{
  g1_path_object_class *best=0;
  
  w32 max_allowed=last_used ? last_used->last_selected_tick[team] : 0xffffffff;
  
  w32 t=total_links(team);
  w32 best_tick=0;
  int past_it=0;
      
  for (int i=0; i<t; i++)
  {
    g1_path_object_class *p=get_link(team,i);
    int tick=p->last_selected_tick[team];
    
    if (tick<=max_allowed)
    {
      if (tick<max_allowed && tick>best_tick)
      {
        best=p;
        best_tick=tick;
      }
      else if (tick==max_allowed && past_it && best_tick!=max_allowed)
      {
        best=p;
        best_tick=tick;
      }
    }
    
    if (p==last_used)
      past_it=1;
  }
  
  return best;
}
    

int g1_path_object_class::total_controlled_objects()
{
  return li_g1_ref_list::get(vars->get(controlled_objects),0)->size();
}

g1_object_class *g1_path_object_class::get_controlled_object(int object_num)
{
  return li_g1_ref_list::get(vars->get(controlled_objects),0)->value(object_num);
}

// returns the total destinations found (banks & etc that are attached to the path)
int g1_path_object_class::find_path_destinations(g1_object_class **list,
                                                 int list_size,
                                                 g1_team_type team)
{
  i4_array<g1_object_class *> objects_to_unmark(128,128);
  i4_array<g1_path_object_class *> unvisited_nodes(128,128);
  
  int off=vars->member_offset(team==G1_ALLY ? "links" : "enemy_links");
  unvisited_nodes.add(this);
  int unvisited_head=0;   
  int t_in_list=0;

  while (unvisited_head<unvisited_nodes.size())
  {  
    g1_path_object_class *p=unvisited_nodes[unvisited_head++];
    
    if (p && !p->get_flag(SCRATCH_BIT))
    {
      p->set_flag(SCRATCH_BIT, 1);
      objects_to_unmark.add(p);

      li_g1_ref_list *l=li_g1_ref_list::get(p->vars->value(off),0);           
      int t=l->size(), i;
      for (i=0; i<t; i++)
      {
        g1_object_class *o=l->value(i);
        if (o)
          unvisited_nodes.add(g1_path_object_class::cast(o));
      }

      if (li_g1_ref_list::get(p->vars->get(controlled_objects),0)->size())
        list[t_in_list++]=p;
    }
  }
   
  for (int i=0; i<objects_to_unmark.size(); i++)
    objects_to_unmark[i]->set_flag(SCRATCH_BIT, 0);

  return t_in_list;
}


int g1_path_object_class::find_path(g1_team_type team,
                                    g1_path_object_class **stack,
                                    int stack_size)
{
  int t=0;
  g1_path_object_class *o=this;
  do
  {
    stack[t++]=o;
    if (o)
      o=o->get_recent_link(team, 0);
  } while (o && t<stack_size);
  
  I4_ASSERT(t<stack_size, "Either paths are too long or path loop encountered!");

  return t;
}

  
int g1_path_object_class::find_path(g1_team_type team, g1_path_object_class *dest, 
                                    g1_path_object_class **stack,
                                    int stack_size)
{
  g1_path_object_class *visited[1024];
  w32 most_recently_selected[256];

  I4_ASSERT(stack_size<=256, "can't find paths greater than 256.  bump this up, if needed.");

  int depth=0, num_visited=0;

  enum {BIG_NUM=0xffffffff };
  
  stack[depth] = this;
  stack[depth+1] = 0;
  most_recently_selected[depth] = BIG_NUM;
  set_flag(g1_object_class::SCRATCH_BIT,1);
  visited[num_visited++] = this;

  do
  {
    g1_path_object_class *o = stack[depth]->get_recent_link(team, stack[depth+1]);
    if (o)
    {
      o->set_flag(g1_object_class::SCRATCH_BIT,1);
      visited[num_visited++] = o;

      depth++;
      stack[depth] = o;
      stack[depth+1] = 0;
    }
    else
      depth--;
  } while (depth>=0 && stack[depth]!=dest);

  for (int i=0; i<num_visited; i++)
    visited[i]->set_flag(g1_object_class::SCRATCH_BIT,0);

  if (depth<0)
    depth=0;

  if (depth)
  {
    while (stack[depth]) // whatsthis? && stack[depth]->total_links(type)>0)
    {
      g1_path_object_class *o = stack[depth]->get_recent_link(team, 0);
      stack[++depth] = o;
    }
  
    stack[++depth]=0;
  }

  for (int j=0; j<depth; j++)
    if (!stack[j])
      return j;

  return depth;
}

void g1_path_object_class::change_player_num(int new_player)
{
  g1_object_class::change_player_num(new_player);
  int t=total_controlled_objects();
  for (int i=0; i<t; i++)
  {
    g1_object_class *o=get_controlled_object(i);
    if (o && o->player_num!=new_player)
    {
      char msg[100];
      w32 color;
      if (new_player==g1_player_man.local_player)
      {
        sprintf(msg, "Building Captured : %s", o->name());
        color=0x00ff00;
      }
      else
      {
        sprintf(msg, "Building Lost : %s", o->name());
        color=0xff0000;
      }

      g1_player_man.show_message(msg, color, g1_player_man.local_player);

      o->change_player_num(new_player);
    }

  }
}

g1_path_object_class* g1_path_object_class::find_next(g1_team_type team,
                                                      g1_path_object_class *dest)
{
  g1_path_object_class *path[256];
  
  find_path(team, dest, path, 256);
  return path[1];
}

void g1_path_object_class::editor_draw(g1_draw_context_class *context)
{
  int i;
  for (i=0; i<total_links(G1_ALLY); i++)
  {
    g1_object_class *o = get_link(G1_ALLY,i);
    if (o)
    {
      g1_render.render_3d_line(i4_3d_point_class(x,y,h+0.1),
                               i4_3d_point_class(o->x, o->y, o->h+0.1),
                               0xffffff, 0, context->transform);
    }
  }

  for (i=0; i<total_links(G1_ENEMY); i++)
  {
    g1_object_class *o = get_link(G1_ENEMY,i);
    if (o)
    {
      g1_render.render_3d_line(i4_3d_point_class(o->x, o->y, o->h+0.1),
                               i4_3d_point_class(x,y,h+0.1),
                               0xffffff, 0, context->transform);
    }
  }
  li_g1_ref_list::get(vars->get(controlled_objects),0)->draw(this, 0xff0000, context);  
}

g1_object_definer<g1_path_object_class> 
g1_path_object_def("path_object",
                   g1_object_definition_class::EDITOR_SELECTABLE | 
                   g1_object_definition_class::TO_PATH_OBJECT);

