/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/map_piece.hh"
#include "map.hh"
#include "map_man.hh"
#include "math/pi.hh"
#include "math/trig.hh"
#include "math/angle.hh"
#include "g1_rand.hh"
#include "resources.hh"
#include "objs/model_draw.hh"
#include "objs/model_id.hh"
#include "saver.hh"
#include "objs/scream.hh"
#include "sound/sfx_id.hh"
#include "sound_man.hh"
#include "objs/target.hh"
#include "player.hh"
#include "g1_texture_id.hh"
#include "g1_render.hh"
#include "sound/sound_types.hh"
#include "objs/shrapnel.hh"
#include "objs/explode_model.hh"
#include "team_api.hh"
#include "time/profile.hh"
#include "li_objref.hh"
#include "lisp/li_types.hh"
#include "lisp/li_class.hh"
#include "lisp/li_init.hh"
#include "objs/path_object.hh"
#include "tile.hh"
#include "map_cell.hh"
#include "tick_count.hh"
#include "map_view.hh"
#include "g1_tint.hh"
#include "controller.hh"


int g1_show_list=0;
li_object *g1_toggle_show_list(li_object *o, li_environment *env)
{
  g1_show_list=!g1_show_list;
  return 0;
}

li_automatic_add_function(g1_toggle_show_list, "toggle_show_list");

enum { DATA_VERSION=8 };

i4_profile_class pf_find_target("map piece::find target");
i4_profile_class pf_suggest_move("map_piece::suggest_move");
i4_profile_class pf_check_move("map_piece::check_move");
i4_profile_class pf_update_rumble_sound("map_piece::update_rumble");
i4_profile_class pf_map_piece_think("map_piece::think");
i4_profile_class pf_map_piece_move("map_piece::move");


//S1_SFX(scream1, "screams/scream0_22khz.wav", 0, 60);   // don't play screams as 3d
//S1_SFX(scream2, "screams/scream00_22khz.wav", 0, 60);

static g1_object_type shrapnel_type=0, shockwave_type=0;

const int battle_step = 9;
i4_float position_table[battle_step] = { -2.0, -0.5, 1.0, -1.5, 2.0, -2.5, 3.0, -3.5, 4.0 };


i4_bool g1_map_piece_class::can_attack(g1_object_class *who) const
{
  return (i4_bool)(who->get_team()!=get_team() &&
                   who->get_flag(TARGETABLE) &&
                   ((who->get_flag(GROUND | UNDERWATER | AERIAL) & 
                     (get_flag(HIT_GROUND | HIT_UNDERWATER | HIT_AERIAL)>>3)))!=0 &&
                   in_range(who));
}

i4_bool g1_map_piece_class::in_range(g1_object_class *o) const
{
  float r=detection_range();
  r*=r;
  float d=(o->x-x)*(o->x-x) + (o->y-y)*(o->y-y);
  return (d<=r);
}

extern g1_object_type g1_supertank_type;

static g1_quad_class mp_tmp_quad;


// this is the default function for handling tinted polygons
g1_quad_class *g1_map_piece_tint_modify(g1_quad_class *in, g1_player_type player)
{
  mp_tmp_quad=*in;

  mp_tmp_quad.material_ref=g1_get_texture("charredvehicle");
  if (g1_tint!=G1_TINT_OFF)
    g1_render.r_api->set_color_tint(g1_player_tint_handles[player]);
    
  return &mp_tmp_quad;
}


void g1_dead_ambient(i4_transform_class *object_to_world, 
                             i4_float &ar, i4_float &ag, i4_float &ab)
{
  g1_get_map()->get_illumination_light(object_to_world->t.x, object_to_world->t.y, ar,ag,ab);
  
  ar *=0.4;
  ag *=0.4;
  ab *=0.4;
}

