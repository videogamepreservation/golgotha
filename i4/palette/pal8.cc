/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "palette/pal8.hh"
#include "error/error.hh"

i4_pal8_manager i4_pal8_man;

i4_pal8_manager::i4_pal8_manager()
{
  memset(palette_references,0,sizeof(palette_references));    // no reference to any palettes yet
}

w8 *i4_pal8_manager::get_remap_table(i4_pal8_handle from, i4_pal8_handle to)
{
  w16 index=from.id*MAX_PALETTES+to.id;

  w8 *data=remap_tables+index*256;

  if (tables_calculated.get(index))
    return data;
  else
  {
    w32 *from_pal=palette_tables+from.id*256;
    w32 *to_pal=palette_tables+to.id*256;

    w16 x,y;
    w32 c1,c2;
    sw16 r1,g1,b1,
         r2,g2,b2,nd,d=-1,best;

    for (x=0;x<256;x++)
    {
      c1=*from_pal;

      r1=(c1&0xff0000)>>16;
      g1=(c1&0xff00)>>8;
      b1=(c1&0xff);
      

      for (y=0;y<256;y++)
      {
	c2=to_pal[y];

	r2=(c2&0xff0000)>>16;
	g2=(c2&0xff00)>>8;
	b2=(c2&0xff);       
	nd=(r1-r2)*(r1-r2)+(g1-g2)*(g1-g2)+(b1-b2)*(b1-b2);	
	if (nd<d)
	{
	  d=nd;
	  best=y;
	}
      }      
      *(data++)=best;      
    }
    return remap_tables+index*256;
  }
}

i4_pal8_handle i4_pal8_manager::register_pal(w32 *palette_data)
{
  int x;
  for (x=0;x<MAX_PALETTES;x++)
  {
    if (!palettes_allocated.get(x))
    {      
      palettes_allocated.set(x,i4_T);
      memcpy(palette_tables+x*256,palette_data,256*4);
      i4_pal8_handle ret(x);
      return ret;
    }
  }

  i4_error("Max palette allocation = %d, exceeded",MAX_PALETTES);
}

void i4_pal8_manager::unregister_pal(i4_pal8_handle id)
{
  if (!palettes_allocated.get(id.id))
    i4_error("unregistering bad palette");
  else if (palette_references[id.id])
    i4_error("unregistering referenced palette");
  else
    palettes_allocated.set(id.id,i4_F);
    

}

