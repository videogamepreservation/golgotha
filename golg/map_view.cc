/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "map_view.hh"
#include "window/window.hh"
#include "image/image.hh"
#include "map.hh"
#include "camera.hh"
#include "window/win_evt.hh"
#include "device/kernel.hh"
#include "player.hh"
#include "map_cell.hh"
#include "tile.hh"
#include "g1_render.hh"
#include "tmanage.hh"
#include "app/app.hh"
#include "border_frame.hh"
#include "status/status.hh"
#include "map_vert.hh"
#include "lisp/lisp.hh"
#include "map_man.hh"
#include "objs/path_object.hh"
#include "objs/bases.hh"
#include "human.hh"
#include "image_man.hh"

class g1_radar_view_class;
static g1_radar_view_class *list=0;


void g1_calc_map_area(int max_width, int max_height, int &x1, int &y1, int &x2, int &y2)
{
  int map_width= g1_get_map()->width(), map_height=g1_get_map()->height();
  int iw=max_width, ih=map_height*max_width/map_width;
  
  if (ih>max_height)
  {
    ih=max_height;
    iw=map_width*max_height/map_height;
  }

  if (iw<max_width)
    x1=max_width/2-iw/2;
  else
    x1=0;
  x2=x1+iw-1;
      
  if (ih<max_height)
    y1=max_height/2-ih/2;
  else
    y1=0;
  y2=y1+ih-1;




}

struct g1_radar_params_struct
{
  int ix1, iy1, ix2, iy2;
  float mx1,my1,mx2,my2;
  float m2gx, m2gy, g2mx, g2my;
  float map_w, map_h;  

  void init(int image_w, int image_h)
  {
    g1_calc_map_area(image_w, image_h, ix1,iy1,ix2,iy2);
    mx1=ix1; mx2=ix2;
    my1=iy1; my2=iy2;

    map_w=(float)g1_get_map()->width();
    map_h=(float)g1_get_map()->height();

    m2gx = map_w/(float)(ix2-ix1+1);
    m2gy = map_h/(float)(iy2-iy1+1);
    
    g2mx = 1.0/m2gx;
    g2my = 1.0/m2gy;
  }

  void mouse_2_game(float mx, float my, float &x, float &y)
  {
    x=((float)mx-mx1) * m2gx;
    y=map_h-((float)my-my1) * m2gy-1;
  }

  void game_2_mouse(float gx, float gy, float &mx, float &my)
  {
    mx=gx * g2mx + mx1;
    my=(map_h-gy-1) * g2my + my1;

  } 
};



//////////////////////////////////////////////////////////////////////////////
// radar background drawing code
//////////////////////////////////////////////////////////////////////////////
static float radar_darkness_multiply;


inline int get_mat_color(g1_map_cell_class *c)
{
  if (c->flags & g1_map_cell_class::FOGGED)
    return 0;

  r1_texture_handle mat=g1_tile_man.get_texture(c->type);
  if (mat==g1_tile_man.get_pink())
    return 0;
  
  if (mat)
  {
    r1_texture_manager_class *tman=g1_render.r_api->get_tmanager();
    return tman->average_texture_color(mat,0);
  }
  else
    return 0;
}



static w8 interp_table[32*32*32];
static w8 light_table[32*32*32];
static w8 tables_calced=0;
static int r_shift, g_shift, b_shift;

static void calc_tables()
{
  if (tables_calced) return;
  tables_calced=1;

  const i4_pal *pal=i4_current_app->get_display()->get_palette();
  const i4_pixel_format *dst_fmt=&pal->source;
  I4_ASSERT(dst_fmt->pixel_depth==I4_16BIT,"");  // 16bit only code below

  b_shift=dst_fmt->blue_shift + (dst_fmt->blue_bits-5);

  g_shift=dst_fmt->green_shift + (dst_fmt->green_bits-5);  

  r_shift=dst_fmt->red_shift + (dst_fmt->red_bits-5);


  w8 *t=interp_table;
  for (int c1=0; c1<32; c1++)
    for (int c2=0; c2<32; c2++)
    {
      int d=c2-c1;
      for (int r=0; r<32; r++)
        *(t++)=d*r/32 + (int)c1;
    }


  t=light_table;
  for (int c=0; c<32; c++)
    for (int l=0; l<32; l++)
      *(t++)=((int)c * l)/32;    
}

inline w8 interpolate_555(w8 c1_0_31, int c2_0_31, int ratio_0_31)
{
  return interp_table[(c1_0_31<<10) | (c2_0_31<<5) | (ratio_0_31 << 0)];
}


