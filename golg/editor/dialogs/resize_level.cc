/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "editor/editor.hh"
#include "editor/pmenu.hh"
#include "gui/create_dialog.hh"
#include "gui/text_input.hh"                                       
#include "remove_man.hh"
#include "m_flow.hh"
#include "editor/e_res.hh"

void g1_editor_class::open_resize_level_window()
{
  if (get_map())
  {
    create_modal(300, 250, "resize_title");

    i4_create_dialog(g1_ges("resize_dialog"), 
                     modal_window.get(),
                     style,
                     get_map()->width(), get_map()->height(), 

                     G1_MIN_MAP_DIMENSION, G1_MAX_MAP_DIMENSION,
                     &resize_dialog.w, get_map()->width(), 

                     G1_MIN_MAP_DIMENSION, G1_MAX_MAP_DIMENSION,
                     &resize_dialog.h, get_map()->height(),

                     this, G1_EDITOR_RESIZE_PLACE_LT,
                     this, G1_EDITOR_RESIZE_PLACE_CT,
                     this, G1_EDITOR_RESIZE_PLACE_RT,
                     this, G1_EDITOR_RESIZE_PLACE_LC,
                     this, G1_EDITOR_RESIZE_PLACE_CC,
                     this, G1_EDITOR_RESIZE_PLACE_RC,
                     this, G1_EDITOR_RESIZE_PLACE_BL,
                     this, G1_EDITOR_RESIZE_PLACE_BC,
                     this, G1_EDITOR_RESIZE_PLACE_BR,

                     this, G1_EDITOR_RESIZE_MAP_OK,
                     this, G1_EDITOR_MODAL_BOX_CANCEL);
    resize_dialog.orient=4;
  }

}

void g1_editor_class::resize_level() 
{
  int w,h;

  i4_str::iterator is=resize_dialog.w->get_edit_string()->begin();
  w=is.read_number();
  is=resize_dialog.h->get_edit_string()->begin();
  h=is.read_number();
  
  
  if (!(w>=G1_MIN_MAP_DIMENSION && h>=G1_MIN_MAP_DIMENSION 
        && w<=G1_MAX_MAP_DIMENSION && h<=G1_MAX_MAP_DIMENSION))
  {
    create_modal(300, 50, "bad_w_h_title");
    i4_create_dialog(g1_ges("bad_map_w_h_dialog"), 
                     modal_window.get(), style, w, h, 
                     G1_MIN_MAP_DIMENSION,
                     G1_MAX_MAP_DIMENSION,
                     this, G1_EDITOR_MODAL_BOX_CANCEL);
  }
  else 
  {
    if (g1_map_is_loaded())
    {
      int sx1,sy1,sx2,sy2, dx1,dy1,dx2,dy2, x,y, i;
      int dir=resize_dialog.orient;
    
      if (w>=get_map()->width())
      {
        if (dir==0 || dir==3 || dir==6)
          dx1=0;
        else if (dir==1 || dir==4 || dir==7)
          dx1=w/2-get_map()->width()/2;
        else
          dx1=w-get_map()->width();

        dx2=dx1+w-1;

        sx1=0; sx2=get_map()->width()-1;
      }
      else                                    // need to chop width
      {
        if (dir==0 || dir==3 || dir==6)       // chop off the right
          sx1=0;
        else if (dir==1 || dir==4 || dir==7)  // chop off left and right
          sx1=get_map()->width()/2-w/2;
        else 
          sx1=get_map()->width()-w;                // chop off left

        sx2=sx1+w-1;
        dx1=0; dx2=w-1;
      }

    
      if (h>=get_map()->height())
      {
        if (dir==6 || dir==7 || dir==8)
          dy1=0;
        else if (dir==3 || dir==4 || dir==5)
          dy1=h/2-get_map()->height()/2;
        else
          dy1=w-get_map()->height();

        dy2=dy1+h-1;

        sy1=0; sy2=get_map()->height()-1;
      }
      else                                    // need to chop height
      {
        if (dir==6 || dir==7 || dir==8)       // chop off the bottom
          sy1=0;
        else if (dir==3 || dir==4 || dir==5)  // chop off top and bottom
          sy1=get_map()->height()/2-h/2;
        else
          sy1=get_map()->height()-h;                // chop off top

        sy2=sy1+h-1;
        dy1=0; dy2=h-1;
      }


      g1_map_cell_class *ncells=(g1_map_cell_class *)i4_malloc(w * h *sizeof(g1_map_cell_class),
                                                               "map_cells");

      g1_map_vertex_class *nverts=(g1_map_vertex_class *)i4_malloc((w+1) * (h+1) *
                                                                   sizeof(g1_map_vertex_class),
                                                                   "map_verts");
      
      // first initial all the new stuff
      g1_map_cell_class *c1=ncells, *c2;
      for (y=0; y<h; y++)
        for (x=0; x<w; x++, c1++)
        {
          c1->init(0, G1_ROTATE_0, i4_F);
          if (x==0 || y==0 || x==w-1 || y==h-1)
            c1->flags=0;
          else
            c1->flags=g1_map_cell_class::IS_GROUND;
        }
      
      g1_map_vertex_class *v1=nverts,*v2;
      for (i=0; i<(w+1)*(h+1); i++, v1++)
        v1->init();

      // copy old area
      int yl=sy2-sy1+1;
      for (y=0; y<yl; y++)
      {
        c1=ncells + (dy1+y)*w + dx1;
        c2=get_map()->cells + (sy1+y)*get_map()->width() + sx1;

        for (x=sx1; x<=sx2; x++, c1++, c2++)
        {
          *c1=*c2;
          c1->object_list=0;
        }
      }

      yl=sy2-sy1+2;
      for (y=0; y<yl; y++)
      {
        v1=nverts + (dy1+y)*(w+1) + dx1;
        v2=get_map()->verts + (sy1+y)*(get_map()->width()+1) + sx1;

        for (x=sx1; x<=sx2+1; x++, v1++, v2++)
          *v1=*v2;
      }   


      // take objects off old map and move them
      g1_object_class *olist[G1_MAX_OBJECTS];
      sw32 t=get_map()->make_object_list(olist, G1_MAX_OBJECTS);
      for (i=0; i<t; i++)
      {
        g1_object_class *o=olist[i];
        o->unoccupy_location();
        o->x+=(dx1-sx1);
        o->y+=(dy1-sy1);

        if (o->x<0 || o->y<0 || o->x>=w || o->y>=h)
        {
          get_map()->request_remove(o);
          g1_remove_man.process_requests();
          olist[i]=0;

        }
        else
        {
          o->lx=o->x;
          o->ly=o->y;
          o->lh=o->h;
        }
      }

      i4_free(get_map()->cells);
      i4_free(get_map()->verts);



      get_map()->cells=ncells;            // swap data with new stuff
      get_map()->verts=nverts;
      get_map()->w=w;
      get_map()->h=h;
      
      // add objects onto the map
      for (i=0; i<t; i++)
        if (olist[i])
          olist[i]->occupy_location();

      // move the movie
      g1_movie_flow_class *movie=get_map()->current_movie;
      if (movie)
      {
        for (i=0; i<movie->t_cut_scenes; i++)
          movie->set[i]->move((dx1-sx1), (dy1-sy1), 0);
      }

      get_map()->mark_for_recalc(0xffffff);

    }

    close_modal();
  }
}