void g1_map_piece_class::draw(g1_draw_context_class *context)
{
  if (g1_show_list)
  {
  if (next_object.get())
    g1_render.render_3d_line(i4_3d_point_class(x+0.1,y+0.1,h+0.1),
                             i4_3d_point_class(next_object->x, 
                                               next_object->y+0.1, 
                                               next_object->h+0.1),
                             0xffff, 0, context->transform);
  if (prev_object.get())
    g1_render.render_3d_line(i4_3d_point_class(x-0.1,y-0.1,h+0.1),
                             i4_3d_point_class(prev_object->x, 
                                               prev_object->y-0.1, 
                                               prev_object->h+0.1),
                             0xff00ff, 0, context->transform);
  }

  g1_model_draw(this, draw_params, context);

  if (context->draw_editor_stuff)
  {
    if (attack_target.valid())
    {
      i4_3d_point_class p1(x,y,h+0.1), p2(attack_target->x,attack_target->y,attack_target->h+0.1);
      g1_render.render_3d_line(p1,p2,0xff8000,0xff0000,context->transform);
    }
  }
}

void g1_map_piece_class::save(g1_saver_class *fp)
{
  g1_object_class::save(fp);
  
  fp->start_version(DATA_VERSION);

  fp->write_format("ffffffffffff222",
                   &speed,&vspeed,
                   &dest_x,&dest_y,&dest_z,
                   &dest_theta,&fire_delay,
                   &path_pos, &path_len,
                   &path_cos, &path_sin, &path_tan_phi,
                   &stagger,
                   &draw_params.frame, &draw_params.animation);

  fp->write_reference(attack_target);
  fp->write_reference(next_object);
  fp->write_reference(prev_object);

  next_path.save(fp);
  if (path_to_follow)
  {
    g1_id_ref *r;
    int t=0;
    for (r=path_to_follow; r->id; r++, t++);  
    fp->write_16(t);
    for (r=path_to_follow; r->id; r++)
      r->save(fp);
  }
  else
    fp->write_16(0);
  
  fp->end_version();
}

void g1_map_piece_class::add_team_flag()
{
  int mini_index=num_mini_objects-1;

  if (mini_index>=0 && mini_objects[mini_index].defmodeltype==0)
  {
    mini_objects[mini_index].defmodeltype = g1_player_man.get(player_num)->team_flag.value;
    i4_3d_vector v;
    if (!draw_params.model->get_mount_point("Flag", v))
      v.set(0,0,0);
    
    mini_objects[mini_index].position(v);
  }
}

g1_map_piece_class::g1_map_piece_class(g1_object_type id, 
                                       g1_loader_class *fp)
  : g1_object_class(id, fp)
{
  rumble_type=G1_RUMBLE_GROUND;

  path_to_follow=0;
  
  w16 ver=0,data_size;
  defaults=g1_object_type_array[id]->defaults;

  damage_direction=i4_3d_vector(0,0,1);
  ticks_to_blink=0;
  
  if (fp)
    fp->get_version(ver,data_size);

  stagger=0;

  switch (ver)
  {
    case DATA_VERSION:
    {
      fp->read_format("ffffffffffff222",
                      &speed,&vspeed,
                      &dest_x,&dest_y,&dest_z,
                      &dest_theta,&fire_delay,
                      &path_pos, &path_len,
                      &path_cos, &path_sin, &path_tan_phi,
                      &stagger,
                      &draw_params.frame, &draw_params.animation);

      fp->read_reference(attack_target);
      fp->read_reference(next_object);
      fp->read_reference(prev_object);

      next_path.load(fp);
      int t=fp->read_16();
      if (t)
      {
        path_to_follow=(g1_id_ref *)i4_malloc(sizeof(g1_id_ref)*(t+1),"");
        for (int i=0; i<t; i++)
          path_to_follow[i].load(fp);

	path_to_follow[t].id=0;
      }

        
    } break;

    case 7:
    {
      draw_params.frame=fp->read_16();
      draw_params.animation=fp->read_16();
      speed=fp->read_float();
      fp->read_float();       // remove in next rev.
      fp->read_float();       // remove in next rev
      health=fp->read_16();
      fire_delay=fp->read_16();

      dest_x=fp->read_float();
      dest_y=fp->read_float();

      fp->read_32();          // fp->read_reference(attack_target); remove
      fp->read_32();          // fp->read_reference(convoy); remove

      int t=fp->read_16();
      if (t)
      {
        path_to_follow=(g1_id_ref *)i4_malloc(sizeof(g1_id_ref)*(t+1),"");
        for (int i=0; i<t; i++)
          path_to_follow[i].load(fp);
	path_to_follow[t].id=0;
      }
        
    } break;

    default:
    {
      if (fp) fp->seek(fp->tell() + data_size);

      health = defaults->health;
      speed=0;
      vspeed=0;
      tread_pan=0;
      dest_x = dest_y = dest_z = -1;
      fire_delay=0;      
      groundpitch = groundroll = 0;
      memset(&attack_target,0,sizeof(g1_typed_reference_class<g1_object_class>));
    } break;
  }

  if (fp)
    fp->end_version(I4_LF);
}