inline w8 light_555(w8 color_0_31, int light_0_31)
{
  return light_table[(color_0_31<<5) | light_0_31];
}


w8 g1_light_555(w8 color_0_31, int light_0_31)
{
 
  calc_tables();
  return light_table[(color_0_31<<5) | light_0_31];
}

void g1_draw_strategy_border(i4_image_class *im)
{
  int mx1,my1,mx2,my2;
  g1_calc_map_area(im->width(), im->height(), mx1,my1,mx2,my2);
  i4_draw_context_class context(0,0,im->width()-1, im->height()-1);
  context.clip.remove_area(mx1,my1,mx2,my2);

  int y=my1;
  while (y>0) y-=2;

  w32 color;

  g1_player_piece_class *com=g1_player_man.get_local()->get_commander();
  if (com)
    color=g1_get_upgrade_color(com->upgrade_level_when_built);
  else
    color=g1_get_upgrade_color(-1);

  color=g1_light_color(color, 0.2);
  
  im->clear(0, context);
  for (; y<im->height(); y+=2)
    im->bar(0,y,im->width()-1,y, color, context);
}



void g1_render_map_area(i4_image_class *im, 
                        g1_radar_params_struct *p,
                        int gx1, int gy1, int gx2, int gy2,
                        i4_status_class *status,
                        i4_bool interlaced)
{
  g1_map_class *map=g1_get_map();

  int map_width=map->width(), map_height=map->height();  
  if (gx1>0)
    gx1-=1;
  if (gy1>0)
    gy1-=1;

  if (gy2<map_height-1)
    gy2+=1;
  if (gx2<map_width-1)
    gx2+=1;

  const i4_pal *pal=i4_current_app->get_display()->get_palette();
  calc_tables();

  float map_x, map_y, map_x_step, map_y_step;


  sw32 x,y;
  float r,g,b;
  float im_y1, im_x1, im_x2, im_y2;
  p->game_2_mouse(gx1,gy1, im_x1, im_y1);
  p->game_2_mouse(gx2,gy2, im_x2, im_y2);
  int i_im_y1=i4_f_to_i(im_y1), i_im_x1=i4_f_to_i(im_x1);
  int i_im_y2=i4_f_to_i(im_y2), i_im_x2=i4_f_to_i(im_x2);


  float map_x_start, map_y_start;
  p->mouse_2_game(i_im_x1, i_im_y1, map_x_start, map_y_start);


  map_x_step=p->m2gx;
  map_y_step=p->m2gy;

  map_y=map_y_start;
  if (interlaced)
    i_im_y1=i_im_y1&(~1);


  for (y=i_im_y1; y>=i_im_y2;)
  {
    if (status)
      status->update(map_y/(float)g1_get_map()->height());
        
    map_x=map_x_start;

    // assuming 16bit
    w16 *i1 = (w16 *)(((w8 *)im->data) + im->bpl*y + i_im_x1*2);

    for (x=i_im_x1; x<=i_im_x2; x++)
    {
      int i_map_x=i4_f_to_i(map_x), i_map_y=i4_f_to_i(map_y);
      g1_map_cell_class *cell1   = map->cell(i_map_x, i_map_y);        
      w32 color;

      if (i_map_x<map_width-1 && i_map_y<map_height-1)
      {
        int ratio=i4_f_to_i((map_x-i_map_x)*32.0);

        g1_map_vertex_class *v=map->vertex(i_map_x, i_map_y);

        if (v[0].light_sum & 0x80000000)
          v[0].recalc_light_sum(i_map_x, i_map_y);

        if (v[1].light_sum & 0x80000000)
          v[1].recalc_light_sum(i_map_x+1, i_map_y);

        int lv1=v[0].light_sum;
        int lv2=v[1].light_sum;


        int c1=get_mat_color(cell1);
        int c2=get_mat_color(cell1+1);

        // seperate color components
        
        c1>>=3;
        int b1=c1&31;  c1>>=8; 
        int g1=c1&31;  c1>>=8; 
        int r1=c1&31;

        c2>>=3;
        int b2=c2&31;  c2>>=8;
        int g2=c2&31;  c2>>=8;
        int r2=c2&31;
        

        lv1>>=3;
        int lv1r=lv1&31;  lv1>>=8;
        int lv1g=lv1&31;  lv1>>=8;
        int lv1b=lv1&31;

        lv2>>=3;
        int lv2r=lv2&31;  lv2>>=8;
        int lv2g=lv2&31;  lv2>>=8;
        int lv2b=lv2&31;


        // interpolate color
        int ur=interpolate_555(r1, r2, ratio);
        int ug=interpolate_555(g1, g2, ratio);
        int ub=interpolate_555(b1, b2, ratio);

        // interpolate light value
        int lvr=interpolate_555(lv1r, lv2r, ratio);
        int lvg=interpolate_555(lv1g, lv2g, ratio);
        int lvb=interpolate_555(lv1b, lv2b, ratio);

        // apply lighting
        int r = light_555(ur, lvr);
        int g = light_555(ug, lvg);
        int b = light_555(ub, lvb);
        

        color = (r<<r_shift) | (g<<g_shift) | (b<<b_shift);
      }
      else
      {
        g1_map_vertex_class *v1=map->vertex(i_map_x, i_map_y);
        if (v1[0].light_sum & 0x80000000)
          v1[0].recalc_light_sum(i_map_x, i_map_y);

        int lv1=v1[0].light_sum;
        lv1>>=3;
        int lv1r=lv1&31;  lv1>>=8;
        int lv1g=lv1&31;  lv1>>=8;
        int lv1b=lv1&31;

        int c1=get_mat_color(cell1);

        // seperate color components        
        c1>>=3;
        int b1=c1&31;  c1>>=8; 
        int g1=c1&31;  c1>>=8; 
        int r1=c1&31;


        // apply lighting
        int r = light_555(r1, lv1r);
        int g = light_555(g1, lv1g);
        int b = light_555(b1, lv1b);
        color = (r<<r_shift) | (g<<g_shift) | (b<<b_shift);
      }

      *i1=color;

      map_x+=map_x_step;
      ++i1;
    }
    
    map_y+=map_y_step;
    y--;
    if (interlaced)
    {
      y--;
      map_y+=map_y_step;
    }

  }



}

