/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "app/app.hh"
#include "main/main.hh"
#include "window/window.hh"
#include "image/image.hh"
#include "palette/pal.hh"
#include "math/transform.hh"
#include "window/wmanager.hh"
#include "memory/malloc.hh"
#include "device/event.hh"
#include "gui/li_pull_menu.hh"
#include "menu/pull.hh"
#include "r1_api.hh"
#include "r1_win.hh"
#include "threads/threads.hh"
#include "device/keys.hh"
#include "window/win_evt.hh"
#include "r1_clip.hh"
#include "loaders/jpg_write.hh"
#include "gui/button.hh"
#include "gui/text.hh"

r1_render_api_class *api=0;

inline float frand() { return ((rand()%5000)-2500.0); }

enum {
  SAVE
};


struct camera_info
{
  i4_3d_vector position;
  i4_3d_vector rotation;

  void calc_transform(i4_transform_class &t)
  {
    t.identity();
    t.mult_rotate_x(rotation.x);
    t.mult_rotate_y(rotation.y);
    t.mult_rotate_z(rotation.z);
    t.mult_translate(position.x, position.y, position.z);

  }


  camera_info()
  {
    position=i4_3d_vector(0,0,1.5);
    rotation=i4_3d_vector(0, 0, 0);
  }
    
};

  
  


void render_thread_start(void *context);

#define S 2.2
enum {T_C=1 };

i4_3d_vector circles[T_C]={ i4_3d_vector(0,0,0)
                            //                            i4_3d_vector(-S,0,0)
//                             i4_3d_vector(S,0,0)


};
                            



float ray_sphere_intersect(i4_3d_vector ray0,   
                           i4_3d_vector ray_dir,
                           i4_3d_vector center,
                           float r)
{
  float i=ray_dir.x, j=ray_dir.y, k=ray_dir.z;
  float l=center.x, m=center.y, n=center.z;

  float a=i*i + j*j + k*k;

  float b=2*(i*(ray0.x - l) + j*(ray0.y -  m) + k*(ray0.z-n));

//   float c=l*l + m*m + n*n + ray0.x*ray0.x + ray0.y*ray0.y + ray0.z*ray0.z -
//           2*(l*ray0.x + m*ray0.y + n*ray0.z) - r*r;

  float c=(ray0.x-l)*(ray0.x-l) + (ray0.y-m)*(ray0.y-m) + (ray0.z-n)*(ray0.z-n) - r*r;



  float descriminant = b*b - 4*a*c;
  if (descriminant<0)
    return -1;
  
  if (descriminant==0)
    return -b/(2*a);
  else
  {
    float sd=sqrt(descriminant);
    float oo_2a=1.0/(2*a);
    float t1=(-b + sd ) * oo_2a;
    float t2=(-b - sd ) * oo_2a;
    if (t1>0)
    {
      if (t1<t2 || t2<0)
        return t1;
      else 
        return t2;
    }
    else return t2;
  }
}

float spline(float x, int nknots, float *knot)
{
  const float cr[16]={-0.5,  1.5, -1.5, 0.5,
                      1.0, -2.5,  2.0, -0.5,
                      -0.5,  0.0,  0.5, 0.0,
                      0.0,  1.0,  0.0, 0.0};

  int nspans=nknots-3;

  if (nspans<1)
  {
    i4_warning("spline has to few knots");
    return 0;
  }

  if (x<0) x=0;
  if (x>1) x=1;
  x*=nspans;

 int span=(int)x;
 if (span>=nknots-3)
   span=nknots-3;

 x-=span;
 knot+=span;

 float c3 = cr[0]*knot[0]  + cr[1]*knot[1]  + cr[2]*knot[2]  + cr[3]*knot[3];
 float c2 = cr[4]*knot[0]  + cr[5]*knot[1]  + cr[6]*knot[2]  + cr[7]*knot[3];
 float c1 = cr[8]*knot[0]  + cr[9]*knot[1]  + cr[10]*knot[2] + cr[11]*knot[3];
 float c0 = cr[12]*knot[0] + cr[13]*knot[1] + cr[14]*knot[2] + cr[15]*knot[3];

 return ((c3*x + c2)*x + c1)*x + c0;        
}


struct path_info
{
  i4_3d_vector start,
    pos,  // location in space where vector hit object 
    normal,          // normal at surface
    bump_normal,
    a,b,             // tangent vectors
    color;
};

