/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "editor/dialogs/pick_win.hh"
#include "image/image.hh"
#include "math/pi.hh"
#include "math/transform.hh"
#include "draw_context.hh"
#include "image/image.hh"
#include "image/color.hh"
#include "window/win_evt.hh"
#include "r1_api.hh"
#include "tmanage.hh"
#include "g1_render.hh"
#include "r1_win.hh"
#include "r1_clip.hh"
#include "resources.hh"

void g1_3d_pick_window::parent_draw(i4_draw_context_class &context)
{    
  i4_transform_class  tmp;
  i4_transform_class  transform;

  transform.identity();

  tmp.translate(0,0,camera.view_dist);  transform.multiply(tmp);    

  tmp.rotate_y(camera.theta);           transform.multiply(tmp);
  tmp.rotate_x(i4_pi()-camera.phi);   transform.multiply(tmp);


  tmp.translate(-camera.center_x,
                -camera.center_y,
                -camera.center_z);      
  transform.multiply(tmp);




  //  context.add_both_dirty(0,0,local_image->width()-1,local_image->height()-1);

  g1_draw_context_class gc;
  gc.window_setup(0,0, width(), height(),
                  g1_render.center_x, g1_render.center_y,
                  g1_render.scale_x, g1_render.scale_y,
                  g1_render.ooscale_x, g1_render.ooscale_y);

  r1_render_api_class *render_api=g1_render.r_api;


  render_api->default_state();

  render_api->set_shading_mode(R1_SHADE_DISABLED);

  render_api->clear_area(0,0, width()-1, height()-1, 
                         active ? 0xffffff : 0x9f9f9f, r1_far_clip_z);


  gc.transform=&transform;
  gc.screen=local_image;
  gc.context=&context;
  gc.draw_editor_stuff=i4_T;
  g1_render.frame_ratio=1;
  
  r1_far_clip_z=5000;


  if (active && !camera.stopped)
  {
    i4_time_class now;
    camera.zrot+=now.milli_diff(start)/2000.0;
    start.get();

    
    i4_float d=fabs(cos(camera.zrot)*4)-i4_pi();
    camera.theta=d;

    if (camera.theta>2*i4_pi())
      camera.theta-=2*i4_pi();
    if (camera.theta<0)
      camera.theta+=2*i4_pi();
  }
  else
    start.get();


  r1_far_clip_z=5000;
        
  render_api->disable_texture();
  render_api->clear_area(0,0,width()-1,height()-1,0,r1_far_clip_z);    

  draw_object(&gc);

  r1_texture_manager_class *tman=render_api->get_tmanager();

}


void g1_3d_pick_window::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::MOUSE_BUTTON_DOWN)
  {
    request_redraw(i4_T);

    do_press();
    i4_kernel.send(reaction);
    camera.stopped=i4_T;

    CAST_PTR(mev, i4_mouse_button_down_event_class, ev);
    if (!grabr && mev->but==i4_mouse_button_down_event_class::RIGHT)
    {
      grabr=i4_T;

      if (!grabl)
      {
        i4_window_request_mouse_grab_class grab(this);
        i4_kernel.send_event(parent, &grab);
      }
    }
    else if (!grabl && mev->but==i4_mouse_button_down_event_class::LEFT)
    {
      grabl=i4_T;

      if (!grabr)
      {
        i4_window_request_mouse_grab_class grab(this);
        i4_kernel.send_event(parent, &grab);
      }
      do_press();

    }

  }
  else if (ev->type()==i4_event::MOUSE_BUTTON_UP)
  {
    CAST_PTR(mev, i4_mouse_button_up_event_class, ev);
    if (grabr && mev->but==i4_mouse_button_up_event_class::RIGHT)
    {
      grabr=i4_F;

      if (!grabl)
      {
        i4_window_request_mouse_ungrab_class ungrab(this);
        i4_kernel.send_event(parent, &ungrab);
      }
    }
    else if (grabl && mev->but==i4_mouse_button_up_event_class::LEFT)
    {
      grabl=i4_F;

      if (!grabr)
      {
        i4_window_request_mouse_ungrab_class ungrab(this);
        i4_kernel.send_event(parent, &ungrab);
      }

    }
    
  } else if (ev->type()==i4_event::MOUSE_MOVE)
  {
    CAST_PTR(mev, i4_mouse_move_event_class, ev);
    if (grabl)
    {
      camera.theta -= 0.01*(mev->x - last_mx);
      camera.phi -= 0.01*(mev->y - last_my);

      if (camera.theta<0.0)
        camera.theta += i4_pi()*2;
      else if (camera.theta>i4_pi()*2)
        camera.theta -= i4_pi()*2;

      if (camera.phi<0.0)
        camera.phi = 0.0;
      else if (camera.phi>i4_pi())
        camera.phi = i4_pi();

      request_redraw(i4_F);
    }
    if (grabr)
    {
      camera.view_dist += (mev->y - last_my)*0.1;
      request_redraw(i4_F);
    }

    last_mx=mev->x;
    last_my=mev->y;
  }
  else if (ev->type()==i4_event::KEY_PRESS)
  {
    CAST_PTR(kev, i4_key_press_event_class, ev);
    if (kev->key=='x')
      camera.theta+=0.2;
    else if (kev->key=='X')
      camera.theta-=0.2;
    if (kev->key=='y')
      camera.phi+=0.2;
    else if (kev->key=='Y')
      camera.phi-=0.2;    
  }

  i4_menu_item_class::receive_event(ev);
}