i4_image_class *g1_create_map_image(int max_width, int max_height,
                                    i4_bool interlace)
{        
  i4_status_class *status=i4_create_status(i4gets("rendering_map"));
                                         
  const i4_pal *pal=i4_current_app->get_display()->get_palette();
  i4_image_class *im = i4_create_image(max_width, max_height, pal);
  i4_draw_context_class context(0,0, max_width-1, max_height-1);
  im->clear(0, context);
  
  radar_darkness_multiply = 1.0/(255.0 * li_get_float(li_get_value("radar_darkness"),0));  


  g1_radar_params_struct p;
  p.init(max_width, max_height);

  g1_render_map_area(im, &p, 0,0, g1_get_map()->width()-1, 
                     g1_get_map()->height()-1, status, interlace);
    
//   g1_draw_takeover_spots(im);
//   g1_draw_paths(im, 0);

  g1_draw_strategy_border(im);
  
  delete status;

  return im;
}



static void quick_add(i4_rect_list_class &list, int x1, int y1, int x2, int y2)
{
  i4_rect_list_class::area_iter i;
  for (i=list.list.begin(); i!=list.list.end(); ++i)
  {
    if (((i->x1<=x2 && i->x1>=x1) ||
         (i->x2<=x2 && i->x2>=x1)) &&
        ((i->y1<=y2 && i->y1>=y1) ||
         (i->y2<=y2 && i->y2>=y1)))
    {
      i->x1=i->x1 < x1 ? i->x1 : x1;
      i->y1=i->y1 < y1 ? i->y1 : y1;
      i->x2=i->x2 > x2 ? i->x2 : x2;
      i->y2=i->y2 > y2 ? i->y2 : y2;

      return ;
    }         
  }
 
  list.list.insert(*(list.new_area(x1,y1,x2,y2)));
}


//////////////////////////////////////////////////////////////////////////////
// radar window management
//////////////////////////////////////////////////////////////////////////////
class g1_radar_view_class : public i4_parent_window_class
{
public:
  g1_radar_view_class *next;
  i4_image_class *background;
  i4_bool grabbing;
  g1_radar_params_struct setup;
  int flags;
  i4_bool restore_strategy_on_top;
  int grab_x, grab_y, grab_x_end, grab_y_end;

  struct controller_win
  {
    int x,y,w,h;
  };

  g1_radar_view_class(w16 w, w16 h, int flags)

    : i4_parent_window_class(w,h), 
      flags(flags)
  {
    restore_strategy_on_top=i4_F;
    next=list;
    list=this;
    background=0;

    grabbing=i4_F;

    recalc_background();
  }
  

