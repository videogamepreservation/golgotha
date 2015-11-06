/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "video/x11/mitshm.hh"
#include "video/x11/x11devs.hh"
#include "error/error.hh"

#include <X11/Xlib.h> 
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#include <signal.h>
#include <string.h>

class x11_shm_image_actual_class : public x11_shm_image_class
{
  public :

  XShmSegmentInfo X_shminfo;    
  XImage *im;
  Display *display;
  GC gc;
  Window window;  

  x11_shm_image_actual_class(Display *display, Window window, GC gc) : 
    display(display), 
    window(window), 
    gc(gc)
  { ; }

  i4_bool copy_part_to_vram(x11_shm_extension_class *use, 
                            i4_coord x, i4_coord y, 
                            i4_coord x1, i4_coord y1, 
                            i4_coord x2, i4_coord y2);

  ~x11_shm_image_actual_class();
} ;



class x11_shm_extension_actual_class : public x11_shm_extension_class
{
  public :
  virtual void init() 
  { 
    x11_display_instance.shm_extension=this;   // tell the display this extension exsist
  }

  void note_event(XEvent &ev)
  {
    if (ev.type==shm_base+ShmCompletion)
      need_sync_event=i4_F;
  }

  virtual i4_bool available(Display *display, char *display_name);

   // actual width and height set by X are returned
  x11_shm_image_class *create_shm_image(Display *display,
					  Window window,
					  GC gc,
					  Visual *X_visual,
					  int visual_depth,
					  w16 &width, w16 &height);

  virtual void shutdown(Display *display)
  {
    XFlush(display);
    XSync(display,False);  // maker sure anything with this image is drawn!
    need_sync_event=i4_F;
  }


  void destroy_shm_image(Display *display, x11_shm_image_class *im);
} shm_instance;

x11_shm_extension_class::x11_shm_extension_class() 
{ 
  need_sync_event=i4_F; 
}

i4_bool x11_shm_extension_actual_class::available(Display *display, char *display_name)
{  
  int major_op;

  if (XQueryExtension(display,"MIT-SHM",&major_op,&shm_base,&shm_error_base))
  {
    char *d = display_name;             // make sure the display is local
    while (*d && (*d != ':')) d++;
    if (*d) *d = 0;
    if (strcasecmp(display_name, "unix") && display_name[0]!=0) 
      return i4_F;

    return i4_T;
  } else return i4_F;
}

void x11_shm_extension_actual_class::destroy_shm_image(Display *display, x11_shm_image_class *im)
{
  XFlush(display);
  XSync(display,False);  // maker sure anything with this image is drawn!

  // Dettach the memory from the server
  XShmDetach(display, &((x11_shm_image_actual_class *)im)->X_shminfo);

  // detach the memory from us, it will be deleted!
  shmdt(((x11_shm_image_actual_class *)im)->X_shminfo.shmaddr);

  delete im;
}

x11_shm_image_class *x11_shm_extension_actual_class::create_shm_image(Display *display,
						   Window window,
						   GC gc,
						   Visual *X_visual,
						   int visual_depth,
						   w16 &width, w16 &height)   // actual width and height set by X are returned
{
  width=(width+3)&(~3);
  x11_shm_image_actual_class *im=new x11_shm_image_actual_class(display,window,gc);
  

  im->im = XShmCreateImage(display,
			   X_visual,
			   visual_depth,
			   ZPixmap,
			   0,
			   &im->X_shminfo,
			   width,
			   height );

  // create the shared memory segment
  im->X_shminfo.shmid = shmget (IPC_PRIVATE, width*height*I4_BYTES_PER_PIXEL, IPC_CREAT | 0777);
  if (im->X_shminfo.shmid<0)  
    i4_error("shmget() failed, go figure");

  im->X_shminfo.readOnly=False;
  

  // attach to the shared memory segment to us
  im->im->data = im->X_shminfo.shmaddr = (char *) shmat(im->X_shminfo.shmid, 0, 0);
  if (!im->im->data)
    i4_error("shmat() failed, go figure");


  // get the X server to attach to it to the X server
  if (!XShmAttach(display, &im->X_shminfo))
    i4_error("XShmAttach() failed, go figure");

  XSync(display,False); // make sure segment gets attached

  if (shmctl(im->X_shminfo.shmid,IPC_RMID,NULL)!=0)
    i4_error("shmctl failed, why?"); 

  im->data=(w8 *) (im->im->data); 
  return im;
}

x11_shm_image_actual_class::~x11_shm_image_actual_class()
{
  XFlush(display);
  XSync(display,False);  // maker sure anything with this image is drawn!

  // Dettach the memory from the server
  if (!XShmDetach(display, &X_shminfo))
    i4_error("XShmDetach() failed, go figure");

  XSync(display,False);  // maker sure server detached

  // detach the memory from us, it will be deleted!
  if (shmdt(X_shminfo.shmaddr)<0)
    i4_error("shmdt failed, oops");

  im->data=0;  // tell X not to try to free the memory, cause we already did.

  XDestroyImage(im);
}

#include <stdio.h>

i4_bool x11_shm_image_actual_class::copy_part_to_vram(x11_shm_extension_class *use, 
                                                   i4_coord x, i4_coord y, 
                                                   i4_coord x1, i4_coord y1, 
                                                   i4_coord x2, i4_coord y2)
{
  XEvent ev;
  XSync(display,False);

  if (y1>y2 || x1>x2)
    return i4_T;


  if (use->need_sync_event)
  {
    XEvent ev;
    while (XCheckTypedEvent(display,use->shm_base+ShmCompletion,&ev)==False)
      XSync(display,False);
    use->need_sync_event=i4_F;
  }

  if (XCheckTypedEvent(display, ConfigureNotify,&ev)==False)
  {
    XShmPutImage(display,window,gc,im,x1,y1,x,y,x2-x1+1,y2-y1+1,True);
    XSync(display,False);
    use->need_sync_event=i4_T;
    return i4_T;
  } else     // screen size changed,  better wait till this event is handled cause put might be invalid
  {
    XPutBackEvent(display,&ev);
    return i4_F;
  }
    
}

