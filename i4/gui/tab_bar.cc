/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "gui/tab_bar.hh"
#include "memory/array.hh"
#include "window/style.hh"

struct tab
{
  i4_menu_item_class *tab_top;
  i4_window_class *tab_body;
  int desired_width;    
};

enum { LEFT_SPACING=5,
       RIGHT_SPACING=5,
       TAB_TOP_MARGIN=6,
       TAB_BOTTOM_MARGIN=4,
       TAB_LEFT_MARGIN=3,
       TAB_RIGHT_MARGIN=3 };
       
       

struct i4_tab_bar_data
{
  i4_array<tab> tabs;
  int current_tab;
  int top_height;
  i4_graphical_style_class *style;

  i4_tab_bar_data()
    : tabs(0,8)
  {
    top_height=0;
  }

};


i4_tab_bar::i4_tab_bar(int width, int height, i4_graphical_style_class *style)
  : i4_menu_class(i4_F)
{
  data=new i4_tab_bar_data();
  data->style=style;
  private_resize(width, height);
    
}

i4_tab_bar::~i4_tab_bar()
{
  delete data;  
 
}


void i4_tab_bar::private_resize(w16 new_width, w16 new_height)
{
  i4_parent_window_class::private_resize(new_width, new_height);
  if (!data->tabs.size())
    return ;
 
 
  int i, t_tabs=data->tabs.size();
  remove_child(data->tabs[data->current_tab].tab_body);

  for (i=0; i<t_tabs; i++)
    remove_child(data->tabs[i].tab_top);




  int available_size=width()-LEFT_SPACING-RIGHT_SPACING-t_tabs*(TAB_LEFT_MARGIN+TAB_RIGHT_MARGIN);

  int suggested_size=available_size / t_tabs;
  int leftover_size=0;
  
  for (i=0; i<t_tabs; i++)
    if (data->tabs[i].desired_width<suggested_size)
      leftover_size+=suggested_size-data->tabs[i].desired_width;

  leftover_size+=available_size-suggested_size*t_tabs;


  int client_w=width()-4;
  int client_h=height()-data->top_height-TAB_TOP_MARGIN-TAB_BOTTOM_MARGIN;
  
  int xpos=LEFT_SPACING + TAB_LEFT_MARGIN;
  int ypos=TAB_TOP_MARGIN-2;
  for (i=0; i<t_tabs; i++)
  {
    int new_w=suggested_size;

    int additional_size_needed=data->tabs[i].desired_width-suggested_size;
    if (additional_size_needed>0)
    {      
      if (additional_size_needed>leftover_size)
      {
        new_w=suggested_size+leftover_size;        
        leftover_size=0;
      }
      else
      {
        leftover_size-=additional_size_needed;
        new_w=data->tabs[i].desired_width;
      }
    }
    else new_w=data->tabs[i].desired_width;
      

    data->tabs[i].tab_top->resize(new_w, data->top_height);
    i4_parent_window_class::add_child(xpos, ypos, data->tabs[i].tab_top);

    data->tabs[i].tab_body->resize(client_w, client_h);
    if (i==data->current_tab)
      i4_parent_window_class::add_child(2, data->top_height+TAB_TOP_MARGIN, data->tabs[i].tab_body);

    xpos+=new_w+TAB_LEFT_MARGIN+TAB_RIGHT_MARGIN;
  }
}



void i4_tab_bar::add_tab(i4_menu_item_class *tab_top, i4_window_class *tab_body)
{
  i4_parent_window_class::add_child(0,0, tab_top);
  if (!data->tabs.size())
  {
    data->current_tab=0;   
    i4_parent_window_class::add_child(0,0, tab_body);
  }

  if (tab_top->height()>data->top_height)
    data->top_height=tab_top->height();


  tab t;
  t.tab_top=tab_top;
  t.tab_body=tab_body;
  t.desired_width=tab_top->width();  
  data->tabs.add(t);

  tab_top->set_menu_parent(this);

  resize(width(), height());
}



void i4_tab_bar::set_current_tab(int tab_number)
{
  if (!data->tabs.size() || tab_number==data->current_tab)
    return ;

  i4_window_class *client=data->tabs[data->current_tab].tab_body;
  int cx=client->x()-x(), cy=client->y()-y();


  remove_child(client);
  if (tab_number<0) tab_number=0;
  if (tab_number>=data->tabs.size())
    tab_number=data->tabs.size()-1;

  data->current_tab=tab_number;
  i4_parent_window_class::add_child(cx, cy, data->tabs[tab_number].tab_body);
}


void i4_tab_bar::note_reaction_sent(i4_menu_item_class *who,
                                    i4_event_reaction_class *ev,
                                    i4_menu_item_class::reaction_type type)
{
  if (type==i4_menu_item_class::PRESSED)
  {
    for (int i=0; i<data->tabs.size(); i++)
      if (who==data->tabs[i].tab_top)
        set_current_tab(i);
  }
}




void i4_tab_bar::parent_draw(i4_draw_context_class &context)
{ 
  data->style->deco_neutral_fill(local_image, 0,0, width()-1, height()-1, context);


  w32 black=0, 
    bright=data->style->color_hint->button.passive.bright,
    med=data->style->color_hint->button.passive.medium,
    dark=data->style->color_hint->button.passive.dark;
    
  i4_image_class *im=local_image;

  int dx=LEFT_SPACING, dy=0;
  int cur=data->current_tab;


  int client_y=TAB_TOP_MARGIN+data->top_height;
  int client_h=data->tabs[0].tab_body->height();

  im->bar(0, client_y-2, width()-1, client_y-2, bright, context);
  im->bar(1, client_y-1, width()-2, client_y-1, med, context);

  im->bar(0, client_y-2, 0, client_y+client_h+2, bright, context);
  im->bar(1, client_y-1, 1, client_y+client_h+1, med, context);

  im->bar(width()-2, client_y, width()-2, client_y+client_h+1, dark, context);
  im->bar(width()-1, client_y-1, width()-1, client_y+client_h+2, black, context);

  im->bar(2, client_y+client_h+1, width()-2, client_y+client_h+1, dark, context);
  im->bar(1, client_y+client_h+2, width()-1, client_y+client_h+2, black, context);

  

  for (int i=0; i<data->tabs.size(); i++)
  {
    int x1=dx, y1=dy+2, x2=dx+data->tabs[i].tab_top->width()+TAB_LEFT_MARGIN+TAB_RIGHT_MARGIN-1, 
      y2=dy+TAB_TOP_MARGIN+data->top_height-3;

    if (i==cur)
    {
      y1-=2;
      y2+=2;
      x1+=1;
      x2+=1;
    }

    if (i-1!=cur)
      im->bar(x1, y1+2, x1, y2, bright, context);    // left edge
    im->bar(x1+1, y1+1, x1+1, y1+1, bright, context);    // round off left edge to top


    im->bar(x1+2, y1, x2-(i+1==cur ? 1 : 2), y1, bright, context);  // top edge
     

    if (i+1!=cur)
    {
      im->bar(x2-1, y1, x2-1, y2, dark, context);
      im->bar(x2, y1+1, x2, y2, black, context);
    }

    if (i==cur)
      data->style->deco_neutral_fill(im, x1+1, y2-2, x2-2, y2, context);
          
    dx+=data->tabs[i].tab_top->width()+TAB_LEFT_MARGIN+TAB_RIGHT_MARGIN;
  }
}