  void recalc_background()
  {
    if (background)
    {
      delete background;
      background=0;
    }

    if (g1_map_is_loaded())
    {
      background=g1_create_map_image(width(), height(), flags & G1_RADAR_INTERLACED);
      setup.init(background->width(), background->height());
    }
  }

  void push_in(float &gx, float &gy)
  {
    enum {B=6};
    if (gx<B) gx=B;
    if (gx>g1_get_map()->width()-1-B) gx=g1_get_map()->width()-1-B;
    if (gy<B) gy=B;
    if (gy>g1_get_map()->height()-1-B) gy=g1_get_map()->height()-1-B;
  }

  void receive_event(i4_event *ev)
  {
    switch (ev->type())
    {
      case i4_event::MOUSE_BUTTON_DOWN:
      {
        CAST_PTR(bev, i4_mouse_button_down_event_class, ev);
        float gx,gy;
        setup.mouse_2_game(bev->x, bev->y, gx,gy);
        push_in(gx,gy);


        if (bev->left())
        {

          if (flags & G1_RADAR_CLICK_SELECTS_PATH)
          {
            g1_human->player_clicked(0, gx,gy);
            request_redraw(i4_F);
          }
          else if (g1_current_view_state())
          {
            if (!(flags & G1_RADAR_CLICK_HOLDS_VIEW))          
            {
              g1_current_view_state()->suggest_camera_mode(G1_STRATEGY_MODE);
              if (g1_border->strategy_on_top)
              {
                restore_strategy_on_top=i4_T;
                li_call("strategy_on_bottom");
              }

            }

            g1_current_view_state()->set_camera_position(gx,gy);

            grab_x=grab_x_end=bev->x;
            grab_y=grab_y_end=bev->y;

            i4_window_request_mouse_grab_class grab(this);
            i4_kernel.send_event(parent, &grab);
            grabbing=i4_T;

          }
        }
        else if (bev->right() && g1_border.get() && g1_border->strategy_on_top)
          g1_current_view_state()->set_camera_position(gx,gy);


      } break;


      case i4_event::MOUSE_BUTTON_UP :
      {
        CAST_PTR(bev, i4_mouse_button_down_event_class, ev);
        if (bev->left())
        {
          if (grabbing)
          {
            if (!(flags & G1_RADAR_CLICK_HOLDS_VIEW))          
            {
              if (restore_strategy_on_top)
              {
                restore_strategy_on_top=i4_F;
                li_call("strategy_on_top");
              }

              g1_current_view_state()->suggest_camera_mode(G1_ACTION_MODE);
            }

            i4_window_request_mouse_ungrab_class ungrab(this);
            i4_kernel.send_event(parent,&ungrab);
            grabbing=i4_F;
          }
        }
      } break;

      case i4_event::MOUSE_MOVE :
      {
        CAST_PTR(bev, i4_mouse_move_event_class, ev);
        float gx,gy,mx,my;

        if (grabbing)
        {
          grab_x_end+=bev->x-bev->lx;
          grab_y_end+=bev->y-bev->ly;

          float mouse_move_scale=width()/5000.0;
          
          mx=((grab_x_end-grab_x) * mouse_move_scale + grab_x);
          my=((grab_y_end-grab_y) * mouse_move_scale + grab_y);
        }
        else 
        {
          mx=bev->x;
          my=bev->y;
        }
        

//         if ((flags & G1_RADAR_CLICK_HOLDS_VIEW)==0 || grabbing)
//         {
//           setup.mouse_2_game(mx, my, gx,gy);
//           push_in(gx,gy);

//           if ((flags & G1_RADAR_CLICK_HOLDS_VIEW)==0)
//             g1_current_view_state()->suggest_camera_mode(G1_STRATEGY_MODE);

//           g1_current_view_state()->set_camera_position(gx,gy);          
//         }

      } break;

      //      case i4_event::WINDOW_MESSAGE:
//       {
//         CAST_PTR(wev, i4_window_message_class, ev);
//         if (wev->sub_type==i4_window_message_class::LOST_MOUSE_FOCUS &&
//             (flags & G1_RADAR_CLICK_HOLDS_VIEW)==0)          
//           g1_current_view_state()->suggest_camera_mode(G1_ACTION_MODE);
//         else if (wev->sub_type==i4_window_message_class::GOT_MOUSE_FOCUS &&
//                  (flags & G1_RADAR_CLICK_HOLDS_VIEW)==0)
//           g1_current_view_state()->suggest_camera_mode(G1_STRATEGY_MODE);
//         else
//           i4_parent_window_class::receive_event(ev);
//       } break;

      default:
        i4_parent_window_class::receive_event(ev);
    }

  }



