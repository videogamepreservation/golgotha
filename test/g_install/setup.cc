/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


#include "window/window.hh"
#include "app/app.hh"
#include "main/main.hh"
#include "window/style.hh"
#include "file/file.hh"
#include "loaders/load.hh"
#include "window/wmanager.hh"
#include "gui/text_input.hh"
#include "gui/list_box.cc"
#include "app/app.hh"
#include "menu/textitem.hh"
#include "video/display.hh"
#include "loaders/load.hh"
#include "gui/text_input.hh"
#include "gui/text.hh"
#include "font/anti_prop.hh"
#include "window/colorwin.hh"
#include "gui/button.hh"
#include "install.hh"
#include "app/registry.hh"
#include "file/get_filename.hh"
#include "gui/text_scroll.hh"
#include "loaders/wav_load.hh"
#include "loaders/mp3_load.hh"
#include "lisp/lisp.hh"
#include "status/status.hh"
#include "../../golg/g1_file.hh"
#include "status/gui_stat.hh"

static i4_window_manager_class *wm;
static i4_graphical_style_class *style;
static i4_list_box_class *display_picker, *size_picker;
static i4_text_scroll_window_class *error_win;
static char inst_path[200];
int inst_sizes[3]={20,40,60};

char *reg_key="SOFTWARE\\Crack dot Com\\Golgotha\\1.0";
char *i4_display_key="SOFTWARE\\Crack dot Com\\I4\\1.0";
char *sfx_key="SOFTWARE\\Crack dot Com\\i4\\1.0";

i4_file_class *uninstall_script=0;

int installed=0;
int col2=120;
int size_needed_to_install=0;

i4_text_input_class *path=0;


enum { 
  BROWSE_OK,
  BROWSE_CANCEL,
  BROWSE,
  SAVE_AND_QUIT,
  SAVE_AND_RUN,
  INSTALL,
  QUIT,
  COMPACT,
  TYPICAL,
  UNINSTALL,
  DISPLAY_SELECT
};



void fix_path(char *f)
{
  for (;*f; f++)
#ifdef __linux
    if (*f=='\\')
      *f='/';
#else
    if (*f=='/')
      *f='\\';
#endif

}


#ifdef __linux
int free_space(char *path)
{
  reutn 100*1024*1024;
}
#else

#include <windows.h>
// int free_space(char *path)
// {
//   DWORD spc, bpc, bps, nfc, tnc;
//   GetDiskFreeSpace(path, &spc, &bpc, &bps, &nfc, &tnc);

//   return  nfc * tnc * bpc;
// }
#endif


class setup_options : public i4_color_window_class
{
public:
  setup_options(w16 w, w16 h, i4_graphical_style_class *style)
    : i4_color_window_class(w,h, style->color_hint->neutral(),style) {}

  void parent_draw(i4_draw_context_class &context)
  {
    undrawn_area.add_area(0,0, width()-1, height()-1);

    i4_color_window_class::parent_draw(context);

    i4_font_class *f=style->font_hint->normal_font;

    char p[200];
    i4_os_string(*(path->get_edit_string()), p, 200);

    char display[200];
    i4_text_item_class *ti=(i4_text_item_class *)display_picker->get_current_item();
    i4_os_string(ti->get_text(), display, 200);

    char size[200];
    ti=(i4_text_item_class *)size_picker->get_current_item();
    i4_os_string(ti->get_text(), size, 200);

    char str[500];
    sprintf(str,
            "Installation Options:\n"
            "  Display           : %s\n"
            "  Installation Path : %s\n"
            "  Install Size      : %s\n\n", display, p, size);

    switch (size_picker->get_current())
    {
      case 0 :
        size_needed_to_install=40*1024*1024;
        li_set_value("install_size", li_get_symbol("compact"));
        sprintf(str+strlen(str), 
                "Small Installation does not have music"); break;
        break;

      case 1 :
        size_needed_to_install=60*1024*1024;
        li_set_value("install_size", li_get_symbol("typical"));
        sprintf(str+strlen(str), 
                "Large Installation has the best of everything"); break;
        break;
    }


  
    f->set_color(0);
    f->put_paragraph(local_image, 6,6, str,
                     context, 1, i4_font_class::LEFT,  width());

    f->set_color(0xffffff);
    f->put_paragraph(local_image, 5,5, str,
                     context, 1, i4_font_class::LEFT,  width());
  }
  char *name() { return "options"; }
};

