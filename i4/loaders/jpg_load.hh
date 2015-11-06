/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef _JPG_LOAD_HH_
#define _JPG_LOAD_HH_

//#define JPEG_INTERNALS
//TODO clean up memory management 
//convert types and memory and file stuff to I4
#include "file/buf_file.hh"
#include "loaders/load.hh"
#include "image/image.hh"
#include "loaders/jpg/cdjpeg.h"		/* Common decls for cjpeg/djpeg applications */
#include "loaders/jpg/jversion.h"		/* for version message */
#include "status/status.hh"
#include "image/image.hh"

#include <ctype.h>		/* to declare isprint() */

static const char * const cdjpeg_message_table[] = {
#include "loaders/jpg/cderror.h"
  NULL
};

static inline w32 read_colorbgr(w8 *cp)
{
  w8 buf[4];

  *(w32 *)buf=*((w32 *)cp);
//  if (!buf[3]) return 0;
  return (((w32)buf[0]<<16) |
              ((w32)buf[1]<<8)  |
              ((w32)buf[2]));
}

LOCAL(unsigned int)
jpeg_getc (j_decompress_ptr cinfo)
/* Read next byte */
{
  struct jpeg_source_mgr * datasrc = cinfo->src;

  if (datasrc->bytes_in_buffer == 0) {
    if (! (*datasrc->fill_input_buffer) (cinfo))
      ERREXIT(cinfo, JERR_CANT_SUSPEND);
  }
  datasrc->bytes_in_buffer--;
  return GETJOCTET(*datasrc->next_input_byte++);
}

METHODDEF(boolean)
COM_handler (j_decompress_ptr cinfo)
{
  boolean traceit = (cinfo->err->trace_level >= 1);
  INT32 length;
  unsigned int ch;
  unsigned int lastch = 0;

  length = jpeg_getc(cinfo) << 8;
  length += jpeg_getc(cinfo);
  length -= 2;			/* discount the length word itself */

  if (traceit)
    fprintf(stderr, "Comment, length %ld:\n", (long) length);

  while (--length >= 0) {
    ch = jpeg_getc(cinfo);
    if (traceit) {
      /* Emit the character in a readable form.
       * Nonprintables are converted to \nnn form,
       * while \ is converted to \\.
       * Newlines in CR, CR/LF, or LF form will be printed as one newline.
       */
      if (ch == '\r') {
	fprintf(stderr, "\n");
      } else if (ch == '\n') {
	if (lastch != '\r')
	  fprintf(stderr, "\n");
      } else if (ch == '\\') {
	fprintf(stderr, "\\\\");
      } else if (isprint(ch)) {
	putc(ch, stderr);
      } else {
	fprintf(stderr, "\\%03o", ch);
      }
      lastch = ch;
    }
  }

  if (traceit)
    fprintf(stderr, "\n");

  return TRUE;
}

class i4_jpg_loader_class : public i4_image_loader_class
{
  public :
  // returns the maximum size you need to identify your file format
  virtual w16 max_header_size() { return 10; }
  virtual i4_bool recognize_header(w8 *buf)
  {
	  if(buf[0]==0xff &&	//marker
		  buf[1]==0xd8 &&  //soi seg
		  buf[2]==0xff &&	//marker
		  buf[3]==0xe0 &&	//app0 seg
		  buf[6]=='J' && buf[7]=='F' &&
		  buf[8]=='I' && buf[9]=='F')
		  return i4_T;
	  else return i4_F;
  }

  i4_bool special_load24(i4_file_class *fp,w8 *dst_texture, sw32 *dst_width, sw32 *dst_height);

  i4_bool special_load16(i4_file_class *fp,
                         w16 *dst_texture,
                         sw32 base_width,
                         const i4_pal *pal);

  i4_bool convert_to_raw16(i4_file_class *fp,
                           FILE *dst_file,
                           i4_status_class *status,
                           const i4_pal *pal,
                           sw32 expected_width,
                           sw32 expected_height);

  // assume fp is at the start of the file
  virtual i4_image_class *load(i4_file_class *fp,
                               i4_status_class *status);
  
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;

};

extern i4_jpg_loader_class i4_jpg_loader_instance;

#endif