class trace_view;
trace_view *trace=0;

class trace_view : public i4_window_class
{
public:
  i4_transform_class t;
  camera_info v;

  int iw,ih;
  i4_3d_vector light;
  int mouse_x, mouse_y;
  float theta;
  r1_render_window_class *rwin_parent;
  int rendering, abort_render, reset_render;
  i4_critical_section_class reset_lock;
  i4_bool mouse_look_grab;
  path_info path[10];
  int path_len;

  r1_render_window_class *rwin() { return rwin_parent; }

  ~trace_view()
  {
    if (rendering)
    {
      abort_render=1;
      while (rendering)
        i4_thread_yield();
    }
  }

  struct noise_struct
  {
    enum {TSIZE=256};
    float data[TSIZE];
    int init;
    noise_struct() { init=0; }

    float gnoise(int x, int y, int z)
    {
      return data[(x+(y*12345+((z*92831243)&(TSIZE-1))&(TSIZE-1))&(TSIZE-1))];
    }

    float bilinear_interp(float p1, float p2, float p3, float p4, float xr, float yr)
    {
      float t1=(p2-p1)*xr+p1;
      float t2=(p4-p3)*xr+p3;
      return (t2-t1)*yr+t1;
    }

    void init_noise()
    {
      if (!init)
      {
        init=1;
        for (int i=0; i<TSIZE; i++)
          data[i]=((rand()&0x7fff)/(double)0x7fff);
      }
    }

    float get(i4_3d_vector p)
    {
      init_noise();
      int ix=(int)p.x, iy=(int)p.y, iz=(int)p.z;      
      float xr=p.x-ix, yr=p.y-iy, zr=p.z-iz;
      float xknots[4], yknots[4], zknots[4];

      for (int k=-1; k<=2; k++)
      {
        for (int j=-1; j<=2; j++)
        {
          for (int i=-1; i<=2; i++)
            xknots[i+1]=gnoise(ix+i, iy+j, iz+k);
          yknots[j+1]=spline(xr, 4, xknots);
        }
        zknots[k+1]=spline(yr, 4, yknots);
      }
      return spline(zr, 4, zknots);
    }

    float get1d(float p)
    {
      init_noise();
      int ix=(int)p;
      float xr=p-ix;
      float xknots[4];

      for (int i=-1; i<=2; i++)
        xknots[i+1]=gnoise(ix+i, 0, 0);
      return spline(xr, 4, xknots);
    }



  } noise;


  float turbulance1d(float v)
  {
    float r=0, f=1.0;
    for (int t=0; t<4; t++)
    {
      r+=(noise.get1d(v)-0.5)/f;
      f*=2.17;
      v*=f;
    }

    r+=0.5;
    if (r<0) r=0;
    if (r>1) r=1;

    return r;
  }

  float turbulance(i4_3d_vector v, int octaves=4)
  {
    float r=0, f=1.0;
    for (int t=0; t<octaves; t++)
    {
      r+=(noise.get(v)-0.5)/f;
      f*=2.17;
      v*=f;
    }

    r+=0.5;
    if (r<0) r=0;
    if (r>1) r=1;

    return r;
  }

  void planet_color(float x, i4_3d_vector &c)
  {
    float b[5]={0.1, 0,    0.06,    0.0 };
    float g[5]={0.3, 0.2,   0.2,    0.6 };
    float r[5]={0.5, 0.8,   0.4,    0.7 };

    c.x=spline(x, 4,r);      c.y=spline(x, 4,g);    c.z=spline(x, 4,b);
  }