inline void check_on_map(const i4_3d_vector &v)
{
  int vx=i4_f_to_i(v.x), vy=i4_f_to_i(v.y);
  if (vx<0 || vy<0 || vx>=g1_get_map()->width() || vy>=g1_get_map()->height())
    i4_error("off map");
}



void g1_map_piece_class::damage(g1_object_class *obj, int hp, i4_3d_vector _damage_dir)
{


  g1_object_class::damage(obj, hp, _damage_dir);
  ticks_to_blink=20;

  
  if (health<=0)
  {
//     if ((g1_tick_counter + global_id)&1)
//       scream1.play();
//     else
//       scream2.play();
  }
  else
    ticks_to_blink=20;
}


void g1_map_piece_class::find_target(i4_bool unfog)
{
  pf_find_target.start();

  if (attack_target.valid() && 
      (!can_attack(attack_target.get()) || !attack_target->get_flag(DANGEROUS)))
    attack_target = 0;

  if (!find_target_now())
  {
    pf_find_target.stop();
    return;
  }

  g1_map_class *map = g1_get_map();

  //find a target in range
  sw32 ix,iy,x_left,x_right,y_top,y_bottom;

  float r=detection_range();
  x_left   = i4_f_to_i(x-r); if (x_left<0)                x_left=0;
  x_right  = i4_f_to_i(x+r); if (x_right>=map->width())   x_right=map->width()-1;
  y_top    = i4_f_to_i(y-r); if (y_top<0)                 y_top=0;
  y_bottom = i4_f_to_i(y+r); if (y_bottom>=map->height()) y_bottom=map->height()-1;

  if (g1_player_man.local_player != player_num)
    unfog=i4_F;


  g1_object_class *potential_target=0;
  i4_float dist,target_dist=r*r;

  int fog_rect_x1=10000, fog_rect_y1=10000,
      fog_rect_x2=-1, fog_rect_y2=-1;
  

  for (iy=y_top;  iy<=y_bottom; iy++)
  {
    g1_map_cell_class *c=map->cell(x_left,iy);
    for (ix=x_left; ix<=x_right;  ix++)
    {
      if (unfog && (c->flags & g1_map_cell_class::FOGGED))
      {
        c->unfog(ix, iy);
        if (ix<fog_rect_x1) fog_rect_x1=ix;
        if (ix>fog_rect_x2) fog_rect_x2=ix;
        if (iy<fog_rect_y1) fog_rect_y1=iy;
        if (iy>fog_rect_y2) fog_rect_y2=iy;
      }

      
      if (!attack_target.valid())
      {
        g1_object_chain_class *p = c->get_obj_list();
        
        while (p)
        {
          g1_object_class *o = p->object;
                    
          if (can_attack(o) && o->get_flag(DANGEROUS))
          {
            dist = (o->x-x)*(o->x-x) + (o->y-y)*(o->y-y);
            if (dist<target_dist)
            {
              potential_target = o;
              target_dist = dist;
            }
          }
          
          p = p->next;
        }
      }
      c++;
    }
  }


  if (fog_rect_x2!=-1)
    g1_radar_refresh(fog_rect_x1, fog_rect_y1, fog_rect_x2, fog_rect_y2);

  if (!attack_target.valid() && potential_target)
  {
    attack_target = potential_target;
    request_think();
  }

  pf_find_target.stop();
}

