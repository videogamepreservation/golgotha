/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "objs/sprite.hh"
#include "error/error.hh"
#include "load3d.hh"
#include "string/string.hh"
#include "error/alert.hh"
#include "obj3d.hh"
#include "objs/sprite_id.hh"
#include "saver.hh"
#include "r1_api.hh"
#include "tmanage.hh"

extern i4_grow_heap_class *g1_object_heap;

g1_sprite_list_class g1_sprite_list_man;

// todo : add a sprite loader instead of mouching the load3d
// its mooching, jonathan. -trey

void g1_sprite_list_class::load(r1_render_api_class *tmap)
{
  i4_const_str *sprites=i4_string_man.get_array("sprite_array");
  w32 count;

  g1_quad_object_loader_class loader(g1_object_heap);

  for (count=0; !sprites[count].null(); count++)
  {  
    I4_ASSERT(count<=G1_MAX_SPRITES, "increase G1_MAX_SPRITES");

    i4_str *name=i4gets("spr_fmt").sprintf(200, &sprites[count]);
    i4_file_class *in_file=i4_open(*name);
    
    
    if (!in_file)
      i4_alert(i4gets("file_missing"),200,sprites+count);
    else
    {
      g1_loader_class *fp=g1_open_save_file(in_file);
      if (fp)
      {
        g1_quad_object_class *tmp=loader.load(fp, sprites[count], tmap->get_tmanager());
        tmp->scale(0.1);

        array[count].add_x=tmp->animation[0].vertex[0].v.x;
        array[count].add_y=tmp->animation[0].vertex[0].v.y;
        array[count].texture       = tmp->quad[0].material_ref;
        array[count].texture_scale = tmp->quad[0].texture_scale;
        array[count].extent        = tmp->extent;                
        
        r1_texture_handle material=tmp->quad[0].material_ref;
        int alen=r1_render_api_class_instance->get_tmanager()->get_animation_length(material);
        array[count].num_animation_frames = alen;

        delete fp;
      }
      else
        i4_alert(i4gets("old_model_file"),200,sprites+count);
    }
    delete name;
  }

  if (count!=G1_SPRITE_LAST)
  {
    i4_alert(i4gets("sprite_num"),100,count,G1_SPRITE_LAST);
    i4_error("");
  }

  i4_free(sprites);
}
