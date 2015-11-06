/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "window/window.hh"
#include "app/app.hh"
#include "main/main.hh"
#include "r1_api.hh"
#include "r1_win.hh"
#include "window/style.hh"
#include "file/file.hh"
#include "loaders/load.hh"
#include "window/wmanager.hh"
#include "math/transform.hh"
#include "gui/text.hh"
#include "gui/button.hh"
#include "loaders/load.hh"
#include "tmanage.hh"


// merge test
// merge test 2.5


r1_render_api_class *api=0;


class test_win_class : public i4_window_class
{  
public:
  float xr,yr,zr;   // rotation numbers for the cube
  r1_texture_handle texture;


  test_win_class(w16 w, w16 h)
    : i4_window_class(w,h)  
  { 
    xr=yr=zr=0;

    r1_texture_manager_class *tman=api->get_tmanager();
    texture=tman->register_image(i4_load_image("texture.tga"));
  }

  void draw(i4_draw_context_class &context)
  { 
    int i;

    // initialize the rendering api to a "normal" state
    api->default_state();

    // the z range tells the api what range your vertexes will be in so it can scale
    // them to fit it's own internal z buffer range
    float far_z=1000;
    api->set_z_range(0.01, far_z);

    // clear the screen black, since frame buffer is initialized to far_z, the clear z
    // bust be less than this to appear
    w32 color=0;    
    api->clear_area(0,0,width()-1,height()-1, color, far_z-1);  


    // points on the cube
    float cube_points[8*3]={-1, 1, -1,  1,1,-1,  1,-1,-1,  -1, -1, -1,
                            -1, 1,  1,  1,1,1,   1,-1,1,   -1, -1, 1};

    // some colors           red    green blue   white
    float colors[4 * 3] = { 1,0,0, 0,1,0, 0,0,1, 1,1,1 };
    int point_colors[8] = { 0,1,2,3,3,2,1,1 };

    // faces on the cube
    int verts[4 * 6] = { 0,1,2,3,  4,5,6,7,  3,2,6,7, 0,1,5,4, 1,5,6,2, 0,3,7,4 };
    float tcoord_s[4] = { 0,1,1,0 };
    float tcoord_t[4] = { 1,1,0,0 };

    // setup of the transformation matrix
    i4_transform_class matrix, tmp_matrix;
    matrix.identity();
    matrix.t.z=4;        // move camera 5 units away from cube

    tmp_matrix.rotate_x(xr);
    matrix.multiply(tmp_matrix);

    tmp_matrix.rotate_y(yr);
    matrix.multiply(tmp_matrix);

    tmp_matrix.rotate_y(zr);
    matrix.multiply(tmp_matrix);

    xr+=0.01;    yr+=0.02;    zr+=0.011;  // change rotation for next draw
    
    // transform the points into view spave
    i4_3d_point_class transformed_points[8];
    for (i=0; i<8; i++)
    {
      i4_3d_point_class p(cube_points[i*3], cube_points[i*3+1], cube_points[i*3+2]);
      matrix.transform(p, transformed_points[i]);
    }

    api->use_texture(texture, 256, 0);

    // now copy the points into an r1_vert and project them onto the scren
    float center_x=width()/2, center_y=height()/2;
    r1_vert v[4];
    for (i=0; i<6; i++)  // loop through the faces
    {
      for (int j=0; j<4; j++)  // loop through the points on the face
      {
        int pn=verts[i*4+j];  //point number

        // this stores the x,y,z of the point in world space (only x & y are actually needed,
        // unless you clip the polygon - which we are not)
        v[j].v=*((r1_3d_point_class *)&transformed_points[pn]);

        // w is the homogenous coordinate (1/z)
        v[j].w=1.0/v[j].v.z;

        // project onto screen and scale to fit the camera view port
        v[j].px=v[j].w * v[j].v.x * center_x + center_x;
        v[j].py=v[j].w * v[j].v.y * center_y + center_y;

        // set the color of the points
        v[j].r=colors[point_colors[pn]*3];       // color values are [0..1] 1=brightest
        v[j].g=colors[point_colors[pn]*3+1];
        v[j].b=colors[point_colors[pn]*3+2];

        // we don't need to set s & t (texture coordinates)
        // because we aren't texture mapping, but just so you know they are there..
        v[j].s = tcoord_s[j];     // texture coordinates are between 0..1, 0 is left or top
        v[j].t = tcoord_t[j];
      }

      api->render_poly(4, v);
    }
    request_redraw();
  }
  
  char *name() { return "test_win"; }
};

class test_app : public i4_application_class
{
public:
  enum { QUIT };

  void init()
  {
    i4_application_class::init();
    
    // create a rendering api (display comes from the application which was
    // created by init)
    // the second parameter is the suggested texture memory size
    api=r1_create_api(display);
    if (!api)
      i4_error("could not find an rending api for this display");

    // wm is the window manager, created by app::init()
    // wm->width is the resolution of the screen
    int w=wm->width()/2, h=wm->height()/2;

    // wm has a "style" associated with it that controlls the way the gui looks
    i4_graphical_style_class *style=wm->get_style();

    // you can get the window manager's style by calling get_style()
    // a mp_window (--meta physical--) is a framed draggable window 
    i4_parent_window_class *p1=style->create_mp_window(-1,-1, w,h, 
                                                       i4_const_str("Test window 1"), 0);

    // all rendering must reside in render_window so the render api can initialize
    // the context before and after the drawing operations (like x/y window offset,
    // clipping rectangles, etc).  
    r1_render_window_class *rwin = api->create_render_window(p1->width(), p1->height());
    
    // render_area_width() & height may be different from width() if you are
    // using double pixel or interlaced mode
    i4_window_class *test_win;
    test_win=new test_win_class(rwin->render_area_width(), rwin->render_area_height());
    
    // this adds the test window into the render_api's render_window
    rwin->add_child(0, 0, test_win);

    // this adds the render_api's window into the mp_window (which is automatically added
    // to your desktop by create_mp_window)
    p1->add_child(0,0,rwin);

    i4_button_class *quit=new i4_button_class(0,
                                              new i4_text_window_class(i4_const_str("Quit"), 
                                                                       style),
                                              style,
                                              new i4_event_reaction_class(this, QUIT));
    wm->add_child(0,0, quit);
  }  


  void receive_event(i4_event *ev)
  {
    if (ev->type()==i4_event::USER_MESSAGE && ((i4_user_message_event_class *)ev)->sub_type==QUIT)
      quit();
    else
      i4_application_class::receive_event(ev);
  }

  void uninit()
  {    
    // cleanup the the render_api
    r1_destroy_api(api);

    // the application will close the display and cleanup memory here
    i4_application_class::uninit();
  }


  char *name() { return "test_app"; }
};

void i4_main(w32 argc, i4_const_str *argv)
{
  test_app test;
  test.run();
}

