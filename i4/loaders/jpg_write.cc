/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "image/image.hh"
#include "file/file.hh"
#include "status/status.hh"
#include "loaders/jpg/jpeglib.h"


boolean i4_jpg_empty(j_compress_ptr cinfo);
void i4_jpg_null(j_compress_ptr cinfo) { }
void i4_jpg_fin(j_compress_ptr cinfo);

enum { JPG_BUF_SIZE=10*1024 };

struct i4_jpg_write_mgr : public jpeg_destination_mgr 
{
  w8 buf[JPG_BUF_SIZE];
  int size;
  i4_file_class *fp;

  i4_jpg_write_mgr(i4_file_class *fp) : fp(fp)
  {
    init_destination=i4_jpg_null;
    empty_output_buffer=i4_jpg_empty;
    term_destination=i4_jpg_fin;

    next_output_byte=(JOCTET *)buf;
    free_in_buffer=JPG_BUF_SIZE;
  } 
} *i4_current_jpg_write_mgr;

boolean i4_jpg_empty(j_compress_ptr cinfo)
{
  i4_jpg_write_mgr *m=i4_current_jpg_write_mgr;

  int ws=JPG_BUF_SIZE;
  i4_current_jpg_write_mgr->fp->write(m->buf, ws);
                                     
  m->size+=ws;
  m->next_output_byte=(JOCTET *)i4_current_jpg_write_mgr->buf;
  m->free_in_buffer=JPG_BUF_SIZE;

  return TRUE;
}


void i4_jpg_fin(j_compress_ptr cinfo)
{
  i4_jpg_write_mgr *m=i4_current_jpg_write_mgr;
  int ws=(w8 *)m->next_output_byte-m->buf;
  i4_current_jpg_write_mgr->fp->write(m->buf, ws);
}

// return size of jpeg written
int i4_write_jpeg(i4_image_class *im,
                    i4_file_class *fp,
                    int quality,           // 0..100
                    i4_status_class *status)
{
  if (im->width()>4096)
    return 0;

  /* This struct contains the JPEG compression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   * It is possible to have several such structures, representing multiple
   * compression/decompression processes, in existence at once.  We refer
   * to any one struct (and its associated working data) as a "JPEG object".
   */
  struct jpeg_compress_struct cinfo;
  /* This struct represents a JPEG error handler.  It is declared separately
   * because applications often want to supply a specialized error handler
   * (see the second half of this file for an example).  But here we just
   * take the easy way out and use the standard error handler, which will
   * print a message on stderr and call exit() if compression fails.
   * Note that this struct must live as long as the main JPEG parameter
   * struct, to avoid dangling-pointer problems.
   */
  struct jpeg_error_mgr jerr;
  /* More stuff */

  /* Step 1: allocate and initialize JPEG compression object */

  /* We have to set up the error handler first, in case the initialization
   * step fails.  (Unlikely, but it could happen if you are out of memory.)
   * This routine fills in the contents of struct jerr, and returns jerr's
   * address which we place into the link field in cinfo.
   */
  cinfo.err = jpeg_std_error(&jerr);
  /* Now we can initialize the JPEG compression object. */
  jpeg_create_compress(&cinfo);

  /* Step 2: specify data destination (eg, a file) */
  /* Note: steps 2 and 3 can be done in either order. */

  
  i4_jpg_write_mgr write_mgr(fp);
  i4_current_jpg_write_mgr=&write_mgr;

  cinfo.dest = &write_mgr;


  /* Step 3: set parameters for compression */
  /* First we supply a description of the input image.
   * Four fields of the cinfo struct must be filled in:
   */
  cinfo.image_width = im->width(); 	/* image width and height, in pixels */
  cinfo.image_height = im->height();
  cinfo.input_components = 3;		/* # of color components per pixel */
  cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
  /* Now use the library's routine to set default compression parameters.
   * (You must set at least cinfo.in_color_space before calling this,
   * since the defaults depend on the source color space.)
   */
  jpeg_set_defaults(&cinfo);
  /* Now you can set any non-default parameters you wish to.
   * Here we just illustrate the use of quality (quantization table) scaling:
   */
  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

  /* Step 4: Start compressor */

  /* TRUE ensures that we will write a complete interchange-JPEG file.
   * Pass TRUE unless you are very sure of what you're doing.
   */
  jpeg_start_compress(&cinfo, TRUE);

  /* Step 5: while (scan lines remain to be written) */
  /*           jpeg_write_scanlines(...); */


  w8 buf[4096*3]; 
 
  int im_w=im->width();

  while (cinfo.next_scanline < cinfo.image_height) 
  {
    if (status)
      status->update((float)cinfo.next_scanline/cinfo.image_height);

    JSAMPROW scanline[1]={buf};
    w8 *c=buf;
        
    for (int x=0; x<im_w; x++)
    {
      i4_color color=im->get_pixel(x, cinfo.next_scanline);
      c[0]=(color>>16)&0xff;
      c[1]=(color>>8)&0xff;
      c[2]=(color)&0xff;
      c+=3;
    }
    jpeg_write_scanlines(&cinfo, scanline, 1);
  }

  /* Step 6: Finish compression */

  jpeg_finish_compress(&cinfo);

  /* Step 7: release JPEG compression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_compress(&cinfo);


  return write_mgr.size;
  /* And we're done! */
}