setup_options *setup_window=0;


#ifdef _WINDOWS
/*
 * build a shortcut in the start menu
 */
void MakeShortcut()
{
  char buf[512];
  char szSetupIni[MAX_PATH];
  int len;
  int fh;

  static char setup_ini[] =
    "[progman.groups]\r\n"
    "groupX=Programs\r\n"
    "[groupX]\r\n"
    "\"Golgotha\",\"golgotha.exe\",,,,\"%s\"\r\n";

  GetWindowsDirectory(szSetupIni, sizeof(szSetupIni));
  sprintf(szSetupIni+strlen(szSetupIni), "\\SETUP.INI");

 
  fix_path(buf);

 
  len = wsprintf(buf, setup_ini, inst_path);
  fh = _lcreat(szSetupIni, 0);

  if (fh != -1)
  {
    _lwrite(fh, buf, len);
    _lclose(fh);
    WinExec("grpconv -o", SW_HIDE);
  }
}
#endif
 


void setup_changed()
{
  if (setup_window)
    setup_window->request_redraw(i4_F);
  error_win->clear();
}

static i4_bool no_space=i4_F;


void write_header(i4_file_class *fp, i4_sound_info &fmt)
{
  fp->write("RIFF",4);
  fp->write_32(36+fmt.size);       // 36 + snd->size
  fp->write("WAVE", 4);
  fp->write("fmt ", 4);

  fp->write_32(16);        // size of chunk

  fp->write_16(1);         // data type

  fp->write_16(fmt.channels);           // num channels
  fp->write_32(fmt.sample_rate);   // sample rate
  fp->write_32(fmt.sample_rate*
               fmt.sample_size*fmt.channels);       // snd->sample_rate * snd->sample_size
  fp->write_16(fmt.sample_size * fmt.channels);
  fp->write_16(fmt.sample_size*8);


  fp->write("data",4);
  fp->write_32(fmt.size);       // snd->size
}



char *forward_slash(char *f)
{
  char *r=f;
  for (;*f; f++)
    if (*f=='\\')
      *f='/';
  return r;
}




li_object *li_uninstall_cmd(li_object *o, li_environment *env)
{
  if (uninstall_script)
  {
    li_object *c=li_eval(li_car(o,env),env);
    li_get_type(c->type())->print(c, uninstall_script);
    return li_true_sym;
  }
  return li_nil;
}


li_object *li_delete(li_object *o, li_environment *env)
{
  char p[200];
  strcpy(p,li_get_string(li_eval(li_car(o,env), env),env));
  fix_path(p);


  if (i4_unlink(p))
    return li_true_sym;
  else return li_nil;
}

li_object *li_mkdir(li_object *o, li_environment *env)
{
  char *fn=li_get_string(li_eval(li_car(o,env),env),env);
  char p[200];
  sprintf(p, "%s/%s", inst_path, fn);
  fix_path(p);

  if (i4_mkdir(p))
  {
    if (uninstall_script)
      uninstall_script->printf("(rmdir \"%s\")\n", forward_slash(p));
    
    return li_true_sym;
  }
  else
    return 0;
}


li_object *li_rmdir(li_object *o, li_environment *env)
{
  char *fn=li_get_string(li_eval(li_car(o,env),env),env);
  fix_path(fn);

  i4_directory_struct ds;

  if (i4_get_directory(fn, ds))
  {
    int i;
    char f2[256], f[256];
    for (i=0; i<ds.tfiles; i++)
    {
      i4_os_string(*ds.files[i], f, 256);
      sprintf(f2, "%s/%s", fn, f);
      fix_path(f2);
      i4_unlink(f2);
    }


    for (i=0; i<ds.tdirs; i++)
    {
      i4_os_string(*ds.dirs[i], f, 256);
      if (!(strcmp(f, ".")==0 ||  strcmp(f, "..")==0))
      {
        sprintf(f2, "%s/%s", fn, f);
        fix_path(f2);
        li_rmdir(li_make_list(new li_string(f2),0),0);
      }
    }
  }

  if (i4_rmdir(fn))
    return li_true_sym;
  else
    return 0;
}


