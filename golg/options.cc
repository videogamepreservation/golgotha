/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "options.hh"
#include "gui/button.hh"
#include "gui/text.hh"
#include "gui/butbox.hh"
#include "loaders/load.hh"
#include "resources.hh"
#include "gui/image_win.hh"
#include "mess_id.hh"
#include "g1_render.hh"
#include "r1_api.hh"
#include "app/app.hh"
#include "g1_speed.hh"

i4_event_handler_reference_class<g1_option_window> g1_options_window;

void g1_option_window::add_buttons()
{
  int i;
  i4_button_box_class *visbox=new i4_button_box_class(this);
  i4_button_class *vi[3];

  vi[0]=create_button(i4gets("vis_low_help"), 
                      g1_resource_class::OPTIONS_VIS_LOW, 0, VIS_LOW);

  vi[1]=create_button(i4gets("vis_med_help"), 
                      g1_resource_class::OPTIONS_VIS_MEDIUM, 0, VIS_MEDIUM);

  vi[2]=create_button(i4gets("vis_hi_help"),
                      g1_resource_class::OPTIONS_VIS_HIGH, 0, VIS_HIGH);

  for (i=0; i<3; i++)
    visbox->add_button(0,0, vi[i]);

  visbox->push_button(vi[g1_resources.radius_mode], i4_F);    
  visbox->arrange_down_right();
  visbox->resize_to_fit_children();

      
  i4_button_box_class *pixelbox=0;
  if (g1_render.r_api->expand_type_supported(R1_COPY_2x2) ||
      g1_render.r_api->expand_type_supported(R1_COPY_1x1_SCANLINE_SKIP))
  {      
    pixelbox=new i4_button_box_class(this);     
    pixelbox->add_button(0,0, create_button(i4gets("normal_pixel_help"),
                                            g1_resource_class::OPTIONS_PIXEL_NORMAL, 0,
                                            NORMAL_PIXEL));

    if (g1_render.r_api->expand_type_supported(R1_COPY_2x2))
      pixelbox->add_button(0,0, create_button(i4gets("double_pixel_help"),
                                              g1_resource_class::OPTIONS_DOUBLEPIXEL, 0,
                                              DOUBLE_PIXEL));

    if (g1_render.r_api->expand_type_supported(R1_COPY_1x1_SCANLINE_SKIP))
      pixelbox->add_button(0,0, create_button(i4gets("interlace_pixel_help"),
                                              g1_resource_class::OPTIONS_INTERLACE, 0,
                                              INTERLACE));
    pixelbox->arrange_down_right();
    pixelbox->resize_to_fit_children();
  }


  int x=4,y=4;
  add_child(x,y, visbox);
  x+=visbox->width()+5;

  if (pixelbox)
  {
    add_child(x, y, pixelbox);
    x+=pixelbox->width()+5;
  }

  i4_button_class *sound_but=create_button(i4gets("3d/2d_sound_help"),
                                           g1_resource_class::OPTIONS_SOUND,
                                           &sound_image_win, SOUND);
  add_child(x,y, sound_but);
  x+=sound_but->width();

#if 0
  i4_button_class *shadow_but=create_button(i4gets("shadows_help"),
                                            g1_enable_shadows ?
                                            g1_resource_class::OPTIONS_SHADOWS_ON : 
                                            g1_resource_class::OPTIONS_SHADOWS_OFF,
                                            &shadow_image_win, SHADOWS);
  shadow_but->set_popup(i4_T);

  add_child(x,y, shadow_but);
  x+=shadow_but->width();
#endif

  i4_button_class *gamespeed_but=create_button(i4gets("gamespeed_help"),
                                               g1_resource_class::OPTIONS_GAME_SPEED, 0,
                                               GAME_SPEED);
  add_child(x,y, gamespeed_but);
  x+=gamespeed_but->width();

}

g1_option_window::g1_option_window(i4_graphical_style_class *style)
  : i4_parent_window_class(0,0),
    style(style)
{
  options_background=i4_load_image(i4gets("options_background"));
  if (!options_background)
    i4_error("missing image options_background (res/g1.res)");

  resize(options_background->width(), options_background->height());

  mode=SLIDE_RIGHT;   
  slide_speed=options_background->width()/2;
  slide_correction=get_correction();

  add_buttons();
}

