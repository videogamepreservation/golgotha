/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "editor/dialogs/path_win.hh"
#include "window/window.hh"
#include "window/style.hh"
#include "gui/button.hh"
#include "gui/image_win.hh"
#include "gui/butbox.hh"

#include "map.hh"
#include "path_api.hh"
#include "critical_graph.hh"
#include "critical_map.hh"
#include "solvemap_astar.hh"
#include "solvegraph_breadth.hh"

#include "image/depth.hh"
#include "image/image32.hh"
#include "device/keys.hh"

#include "editor/editor.hh"

enum {
  P1_SET_START,
  P1_SET_DESTINATION,
  P1_SET_CRITICAL,
  P1_LOAD_HOTSPOTS,

  P1_SET_GRADE1,
  P1_SET_GRADE2,
  P1_SET_GRADE3,
  P1_SET_GRADE4,

  P1_SET_SIZE1,
  P1_SET_SIZE3,

  P1_RELOAD_MAP,

  P1_SET_BLOCK,
  P1_LAY_OBJECT,

  P1_STOP,
  P1_STEP,

  P1_UNSET_BLOCK_EVENT
};




g1_path_tool_window_class::g1_path_tool_window_class(i4_graphical_style_class *style, 
                                                     i4_event_handler_class *send_to,
                                                     int buttons, i4_image_class **img, 
                                                     i4_const_str **help_names)
  : i4_button_box_class(send_to, i4_F)
//{{{
{
  int x=0,y=0,ny=0;
  for (int i=0; i<buttons; i++)
  {
    if (img[i])
    {
      i4_button_class *b;
        
      i4_user_message_event_class *uev=new i4_user_message_event_class(i);
      i4_event_reaction_class *re=new i4_event_reaction_class(send_to, uev);
        
      b=new i4_button_class(help_names[i], new i4_image_window_class(img[i]), style, re);
      b->set_popup(i4_T);

      if (x+b->width()>40)
      {
        x=0;
        y+=ny;
        ny=0;
      }
      add_button(x,y, b);
      x += b->width();
      ny = b->height()>ny ? b->height() : ny;
    }
  }
    
  resize_to_fit_children();
}
//}}}


g1_path_window_class::g1_path_window_class(g1_map_class *map, 
                                           i4_image_class **icons)
  : map(map), 
    critical_graph(map->get_critical_graph()),
    i4_parent_window_class(map->width()*CELL_SIZE,map->height()*CELL_SIZE)
//{{{
{
  start_icon = icons[0];
  dest_icon = icons[1];
  crit_icon = icons[2];
  
  bitmap=0;
  map_changed = 1;

  grade = 0;
  size = 2;
  tofrom = 0;
  points = 0;
  maker = new g1_critical_map_maker_class();
  maker->make_criticals(map, critical_graph);
  solvemap = new g1_astar_map_solver_class(map->get_block_map(grade));
  solvegraph = new g1_breadth_first_graph_solver_class(critical_graph);
  solve();
}
//}}}

g1_path_window_class::~g1_path_window_class()
//{{{
{
  if (maker)
    delete maker;
  if (solvemap)
    delete solvemap;
  if (solvegraph)
    delete solvegraph;
  if (bitmap)
    delete bitmap;
}
//}}}

w32 g1_path_window_class::critical_color(w16 x, w16 y)
//{{{
{
  w32 crit_col[6] = { 0x200000, 0x002000, 0x000020,
                      0x400000, 0x004000, 0x000040 };
  w32 col=0;
  w16 crit;

  crit = map->cell(x,y).nearest_critical[grade][tofrom][0];
    
  for (int i=0; i<6; i++)
    if (crit & (1<<i))
      col |= crit_col[i];
    
  return col;;
}
//}}}

g1_astar_map_solver_class *g1_solvemap=0; //(OLI) Debug hack