void down_sample_wav(char *in_file, char *out_file, int rate)
{
  i4_sound_info s;
  i4_file_class *in=i4_open(in_file);
  if (!in) return;
  i4_file_class *out=i4_open(out_file, I4_WRITE);
  if (!out) { delete in; return ; }


  if (i4_load_wav_info(in, s))
  {   
    w16 *inm=(w16 *)i4_malloc(s.size,"");
    in->read(inm, s.size);

    float x_end=s.size/2;
    float x=0, x_step=s.sample_rate/(float)rate;
    int out_size=(int)((double)s.size * (double)rate / (double)s.sample_rate), outc=0;
    int out_samples=out_size/2;


    w16 *outm=(w16 *)i4_malloc(out_size,"");

    
    while (outc<out_samples)
    {
      outm[outc]=inm[(int)x];
      x+=x_step;
      outc++;
    }
   

    s.sample_rate=rate;
    s.size=out_size;
    write_header(out, s);
    out->write(outm, out_size);

    i4_free(outm);
    i4_free(inm);
  }

  delete in;
  delete out;
}

li_object *mp3_2_wav(li_object *o, li_environment *env)
{
  char *fn=li_get_string(li_eval(li_car(o,env),env),env);
  i4_filename_struct split;

  int resample_rate=-1;
  if (li_cdr(o,env))
    resample_rate=li_get_int(li_eval(li_second(o,env),env),env);


  i4_split_path(fn, split);
  char out_name[256], tmp_name[256];

  sprintf(out_name, "%s/%s/%s.wav", inst_path, split.path, split.filename);
  sprintf(tmp_name, "%s/%s/%s_tmp.wav", inst_path, split.path, split.filename);
  

  fix_path(out_name);
  fix_path(tmp_name);

  
  i4_file_class *out=i4_open(resample_rate==-1 ? out_name : tmp_name, I4_WRITE);
  if (!out)
    return 0;

  if (uninstall_script)
    uninstall_script->printf("(delete \"%s\")\n", forward_slash(out_name));


  fix_path(fn);
  i4_file_class *in=i4_open(fn);
  if (!in)
  {
    delete out;
    return 0;
  }

  i4_sound_info fmt;
  write_header(out, fmt);

  char stat[200];
  sprintf(stat, "Decoding MP3 : %s", fn);
  i4_status_class *status=i4_create_status(stat);

  i4_bool ret=i4_load_mp3(in, out, fmt, status);
  out->seek(0);
  write_header(out, fmt);
    
  delete status;
  delete in;
  delete out;

  if (resample_rate)
  {
    down_sample_wav(tmp_name, out_name, resample_rate);
    i4_unlink(tmp_name);
  }


  if (!ret)
    return 0;

  return li_true_sym; 
}




li_object *copy_file(li_object *o, li_environment *env)
{
  char *f1=li_get_string(li_eval(li_first(o,env),env),env);
  char *f2=li_get_string(li_eval(li_second(o,env),env),env);
  char full_f2[256];
  sprintf(full_f2, "%s/%s", inst_path, f2);

  fix_path(full_f2);
  i4_file_class *out=i4_open(full_f2, I4_WRITE | I4_NO_BUFFER);
  if (!out)
  {
    error_win->printf("Could not open output file %s\n", f2);
    return 0;
  }

  if (uninstall_script)
    uninstall_script->printf("(delete \"%s\")\n", forward_slash(full_f2));

  fix_path(f1);
  i4_file_class *in=i4_open(f1, I4_NO_BUFFER | I4_READ);
  if (!in)
  {
    error_win->printf("Could not open %s\nAre you running setup from the correct directory?", f1);
    return 0;
  }

  char s[200];
  sprintf(s, "Copying to %s", f2);
  i4_status_class *status=i4_create_status(s);


  char buf[4096];
  int tot=in->size(),size=0;

  while (size<tot)
  {
    status->update((size+2)/(float)(tot+1));

    int t=in->read(buf, sizeof(buf));
    if (out->write(buf, t)!=t)
    {
      error_win->printf("Could write, out of disk space?\n");
      return 0;
    }

    size+=t;
  }
  delete status;

  delete in;
  delete out;
  return li_true_sym;
}


