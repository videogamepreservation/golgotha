/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "file/get_filename.hh"
#include "window/style.hh"
#include "window/window.hh"
#include "string/string.hh"
#include "device/device.hh"
#include "device/kernel.hh"
#include "string/string.hh"
#include "gui/text_input.hh"
#include "window/colorwin.hh"
#include "gui/create_dialog.hh"
#include <unistd.h>

class open_string : public i4_str
{
public:
  open_string(char *fname)
    : i4_str(strlen(fname))
  {
    len=strlen(fname);
    memcpy(ptr, fname, len);

  }
};


void i4_create_file_open_dialog(i4_graphical_style_class *style,
                                const i4_const_str &title_name,
                                const i4_const_str &start_dir,
                                const i4_const_str &file_mask,
                                const i4_const_str &mask_name,
                                i4_event_handler_class *tell_who,
                                w32 ok_id, w32 cancel_id)
{
  char mname[256], m[256], tname[256], idir[256], fname[256], pcmd[1000];

  i4_os_string(mask_name, mname, sizeof(mname));
  i4_os_string(file_mask, m, sizeof(m));
  i4_os_string(title_name, tname, sizeof(tname));
  i4_os_string(start_dir, idir, sizeof(idir));

  char *display=getenv("DISPLAY");
  if (!display || display[0]==0)
    display=":0.0";

  char *tmp_name="/tmp/open_filename.tmp";
  sprintf(pcmd, "xgetfile -title \"%s\" -pattern \"%s\" -path \"%s\" -popup -display %s > %s",
          tname, m, idir, display, tmp_name);

  unlink(tmp_name);
  system(pcmd);
  FILE *fp=fopen(tmp_name,"rb");
  if (!fp || !fgets(m, 256, fp))
    m[0]=0;
  else
    while (m[strlen(m)-1]=='\n')
      m[strlen(m)-1]=0;
  
  if (fp) fclose(fp);
  unlink(tmp_name);


  char *s=m;

  if (*s==0 || *s=='\n')
  {
    i4_user_message_event_class o(cancel_id);
    i4_kernel.send_event(tell_who, &o);
  }
  else
  {
    i4_file_open_message_class o(ok_id, new open_string(s));
    i4_kernel.send_event(tell_who, &o);
  }


}


class i4_get_save_dialog : public i4_color_window_class
{
  i4_text_input_class *ti;
  i4_graphical_style_class *style;
  w32 ok_id, cancel_id;
  i4_event_handler_class *tell_who;
public:
  i4_parent_window_class *mp_window;

  enum { OK, CANCEL, CLOSE };

  i4_get_save_dialog(i4_const_str default_name,
                     i4_graphical_style_class *style,
                     w32 ok_id, w32 cancel_id,
                     i4_event_handler_class *tell_who)
    : i4_color_window_class(0,0, style->color_hint->neutral(), style),
      style(style),
      ok_id(ok_id),
      cancel_id(cancel_id),
      tell_who(tell_who)
  {    
    mp_window=0;
    i4_create_dialog(i4gets("get_savename_dialog"), 
                     this,
                     style,
                     &ti,
                     &default_name,
                     this, OK,
                     this, CANCEL);
                     
    resize_to_fit_children();
  }

  void receive_event(i4_event *ev)
  {
    if (ev->type()==i4_event::USER_MESSAGE)
    {
      CAST_PTR(uev, i4_user_message_event_class, ev);
      if (uev->sub_type==OK)          // tell who-ever what the user typed
      {
        i4_file_open_message_class o(ok_id,ti->get_edit_string());
        i4_kernel.send_event(tell_who, &o);
      }
      else
      {
        i4_user_message_event_class c(cancel_id); // tell whoever the user canceled
        i4_kernel.send_event(tell_who, &c);
      }

      style->close_mp_window(mp_window);   // close ourself
    } 
    else i4_parent_window_class::receive_event(ev);
  }
  
  char *name() { return "save dialog"; }
};



void i4_create_file_save_dialog(i4_graphical_style_class *style,
                                const i4_const_str &default_name,
                                const i4_const_str &title_name,
                                const i4_const_str &start_dir,
                                const i4_const_str &file_mask,
                                const i4_const_str &mask_name,
                                i4_event_handler_class *tell_who,
                                w32 ok_id, w32 cancel_id)
{
  i4_get_save_dialog *dlg=new i4_get_save_dialog(default_name, style, 
                                                 ok_id, cancel_id, tell_who);

  i4_event_reaction_class *re;
  re=new i4_event_reaction_class(dlg, 
                                 new i4_user_message_event_class(i4_get_save_dialog::CLOSE));


  i4_parent_window_class *p;

  p=style->create_mp_window(-1, -1, dlg->width(), dlg->height(),
                            title_name,
                            re);

  p->add_child(0,0, dlg);
  dlg->mp_window=p;
}