void g1_path_window_class::solve()
//{{{
{
  g1_graph_node start_node = map->cell(start.x,start.y).nearest_critical[grade][tofrom][0];
  g1_graph_node dest_node = map->cell(dest.x,dest.y).nearest_critical[grade][tofrom][0];

  solvemap->path_solve(start.x, start.y, dest.x, dest.y, size, size, grade, point, points);
  solvegraph->path_solve(start_node, dest_node, size, grade, point, points);

  g1_solvemap = solvemap; //(OLI) Debug hack
  printf("Corridor size %f!\n",
         map->get_block_map(grade)->line_of_sight(start.x,start.y,dest.x,dest.y));
  g1_solvemap = 0; //(OLI) Debug hack
}
//}}}

void g1_path_window_class::draw_to_bitmap()
//{{{
{
  map_changed=0;

  int bmw=map->width()*CELL_SIZE, bmh=map->height()*CELL_SIZE, x,y, i,j, 
    mw=map->width(), mh=map->height(), px, py, ry;

  if (bitmap && (bitmap->width() !=  bmw  ||  bitmap->height() != bmh))
  {
    delete bitmap;
    bitmap=0;
  }

  if (!bitmap)
  {
    i4_pixel_format fmt;
    fmt.default_format();
    bitmap=new i4_image32(bmw, bmh, i4_pal_man.register_pal(&fmt));   
  }

  i4_image32::iterator block_pixel=bitmap->create_iterator(0,0), pixel;

  g1_block_map_class *block_map = map->get_block_map(grade);
  for (ry=0; ry<mh; ry++)
  {
    y = mh-1-ry;
    for (x=0; x<mw; x++)
    {
      w32 color = map->vertex(x,y).height;
      
      color = critical_color(x,y);
      
      if (solvemap->is_visited(x,y))
        color |= 0x004f00;
      if (solvemap->is_ok(x,y))
        color = 0x008f00;

      if (block_map->is_blocked(x,y, G1_NORTH | G1_SOUTH | G1_EAST | G1_WEST))
        color |= 0x800000;

      w32 half_color = (color & 0xfefefe)>>1;

      int blockN, blockW;

      blockN = (y==mh-1 ||
                block_map->is_blocked(x,y,G1_NORTH) ||
                block_map->is_blocked(x,y+1,G1_SOUTH));
      blockW = (x==0 ||
                block_map->is_blocked(x,y,G1_WEST) ||
                block_map->is_blocked(x-1,y,G1_EAST));
      pixel=block_pixel;
      for (py=0; py<CELL_SIZE; py++)
      {
        for (px=0; px<CELL_SIZE; px++, ++pixel)
          bitmap->iterator_store(pixel, (  (px == 0 & blockW)? 0xffffff 
                                         : (py == 0 & blockN)? 0xffffff
                                         : (px|py == 0)? half_color 
                                         : color ));

        pixel += bitmap->width() - CELL_SIZE;
      }

      block_pixel += CELL_SIZE;

    }
    block_pixel += (CELL_SIZE-1) * bitmap->width();
  }

  i4_draw_context_class tmp_context(0,0,bitmap->width()-1,bitmap->height()-1);

#if 0
  //{{{ Draw Section Boundaries
  test_block_map::CBounds::CBlockPoint *p,*q;
  w16 pi=map->bounds.begin(); 
  while (map->bounds.next_point(pi)) 
  {
    p = map->bounds.get_point(pi);
    for (int l=0; l<4; l++) 
    {
      if (p->edge[l]>pi) {
        q = map->bounds.get_point(p->edge[l]);
        bitmap->line(int(p->x*CELL_SIZE),int((mh-p->y)*CELL_SIZE-1), 
                     int(q->x*CELL_SIZE),int((mh-q->y)*CELL_SIZE-1), 
                     0x808080, tmp_context);
      }
      bitmap->put_pixel(int(p->x*CELL_SIZE),int((mh-p->y)*CELL_SIZE)-1, 0x0080ff, tmp_context);
    }
  }
  //}}}
#endif

  for (j=1; j<critical_graph->criticals; j++)
  {
    g1_critical_graph_class::critical_point_class *crit = &critical_graph->critical[j];
    for (i=0; i<crit->connections; i++) 
    {
      if (crit->connection[i].size[grade])
      {
        w32 k=crit->connection[i].ref;
        i4_float x1,y1, x2,y2, x3,y3, x4,y4;
        x1 = crit->x*CELL_SIZE + CELL_SIZE/2;
        y1 = (mh*CELL_SIZE-1) - (crit->y*CELL_SIZE + CELL_SIZE/2);
        x4 = critical_graph->critical[k].x*CELL_SIZE + CELL_SIZE/2;
        y4 = (mh*CELL_SIZE-1) - (critical_graph->critical[k].y*CELL_SIZE + CELL_SIZE/2);
        
        x2 = (x4-x1)*0.3 + x1;
        y2 = (y4-y1)*0.3 + y1;
        x3 = (x4-x1)*0.7 + x1;
        y3 = (y4-y1)*0.7 + y1;
        
        bitmap->line((w16)x1,(w16)y1,(w16)x2,(w16)y2, 
                     0x000800*(crit->connection[i].size[grade]), tmp_context);
        bitmap->line((w16)x2,(w16)y2,(w16)x3,(w16)y3, 0x404020, tmp_context);
      }
    }
  }

  
  for (j=points-1; j>0; j--)    // points solved in backwards order
  {
    w32
      x1 = w32(point[j*2-2])*CELL_SIZE + CELL_SIZE/2,
      y1 = (mh*CELL_SIZE-1) - w32(point[j*2-1])*CELL_SIZE - CELL_SIZE/2,
      x2 = w32(point[j*2+0])*CELL_SIZE + CELL_SIZE/2,
      y2 = (mh*CELL_SIZE-1) - w32(point[j*2+1])*CELL_SIZE - CELL_SIZE/2;

    bitmap->line(x1,y1,x2,y2,0xffff00, tmp_context);
  }

}
//}}}