  void draw_all_paths(g1_team_type team, 
                      g1_path_object_class *po, 
                      w32 color,
                      i4_draw_context_class &context)
  {
    if (po)
    {
      int ix=i4_f_to_i(po->x), iy=i4_f_to_i(po->y);
      if ((flags & G1_RADAR_DRAW_ALL_PATHS) ||
          (g1_cells[ix+iy*g1_map_width].flags & g1_map_cell_class::FOGGED)==0)
      {
        int t=po->total_links(team);
        for (int k=0; k<t; k++)
        {
          g1_path_object_class *p2=po->get_link(team,k);
          if (p2)
          {
            float x1,y1,x2,y2;
            setup.game_2_mouse(po->x, po->y, x1,y1);
            setup.game_2_mouse(p2->x, p2->y, x2,y2);
            local_image->line(i4_f_to_i(x1), i4_f_to_i(y1),
                              i4_f_to_i(x2), i4_f_to_i(y2),
                              color, context);

            draw_all_paths(team, p2, color, context);
          }
        }
      }
    }
  }




  void draw_recent_path(g1_team_type team, 
                      g1_path_object_class *po, 
                      w32 color,
                         i4_draw_context_class &context)
  {
    if (po)
    {
      int ix=i4_f_to_i(po->x), iy=i4_f_to_i(po->y);
      if ((flags & G1_RADAR_DRAW_ALL_PATHS) ||
          (g1_cells[ix+iy*g1_map_width].flags & g1_map_cell_class::FOGGED)==0)
      {
        g1_path_object_class *p2=po->get_recent_link(team, 0);
        if (p2)
        {
          float x1,y1,x2,y2;
          setup.game_2_mouse(po->x, po->y, x1,y1);
          setup.game_2_mouse(p2->x, p2->y, x2,y2);
          local_image->line(i4_f_to_i(x1), i4_f_to_i(y1),
                            i4_f_to_i(x2), i4_f_to_i(y2),
                            color, context);

          draw_recent_path(team, p2, color, context);
        }
      }
    }
  }

 



  void parent_draw(i4_draw_context_class &context)
  {
    if (background)
    {
      if (flags & G1_RADAR_USE_DIRTIES)
      {
        i4_rect_list_class::area_iter i;
        for (i=undrawn_area.list.begin(); i!=undrawn_area.list.end(); ++i)
        {
          int x=i->x1, y=i->y1;      
          background->put_part(local_image, x,y, i->x1, i->y1, i->x2, i->y2, context);
        }       
      }
      else if (background)    
        background->put_image(local_image,0,0, context);

      int radius[5]={0, (int)(0.5 * setup.g2mx), (int)(1.0 * setup.g2mx), 
                       (int)(0.1 * setup.g2mx), 
                       (int)(2.0 * setup.g2mx)};
      float ix,iy;

      int map_width=g1_get_map()->width();


      int i;
      w32 color;
      for (i=0; i<G1_MAX_PLAYERS; i++)
      {
        g1_team_type team=g1_player_man.get(i)->get_team();

        color=g1_player_man.get(i)->map_player_color;

        i4_array<w32> &a=g1_player_man.get(i)->owned_objects;
        int t=a.size();
        for (int j=0; j<t; j++)
        {
          g1_object_class *p=g1_global_id.checked_get(a[j]);
          
          if (p && p->radar_type!=G1_RADAR_NONE)
          {            
            int cx=i4_f_to_i(p->x);
            int cy=i4_f_to_i(p->y);

            if ((g1_cells[map_width * cy + cx].flags & g1_map_cell_class::FOGGED)==0)
            {
              setup.game_2_mouse(p->x, p->y, ix,iy);

              
              if ((flags & G1_RADAR_USE_ICONS)==0 || !p->radar_image)
              {
                int r=radius[p->radar_type];
                
                int ix1=i4_f_to_i(ix)-r, iy1=i4_f_to_i(iy)-r;
                int ix2=i4_f_to_i(ix)+r, iy2=i4_f_to_i(iy)+r;
                local_image->bar(ix1-1,iy1-1,ix2+1,iy2+1, 0, context);
                local_image->bar(ix1,iy1,ix2,iy2, color, context);
              }
            }            
          } 
        }
      }

      g1_team_type my_team=g1_player_man.get_local()->get_team();

      if (flags & G1_RADAR_DRAW_UNHIDDEN_PATHS)
      {
        g1_factory_class *f;
        for (f=g1_factory_list.first(); f; f=f->next)
        {
          if (g1_player_man.get(f->player_num)->get_team()==my_team)
          {
            g1_path_object_class *start=f->get_start();
            if (start)
            {
              draw_all_paths(my_team, start, f->get_path_color(),
                             context);

              draw_recent_path(my_team, start, f->get_selected_path_color(),
                               context);
            }
          }
        }
      }

      if (flags & G1_RADAR_USE_ICONS)
      {
        for (i=0; i<G1_MAX_PLAYERS; i++)
        {
          i4_array<w32> &a=g1_player_man.get(i)->owned_objects;
          int t=a.size();

          for (int j=0; j<t; j++)
          {
            g1_object_class *p=g1_global_id.checked_get(a[j]);

            if (p &&  p->radar_image)
            {
              int cx=i4_f_to_i(p->x);
              int cy=i4_f_to_i(p->y);

              if ((g1_cells[map_width * cy + cx].flags & g1_map_cell_class::FOGGED)==0)
              {
                setup.game_2_mouse(p->x, p->y, ix,iy);


                i4_image_class *i=p->radar_image->tinted_icons[p->player_num];
                int ix1=i4_f_to_i(ix)-i->width()/2;
                int iy1=i4_f_to_i(iy)-i->height()/2;

                i->put_image(local_image, ix1,iy1,context);
              }
            }
          }
        }
      }

                         
      //      draw_paths();


    } else local_image->clear(0, context);
  }