  void marble_color(float x, i4_3d_vector &c)
  {
    float b[5]={0.2,   0.12, 0.06,    0.10 };
    float g[5]={0.3,   0.20, 0.05,    0.31 };
    float r[5]={0.5,   0.29, 0.12,    0.11 };


//     x=1.0-x*x;  kiwi color
//     float b[5]={0.5,   0.3,  0.15,   0.1 };
//     float g[5]={0.5,   0.4,  0.2,   0.1 };
//     float r[5]={0.5,   0.3,  0.2,   0.5 };


    c.x=spline(x, 4,r)*2;  
    c.y=spline(x, 4,g)*2;
    c.z=spline(x, 4,b)*2;

  }


                   
  void shade_point(i4_3d_vector eye, path_info &p)
  {
    i4_3d_vector bump_pos=p.pos, bpa, bpb;
    bump_pos.x+=100;     bump_pos.y+=100;     bump_pos.z+=100;
    //    bump_pos*=4;
    

    bpa.x=bump_pos.x+p.a.x*0.0001;
    bpa.y=bump_pos.y+p.a.y*0.0001;
    bpa.z=bump_pos.z+p.a.z*0.0001;

    bpb.x=bump_pos.x+p.b.x*0.0001;
    bpb.y=bump_pos.y+p.b.y*0.0001;
    bpb.z=bump_pos.z+p.b.z*0.0001;

    float bump_h=turbulance(bump_pos,4); 
    float bump_dx=(turbulance(bpa,4)-bump_h)*2.0;
    float bump_dy=(turbulance(bpb,4)-bump_h)*2.0;

    p.bump_normal.x=p.normal.x + bpa.x*bump_dx + bpb.x*bump_dy;
    p.bump_normal.y=p.normal.y + bpa.y*bump_dx + bpb.y*bump_dy;
    p.bump_normal.z=p.normal.z + bpa.z*bump_dx + bpb.z*bump_dy;

    p.bump_normal.normalize();
    
       
    
    



    float c1=-p.bump_normal.dot(light);
    i4_3d_vector reflect=i4_3d_vector(light.x + 2*p.bump_normal.x*c1,
                                      light.y + 2*p.bump_normal.y*c1,
                                      light.z + 2*p.bump_normal.z*c1);
                                          


    float light_v=-light.dot(p.bump_normal)*0.4;

    i4_3d_vector view_dir=p.pos;
    view_dir-=eye;
    view_dir.normalize();
    float view_dot_r=-view_dir.dot(reflect);
    if (view_dot_r>=0)
    {
      light_v += pow(view_dot_r, 4)*0.5;
    }
    
    light_v+=0.5;  // ambient

    i4_3d_vector c=p.pos;
    c.x+=100; c.y+=100; c.z+=100;
    c*=4.0;
    float v=turbulance(c);
    //    v=((sin(c.x)+sin(c.y)+sin(c.z))*v/3.0);


    //    p.color=i4_3d_vector(v,v,v);

    /*
    i4_3d_vector c=p.pos;
    c.x+=10;
    c.y+=10;
    c.z+=10;

    c*=20.0;
    float v;


    float dist=c.length();
    float q1=cos(c.x+c.y+c.z)*2.0;
    float q2=sin(c.x-c.y-c.z)*2.0;
    float q3=sin(dist*4);


    v=turbulance(i4_3d_vector(c.x,c.y,c.z));

    v=((sin(c.x)*0.1 + q3*0.9)*0.5+0.5)*v;

      
    
    if (v>1 || v<0)
      i4_error("bad v");

    marble_color(v, p.color);
*/

    planet_color(v, p.color);

    p.color*=light_v;


    if (p.color.x<0) p.color.x=0;   
    if (p.color.x>1) p.color.x=1;
    if (p.color.y<0) p.color.y=0;   
    if (p.color.y>1) p.color.y=1;
    if (p.color.z<0) p.color.z=0;   
    if (p.color.z>1) p.color.z=1;
  }


  void calc_transform()
  {
    v.calc_transform(t);
    //    t.inverse_transform(i4_3d_vector(0,0,0), eye);
    request_redraw();
    render();
    

  }

  void move_cam(float x, float y, float z)
  {
    i4_3d_vector i,j,k;
    i4_3d_vector c;
    t.inverse_transform(i4_3d_vector(0,0,0), c);
    t.inverse_transform(i4_3d_vector(1,0,0), i); i-=c; i*=x;
    t.inverse_transform(i4_3d_vector(0,1,0), j); j-=c; j*=y;
    t.inverse_transform(i4_3d_vector(0,0,1), k); k-=c; k*=z;
   
    v.position-=i;
    v.position-=j;
    v.position-=k;

    calc_transform();
  }