void g1_option_window::forget_redraw()
{
  if (mode==SLIDE_RIGHT)
  {
    i4_user_message_event_class slide(SLIDE);
    i4_kernel.send_event(this, &slide);
  }
  else if (mode==SLIDE_LEFT)
  {
    i4_user_message_event_class slide(SLIDE_AWAY);
    i4_kernel.send_event(this, &slide);
  }

}

i4_button_class *g1_option_window::create_button(const i4_const_str &help, 
                                                 int im1,
                        i4_event_handler_reference_class<i4_image_window_class> *win_ref,
                                                 int mess_id)
{
  i4_image_class *im=g1_resources.images[im1];
  i4_image_window_class *im_win=new i4_image_window_class(im);
  if (win_ref)
    (*win_ref)=im_win;

  i4_user_message_event_class *uev=new i4_user_message_event_class(mess_id);
  i4_event_reaction_class *rev=new i4_event_reaction_class(this, uev);

  i4_button_class *b=new i4_button_class(&help, im_win, style, rev);
  return b;
}

void g1_option_window::parent_draw(i4_draw_context_class &context)
{
  forget_redraw();
  options_background->put_image(local_image, width()-options_background->width(), 0, context);
}
                          

int g1_option_window::get_correction()
{
  int sum=0, x=options_background->width();
  while (x)
  {
    x/=2;
    sum+=x;
  }
  return options_background->width()-sum;
}


void g1_option_window::receive_event(i4_event *ev)
{
  if (ev->type()==i4_event::USER_MESSAGE)    
  {
    CAST_PTR(uev, i4_user_message_event_class, ev);
    switch (uev->sub_type)
    {
      case SLIDE :
      {
        if (mode!=SLIDE_LEFT)
        {
          move(slide_speed + slide_correction, 0);
          slide_speed/=2;
          if (slide_speed==0)
            mode=STOPPED;
          slide_correction=0;        
        }
      } break;

      case SLIDE_AWAY :
      {
        if (g1_options_window.get() == this)
          g1_options_window=0;
          
        if (mode!=SLIDE_LEFT)
        {
          mode=SLIDE_LEFT;   
          slide_speed=options_background->width()/2;
          slide_correction=get_correction();        
        }


        move(-(slide_speed + slide_correction), 0);
        slide_speed/=2;
        if (slide_speed==0)
          i4_kernel.delete_handler(this);

        slide_correction=0;
      } break;
      

      case VIS_LOW :
      case VIS_MEDIUM :
      case VIS_HIGH :
        g1_resources.radius_mode=(g1_resource_class::radius_mode_type)(uev->sub_type-VIS_LOW);
        break;

      case GAME_SPEED :
      {
        if (G1_HZ==10)
          G1_HZ=20;
        else
          G1_HZ=10;
      } break;
      
      case INTERLACE :
      {
        i4_user_message_event_class uev(G1_INTERLACE_PIXEL);
        i4_kernel.send_event(i4_current_app, &uev);
      } break;

      case DOUBLE_PIXEL :
      {
        i4_user_message_event_class uev(G1_DOUBLE_PIXEL);
        i4_kernel.send_event(i4_current_app, &uev);
      } break;

      case NORMAL_PIXEL :
      {
        i4_user_message_event_class uev(G1_NORMAL_PIXEL);
        i4_kernel.send_event(i4_current_app, &uev);
      } break;

#if 0
      case SHADOWS :
        g1_enable_shadows= !g1_enable_shadows;
        if (g1_enable_shadows && shadow_image_win.get())
          shadow_image_win.get()->change_image(
              g1_resources.images[g1_resource_class::OPTIONS_SHADOWS_ON]);
        else if (!g1_enable_shadows && shadow_image_win.get())
          shadow_image_win.get()->change_image(
              g1_resources.images[g1_resource_class::OPTIONS_SHADOWS_OFF]);       
        break;
#endif
    }
  }
  else i4_parent_window_class::receive_event(ev);
}


g1_option_window::~g1_option_window()
{
  delete options_background;
}