  void refresh_area(int gx1, int gy1, int gx2, int gy2)
  {
    g1_render_map_area(background,
                       &setup, gx1,gy1,gx2,gy2,
                       0, (flags & G1_RADAR_INTERLACED) ? i4_T : i4_F);

                    

    request_redraw(i4_F);
  }


  ~g1_radar_view_class()
  {
    if (background)
    {
      delete background;
      background=0;
    }

    if (this==list)
      list=list->next;
    else
    {
      g1_radar_view_class *p=list;
      while (p->next!=this)
        p=p->next;
      p->next=next;
    }
  }

  char *name() { return "radar_view"; }
};





void g1_radar_recalculate_backgrounds()
{
  for (g1_radar_view_class *v=list; v; v=v->next)
    v->recalc_background();
}

void g1_radar_looking_at(float x1, float y1, float x2, float y2)
{
}


i4_parent_window_class *g1_create_radar_view(int max_w, int max_h, int flags)
{
  g1_radar_view_class *rv=new g1_radar_view_class(max_w, max_h, flags);
  return rv;
}



void g1_radar_refresh(int game_x1, int game_y1, int game_x2, int game_y2)
{
  for (g1_radar_view_class *v=list; v; v=v->next)
    v->refresh_area(game_x1, game_y1, game_x2, game_y2);
}

void g1_radar_update()
{
  for (g1_radar_view_class *v=list; v; v=v->next)
    v->request_redraw(i4_F);
}


void g1_unfog_radius(const i4_3d_vector &v, float r)
{
  int fog_rect_x1=10000, fog_rect_y1=10000,
      fog_rect_x2=-1, fog_rect_y2=-1, ix,iy;
  
  g1_map_class *map=g1_get_map();

  int x_left   = i4_f_to_i(v.x-r); if (x_left<0)                x_left=0;
  int x_right  = i4_f_to_i(v.x+r); if (x_right>=map->width())   x_right=map->width()-1;
  int y_top    = i4_f_to_i(v.y-r); if (y_top<0)                 y_top=0;
  int y_bottom = i4_f_to_i(v.y+r); if (y_bottom>=map->height()) y_bottom=map->height()-1;  

  for (iy=y_top;  iy<=y_bottom; iy++)
  {
    g1_map_cell_class *c=map->cell(x_left,iy);
    for (ix=x_left; ix<=x_right;  ix++)
    {
      if (c->flags & g1_map_cell_class::FOGGED)
      {
        c->unfog(ix, iy);
        if (ix<fog_rect_x1) fog_rect_x1=ix;
        if (ix>fog_rect_x2) fog_rect_x2=ix;
        if (iy<fog_rect_y1) fog_rect_y1=iy;
        if (iy>fog_rect_y2) fog_rect_y2=iy;
      }
	  c++;
    }
      
  }
  
  if (fog_rect_x2!=-1)
    g1_radar_refresh(fog_rect_x1, fog_rect_y1, fog_rect_x2, fog_rect_y2);

}