i4_bool install(int install_type)
{
  char display[200];
  i4_text_item_class *ti=(i4_text_item_class *)display_picker->get_current_item();
  i4_os_string(ti->get_text(), display, 200);

#ifdef _WINDOWS
    MakeShortcut();
#endif
  

  if (!installed)
  {

    i4_os_string(*(path->get_edit_string()), inst_path, 200);

    i4_file_status_struct r_stat;

    int l=strlen(inst_path);

    while (l && (inst_path[l-1]==' ' ||
                 inst_path[l-1]=='\\' ||
                 inst_path[l-1]=='/'))
    {
      inst_path[l-1]=0;
      l--;
    }

    // try to make the installation path
    if (!i4_mkdir(inst_path))
    {
      if (!i4_get_status(inst_path, r_stat))
      {
        error_win->printf("Could not create Directory '%s'\n", inst_path);
        return i4_F;
      }
      else if ((r_stat.flags & I4_FILE_STATUS_DIRECTORY)==0)
      {
        error_win->printf("%s is a FILE not a Directory!\n", inst_path);
        return i4_F;
      }
    }

    char uscript[256];
    sprintf(uscript, "%s/uninstall.scm", inst_path);
    uninstall_script=i4_open(uscript, I4_WRITE);
    if (!uninstall_script)
    {
      error_win->printf("Could not open %s for writing\n", uscript);
      return i4_F;
    }


    char cd_dest[200];
    sprintf(cd_dest,"%s/golgotha.cd",inst_path);


    if (copy_file(li_make_list(new li_string("golgotha.cd"), 
                               new li_string("golgotha.cd"),0),0)==0)
    {
      delete uninstall_script; uninstall_script=0;
      return i4_F;
    }

    g1_set_cd_image(cd_dest);



    i4_status_class *s=i4_create_status("Decompressing Files");
    li_load("install.scm", 0, s);
    delete s;

    delete uninstall_script; 
    uninstall_script=0;


  // check for enough free space
    w32 needed_size=inst_sizes[size_picker->get_current()]*1024*1024;
  
  
//     w32 free_space=i4_disk_free_space(inst_path);
//     if (free_space<needed_size)
//     {
//       error_win->printf("Free space = %dMB, I need %dMB to install!\n"
//                         "Please Remove some files or select smaller install\n",
//                         free_space/(1024*1024), needed_size/(1024*1024));
//       return i4_F;
//     }    
  }


  char tmp_data[256];

  // don't set sfx path if it's already set
  if (install_type==INSTALL ||
      !i4_get_registry(I4_REGISTRY_USER, sfx_key, "G_SFX_PATH", tmp_data, 256))
  {
    if (!i4_set_registry(I4_REGISTRY_USER, sfx_key, "G_SFX_PATH", "sfx"))
      error_win->printf("Could not set Registry item\n");
  }

  // don't set install path if it's already set
  if (install_type==INSTALL ||
      !i4_get_registry(I4_REGISTRY_USER, reg_key, "install_path", tmp_data, 256))
  {
    if (!i4_set_registry(I4_REGISTRY_USER, reg_key, "install_path", inst_path))
      error_win->printf("Could not set Registry item\n");
  }

  // always set the display they picked
  if (!i4_set_registry(I4_REGISTRY_MACHINE, i4_display_key, "display", display))
  {
    error_win->printf("Could not set Registry item\n");
    return i4_F;
  }
     
  if (install_type==INSTALL || install_type==SAVE_AND_RUN)
  {
    i4_chdir(inst_path);
    system("golgotha");
  }
 
  return i4_T;

}

class setup_app : public i4_application_class
{
public:

  


