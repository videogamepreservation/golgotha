/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "window/window.hh"
#include "device/event.hh"
#include "gui/text.hh"
#include "gui/text_input.hh"
#include "window/colorwin.hh"
#include "error/alert.hh"
#include "gui/button.hh"
#include "gui/butbox.hh"
#include "gui/deco_win.hh"

char *tokens[]={"=", "(", ")",
                "[", "]", "right", "down", "up_deco", "down_deco", "'", "text_input", "text",
                "button", "butbox", "obj_ev", "user_ev", "x+", "y+", 0};

enum {
  TK_EQUAL, TK_LPAREN, TK_RPAREN,
  TK_LBRACE, TK_RBRACE, TK_RIGHT, TK_DOWN, TK_UP_DECO, TK_DOWN_DECO, TK_TICK, 
  TK_TEXT_INPUT, TK_TEXT,
  TK_BUTTON, TK_BUTBOX, TK_OBJ_EV,  TK_USER_EV,
  TK_XPLUS, TK_YPLUS, TK_NUMBER, TK_POINTER, TK_NONE };
  


void i4_expected(char *why, i4_const_str::iterator i, const i4_const_str &s)
{
  i4_str *er=new i4_str(i4gets("expected"),2000);
  i4_str::iterator x=er->end();
  
  while (i!=s.end())
  {
    x.set(i.get());
    ++i;
    ++x;
    er->set_length(er->length()+1);
  }

  i4_warning(why);
  i4_alert(*er, 2000);
  i4_error("expected error : see error output");
}

int i4_is_space(i4_const_str::iterator i)
{
  if (i.get().value()==' ' ||
      i.get().value()=='\t' ||
      i.get().value()=='\n' ||
      i.get().value()=='\r')
    return 1;
  else return 0;
}

int i4_cmp_char_str(i4_const_str::iterator i, const i4_const_str &s, char *c)
{
  while (*c && i!=s.end())
  {
    if (i.get().value()!=*c)
      return 0;

    c++;
    ++i;
  }

  if (*c)
    return 0;
  else return 1;

}


int i4_read_dlg_token(i4_const_str::iterator &i,
                      const i4_const_str &fmt,
                      sw32 &num,
                      void *&ptr,
                      va_list &ap)
{
  while (i!=fmt.end() && i4_is_space(i))
    ++i;

  if (i==fmt.end()) return TK_NONE;

  if (i.get().value()=='%')
  {
    ++i;
    if (i.get().value()=='d')
    {
      ++i;
      num=va_arg(ap,int);
      return TK_NUMBER;
    }
    else if (i.get().value()=='p')
    {
      ++i;
      ptr=va_arg(ap,void *);
      return TK_POINTER;
    }
    else i4_error("expecting p or d after %");
  }
  else
  {
    if ((i.get().value()>='0' && i.get().value()<='9') || i.get().value()=='-')
    {
      int neg=0;
      if (i.get().value()=='-')
      {
        neg=1;
        ++i;
      }

      num=0;
      while (i!=fmt.end() && !i4_is_space(i) && i.get().value()>='0' && i.get().value()<='9')
      {
        num=num*10+i.get().value()-'0';
        ++i;
      }

      if (neg) num=-num;
      return TK_NUMBER;
    }

    for (int j=0; tokens[j]; j++)
      if (i4_cmp_char_str(i, fmt, tokens[j]))
      {
        int l=strlen(tokens[j]);
        while (l) 
        {
          ++i;
          l--;
        }
            
        return j;
      }

    i4_expected("unknown token", i, fmt);

  }
  return TK_NONE;
}


int i4_next_token_is_rbrace(i4_const_str::iterator i,
                            const i4_const_str &fmt)
{
  while (i!=fmt.end() && i4_is_space(i))
    ++i;

  if (i==fmt.end()) return 1;

  if (i.get().value()==']') return 1;
  else return 0;
}


i4_str *i4_read_str(i4_const_str::iterator &i, const i4_const_str &fmt, va_list &ap)
{
  sw32 n; void *p;

  if (i4_read_dlg_token(i, fmt, n, p, ap)!=TK_TICK)
    i4_expected("'",i,fmt);

  i4_str *s=new i4_str(i4_string_man.get(0), 200);
  i4_str::iterator x=s->begin();
  while (i!=fmt.end() && i.get().value()!='\'')
  {
    x.set(i.get().value());
    ++i;
    ++x;
    s->set_length(s->length()+1);
  }
  ++i;

  i4_str *ret=s->vsprintf(200, ap);

  delete s;
  return ret;
}

