/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "r1_api.hh"
#include "r1_win.hh"
#include "image/image.hh"
#include "palette/pal.hh"
#include "threads/threads.hh"
#include "tmanage.hh"
#include "time/time.hh"

// this lock prevents 2 threads from drawing at the same time
static i4_critical_section_class softz_lock;
static int defer_draws=0;

static i4_pixel_format softz_pixel_fmt;
class r1_softz_window;
static r1_softz_window *cur_win=0;

class r1_softz_window : public r1_render_window_class
{
public:
  i4_image_class *im;
  float *zbuffer;
  i4_draw_context_class ctext;
  volatile int rendering;
 
  r1_softz_window(int w, int h, r1_expand_type type);
  void clear_z(int x1, int y1, int x2, int y2, float z)
  {
    float *s=zbuffer + y1*width()+x1;
    int t=(x2-x1+1);
    for (int y=y1; y<=y2; y++)
    {
      for (int i=0; i<t; i++)
        s[i]=z;
      s+=width();
    }
  }

  void draw(i4_draw_context_class &context)
  {
    if (!rendering)
      im->put_image(local_image, 0,0, context);
    r1_render_window_class::draw(context);
    
  }

  virtual void begin_render()
  {
    softz_lock.lock();
    rendering=1;
    cur_win=this;
    if (cur_win->width()!=cur_win->im->width() ||
        cur_win->height()!=cur_win->im->height())
      i4_error("image wrong size");
        
        
  }


  virtual void end_render()
  {
    cur_win=0;
    rendering=0;
    softz_lock.unlock();
  }

  char *name() { return "softz_window"; }

  virtual void private_resize(w16 new_width, w16 new_height)
  {
    r1_render_window_class::private_resize(new_width, new_height);

    delete im;
    i4_free(zbuffer);
    zbuffer=(float *)i4_malloc(new_width* new_height*sizeof(float), "");  
    im=i4_create_image(new_width, new_height, i4_pal_man.register_pal(&softz_pixel_fmt));
  }

  ~r1_softz_window()
  {
    while (rendering)
    {
      i4_warning("waiting on render to finish");
      i4_sleep(1);
    }


    delete im;
    i4_free(zbuffer);
  }
};



class r1_softz_tman : public r1_texture_manager_class
{  
public:
  r1_softz_tman(const i4_pal *pal) : r1_texture_manager_class(pal) {}

  virtual void init() {}
  virtual void uninit() {}

  virtual void next_frame() {}

  virtual void reset() {}

  virtual void toggle_texture_loading() {}
  
  virtual r1_miplevel_t *get_texture(r1_texture_handle handle,
                                     w32 frame_counter,
                                     sw32 desired_width,
                                     sw32 &w, sw32 &h)
  {
    return 0;
  }

  virtual i4_bool valid_handle(r1_texture_handle handle)
  {
    return i4_F;
  }

public:
  virtual i4_bool immediate_mip_load(r1_mip_load_info *load_info) { return i4_T; }

  virtual i4_bool async_mip_load(r1_mip_load_info *load_info) { return i4_T; }

  virtual void free_mip(r1_vram_handle_type vram_handle) {}
};


class r1_softz_api : public r1_render_api_class
{

public:
  char *name() { return "Software Z Buffer"; }

  virtual void use_texture(r1_texture_handle material_ref, 
                           sw32 desired_width,
                           w32 frame)
  {}

  virtual void disable_texture() {}
  virtual void set_z_range(float near_z, float far_z) {}

  virtual void render_poly(int t_verts, r1_vert *verts)
  {
    render_lines(t_verts-1, verts);
    r1_vert l[2];
    l[0]=verts[0];
    l[1]=verts[t_verts-1];
    render_lines(1, l);
  }

  r1_softz_api()
  {
    softz_pixel_fmt.default_format();
    softz_pixel_fmt.alpha_mask=0;
    softz_pixel_fmt.calc_shift();
  }