void g1_map_piece_class::lead_target(i4_3d_point_class &lead, i4_float shot_speed)
{
  if (!attack_target.valid())
    return;

  lead.set(attack_target->x, attack_target->y, attack_target->h);

  g1_map_piece_class *mp = g1_map_piece_class::cast(attack_target.get());

  if (!mp) 
    return;

  if (shot_speed<0)
  {
    g1_object_type shot = g1_get_object_type(defaults->fire_type);
    shot_speed = g1_object_type_array[shot]->get_damage_map()->speed;
  }

  if (shot_speed>0)
  {
    i4_float 
      dx = mp->x - x,
      dy = mp->y - y,
      t = sqrt(dx*dx + dy*dy)/shot_speed;

    i4_3d_vector mp_diff(mp->x - mp->lx, mp->y - mp->ly, mp->h - mp->lh);
    mp_diff*=t;
    lead += mp_diff;
  }
}



void g1_map_piece_class::init()
{

}


void g1_map_piece_class::init_rumble_sound(g1_rumble_type type)
{
  rumble_type=type;
}




static li_object_class_member links("links");
static li_symbol_ref reached("reached");

void g1_map_piece_class::unlink()
{
  g1_path_object_class *path;
  g1_map_piece_class *mp;

  if (path = g1_path_object_class::cast(prev_object.get()))
  {
    int i=path->get_object_index(this);
    if (i>=0)
      path->link[i].object = next_object.get();
    else
      i4_warning("unlinking bad link!");
  }
  else if (mp = g1_map_piece_class::cast(prev_object.get()))
    if (mp->next_path.get() == next_path.get())
      mp->next_object = next_object.get();
    else
      mp->prev_object = next_object.get();

  if (path = g1_path_object_class::cast(next_object.get()))
  {
    int i=path->get_object_index(this);
    if (i>=0)
      path->link[i].object = prev_object.get();
    else
      i4_warning("unlinking bad link!");
  }
  else if (mp = g1_map_piece_class::cast(next_object.get()))
    if (mp->next_path.get() == next_path.get())
      mp->prev_object = prev_object.get();
    else
      mp->next_object = prev_object.get();

  prev_object=0;
  next_object=0;
}

void g1_map_piece_class::link(g1_object_class *origin)
{
  g1_object_class *origin_next=0;
  g1_path_object_class *path;
  g1_map_piece_class *mp;

  if (path = g1_path_object_class::cast(origin))
  {
    int i = path->get_path_index(g1_path_object_class::cast(next_path.get()));
    if (i>=0)
    {
      origin_next = path->link[i].object.get();
      path->link[i].object = this;
    }
    else
      i4_warning("linking bad link!");
  }
  else if (mp = g1_map_piece_class::cast(origin))
  {
    if (mp->next_path.get() == next_path.get())
    {
      origin_next = mp->next_object.get();
      mp->next_object = this;
    }
    else
    {
      origin_next = mp->prev_object.get();
      mp->prev_object = this;
    }
  }

  if (path = g1_path_object_class::cast(origin_next))
  {
    int i=path->get_object_index(origin);
    if (i>=0)
      path->link[i].object = this;
    else
      i4_warning("linking bad link!");
  }
  else if (mp = g1_map_piece_class::cast(origin_next))
    if (mp->next_path.get() == next_path.get())
      mp->prev_object = this;
    else
      mp->next_object = this;

  next_object = origin_next;
  prev_object = origin;
}