i4_event_reaction_class *i4_read_reaction(i4_const_str::iterator &i, 
                                          const i4_const_str &fmt, 
                                          va_list &ap)
{
  sw32 x,id; void *p, *from, *to;
  int t=i4_read_dlg_token(i, fmt, x, p, ap);

  if (t==TK_OBJ_EV)
  {
    if (i4_read_dlg_token(i, fmt, x, p, ap)!=TK_LPAREN)
      i4_expected("(",i,fmt);

    if (i4_read_dlg_token(i, fmt, x, from, ap)!=TK_POINTER)
      i4_expected("pointer",i,fmt);

    if (i4_read_dlg_token(i, fmt, x, to, ap)!=TK_POINTER)
      i4_expected("pointer",i,fmt);

    if (i4_read_dlg_token(i, fmt, id, p, ap)!=TK_NUMBER)
      i4_expected("number",i,fmt);
  
    if (i4_read_dlg_token(i, fmt, x, p, ap)!=TK_RPAREN)
      i4_expected(")",i,fmt);
    
    i4_object_message_event_class *om;
    om=new i4_object_message_event_class((i4_event_handler_class *)from, id);
    return new i4_event_reaction_class((i4_event_handler_class *)to, om);
  }
  else if (t==TK_USER_EV)
  {
    if (i4_read_dlg_token(i, fmt, x, p, ap)!=TK_LPAREN)
      i4_expected("(",i,fmt);

    if (i4_read_dlg_token(i, fmt, x, to, ap)!=TK_POINTER)
      i4_expected("pointer",i,fmt);

    if (i4_read_dlg_token(i, fmt, id, p, ap)!=TK_NUMBER)
      i4_expected("number",i,fmt);

    if (i4_read_dlg_token(i, fmt, x, p, ap)!=TK_RPAREN)
      i4_expected(")",i,fmt);
    
    i4_user_message_event_class *uev;
    uev=new i4_user_message_event_class(id);

    return new i4_event_reaction_class((i4_event_handler_class *)to, uev);
  }
  else 
  {
    i4_expected("obj_ev or user_ev",i,fmt);
    return 0;
  }
}

