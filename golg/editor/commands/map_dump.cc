/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "map_cell.hh"
#include "map_vert.hh"
#include "map_man.hh"
#include "lisp/lisp.hh"
#include "lisp/li_init.hh"
#include "map.hh"
#include "objs/path_object.hh"
#include "g1_render.hh"
#include "r1_api.hh"
#include "tmanage.hh"
#include "tile.hh"
#include "app/app.hh"
#include "window/window.hh"
#include "r1_win.hh"
#include "loaders/tga_write.hh"
#include "window/wmanager.hh"



static float tri1_s[8*3]={0,1,1, 1,1,0, 1,0,0, 0,0,1, 0,0,1, 1,0,0, 1,1,0, 0,1,1};
static float tri1_t[8*3]={1,1,0, 1,0,0, 0,0,1, 0,1,1, 1,0,0, 1,1,0, 0,1,1, 0,0,1};

static float tri2_s[8*3]={0,1,0, 1,0,0, 1,0,1, 0,1,1, 0,1,1, 1,0,1, 1,0,0, 0,1,0};
static float tri2_t[8*3]={1,0,0, 1,0,1, 0,1,1, 0,1,0, 1,0,1, 1,0,0, 0,1,0, 0,1,1};


class map_renderer_class : public i4_window_class
{
public:
  i4_bool do_it_again;
  int ax1, ay1, ax2, ay2;

  map_renderer_class(int w, int h) : i4_window_class(w,h)
  {
    do_it_again=i4_T;
  }

  void request_redraw(i4_bool for_a_child=i4_F) 
  { 
    i4_window_class::request_redraw(for_a_child);
    do_it_again=i4_T;
  }

  void draw(i4_draw_context_class &context)
  {
    r1_render_api_class *api=g1_render.r_api;
    r1_texture_manager_class *tman=api->get_tmanager();
    
    do_it_again=i4_F;
    
    float s, t, s_step, t_step;
    s_step=width()/(float)(ax2-ax1);
    t_step=height()/(float)(ay2-ay1);

    api->default_state();
    api->clear_area(0,0,width()-1,height()-1, 0, 100);  

    api->set_filter_mode(R1_NO_FILTERING);

    int x,y;
    t=0;
    for (y=ay1; y<ay2; y++, t+=t_step)
    {
      s=0;
      for (x=ax1; x<ax2; x++, s+=s_step)
      {
        g1_map_cell_class *c=g1_cells + g1_map_width*y+x;
        g1_map_vertex_class *v1=g1_verts + (g1_map_width+1)*y+x, *v2,*v3,*v4;
        v2=v1+1;
        v3=v2+g1_map_width+1;
        v4=v3-1;

        int texture=g1_tile_man.get(c->type)->texture;
        api->use_texture(texture, (int)s_step, 0);

        int st_index=c->get_rotation();
        if (c->mirrored())
          st_index+=4;

        st_index*=3;

        r1_vert v[4];
  
        v[0].px=s;              v[0].py=t;
        v[1].px=s+s_step;       v[1].py=t;
        v[2].px=s+s_step;       v[2].py=t+t_step;
        v[3].px=s;              v[3].py=t+t_step;
  

        v[0].s=tri1_s[st_index];      v[0].t=tri1_t[st_index];
        v[1].s=tri1_s[st_index+1];    v[1].t=tri1_t[st_index+1];
        v[2].s=tri1_s[st_index+2];    v[2].t=tri1_t[st_index+2];
        v[3].s=tri2_s[st_index+2];    v[3].t=tri2_t[st_index+2];

        float z=1.0;
        float w=1.0/z;
        v[0].w=w; v[0].v.z=z;
        v[1].w=w; v[1].v.z=z;
        v[2].w=w; v[2].v.z=z;
        v[3].w=w; v[3].v.z=z;

        v[0].a=v[1].a=v[2].a=v[3].a=1;

#if 0
        v[0].r=v[0].g=v[0].b=v1->get_non_dynamic_ligth_intensity(x,y);
        v[1].r=v[1].g=v[1].b=v2->get_non_dynamic_ligth_intensity(x+1,y);
        v[2].r=v[2].g=v[2].b=v3->get_non_dynamic_ligth_intensity(x+1,y+1);
        v[3].r=v[3].g=v[3].b=v4->get_non_dynamic_ligth_intensity(x,y+1);
#else
        v[0].r = v[0].g = v[0].b = 1.0;
        v[1].r = v[1].g = v[1].b = 1.0;
        v[2].r = v[2].g = v[2].b = 1.0;
        v[3].r = v[3].g = v[3].b = 1.0;
#endif
        api->render_poly(4, v);
      }
    }

    api->set_filter_mode(R1_BILINEAR_FILTERING);
  }
  char *name() { return "renderer"; }
};


