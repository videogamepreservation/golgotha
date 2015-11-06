/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "map.hh"

#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif

void g1_map_class::make_block_maps()
{
  static i4_float grade_max[G1_GRADE_LEVELS] = { 10, 17, 30, 256 };
  w32 grade, x,y;

  for (grade = 0; grade<G1_GRADE_LEVELS; grade++)
  {
    block[grade].init(width(),height());
    block[grade].clear();

    for (x=0; x<width(); x++)
      for (y=0; y<height(); y++)
      {
        if (x==0 || y==0 || x==width()-1 || y==height()-1 || 
            (cell(x,y)->is_blocking() && grade<G1_GRADE_LEVELS-1))
          block[grade].block(x,y, G1_NORTH|G1_SOUTH|G1_WEST|G1_EAST);
        else
        {
          g1_object_chain_class *ch = cell(x,y)->get_solid_list();
          while (ch &&
                 (!ch->object->get_flag(g1_object_class::BLOCKING) ||
                  g1_object_type_array[ch->object->id]->
                  get_flag(g1_object_definition_class::MOVABLE)))
            ch = ch->next;

          if (ch)
            block[grade].block(x,y, G1_NORTH|G1_SOUTH|G1_WEST|G1_EAST);
          else
          {
            i4_float max_grade = grade_max[grade];
            g1_map_vertex_class *v1, *v2, *v3, *v4;
          
            v1=verts + x + y * (w+1);
            v2=v1+1;
            v3=v1+w+1;
            v4=v3+1;
          
            w8 h1=v1->height, 
              h2=v2->height, 
              h3=v3->height, 
              h4=v4->height;
          
            i4_float
              grade_ns = max(h3-h1, h4-h2),
              grade_ew = max(h2-h1, h4-h3),
              grade_sn = max(h1-h3, h2-h4),
              grade_we = max(h1-h2, h3-h4);
          
            if (grade_ns>max_grade || grade_sn>max_grade)
            {
              if (grade_ns>max_grade)
                block[grade].block(x,y,G1_SOUTH);
              if (grade_sn>max_grade)
                block[grade].block(x,y,G1_NORTH);

              if (grade_ew>0)
                block[grade].block(x,y,G1_WEST);
              if (grade_we>0)
                block[grade].block(x,y,G1_EAST);
            }
            if (grade_ew>max_grade || grade_we>max_grade)
            {
              if (grade_ew>max_grade)
                block[grade].block(x,y,G1_WEST);
              if (grade_we>max_grade)
                block[grade].block(x,y,G1_EAST);

              if (grade_ns>0)
                block[grade].block(x,y,G1_SOUTH);
              if (grade_sn>0)
                block[grade].block(x,y,G1_NORTH);
            }
          }
        }
      }
  }
}