  void rotate_cam(float x, float y, float z)
  {
    v.rotation.x+=x;
    v.rotation.y+=y;
    v.rotation.z+=z;
    calc_transform();
  }


  
  void trace_ray(i4_3d_vector eye, i4_3d_vector r0, i4_3d_vector ray_dir, 
                 int depth, 
                 float &z, 
                 path_info &cpath,
                 i4_bool store_path=i4_F)
  {
    if (depth>5)      
      cpath.color=i4_3d_vector(0,0,0);

    float closest_dist=10000;
    int closest=-1;
    
    for (int i=0; i<T_C; i++)
    {            
      float time=ray_sphere_intersect(r0, ray_dir, circles[i], 1);

      if (time>0.001 && time<closest_dist)
      {
        closest_dist=time;
        closest=i;
      }
    }

    if (closest!=-1)
    {
      i4_3d_vector i_spot=ray_dir, ti_spot;
      i_spot*=closest_dist;
      i_spot+=r0;

      i4_3d_vector cir=circles[closest];
      float cr_dist=sqrt((cir.x-i_spot.x)*(cir.x-i_spot.x)+
                         (cir.y-i_spot.y)*(cir.y-i_spot.y)+
                         (cir.z-i_spot.z)*(cir.z-i_spot.z));
                         

      t.transform(i_spot, ti_spot);
      z=ti_spot.z;



      i4_3d_vector normal=i_spot;
      normal-=circles[closest];
      normal.normalize();


      cpath.start=eye;
      cpath.pos=i_spot;
      cpath.normal=normal;

      float phi=acos(i4_3d_vector(0,0,1).dot(normal)); 
      float zb=i4_3d_vector(0,0,1).dot(normal);

      cpath.a.cross(i4_3d_vector(0,0,1),normal);

//       cpath.a=i4_3d_vector(cos(phi),sin(phi),sin(zb));
//       cpath.a.normalize();
      cpath.b.cross(cpath.normal, cpath.a);

      if (store_path)
      {
        path[path_len]=cpath;
        path_len++;
      }


      shade_point(eye, cpath);
        
      float c1=-ray_dir.dot(cpath.bump_normal);
      i4_3d_vector reflect=i4_3d_vector(ray_dir.x + 2*cpath.bump_normal.x*c1,
                                        ray_dir.y + 2*cpath.bump_normal.y*c1,
                                        ray_dir.z + 2*cpath.bump_normal.z*c1);
      reflect.normalize();
        
            
      float discard_z;
      path_info npath;
      trace_ray(eye, i_spot, reflect, depth+1, discard_z, npath, store_path);
      
      npath.color*=0.10;
      cpath.color+=npath.color;

      if (cpath.color.x<0) cpath.color.x=0;   
      if (cpath.color.x>1) cpath.color.x=1;
      if (cpath.color.y<0) cpath.color.y=0;   
      if (cpath.color.y>1) cpath.color.y=1;
      if (cpath.color.z<0) cpath.color.z=0;   
      if (cpath.color.z>1) cpath.color.z=1;
    }
    else
      cpath.color=i4_3d_vector(0,0,0);
  }

  void thread_render()
  {    
    reset_lock.lock();
    i4_set_thread_priority(i4_get_thread_id(), I4_THREAD_PRIORITY_LOW);

    do
    {
      reset_render=0;
      reset_lock.unlock();

      api->clear_area(0,0, width()-1, height()-1, 0x4f4f4f, 1000);
   
      srand(0);
    

      i4_3d_vector center;
      i4_3d_vector xi, yi;
      i4_3d_vector ray, eye;
      

      t.inverse_transform(i4_3d_vector(0,0,0), eye);
      t.inverse_transform(i4_3d_vector(0,0,1), center);
      t.inverse_transform(i4_3d_vector(1,0,1), xi);  xi-=center;
      t.inverse_transform(i4_3d_vector(0,1,1), yi);  yi-=center;
      
      float xm=width()/2.0;
      float ym=height()/2.0;
        


      for (int y=0; y<height(); y++)
      {
        float fy=(y-ym)/ym;

        i4_3d_vector ya=yi;
        ya*=fy;
      

        for (int x=0; x<width(); x++)
        {
          float fx=(x-xm)/xm;
                   
          i4_3d_vector xa=xi;
          xa*=fx;

          i4_3d_vector r1=center;
          r1+=xa;
          r1+=ya;
          r1-=eye;
     
          float z=10000;
          path_info c;
          trace_ray(eye, eye, r1, 1, z, c); 


          r1_vert v;
          v.px=x;
          v.py=y;
          v.v.z=z;
              
          v.r=c.color.x;
          v.g=c.color.y;
          v.b=c.color.z;


          v.a=1;
          api->render_pixel(&v);

        }

        if (abort_render || reset_render)
          break;
      }


       if (!reset_render && !abort_render)
       {
         for (int i=0; i<path_len; i++)
         {
           w32 colors[5]={0x7f0000, 0xff0000, 0x007f00, 0xff00, 0x7f7f00 };
           for (i=0; i<path_len; i++)
           {
             i4_3d_vector dir=path[i].pos;
             dir-=path[i].start;
             
             draw_vector(path[i].start, dir, colors[0]);
             draw_vector(path[i].pos, path[i].normal, colors[1]);
             draw_vector(path[i].pos, path[i].a, colors[2]);
             draw_vector(path[i].pos, path[i].b, colors[3]);
           }


         }
       } 

      reset_lock.lock();
    } while (reset_render);

    
    rwin()->end_render();
    rendering=0;
    request_redraw();
    reset_lock.unlock();
  }