i4_image_class *render_map_section(int x1, int y1, int x2, int y2, int im_w, int im_h)
{
  r1_render_api_class *api=g1_render.r_api;

  r1_render_window_class *rwin=api->create_render_window(im_w, im_h);
  map_renderer_class *map_r=new map_renderer_class(im_w, im_h);

  map_r->ax1=x1;
  map_r->ay1=y1;
  map_r->ax2=x2;
  map_r->ay2=y2;


  rwin->add_child(0,0, map_r);
  i4_current_app->get_window_manager()->add_child(0,0,rwin);
  
  
  i4_draw_context_class context(0,0,im_w-1, im_h-1);

  i4_display_class *display=i4_current_app->get_display();
  
  int tries=0;
  r1_texture_manager_class *tman=api->get_tmanager();
  do
  {
    tman->next_frame();
    rwin->draw(context);
    display->flush();

    tries++;
    // repeat until textures have rez-ed in
    // or it doens't look like it'll happen
  } while (map_r->do_it_again && tries<100);



  i4_pixel_format fmt;
  fmt.default_format();
  fmt.alpha_mask=0;
  fmt.calc_shift();
  const i4_pal *pal=i4_pal_man.register_pal(&fmt);

  i4_image_class *fb;
  i4_image_class *to=0;

  fb=display->lock_frame_buffer(I4_BACK_FRAME_BUFFER, I4_FRAME_BUFFER_READ);
  if (fb)
  {
    to = i4_create_image(im_w, im_h, pal);
    fb->put_part(to, 0,0, 0,0, im_w-1, im_h-1, context);
    display->unlock_frame_buffer(I4_BACK_FRAME_BUFFER);
  }

  delete rwin;

  return to;
}


struct area
{
  int x1,y1,x2,y2;
  area(int x1, int y1, int x2, int y2) : x1(x1),y1(y1),x2(x2),y2(y2) {}
  area() {}
};

static i4_array<area *> *list;

static void split_gather(int x1, int y1, int x2, int y2, int level)
{
  if (list->size()>=6)
    return ;

  
  int xd=x2-x1+1, yd=y2-y1+1;
  if (xd>yd)
  {
    int xs=(x2+x1)/2;
    list->add(new area(x1, y1, xs, y2));
    list->add(new area(xs, y1, x2, y2));

    if (level!=1)
    {
      split_gather(x1, y1, xs, y2, level+1);
      split_gather(xs, y1, x2, y2, level+1);
    }
  }
  else
  {
    int ys=(y2+y1)/2;

    list->add(new area(x1, y1, x2, ys));
    list->add(new area(x1, ys, x2, y2));

    if (level!=1)
    {
      split_gather(x1, y1, x2, ys, level+1);
      split_gather(x1, ys, x2, y2, level+1);
    }
  }
}

li_object *g1_dump_level(li_object *o, li_environment *env)
{
  i4_file_class *fp=i4_open("dump_level", I4_WRITE);
  if (!fp) 
    return 0;
  g1_map_class *map=g1_get_map();

  int w=map->width(), h=map->height(),i,x,y;
  fp->write_32(0xabcf);   // version
  fp->write_16(w);
  fp->write_16(h);

  // save off a 1 pixel bitmap
  g1_map_cell_class *c=g1_cells;
  for (y=0; y<h; y++)
    for (x=0; x<w; x++, c++)
    {
      int type=g1_tile_man.get_texture(c->type);
      w32 color= g1_render.r_api->get_tmanager()->average_texture_color(type, 0);
      g1_map_vertex_class *v=g1_verts+x+y*(w+1);

      float tr,tg,tb;
      v->get_rgb(tr,tg,tb, x,y);

      int r=int(((color>>16)&0xff) * tr);
      int g=int(((color>> 8)&0xff) * tg);
      int b=int(((color>> 0)&0xff) * tb);


      fp->write_32((r<<16)|(g<<8)|b);
    }
  


  // save vert hights and normals
  g1_map_vertex_class *v=g1_verts;
  for (y=0; y<=h; y++)
    for (x=0; x<=w; x++, v++)
    {
      i4_3d_vector normal;
      v->get_normal(normal, x,y);

      fp->write_float(normal.x);
      fp->write_float(normal.y);
      fp->write_float(normal.z);
      fp->write_float(v->get_height());
      

      float r,g,b;
      v->get_rgb(r,g,b, x,y);
      fp->write_float(r);
      fp->write_float(g);
      fp->write_float(b);
    }


  i4_array<area *> mlist(0,32);

  // save cell texture names
  c=g1_cells;
  for (i=0; i<w*h; i++, c++)
  {
    int type=g1_tile_man.get_texture(c->type);
    char *tname=g1_render.r_api->get_tmanager()->get_texture_name(type);
    int len=strlen(tname)+1;
    fp->write_16(len);
    fp->write(tname,len);

    int flags=c->get_rotation();
    if (c->mirrored())
      flags|=4;

    fp->write_8(flags);
  }


  list=&mlist;
  int x1=0,y1=0, x2=g1_map_width, y2=g1_map_height;
  split_gather(x1,y1,x2,y2,0 );
  
  for (i=2; i<6; i++)
  {
    int x1=mlist[i]->x1, y1=mlist[i]->y1, x2=mlist[i]->x2, y2=mlist[i]->y2;

    printf("%d %d %d %d\n",x1,y1,x2,y2);
    i4_image_class *to;

    if (to = render_map_section(x1,y1,x2,y2, 256,256))
    {
      char fn[100];
      sprintf(fn,"x:/jc/lod_test/%d.tga", i-2);
      i4_file_class *fp=i4_open(fn, I4_WRITE);
      i4_tga_write(to, fp, 0);
      delete to;
    }
  }

  for (i=0; i<mlist.size(); i++)
    delete mlist[i];

  delete fp;
  return 0;
}

li_automatic_add_function(g1_dump_level, "dump_level");