void g1_map_piece_class::think()
{
  pf_map_piece_think.start();

  request_think();

  pitch = 0;
  roll  = 0;

  // limit the search for target to 1 every 4 ticks
  find_target();

  if (fire_delay>0)
    fire_delay--;
  
  if (next_path.valid())
  {
    dest_x = next_path->x;
    dest_y = next_path->y;
  }

  i4_float dist, dtheta, dx, dy, old_pathpos=path_pos;
  suggest_move(dist, dtheta, dx, dy, 0);
  if (check_move(dx,dy))
  {
    if ((g1_rand(62)&63)==0)
    {
      g1_camera_event cev;
      cev.type=G1_WATCH_IDLE;
      cev.follow_object=this;
      g1_current_view_state()->suggest_camera_event(cev);
    }
    
    move(dx,dy);
  }
  else
  {
    get_terrain_info(); 
    path_pos = old_pathpos;
  }
  if (dist<speed)
    advance_path();
  
  if (h+vspeed-0.001 > terrain_height)
  {
    //he's off the ground
    //dont set these to 0, just leave them the way they were
    groundpitch = lgroundpitch;
    groundroll  = lgroundroll;

    h += vspeed;
    vspeed -= g1_resources.gravity;    
    request_think();
  }
  else
  {
    if (!get_flag(ON_WATER))
      h = terrain_height;
    else
    {
      h -= g1_resources.sink_rate;
      damage(0,g1_resources.water_damage,i4_3d_vector(0,0,1));
    }
    
    if (h!=lh)
      hit_ground();
    
    vspeed = h - lh - g1_resources.gravity;
  }

  pf_map_piece_think.stop();
}

i4_bool g1_map_piece_class::find_target_now() const
{
  return (((g1_tick_counter+global_id)&3)==0);
}

void g1_map_piece_class::hit_ground()
{
  if (vspeed<-0.4)
    damage(0,health,i4_3d_vector(0,0,1));
}

void g1_map_piece_class::advance_path()
{
  g1_team_type type=g1_player_man.get(player_num)->get_team();
      
  if (next_path.valid())
  {
    message(reached.get(), new li_g1_ref(next_path->global_id), 0);

    unlink();

    g1_path_object_class *from = g1_path_object_class::cast(next_path.get());

    // find next path node to go to
    if (path_to_follow)
    {
      g1_id_ref *r;
      for (r=path_to_follow; r->id && r->id!=next_path.id; r++);
      if (r->id) r++;
      if (r->id)
        next_path=*r;
      else
        next_path.id=0;
    }

    if (next_path.valid())
    {
      g1_path_object_class *next=(g1_path_object_class *)next_path.get();

      path_cos = next->x - from->x;
      path_sin = next->y - from->y;
      path_tan_phi = next->h - from->h;
      stagger = g1_float_rand(8)*2.0-1.0;

      path_pos = 0;
      
      path_len = sqrt(path_cos*path_cos + path_sin*path_sin);
      i4_float dist_w = 1.0/path_len;
      path_cos *= dist_w;
      path_sin *= dist_w;
      path_tan_phi *= dist_w;

      link(from);
    }
    else
    {
      unoccupy_location();
      request_remove();
    }
  }
}



i4_bool g1_map_piece_class::check_turn_radius()
//{{{
{  
  i4_float cx,cy;
  i4_float r,d,d1,d2,rx,ry; 
  
  //get the radius of the circle he's currently capable of turning through  
  //make it extra-large so he doesnt make ridiculously large turns
  //check r^2... a bit faster
  r = (speed/defaults->turn_speed) * 1.5;

  rx = r*sin(theta);
  ry = r*cos(theta);

  r = r*r;
  
  //check the two circles that are currently unreachable
  //if the destination lies within either circle, return false  
  cx = x - rx;
  cy = y + ry;
  d1 = (dest_x - cx);
  d1 *= d1;
  d2 = (dest_y - cy);
  d2 *= d2;
  d  = d1+d2;
  if (d<r) return i4_F;  
    
  cx = x + rx;
  cy = y - ry;
  d1 = (dest_x - cx);
  d1 *= d1;
  d2 = (dest_y - cy);
  d2 *= d2;
  d  = d1+d2;
  if (d<r) return i4_F;
  
  return i4_T;
}
//}}}