  void render()
  {
    reset_lock.lock();
    
    if (rendering)
      reset_render=1;
    else
    {
      rwin()->begin_render();      
      rendering=1;       
      i4_add_thread(render_thread_start, 200*1024, this);    
    }

    reset_lock.unlock();
  }

  void draw_vector(i4_3d_vector pos, i4_3d_vector dir, w32 color, i4_draw_context_class *context=0)
  {
    i4_3d_vector end=pos;
    end+=dir;
    draw_3d_line(pos, end, color, context);
    
    i4_3d_vector xs, ys;
    if (dir.x>dir.y && dir.x>dir.z)
    {
      xs.cross(dir, i4_3d_vector(0,1,0)); 
      ys.cross(dir, i4_3d_vector(0,0,1));  
    }
    else if (dir.y>dir.z)
    {
      xs.cross(dir, i4_3d_vector(1,0,0)); 
      ys.cross(dir, i4_3d_vector(0,0,1));  
    }
    else
    {
      xs.cross(dir, i4_3d_vector(1,0,0)); 
      ys.cross(dir, i4_3d_vector(0,1,0));  
    }

    i4_3d_vector p5=dir;
    p5*=0.8; p5+=pos;    

    xs.normalize();
    ys.normalize();

    xs*=0.1;
    ys*=0.1;
    i4_3d_vector p1=p5,p2=p5,p3=p5,p4=p5;
    p1+=xs; p1+=ys;
    p2-=xs; p2+=ys;
    p3-=xs; p3-=ys;
    p4+=xs; p4-=ys;


    draw_3d_line(p1,p2, color, context);
    draw_3d_line(p2,p3, color, context);
    draw_3d_line(p3,p4, color, context);
    draw_3d_line(p4,p1, color, context);

    draw_3d_line(p1,end, color, context);
    draw_3d_line(p2,end, color, context);
    draw_3d_line(p3,end, color, context);
    draw_3d_line(p4,end, color, context);
  }

  void draw_wire_3d_sphere(i4_3d_vector pos, float r, i4_draw_context_class *context)
  {
    draw_3d_line(i4_3d_vector(pos.x-r, pos.y, pos.z),
                 i4_3d_vector(pos.x+r, pos.y, pos.z), 0xffffff, context);
    draw_3d_line(i4_3d_vector(pos.x, pos.y-r, pos.z),
                 i4_3d_vector(pos.x, pos.y+r, pos.z), 0xffffff, context);
    draw_3d_line(i4_3d_vector(pos.x, pos.y, pos.z-r),
                 i4_3d_vector(pos.x, pos.y, pos.z+r), 0xffffff, context);
                 
  }

  void draw_3d_line(i4_3d_vector p1, i4_3d_vector p2, w32 color, i4_draw_context_class *context)
  {
    if (!context)
    {
      r1_vert v[2];
      v[0].a=1;
      v[0].r=((color&0xff0000)>>16)/255.0;
      v[0].g=((color&0xff00)>>8)/255.0;
      v[0].b=((color&0xff)>>0)/255.0;

      v[1]=v[0];

      if (project_point(p1, v[0]) &&
          project_point(p2, v[1]))
        r1_clip_render_lines(1, v, width()/2, height()/2, api);
    }
    else
    {
      r1_vert tp1, tp2;
      if (project_point(p1,tp1) && project_point(p2,tp2))   
        local_image->line((int)tp1.px, (int)tp1.py,
                          (int)tp2.px, (int)tp2.py, color, *context);
    }
                        
  }