void g1_path_window_class::parent_draw(i4_draw_context_class &context)
//{{{
{
  int mw=map->width(), mh=map->height();

  if (map_changed)
    draw_to_bitmap();
    
  bitmap->put_image(local_image, 0,0, context);

  int x,y;
  x = start.x * CELL_SIZE + CELL_SIZE/2+1 - start_icon->width()/2;
  y = (mh-1-start.y) * CELL_SIZE + CELL_SIZE/2+1 - start_icon->height()/2;

  start_icon->put_image_trans(local_image, x, y, 0, context);

  x = dest.x * CELL_SIZE + CELL_SIZE/2+1 - dest_icon->width()/2;
  y = (mh-1-dest.y) * CELL_SIZE + CELL_SIZE/2+1 - dest_icon->height()/2;

  dest_icon->put_image_trans(local_image, x, y, 0, context);

  for (int i=0; i<critical_graph->criticals; i++)
  {
    x = sw32(critical_graph->critical[i].x)*CELL_SIZE
      + CELL_SIZE/2+1 - crit_icon->width()/2;
    y = (mh-1-sw32(critical_graph->critical[i].y))*CELL_SIZE
      + CELL_SIZE/2+1 - crit_icon->height()/2;
      
    crit_icon->put_image_trans(local_image, x, y, 0, context);
  }
}
//}}}
  