  void add_display_picker(int x, int y, i4_parent_window_class *p)
  {
    i4_text_window_class *ti=new i4_text_window_class("Rendering Method", style);
    p->add_child(x,y, ti);


    x+=col2;

    i4_list_box_class *lbox=new i4_list_box_class(250, style, wm);

    char old_name[256];
    if (!i4_get_registry(I4_REGISTRY_MACHINE, i4_display_key, "display", old_name, 256))
      old_name[0]=0;

    int i=0, use=0;
    for (i4_display_list_struct *f=i4_display_list; f; f=f->next) 
    {
      
      if (strcmp(f->name, "Windowed GDI") &&  // these displays don't work with render/
          strcmp(f->name, "X Windows"))
      {
        if (strcmp(f->name, old_name)==0)
          use=i;

        i4_str *s=new i4_str(f->name);
        lbox->add_item(new i4_text_item_class(*s, style, 0,0, 
                                              new i4_event_reaction_class(this,DISPLAY_SELECT + i)));      
        i++;
      }        
    }

    if (i==0)
      i4_error("Golgotha requires DirectX5, or a 3dfx card\n"
               "Make sure you have DX5 (and are in 16bit color mode)"
               "Or that you have the current version Glide (for 3dfx) installed");

    lbox->set_current_item(use);
    display_picker=lbox;


    p->add_child(x,y, lbox);
  }

  
  void add_install_path(int x, int y, i4_parent_window_class *p)
  {
    p->add_child(x,y, new i4_text_window_class("Install Path", style));
    x+=col2;

    path=new i4_text_input_class(style, "c:\\golg", 200, 256, this);
    p->add_child(x,y,path);

    x+=path->width();
    
//     i4_button_class *but;
//     but=new i4_button_class(0, new i4_text_window_class("Browse", style), style,
//                                           new i4_event_reaction_class(this, BROWSE));
//     but->set_popup(i4_T);
//     p->add_child(x,y, but);
                                          

  }


  void add_texture_size_picker(int x, int y, i4_parent_window_class *p)
  {
    i4_text_window_class *ti=new i4_text_window_class("Install size", style);
    p->add_child(x,y, ti);
    x+=col2;

    i4_list_box_class *lbox=new i4_list_box_class(250, style, wm);
    lbox->add_item(new i4_text_item_class("75MB (Compact)", style, 0,0, 
                                          new i4_event_reaction_class(this, COMPACT)));


    lbox->add_item(new i4_text_item_class("100MB (Typical)", style, 0,0,
                                          new i4_event_reaction_class(this, TYPICAL)));

    size_needed_to_install=60*1024*1024;
    lbox->set_current_item(1);
    p->add_child(x,y, lbox);
    size_picker=lbox;
    

  }

  void add_buttons(int x, int y, i4_parent_window_class *p)
  {
    i4_button_class *b;
    if (installed)
    {
      b=new i4_button_class(0, new i4_text_window_class("Save & Quit", style), style,
                            new i4_event_reaction_class(this, SAVE_AND_QUIT));
      b->set_popup(i4_T);
      p->add_child(x,y,b);
      x+=b->width();

      b=new i4_button_class(0, new i4_text_window_class("Save & Run", style), style,
                            new i4_event_reaction_class(this, SAVE_AND_RUN));
    }
    else
      b=new i4_button_class(0, new i4_text_window_class("Install & Run", style), style,
                            new i4_event_reaction_class(this, INSTALL));
    b->set_popup(i4_T);
    p->add_child(x,y,b);


    x+=b->width();
    b=new i4_button_class(0, new i4_text_window_class("      Exit     ", style), style,
                          new i4_event_reaction_class(this, QUIT));
    p->add_child(x,y,b);
    x+=b->width();


    if (installed)
    {
      b=new i4_button_class(0, new i4_text_window_class("    Uninstall    ", style), style,
                            new i4_event_reaction_class(this, UNINSTALL));
      p->add_child(x,y,b);
    }


  }

  i4_bool check_installed()
  {
    for (int i=1; i<i4_global_argc; i++)
      if (i4_global_argv[i]=="-installed")
      {
        installed=1;
        strcpy(inst_path, "c:/golg");
        return i4_T;
      }

    if (i4_get_registry(I4_REGISTRY_USER, reg_key, "install_path", inst_path, 256))
    {
      i4_file_status_struct s;
      char f1[256];
      sprintf(f1,"%s/golgotha.exe",inst_path);


      if (!i4_get_status(f1, s))
        return i4_F;


      sprintf(f1,"%s/golgotha.cd",inst_path);
      if (!i4_get_status(f1, s))
        return i4_F;

      installed=1;
      return i4_T;
    }

    return i4_F;
  }

