/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "file/get_filename.hh"
#include "editor/editor.hh"
#include "gui/create_dialog.hh"
#include "mess_id.hh"
#include "loaders/load.hh"
#include "image/image32.hh"
#include "loaders/tga_write.hh"
#include "status/status.hh"
#include "editor/e_res.hh"
#include "map_vert.hh"

void g1_editor_class::load_height_bitmap()
{
  i4_create_file_open_dialog(style,
                             g1_ges("load_height_map_title"),
                             g1_ges("load_height_start_dir"),
                             g1_ges("load_height_file_mask"),
                             g1_ges("load_height_mask_name"),
                             this,
                             G1_EDITOR_LOAD_TERRAIN_HEIGHT_OK,
                             G1_EDITOR_NO_OP);
}

void g1_editor_class::save_height_bitmap()
{
  if (get_map())
  {
    i4_filename_struct fn;
    i4_split_path(get_map()->get_filename(), fn);

    char dname[256];
    sprintf(dname, "%s.tga", fn.filename);
    
    i4_create_file_save_dialog(style,
                               dname,
                               g1_ges("save_height_map_title"),
                               g1_ges("save_height_start_dir"),
                               g1_ges("save_height_file_mask"),
                               g1_ges("save_height_mask_name"),
                               this,
                               G1_EDITOR_SAVE_TERRAIN_HEIGHT_OK,
                               G1_EDITOR_NO_OP);
  }
}


void g1_editor_class::save_height_bitmap_ok(i4_event *ev)
{
  int i;
  CAST_PTR(fo, i4_file_open_message_class, ev);
  
  if (fo->filename && get_map())
  {
    i4_file_class *fp=i4_open(*fo->filename, I4_WRITE);
    if (!fp)
    {
      create_modal(500, 100, "save_height_bad_file_tile");
      i4_create_dialog(g1_ges("save_height_bad_file_dialog"), 
                         modal_window.get(), style, fo->filename, 
                         this, G1_EDITOR_MODAL_BOX_CANCEL);
    }
    else
    {
      i4_pixel_format fmt;
      fmt.default_format();
      
      const i4_pal *pal=i4_pal_man.register_pal(&fmt);

      int mw=get_map()->width()+1, mh=get_map()->height()+1;

      i4_image_class *im=i4_create_image(mw, mh, pal);
      w32 *i32=(w32 *)im->data;

      g1_map_vertex_class *v=get_map()->verts;
      int x,y;
      for (y=0; y<mh; y++)
        for (x=0; x<mw; x++, v++)
        {
          *i32=get_map()->vertex(x,mh-y-1)->height;
          i32++;
        }


      i4_tga_write(im,fp);
      delete fp;
      delete im;

    }
  }
}




void g1_editor_class::load_height_bitmap_ok(i4_event *ev)
{
  int i;
  CAST_PTR(fo, i4_file_open_message_class, ev);
  
  if (fo->filename && get_map())
  {
    int mw=get_map()->width()+1, mh=get_map()->height()+1;

    
    i4_status_class *stat=i4_create_status(g1_ges("loading_terrain_bitmap"));

    i4_image_class *im=i4_load_image(*fo->filename, stat);

    if (stat)
      delete stat;

    

    if (im)
    {

      if (im->width()!=mw && im->height()!=mh)
      {
        create_modal(500, 100, "load_height_bad_size_title");
        i4_create_dialog(g1_ges("load_height_bad_size_dialog"), 
                         modal_window.get(), style, fo->filename, 
                         im->width(), im->height(), get_map()->width()+1, get_map()->height()+1,
                         this, G1_EDITOR_MODAL_BOX_CANCEL);
        delete im;
      }
      else
      {
        i4_status_class *stat=i4_create_status(g1_ges("applying_terrain_map"));

        const i4_pixel_format *fmt=&im->get_pal()->source;
        i4_draw_context_class c(0,0,mw-1, mh-1);
        g1_map_vertex_class *v=get_map()->verts;

        g1_object_class *olist[G1_MAX_OBJECTS];
        sw32 t=get_map()->make_object_list(olist, G1_MAX_OBJECTS);
        for (i=0; i<t; i++)
          olist[i]->unoccupy_location();

        int x,y;
        for (y=0; y<mh; y++)
        {
          if (stat)
            stat->update(y/(float)mh);

          for (x=0; x<mw; x++, v++)
          {
            w8 h=i4_pal_man.convert_32_to(im->get_pixel(x,y,c), fmt) & 0xff;
            get_map()->change_vert_height(x,mh-y-1, h);
          }
          
        }

        if (stat)
          delete stat;
        delete im;

        for (i=0; i<t; i++)
          olist[i]->occupy_location();
      }
    }
    else 
    {
      create_modal(500, 100, "couldn't_load_image_title");
      i4_create_dialog(g1_ges("couldn't_load_image_dialog"), 
                       modal_window.get(), style, fo->filename, this, G1_EDITOR_MODAL_BOX_CANCEL);
    }
  }
}