  void draw(i4_draw_context_class &context)
  {
    if (rendering)
    {
      local_image->clear(0,context);
      int i;
      for (i=0; i<T_C; i++)
        draw_wire_3d_sphere(circles[i], 1, &context);

      w32 colors[5]={0x7f0000, 0xff0000, 0x007f00, 0xff00, 0x7f7f00 };

      for (i=0; i<path_len; i++)
      {
        i4_3d_vector dir=path[i].pos;
        dir-=path[i].start;
        draw_vector(path[i].start, dir, colors[0], &context);
        draw_vector(path[i].pos, path[i].normal, colors[1], &context);
        draw_vector(path[i].pos, path[i].a, colors[2], &context);
        draw_vector(path[i].pos, path[i].b, colors[3], &context);
      }

      draw_vector(i4_3d_vector(0,0,0), i4_3d_vector(1,0,0), 0xff0000, &context);
      draw_vector(i4_3d_vector(0,0,0), i4_3d_vector(0,1,0), 0x00ff00, &context);
      draw_vector(i4_3d_vector(0,0,0), i4_3d_vector(0,0,1), 0x0000ff, &context);

      draw_vector(i4_3d_vector(0,0,0), light, 0xffff00, &context);


//       draw_3d_line(i4_3d_vector(-1, 0, 0), i4_3d_vector(1,0,0), 0xff0000, context);
//       draw_3d_line(i4_3d_vector(0, -1, 0), i4_3d_vector(0,1,0), 0x00ff00, context);
//       draw_3d_line(i4_3d_vector(0, 0, -1), i4_3d_vector(0,0,1), 0x0000ff, context);


      
    }
    i4_window_class::draw(context);

    

  }


  char *name() { return "trace_view"; }
  trace_view(int w, int h, r1_render_window_class *rwin_parent) 
    : i4_window_class(w,h), rwin_parent(rwin_parent)
  {
    path_len=0;
    mouse_look_grab=i4_F;
    theta=0;
    rendering=0;
    abort_render=0;

    i4_pixel_format fmt;
    fmt.default_format();
    fmt.alpha_mask=0;
    fmt.calc_shift();

    v.calc_transform(t);
    light=i4_3d_vector(0,4,1);
    light.normalize();

    render();
  
  }


  void get_mouse_ray(int mx, int my, i4_3d_vector &ray)
  {
    float w2=width()/2.0, h2=height()/2.0;
    i4_3d_vector eye_space_ray((mx-w2)/w2,  (my-h2)/h2, 1);
    eye_space_ray.normalize();
    
    i4_3d_vector eye;
    t.inverse_transform(i4_3d_vector(0,0,0), eye);
    t.inverse_transform(eye_space_ray, ray);
      
    ray-=eye;
  }

  i4_bool project_point(i4_3d_vector v, r1_vert &q)
  {
    t.transform(v, q.v);
    if (q.v.z>0)
    {
      float ooz=1.0/q.v.z;
      q.px=q.v.x * ooz * width()/2.0 + width()/2.0;
      q.py=q.v.y * ooz * height()/2.0 + height()/2.0;
      return i4_T;
    }
    return i4_F;

  }


  void save()
  {
    i4_draw_context_class context(0,0, width()-1, height()-1);

    i4_image_class *im=i4_create_image(width(), height(), local_image->pal);
    local_image->put_part(im, 0,0, x(), y(), x()+width()-1, y()+height()-1, context);
    

    i4_file_class *fp=i4_open("im.jpg", I4_WRITE);
    i4_write_jpeg(im, fp, 100);
    delete fp;    
    delete im;
  }