void g1_map_piece_class::request_remove()
//{{{
{
  if (path_to_follow)
    i4_free(path_to_follow);
  path_to_follow = 0;
  unlink();
  g1_object_class::request_remove();
}
//}}}

i4_bool g1_map_piece_class::suggest_move(i4_float &dist,
                                         i4_float &dtheta,
                                         i4_float &dx, i4_float &dy,
                                         i4_float braking_friction,
                                         i4_bool reversible)
{
  if (!prev_object.get() || !next_object.get())
  {
    dx=dy=0;
    return i4_F;
  }

  pf_suggest_move.start();

  i4_float angle, scale=0.0;

  speed = defaults->speed * (1.0-damping_fraction);


  if (path_len>6.0)
  {
    if (path_pos<3.0)
      scale = path_pos/3.0;
    else if (path_pos>path_len-3.0)
      scale = (path_len-path_pos)/3.0;
    else
      scale = 1.0;
  }

  path_pos += speed;
  dx = (dest_x - path_cos*(path_len - path_pos) - path_sin*stagger*scale - x);
  dy = (dest_y - path_sin*(path_len - path_pos) + path_cos*stagger*scale - y);

  //aim the vehicle    
  angle = i4_atan2(dy,dx);
  i4_normalize_angle(angle);    
  
  dtheta = angle - theta;
  if (dtheta<-i4_pi()) dtheta += 2*i4_pi();
  else if (dtheta>i4_pi()) dtheta -= 2*i4_pi();

  if (dtheta<-defaults->turn_speed) dtheta = -defaults->turn_speed;
  else if (dtheta>defaults->turn_speed) dtheta = defaults->turn_speed;
  theta += dtheta;
  i4_normalize_angle(theta);

  dist = path_len-path_pos;

  pf_suggest_move.stop();
  return (dist==0);
}

i4_bool g1_map_piece_class::suggest_air_move(i4_float &dist,
                                             i4_float &dtheta,
                                             i4_3d_vector &d)
{
  pf_suggest_move.start();

  i4_float angle, scale=0.0;

  speed = defaults->speed;

  if (path_len>6.0)
  {
    if (path_pos<3.0)
      scale = path_pos/3.0;
    else if (path_pos>path_len-3.0)
      scale = (path_len-path_pos)/3.0;
    else
      scale = 1.0;
  }

  path_pos += speed;
  d.x = (dest_x - path_cos*(path_len - path_pos) - path_sin*stagger*scale - x);
  d.y = (dest_y - path_sin*(path_len - path_pos) + path_cos*stagger*scale - y);
  d.z = (dest_z - path_tan_phi*(path_len - path_pos) - h);

  //aim the vehicle    
  angle = i4_atan2(d.y,d.x);
  i4_normalize_angle(angle);    
  
  dtheta = angle - theta;
  if (dtheta<-i4_pi()) dtheta += 2*i4_pi();
  else if (dtheta>i4_pi()) dtheta -= 2*i4_pi();

  if (dtheta<-defaults->turn_speed) dtheta = -defaults->turn_speed;
  else if (dtheta>defaults->turn_speed) dtheta = defaults->turn_speed;
  theta += dtheta;
  i4_normalize_angle(theta);

  dist = path_len-path_pos;

  pf_suggest_move.stop();
  return (dist==0);
}

i4_bool g1_map_piece_class::check_move(i4_float dx,i4_float dy) const
{
  pf_check_move.start();
  i4_bool ret=i4_T;
  if (next_object.valid())
  {
    g1_path_object_class *path;
    g1_map_piece_class *mp;
    
    if (path = g1_path_object_class::cast(next_object.get()))
    {
      g1_team_type team=g1_player_man.get(player_num)->get_team();
    
      // check branches
      for (int i=0; i<path->total_links(team); i++)
      {
        mp = g1_map_piece_class::cast(path->get_object_link(team,i));

        if (mp)
        {
          i4_float dist=path_len - path_pos;
          
          if (mp->player_num!=player_num)
          {
            // enemy vehicle
            dist+=mp->path_len-mp->path_pos;
            if (dist<2)
              ret=i4_F;
          }
          else
          {
            // allied vehicle
            dist += mp->path_pos;
            if (dist<0.5)
              ret=i4_F;
          }
        }
      }
    }
    else if (mp = g1_map_piece_class::cast(next_object.get()))
    {
      i4_float dist = -path_pos;
      
      if (mp->player_num!=player_num)
      {
        // enemy vehicle
        dist+=mp->path_len-mp->path_pos;
        if (dist<2)
          ret=i4_F;
      }
      else
      {
        // allied vehicle
        dist += mp->path_pos;
        if (dist<0.5)
          ret=i4_F;
      }
    }
  }
  pf_check_move.stop();
  return ret;
}