void g1_path_window_class::receive_event(i4_event *ev)
{
    
  switch (ev->type()) 
  {
    case i4_event::MOUSE_MOVE: 
      //{{{
    {
      CAST_PTR(mev, i4_mouse_move_event_class, ev);
        
      last_x = mev->x; last_y = mev->y;
    } break;
    //}}}
    case i4_event::MOUSE_BUTTON_UP: 
      //{{{
    {
      CAST_PTR(mbev, i4_mouse_button_up_event_class, ev);
        
      int cell_x=last_x/CELL_SIZE,
        cell_y=map->height()-1-(last_y/CELL_SIZE);
        
      // determine type
        
      switch (mode)
      {
        case P1_SET_START :
          //{{{
        {
          if (mbev->left())
          {
            start.x=cell_x;
            start.y=cell_y;
          }
          else
          {
            dest.x=cell_x;
            dest.y=cell_y;
          }
          solve();
          changed();
        } break;
        //}}}
        case P1_SET_DESTINATION :
          //{{{
        {
          dest.x=cell_x;
          dest.y=cell_y;
          solve();
          changed();
        } break;
        //}}}
        case P1_SET_BLOCK:
          //{{{
        {
          int flags=G1_NORTH | G1_EAST | G1_WEST | G1_SOUTH;
              
          if (mbev->left())
            map->get_block_map(grade)->block(cell_x, cell_y, flags);
          else
            map->get_block_map(grade)->unblock(cell_x, cell_y, flags);
              
          maker->make_criticals(map, critical_graph);
          solve();
          changed();
        } break;
        //}}}
        case P1_SET_CRITICAL:
          //{{{
        {
          g1_editor_instance.add_undo(G1_MAP_CRITICAL_POINTS);
          if (map) map->mark_for_recalc(G1_RECALC_CRITICAL_DATA);

          if (mbev->left() && mbev->right())
            critical_graph->criticals=0;
          else if (mbev->left()) 
            critical_graph->add_critical_point(cell_x, cell_y);
          else if (mbev->right())
          {
            if (critical_graph->criticals>0)
              --critical_graph->criticals;
          }
            
          request_redraw();
        } break;
        //}}}
      }
        
    } break;
    //}}}
    case i4_event::USER_MESSAGE:
      //{{{
    {
      CAST_PTR(uev, i4_user_message_event_class, ev);
      switch (uev->sub_type) 
      {
        case P1_SET_BLOCK :
        case P1_LAY_OBJECT :
        case P1_SET_START :
        case P1_SET_DESTINATION :
        case P1_SET_CRITICAL :
        case P1_STOP : 
        case P1_STEP :
          mode=uev->sub_type;
          break;

        case P1_SET_GRADE1:
        case P1_SET_GRADE2:
        case P1_SET_GRADE3:
        case P1_SET_GRADE4:
          if (grade == uev->sub_type - P1_SET_GRADE1)
            tofrom = !tofrom;
          else
          {
            grade = uev->sub_type - P1_SET_GRADE1;
            solvemap->set_block_map(map->get_block_map(grade));
            solve();
          }
          changed();
          break;

        case P1_SET_SIZE1:
        case P1_SET_SIZE3:
          size = uev->sub_type - P1_SET_SIZE1 + 1;
          solve();
          changed();
          break;

        case P1_RELOAD_MAP:
          //{{{
        {
          g1_editor_instance.add_undo(G1_MAP_CRITICAL_DATA);
          critical_graph = map->get_critical_graph();
          maker->make_criticals(map, critical_graph);
          changed();
        } break;
        //}}}

        case P1_LOAD_HOTSPOTS:
          //{{{
        {
          int i,j,n;
          g1_map_cell_class *c=&map->cell(0,0);

          int g1_takeover_pad_type=g1_get_object_type("takeover_pad");

          g1_editor_instance.add_undo(G1_MAP_CRITICAL_POINTS);
          for (j=0; j<map->height(); j++)
            for (i=0; i<map->height(); i++, c++)
            {
              for (g1_object_chain_class *obj=c->get_obj_list(); obj; obj=obj->next)
              {
                if (obj->object->id == g1_takeover_pad_type)
                {
                  int found=0;
                  for (n=0; n<critical_graph->criticals; n++)
                    if (critical_graph->critical[n].x==i &&
                        critical_graph->critical[n].y==j)
                      found = 1;
                  if (!found)
                  {
                    critical_graph->critical[critical_graph->criticals].x = i;
                    critical_graph->critical[critical_graph->criticals].y = j;
                    critical_graph->criticals++;
                  }
                }
              }
            }
              
          changed();
        } break;
        //}}}
      }      
    }
    //}}}
  }
}

//{{{ Emacs Locals
// Local Variables:
// folded-file: t
// End:
//}}}