  void receive_event(i4_event *ev)
  {

    switch (ev->type())
    {
      case i4_event::USER_MESSAGE :
      {
        CAST_PTR(uev, i4_user_message_event_class, ev);
        switch (uev->sub_type)
        {
          case SAVE:
            save();
            break;
        }

      } break;

      case i4_event::MOUSE_MOVE :
      {
        CAST_PTR(mev, i4_mouse_move_event_class, ev);
        if (mouse_look_grab)
        {
          rotate_cam((mev->ly-mev->y)/200.0,
                     (mev->x-mev->lx)/200.0,
                     0);
        }
      } break;
 
      case i4_event::MOUSE_BUTTON_DOWN :
      {
        CAST_PTR(bev, i4_mouse_button_event_class, ev);
        if (bev->right())
        {
          mouse_look_grab=i4_T;
          
          i4_window_request_mouse_grab_class grab(this);
          i4_kernel.send_event(parent, &grab);
          //          i4_current_app->get_display()->set_mouse_raw_mode(i4_T);
        }
        else if (bev->left())
        {
          i4_3d_vector ray_dir;
          get_mouse_ray(bev->x, bev->y, ray_dir);

          path_len=0;
          float z;

          path_info c;
          i4_3d_vector eye;
          t.inverse_transform(i4_3d_vector(0,0,0), eye);
          trace_ray(eye, eye, ray_dir, 1, z, c, i4_T);
          calc_transform();
        }


      } break;

      case i4_event::MOUSE_BUTTON_UP :
      {
        CAST_PTR(bev, i4_mouse_button_event_class, ev);
        if (bev->right() && mouse_look_grab)
        {
          mouse_look_grab=i4_F;
          //          i4_current_app->get_display()->set_mouse_raw_mode(i4_F);
          i4_window_request_mouse_ungrab_class ungrab(this);
          i4_kernel.send_event(parent, &ungrab);
        }

      } break;

    

      case i4_event::KEY_PRESS :
      {
        CAST_PTR(kev, i4_key_press_event_class, ev);
        switch (kev->key)
        {
          case I4_UP : move_cam(0,0,0.5); break;
          case I4_DOWN : move_cam(0,0,-0.5); break;
          case I4_LEFT : move_cam(-0.5,0,0); break;
          case I4_RIGHT : move_cam(0.5,0,0); break;
          case 'z' : rotate_cam(-0.1, 0,0); break;
          case 'x' : rotate_cam(0.1, 0,0); break;
          case 'c' : rotate_cam(0, -0.1,0); break;
          case 'v' : rotate_cam(0, 0.1,0); break;
          case 'w' : 
          case ' ' : 
          {
            light.x+=0.1; light.normalize(); calc_transform();
          } break;
        }

      }
    
      default:
        i4_window_class::receive_event(ev);
    }
  }
};


void render_thread_start(void *context)
{
  ((trace_view *)context)->thread_render();
}




i4_window_class *create_view(int w, int h)
{
  r1_render_window_class *win=api->create_render_window(w,h);
  trace=new trace_view(w,h, win);
  win->add_child(0,0, trace);
  return win;
}


class ray_tracer_app : public i4_application_class
{
public:
  i4_bool get_display_name(char *name, int max_len)
  {
    strcpy(name, "Windowed GDI");
    return i4_T;
  }

  void init()
  {
    i4_application_class::init();

    i4_pull_menu_class *menu=li_create_pull_menu("menu.scm");
    menu->show(wm, 0,0);

    api=r1_create_api(get_display(), "Software Z Buffer");
    if (!api)
      i4_error("no render api");

    i4_graphical_style_class *style=get_style();
    i4_window_class *w=create_view(400, 400);

    i4_parent_window_class *p;
    p=style->create_mp_window(0, menu->height(), w->width(), w->height(), "Render");
    p->add_child(0,0, w);

    
    i4_button_class *b=new i4_button_class(0, new i4_text_window_class("Save", style), style, 
                                           new i4_event_reaction_class(trace, SAVE));
    wm->add_child(wm->width()-b->width(), wm->height()-b->height(), b);
  }

  void calc_model()
  {
    if (trace->path_len)
    {
      float z=10000;
      path_info c;
      i4_3d_vector dir=trace->path[0].pos;
      dir-=trace->path[0].start;
      i4_3d_vector eye;
      trace->t.inverse_transform(i4_3d_vector(0,0,0), eye);
      trace->trace_ray(eye, trace->path[0].start, dir, 1, z, c); 
    }

  }

  void uninit()
  {
    trace->abort_render=1;
    i4_application_class::uninit();
  }


  char *name() { return "ray tracer"; }
};


void i4_main(w32 argc, i4_const_str *argv)
{
  ray_tracer_app test;
  test.run();
}