i4_window_class *i4_read_object(i4_parent_window_class *parent,
                                i4_graphical_style_class *style,
                                sw32 &cx, sw32 &cy,
                                i4_const_str::iterator &i, const i4_const_str &fmt,
                                va_list &ap,
                                int in_buttonbox)
{
  sw32 x;
  void *p;
  i4_const_str::iterator start_i=i;
  i4_window_class *ret=0;


  int token=i4_read_dlg_token(i, fmt, x, p, ap);
  switch (token)
  {
    case TK_POINTER :
    {
      token=i4_read_dlg_token(i, fmt, x, p, ap);
      if (token!=TK_EQUAL)
        i4_expected("expected = after %p", start_i, fmt);

      i4_window_class *r=i4_read_object(parent, style, cx, cy, i, fmt, ap, in_buttonbox);
      *((i4_window_class **)p)=r;
      ret=r;
    } break;

    case TK_LBRACE :
    {
      int dir=i4_read_dlg_token(i, fmt, x, p, ap);
      if (dir!=TK_RIGHT && dir!=TK_DOWN)
        i4_expected("right or down after [", start_i, fmt);
     
      sw32 ncx=cx, ncy=cy;
      int max_w=0, max_h=0;
      i4_window_class *r;
      while (!i4_next_token_is_rbrace(i, fmt))
      {
        r=i4_read_object(parent, style, ncx, ncy, i, fmt, ap, in_buttonbox);
        if (r)
        {
          if (!ret) ret=r;
          if (dir==TK_RIGHT)
            ncx+=r->width();
          else ncy+=r->height();
        }        
      }
      i4_read_dlg_token(i, fmt, x, p, ap);

    } break;

    case TK_RIGHT :
    case TK_DOWN :
    case TK_NUMBER :
    case TK_RBRACE :
      i4_expected("out of place token",start_i,fmt);
      break;

    case TK_XPLUS :
    {
      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_LPAREN)
        i4_expected("(",start_i,fmt);

      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_NUMBER)
        i4_expected("number",start_i,fmt);

      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_RPAREN)
        i4_expected(")",start_i,fmt);

      cx+=x;
    } break;

    case TK_YPLUS :
    {
      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_LPAREN)
        i4_expected("(",start_i,fmt);

      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_NUMBER)
        i4_expected("number",start_i,fmt);
      cy+=x;

      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_RPAREN)
        i4_expected(")",start_i,fmt);

    } break;

    case TK_UP_DECO :
    {
      sw32 w,h;
      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_LPAREN)
        i4_expected("(",start_i,fmt);

      if (i4_read_dlg_token( i, fmt, w, p, ap)!=TK_NUMBER)
        i4_expected("number",start_i,fmt);

      if (i4_read_dlg_token( i, fmt, h, p, ap)!=TK_NUMBER)
        i4_expected("number",start_i,fmt);

      i4_deco_window_class *cw=new i4_deco_window_class(w,h, i4_T, style);

      sw32 ncx=0, ncy=0;
      i4_window_class *r=i4_read_object(cw, style, ncx, ncy, i, fmt, ap, 0);
      
      parent->add_child(cx,cy, cw);

      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_RPAREN)
        i4_expected(")",start_i,fmt);

      ret=cw;
    } break;

    case TK_TEXT :
    {
      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_LPAREN)
        i4_expected("(",start_i,fmt);

      i4_str *s=i4_read_str(i, fmt, ap);
      if (s)
      {
        i4_text_window_class *tw=new i4_text_window_class(*s, style);
        delete s;
        parent->add_child(cx, cy, tw);
        ret=tw;
      }

      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_RPAREN)
        i4_expected(")",start_i,fmt);

    } break;

    case TK_TEXT_INPUT :
    {
      sw32 w;
      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_LPAREN)
        i4_expected("(",start_i,fmt);

      if (i4_read_dlg_token( i, fmt, w, p, ap)!=TK_NUMBER)
        i4_expected("number",start_i,fmt);

      i4_str *s=i4_read_str(i, fmt, ap);
      if (s)
      {
        i4_text_input_class *ti=new i4_text_input_class(style, *s, w, 256);
        delete s;
        parent->add_child(cx, cy, ti);
        ret=ti;
      }

      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_RPAREN)
        i4_expected(")",start_i,fmt);
    } break;

      
    case TK_BUTTON :
    {
      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_LPAREN)
        i4_expected("(",start_i,fmt);

      i4_color_window_class tmp_win(0,0,0,style);
      i4_window_class *r=i4_read_object(&tmp_win, style, cx, cy, i, fmt, ap, in_buttonbox);      
      i4_event_reaction_class *re=i4_read_reaction(i, fmt, ap);
      
      if (r)
      {
        tmp_win.remove_child(r);

        i4_button_class *b=new i4_button_class(0, r, style, re);
        
        if (in_buttonbox)
          ((i4_button_box_class *)parent)->add_button(cx, cy, b);
        else
        {
          b->set_popup(i4_T);
          parent->add_child(cx,cy, b);
        }
        ret=b;
      }

      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_RPAREN)
        i4_expected(")",start_i,fmt);


    } break;


    case TK_BUTBOX :
    {
      sw32 def_down;

      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_LPAREN)
        i4_expected("(",start_i,fmt);

      if (i4_read_dlg_token( i, fmt, def_down, p, ap)!=TK_NUMBER)
        i4_expected("number",start_i,fmt);

      
      i4_button_box_class *bbox=new i4_button_box_class(0);
      
      sw32 ncx=0,ncy=0;
      i4_window_class *r=i4_read_object(bbox, style, ncx, ncy, i, fmt, ap, 1);
      bbox->resize_to_fit_children();

      i4_button_class *b=(i4_button_class *)bbox->get_nth_window(def_down);
      if (b)
        bbox->push_button(b,0);

      parent->add_child(cx, cy, bbox);

      if (i4_read_dlg_token( i, fmt, x, p, ap)!=TK_RPAREN)
        i4_expected(")",start_i,fmt);
      
      return bbox;
    } break;

  }
  return ret;
}

void i4_create_dialog(const i4_const_str &fmt,
                      i4_parent_window_class *parent,
                      i4_graphical_style_class *style,
                      ...)
{
  va_list ap;
  va_start(ap, style);


  i4_const_str::iterator i=fmt.begin();

  sw32 cx=0, cy=0;

  i4_read_object(parent, style, cx, cy, i, fmt, ap, 0);

  va_end(ap);

}



