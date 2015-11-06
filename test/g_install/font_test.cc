/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

/*
Font string used in Gimp

!  "  #  $  %  & '  (  )  *  +  ,  -  .  /  0  1  2  3  4  5  6  7   8   9 :  ; (  =  )  ?  @  A  B C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V   W  X  Y  Z [  /  ] ^  -  `  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o  p  q  r  s  t   u  v  w  x  y  z  {  |  }  ~ 

"
*/

#include "window/window.hh"
#include "app/app.hh"
#include "main/main.hh"
#include "window/style.hh"
#include "file/file.hh"
#include "loaders/load.hh"
#include "window/wmanager.hh"
#include "gui/text_input.hh"

class font_win : public i4_window_class
{
public:
  i4_graphical_style_class *style;
  font_win(i4_graphical_style_class *style) 
    : i4_window_class(500,480), style(style)
  {
    
  }

  void draw(i4_draw_context_class &context)
  {
    i4_font_class *font=style->font_hint->normal_font;

    local_image->clear(0,context);
    font->set_color(0xffff00);
    font->put_paragraph(local_image, 0,15, 
                        i4_const_str("Helicopter:\n"
                                     "The helicopter is not limited by terrain and moves "
                                     "quickly.  It is a fairly weak vehicle."), 
                        context,
                        0, i4_font_class::FULL, 200);
    local_image->bar(width()/2, 0, width()/2, height()-1, 0xff, context);

    font->put_paragraph(local_image, 0,200, 
                        i4_const_str("    !  \"  #  $  %  & '  (  )  *  +  ,  -  .  /  0  1  2  3  4  5  6  7   8   9 :  ; (  =  )  ?  @  A  B C  D  E  F  G  H  I  J  K  L  M  N  O  P  Q  R  S  T  U  V   W  X  Y  Z [  /  ] ^  -  `  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o  p  q  r  s  t   u  v  w  x  y  z  {  |  }  ~ "),
                        context,
                        0, i4_font_class::FULL, 500);



  }

  char *name() { return "font_win"; }
};

class test_app : public i4_application_class
{
public:
  void init()
  {    
    i4_application_class::init();


    i4_graphical_style_class *style=wm->get_style();

    wm->add_child(0,50, new font_win(style));
    /*    i4_window_class *ti=new i4_text_input_class(style, i4_const_str("Testing"), 400, 500);
    i4_parent_window_class *mp=style->create_mp_window(80,200, ti->width(), ti->height(), 
                                                       i4_const_str("Text input"));
    mp->add_child(0,0,ti); */
  }

  char *name() { return "test_app"; }
};

void i4_main(w32 argc, i4_const_str *argv)
{
  test_app test;
  test.run();
}





