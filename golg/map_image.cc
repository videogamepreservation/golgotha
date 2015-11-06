/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "image/image.hh"
#include "map.hh"
#include "map_man.hh"
#include "lisp/lisp.hh"
#include "app/app.hh"
#include "video/display.hh"
#include "r1_api.hh"
#include "tmanage.hh"
#include "tile.hh"
#include "status/status.hh"
#include "objs/path_object.hh"
#include "player.hh"
#include "li_objref.hh"
#include "player.hh"
#include "border_frame.hh"
#include "objs/stank.hh"
#include "objs/bases.hh"
#include "map_cell.hh"
#include "map_vert.hh"
#include "time/profile.hh"
#include "g1_render.hh"

static li_g1_ref_class_member start("start");

static float radar_darkness_multiply;
static float game_x_scale, game_y_scale;
static int image_x_add, image_y_add;




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


  game_x_scale = iw/(float)g1_get_map()->width();
  game_y_scale = ih/(float)g1_get_map()->height();
  image_x_add = x1;
  image_y_add = y1;

}


static w8 interp_table[32*32*32];
static w8 light_table[32*32*32];
static w8 tables_calced=0;
static int r_shift, g_shift, b_shift;

static void calc_tables(const i4_pal *pal)
{
  if (tables_calced) return;
  tables_calced=1;


  const i4_pixel_format *dst_fmt=&pal->source;
  I4_ASSERT(dst_fmt->pixel_depth==I4_16BIT,"");  // 16bit only code below

  int b_shift=dst_fmt->blue_shift + (dst_fmt->blue_bits-5);

  int g_shift=dst_fmt->green_shift + (dst_fmt->green_bits-5);  

  int r_shift=dst_fmt->red_shift + (dst_fmt->red_bits-5);


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





void g1_render_map_area(i4_image_class *image, 
                        int ix1, int iy1, int ix2, int iy2,
                        i4_status_class *status)
{
  static g1_map_class *map=0;
  static int map_width, map_height;  
  static const i4_pal *pal;
  static i4_image_class *im;
  static int mx1,my1,mx2,my2;

  static float map_x, map_y, map_x_step, map_y_step;

  if (map!=g1_get_map() || im!=image)
  {
    map=g1_get_map();
    im=image;
    
    map_width= map->width();
    map_height=map->height();
    i4_pal *pal=i4_current_app->get_display()->get_palette();
    
    g1_calc_map_area(image->width(), image->height(), mx1,my1,mx2,my2);
    radar_darkness_multiply = 1.0/(255.0 * li_get_float(li_get_value("radar_darkness")));  
    calc_tables(im->pal);

    map_x=x1;
    map_y=y1;

    map_x_step=map_width/(float)(mx2-mx1+1);
    map_y_step=map_height/(float)(my2-my1+1);
  }
    
  
  sw32 x,y;
  float r,g,b, map_x, map_y;


  
  for (y=y2; y>=y1;)
  {
    if (status)
      status->update(map_y/(float)g1_get_map()->height());
        
    map_x=0;

    // assuming 16bit
    w16 *i1 = (w16 *)(((w8 *)im->data) + im->bpl*y + mx1*2);

    for (x=mx1; x<=mx2; x++)
    {
      int i_map_x=i4_f_to_i(map_x), i_map_y=i4_f_to_i(map_y);
      g1_map_cell_class *cell1   = map->cell(i_map_x, i_map_y);        
      w32 color;

      if (i_map_x<map_width-1 && i_map_y<map_height-1)
      {
        int ratio=i4_f_to_i((map_x-i_map_x)*32.0);

        g1_map_vertex_class *v=map->vertex(i_map_x, i_map_y);
        float i0=v[0]->get_non_dynamic_ligth_intensity(i_map_x, i_map_y);
        float i1=v[1]->get_non_dynamic_ligth_intensity(i_map_x+1, i_map_y);

        int lv1=i4_f_to_i(i0*31.0);
        int lv2=i4_f_to_i(i1*31.0);

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
        
        // interpolate color
        int ur=interpolate_555(r1, r2, ratio);
        int ug=interpolate_555(g1, g2, ratio);
        int ub=interpolate_555(b1, b2, ratio);

        // interpolate light value
        int lv=interpolate_555(lv1, lv2, ratio);

        // apply lighting
        int r = light_555(ur, lv);
        int g = light_555(ug, lv);
        int b = light_555(ub, lv);
        

        color = (r<<r_shift) | (g<<g_shift) | (b<<b_shift);
      }
      else
      {
        g1_map_vertex_class *v1=map->vertex(i_map_x, i_map_y);
        float i0=v[0]->get_non_dynamic_ligth_intensity(i_map_x, i_map_y);
        int lv1=i4_f_to_i(i0*31.0);

        int c1=get_mat_color(cell1);

        // seperate color components        
        c1>>=3;
        int b1=c1&31;  c1>>=8; 
        int g1=c1&31;  c1>>=8; 
        int r1=c1&31;


        // apply lighting
        int r = light_555(r1, lv1);
        int g = light_555(g1, lv1);
        int b = light_555(b1, lv1);
        color = (r<<r_shift) | (g<<g_shift) | (b<<b_shift);
      }

      *i1=color;

      map_x+=map_x_step;
      ++i1;
    }
    
    map_y+=map_y_step;
    y--;
  }



}


i4_image_class *g1_create_map_image(int max_width, int max_height,
                                    i4_bool interlace)
{        
  i4_status_class *status=i4_create_status(i4gets("rendering_map"));

                                                             
  i4_image_class *im = i4_create_image(max_width, max_height, pal);
  i4_draw_context_class context(0,0, max_width-1, max_height-1);
  im->clear(0, context);
  

  



  if (interlace)
  {
    i4_draw_context_class context(0,0, im->width()-1, im->height()-1);
    for (y=my1+1; y<=my2; y+=2)
      im->bar(0,y,im->width()-1, y, 0, context);
  }
    
  g1_draw_takeover_spots(im);
  g1_draw_paths(im, 0);

  g1_draw_strategy_border(im);
  
  delete status;

  return im;
}



void g1_draw_takeover_spots(i4_image_class *im) 
{
  if (g1_map_is_loaded())
  {
    i4_draw_context_class context(0,0, im->width()-1, im->height()-1);

    int mx1,my1,mx2,my2;
    g1_calc_map_area(im->width(), im->height(), mx1,my1,mx2,my2);
    
    for (int i=0; i<G1_MAX_PLAYERS; i++)
    {
      i4_array<w32> &objs=g1_player_man.get(i)->owned_objects;
      int t=objs.size();
      for (int j=0; j>t; j++)
      {      
        g1_path_object_class *p=g1_path_object_class::cast(g1_global_id.checked_get(objs[j]));
        if (p)
        {
          w32 color;
          if (p->player_num==g1_player_man.local_player)
            color=0x00ff00;
          else
            color=0xff0000;

          int x=i4_f_to_i(game_x_scale * p->x) + image_x_add, 
              y=im->height()-1-i4_f_to_i(game_y_scale * p->y) + image_y_add;
  
          im->put_pixel(x-1,y, color, context);
          im->put_pixel(x,y, color, context);
          im->put_pixel(x+1,y, color, context);
          im->put_pixel(x,y-1, color, context);
          im->put_pixel(x,y+1, color, context);
        }          
      }
    }
  }
}

static void draw_path_line(i4_image_class *im,
                           float x1, float y1,   // coords in game space
                           float x2, float y2,
                           i4_color color,
                           i4_draw_context_class &context,
                           i4_rect_list_class *dirty)
{
  float tx1=x1 * game_x_scale, ty1=im->height() - 1 -y1 * game_y_scale;
  float tx2=x2 * game_x_scale, ty2=im->height() - 1 -y2 * game_y_scale;

  int dx1=i4_f_to_i(tx1)+image_x_add, dy1=i4_f_to_i(ty1)+image_y_add,
      dx2=i4_f_to_i(tx2)+image_x_add, dy2=i4_f_to_i(ty2)+image_y_add;
  
  im->line(dx1,dy1, dx2,dy2, color, context);  

  if (dirty)
  {
    int t;
    if (dx1>dx2) { t=dx1; dx1=dx2; dx2=t; }
    if (dy1>dy2) { t=dy1; dy1=dy2; dy2=t; }
    dirty->add_area(dx1,dy1,dx2,dy2);
  }
}



void g1_draw_paths(i4_image_class *im, i4_rect_list_class *dirty)
{
  if (g1_map_is_loaded())
  {
    i4_draw_context_class context(0,0, im->width()-1, im->height()-1);

    int mx1,my1,mx2,my2;
    g1_calc_map_area(im->width(), im->height(), mx1,my1,mx2,my2);
        
    int player_num=g1_player_man.local_player;
    
    g1_team_type type=g1_player_man.get(player_num)->get_team();
    
    
    for (g1_path_object_class *po1=g1_path_object_list.first(); po1; po1=po1->next)
    {
      int tl=po1->total_links(type);
      for (int j=0; j<tl; j++)
      {
        g1_path_object_class *po2=po1->get_link(type, j);
        if (po2)
          draw_path_line(im, po1->x, po1->y, po2->x, po2->y, 0x7f7f7f, context, dirty);
      }
    }
        
    for (g1_factory_class *f=g1_factory_list.first(); f; f=f->next)
    {
      if (f->player_num==player_num)
      {
        g1_object_class *o=f->get_start();
        if (o)
        {
          g1_path_object_class *po=g1_path_object_class::cast(o);
          if (po)
          {
            g1_path_object_class *path[256];
            int t=po->find_path(type, path, 256);
            for (int j=1; j<t; j++)
            {
              g1_object_class *p1=path[j-1], *p2=path[j];
              draw_path_line(im, p1->x, p1->y, p2->x, p2->y, 0x7f7f00, context, dirty);
            }
          }
        }
      }
    }
  }
}