  void init()
  {    
    g1_set_cd_image(0);

    i4_application_class::init();
    style=get_style();
    wm=get_window_manager();

    i4_init_gui_status(wm, display);

    li_add_function("mp3_2_wav", mp3_2_wav);
    li_add_function("mkdir", li_mkdir);
    li_add_function("rmdir", li_rmdir);
    li_add_function("delete", li_delete);
    li_add_function("copy_file", copy_file);
    li_add_function("uninstall", li_uninstall_cmd);
    
    style->font_hint->normal_font=new i4_anti_proportional_font_class(i4_load_image("setup_font.tga"));
    style->icon_hint->background_bitmap=i4_load_image("setup.jpg");

    style->color_hint->text_foreground=0xffffff;
    style->color_hint->text_background=0;

    i4_image_window_class *im=new i4_image_window_class(style->icon_hint->background_bitmap,
                                                        i4_T, i4_F);


    wm->add_child(0,0, im);

    i4_deco_window_class *deco=new i4_deco_window_class(400, 100, i4_F, style);
    int x=120, y=110;

    im->add_child(x, y, deco); 
    y+=deco->height()+20;

    i4_graphical_style_class *style=wm->get_style();


    add_display_picker(10,10, deco);

    if (!check_installed())
    {
      add_install_path(10, 40, deco);

      add_texture_size_picker(10,70, deco);
    
      setup_window=new setup_options(380, 80, style);
      deco=new i4_deco_window_class(400, 100, i4_F, style);
      deco->add_child(deco->get_x1(), deco->get_y1(), setup_window);
      wm->add_child(x,y, deco);
      y+=deco->height()+5;
    }

    error_win=new i4_text_scroll_window_class(style, 0xff0000, style->color_hint->neutral(), 
                                              400, 30);
    wm->add_child(x, y, error_win);
    y+=error_win->height()+10;

    add_buttons(x, y, wm);

  }

  void receive_event(i4_event *ev)
  {
    if (ev->type()==i4_event::OBJECT_MESSAGE)
      setup_changed();
    else if (ev->type()==i4_event::USER_MESSAGE)
    {

      CAST_PTR(uev, i4_user_message_event_class, ev);
      switch (uev->sub_type)
      {
        case QUIT :
          quit();
          break;
          
        case UNINSTALL :
        {
          char uscript[256];
          sprintf(uscript, "%s/uninstall.scm", inst_path);
          i4_status_class *stat=i4_create_status("Uninstalling");
          li_load(uscript,0,stat);
          delete stat;
          quit();
        } break;

//         case CHECK_SPACE_BEFORE_INSTALL :
//         {
//           if (i4_disk_free_space(inst_path)<size_needed_to_install)
//           {
//             i4_deco_window_class *deco=new i4_deco_window_class(200, 50, i4_T, style);
//             i4_text_window_class *t=new i4_text_window_class("");

//           deco->add_child(

        //        } break;

        case SAVE_AND_QUIT :
        case SAVE_AND_RUN :
        case INSTALL :
        {
          i4_image_class *im=i4_load_image("thanks.jpg");
          i4_image_window_class *iwin=new i4_image_window_class(im, i4_T, i4_F);
          wm->add_child(0,0,iwin);
          
          if (install(uev->sub_type))
            quit();


          delete iwin;


        } break;

        case BROWSE :
        {
            
          char p[200];
          i4_os_string(*(path->get_edit_string()), p, 200);

          i4_create_file_save_dialog(style, 
                                     p,
                                     "Installation Path",
                                     p,
                                     "*",
                                     "Directory",
                                     this, BROWSE_OK,
                                     BROWSE_CANCEL);



        } break;

        case BROWSE_OK :
        {
          CAST_PTR(fev, i4_file_open_message_class, ev);
          path->change_text(*fev->filename);

          setup_changed();
        } break;

        default:
          setup_changed();
          break;

      }
    }
    else i4_application_class::receive_event(ev);
  }

  
  i4_bool get_display_name(char *name, int max_len)
  {
    strcpy(name, "Windowed GDI");
    return i4_T;
  }

  char *name() { return "test_app"; }
};

void i4_main(w32 argc, i4_const_str *argv)
{
  setup_app test;
  test.run();
}