  i4_bool init(i4_display_class *display)
  {
    tmanager=new r1_softz_tman(display->get_palette());
    return i4_T;
  }

  
  void uninit()
  {
    delete tmanager;
  }

  virtual void render_pixel(r1_vert *pixel)
  {
    int iw=cur_win->width();
    float *zbuffer=cur_win->zbuffer;
    int off=(int)pixel->px + ((int)pixel->py)*iw;

    if (write_mask & R1_COMPARE_W)
      if (zbuffer[off]<=pixel->v.z)
        return ;
     

    w32 *im_data=(w32 *)cur_win->im->data;

    if (write_mask & R1_WRITE_COLOR)
    {
      int r=(int)(pixel->r*255.0),
          g=(int)(pixel->g*255.0),
          b=(int)(pixel->b*255.0);
        
      w32 c=(r<<16)|(g<<8)|b;
      im_data[off]=c;
    }

    if (write_mask & R1_WRITE_W)   
      zbuffer[off]=pixel->v.z;

  }

  virtual void render_lines(int t_lines, r1_vert *verts)
  {
    w32 *im_data=(w32 *)cur_win->im->data;
    int iw=cur_win->im->width();
    float *zbuffer=cur_win->zbuffer;

    for (int i=0; i<t_lines; i++)
    {
      float x=verts[i].px, y=verts[i].py, z=verts[i].v.z;
      float xd=(int)fabs(verts[i+1].px-x), yd=(int)fabs(verts[i+1].py-y);
      int steps = xd>yd ? xd+1 : yd+1;
      float r=verts[i].r, g=verts[i].g, b=verts[i].b;

      float rs=(verts[i+1].r-r)/(float)steps;
      float gs=(verts[i+1].g-g)/(float)steps;
      float bs=(verts[i+1].b-b)/(float)steps;
      float xs=(verts[i+1].px-x)/(float)steps;
      float ys=(verts[i+1].py-y)/(float)steps;
      float zs=(verts[i+1].v.z-z)/(float)steps;

      for (int j=0; j<steps; j++)
      {
        int off=(int)x + ((int)y)*iw;

        if ((write_mask & R1_COMPARE_W)==0 || zbuffer[off]>z)
        {

          if (write_mask & R1_WRITE_COLOR)
          {
            int ir=(int)(r*255.0), ig=(int)(g*255.0), ib=(int)(b*255.0);
            w32 c=(ir<<16)|(ig<<8)|ib;
            im_data[off]=c;

            r+=rs;
            g+=gs;
            b+=bs;
          }
        
          if (write_mask & R1_WRITE_W)
            zbuffer[off]=z;     
        }

        x+=xs;
        y+=ys;
        z+=zs;          
    
      }     
    }
  }

  virtual i4_image_class *create_compatible_image(w16 w, w16 h)
  {
    return i4_create_image(w,h, i4_pal_man.register_pal(&softz_pixel_fmt));
  }

  virtual void clear_area(int x1, int y1, int x2, int y2, w32 color, float z)
  {
    if (write_mask & R1_WRITE_COLOR)
      cur_win->im->bar(x1,y1,x2,y2, color, cur_win->ctext);
    
    if (write_mask & R1_WRITE_W)
      cur_win->clear_z(x1,y1,x2,y2, z);
  }



  virtual r1_render_window_class *create_render_window(int visable_w, int visable_h,
                                                       r1_expand_type type)
  {
    return new r1_softz_window(visable_w, visable_h, type);
  }

  virtual void copy_part(i4_image_class *im,                                          
                         int x, int y,             // position on screen
                         int x1, int y1,           // area of image to copy 
                         int x2, int y2) {}
};

static r1_softz_api softz;



r1_softz_window::r1_softz_window(int w, int h, r1_expand_type type)
  : r1_render_window_class(w,h, type, &softz),
    ctext(0,0,w-1,h-1)
{
  zbuffer=(float *)i4_malloc(w*h*sizeof(float), "");
  im=i4_create_image(w,h, i4_pal_man.register_pal(&softz_pixel_fmt));
  rendering=0;
}