i4_bool g1_map_piece_class::check_life(i4_bool remove_if_dead)
{
  if (ticks_to_blink)
    ticks_to_blink--;

  if (health<=0)
  {
    //they were just killed. free their resources
    if (remove_if_dead)
    {
      unoccupy_location();
      request_remove();

      //keep thinking so that death scenes can be played out
    }


    char msg[100];
    sprintf(msg, "Unit Lost : %s", name());
    g1_player_man.show_message(msg, 0xff0000, player_num);
    
    return i4_F;
  }
  return i4_T;
}

static li_symbol_ref explode_model("explode_model");


 
static li_symbol_ref set_course("set_course");

void g1_map_piece_class::set_path(g1_id_ref *list)
{
  if (path_to_follow)
    i4_free(path_to_follow);
      
  path_to_follow=list;
  next_path=list[0].id;
  
  advance_path();
  if (!next_path.valid())
    i4_warning("i can't get there!");
  ltheta = theta = i4_atan2(path_sin,path_cos);
  request_think();
}



i4_bool g1_map_piece_class::move(i4_float x_amount,
                             i4_float y_amount)
{
  pf_map_piece_move.start();  

  unoccupy_location();
  
  x += x_amount;
  y += y_amount;
  
  if (!occupy_location())
  {
    pf_map_piece_move.stop();
    return i4_F;  
  }

  g1_add_to_sound_average(rumble_type, i4_3d_vector(x,y,h));


  pf_map_piece_move.stop();

  return i4_T;
}

void g1_map_piece_class::get_terrain_info()
{
  sw32 ix,iy;
  
  ix=i4_f_to_i(x);
  iy=i4_f_to_i(y);
  g1_map_cell_class *cell_on=g1_get_map()->cell(ix,iy);
  
  w16 handle=cell_on->type;
  i4_float newheight;
  
  g1_get_map()->calc_height_pitch_roll(x,y,h, terrain_height, groundpitch, groundroll);

  g1_tile_class *t = g1_tile_man.get(handle);
  damping_fraction = t->friction_fraction;
  set_flag(ON_WATER, (t->flags & g1_tile_class::WAVE)!=0 && 
           terrain_height<=g1_get_map()->terrain_height(x,y));
}

void g1_map_piece_class::calc_world_transform(i4_float ratio, i4_transform_class *t)
{
  if (!t)
    t = world_transform;

  i4_float z_rot        = i4_interpolate_angle(ltheta,theta, ratio);
  i4_float y_rot        = i4_interpolate_angle(lpitch,pitch, ratio);
  i4_float x_rot        = i4_interpolate_angle(lroll ,roll , ratio);
  
  i4_float ground_x_rot = i4_interpolate_angle(lgroundroll,  groundroll, ratio);
  i4_float ground_y_rot = i4_interpolate_angle(lgroundpitch, groundpitch, ratio);

  i4_float tx=i4_interpolate(lx,x,ratio);
  i4_float ty=i4_interpolate(ly,y,ratio);
  i4_float tz=i4_interpolate(lh,h,ratio);

  
  t->translate(tx,ty,tz);  
  t->mult_rotate_x(ground_x_rot);
  t->mult_rotate_y(ground_y_rot);
  t->mult_rotate_z(z_rot);
  t->mult_rotate_y(y_rot);
  t->mult_rotate_x(x_rot);
} 
