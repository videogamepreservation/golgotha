/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "device/svgalib/keyboard.hh"

svgalib_keyboard_class svgalib_keyboard_instance;

static w32 sh=0;

void key_handler(int scancode, int press)
{
  int k=0;
  switch(scancode)
  {
    case SCANCODE_CURSORUP          : k=i4_UP;        break;
    case SCANCODE_CURSORLEFT	    : k=i4_LEFT;      break;
    case SCANCODE_CURSORRIGHT	    : k=i4_RIGHT;     break;
    case SCANCODE_CURSORDOWN	    : k=i4_DOWN;      break;

    case SCANCODE_CURSORUPLEFT	    : k=i4_HOME;      break;
    case SCANCODE_CURSORUPRIGHT	    : k=i4_PAGEUP;    break;  
    case SCANCODE_CURSORDOWNLEFT    : k=i4_END;       break; 
    case SCANCODE_CURSORDOWNRIGHT   : k=i4_PAGEDOWN;  break; 

    case SCANCODE_CURSORBLOCKUP	    : k=i4_UP;        break; 
    case SCANCODE_CURSORBLOCKLEFT   : k=i4_LEFT;      break; 
    case SCANCODE_CURSORBLOCKRIGHT  : k=i4_RIGHT;     break; 
    case SCANCODE_CURSORBLOCKDOWN   : k=i4_DOWN;      break; 

    case SCANCODE_KEYPAD0	    : k=i4_INSERT;    break; 
    case SCANCODE_KEYPAD5	    : k='5';          break; 
    case SCANCODE_KEYPADPLUS        : k='+';          break; 
    case SCANCODE_KEYPADMINUS	    : k='-';          break;

    case SCANCODE_Q		    : k=sh?'Q':'q';   break; 
    case SCANCODE_W		    : k=sh?'W':'w';   break; 
    case SCANCODE_E		    : k=sh?'E':'e';   break; 
    case SCANCODE_R		    : k=sh?'R':'r';   break; 
    case SCANCODE_T	            : k=sh?'T':'t';   break;    
    case SCANCODE_Y		    : k=sh?'Y':'y';   break; 
    case SCANCODE_U		    : k=sh?'U':'u';   break; 
    case SCANCODE_I		    : k=sh?'I':'i';   break;
    case SCANCODE_O		    : k=sh?'O':'o';   break;
    case SCANCODE_P		    : k=sh?'P':'p';   break;        

    case SCANCODE_A		    : k=sh?'A':'a';   break;
    case SCANCODE_S		    : k=sh?'S':'s';   break;
    case SCANCODE_D		    : k=sh?'D':'d';   break;
    case SCANCODE_F		    : k=sh?'F':'f';   break;
    case SCANCODE_G		    : k=sh?'G':'g';   break;
    case SCANCODE_H		    : k=sh?'H':'h';   break;
    case SCANCODE_J		    : k=sh?'J':'j';   break;
    case SCANCODE_K		    : k=sh?'K':'k';   break;
    case SCANCODE_L		    : k=sh?'L':'l';   break;

    case SCANCODE_Z		    : k=sh?'Z':'z';   break;
    case SCANCODE_X		    : k=sh?'X':'x';   break;
    case SCANCODE_C		    : k=sh?'C':'c';   break;
    case SCANCODE_V		    : k=sh?'V':'v';   break;
    case SCANCODE_B		    : k=sh?'B':'b';   break;
    case SCANCODE_N		    : k=sh?'N':'n';   break;
    case SCANCODE_M		    : k=sh?'M':'m';   break;

    case SCANCODE_ESCAPE	    : k=i4_ESC;       break;
    case SCANCODE_ENTER		    : k=i4_ENTER;     break;
    case SCANCODE_RIGHTCONTROL	    : k=i4_CTRL_R;    break;
    case SCANCODE_RIGHTALT	    : k=i4_ALT_R;     break;
    case SCANCODE_LEFTCONTROL	    : k=i4_CTRL_L;    break;
    case SCANCODE_LEFTALT	    : k=i4_ALT_L;     break;
    case SCANCODE_SPACE		    : k=' ';          break;

    case SCANCODE_F1		    : k=i4_F1;        break;
    case SCANCODE_F2		    : k=i4_F2;        break;
    case SCANCODE_F3		    : k=i4_F3;        break;
    case SCANCODE_F4	            : k=i4_F4;        break;
    case SCANCODE_F5		    : k=i4_F5;        break;
    case SCANCODE_F6		    : k=i4_F6;        break;
    case SCANCODE_F7		    : k=i4_F7;        break;
    case SCANCODE_F8		    : k=i4_F8;        break;
    case SCANCODE_F9		    : k=i4_F9;        break;
    case SCANCODE_F10		    : k=i4_F10;       break;
    case 43            		    : k=sh?'|':'\\';  break;   				     
    case 13            		    : k=sh?'+':'=';   break;   				     
    case 12            		    : k=sh?'_':'-';   break;
    case 2             		    : k=sh?'!':'1';   break;
    case 3             		    : k=sh?'@':'2';   break;
    case 4             		    : k=sh?'#':'3';   break;
    case 5             		    : k=sh?'$':'4';   break;
    case 6             		    : k=sh?'%':'5';   break;
    case 7             		    : k=sh?'^':'6';   break;
    case 8             		    : k=sh?'&':'7';   break;
    case 9             		    : k=sh?'*':'8';   break;
    case 10            		    : k=sh?'(':'9';   break;
    case 11            		    : k=sh?')':'0';   break;

    case 41            		    : k=sh?'~':'`';   break;
    case 15            		    : k=i4_TAB;       break;
    case 58            		    : k=i4_CAPS;      break;
    case 26            		    : k=sh?'{':'[';   break;
    case 27            		    : k=sh?'}':']';   break;

    case 39            		    : k=sh?':':';';   break;
    case 40            		    : k=sh?'"':'\'';  break;

    case 51            		    : k=sh?'<':',';   break;
    case 52            		    : k=sh?'>':'.';   break;

    case 53            		    : k=sh?'?':'/';   break;
    case 69            		    : k=i4_NUM_LOCK;  break;
    case 14              	    : k=i4_BACKSPACE; break;
    case 110             	    : k=i4_INSERT;    break;
    case 102             	    : k=i4_HOME;      break;
    case 104              	    : k=i4_PAGEUP;    break;

    case 111              	    : k=i4_DEL;       break;
    case 107              	    : k=i4_END;       break;
    case 109              	    : k=i4_PAGEDOWN;  break;
    case 83               	    : k=i4_DEL;       break;
    case 55               	    : k='*';          break;
    case 98               	    : k='/';          break;
    case 119              	    : k='/';          break;
    case 96                  	    : k=i4_ENTER;     break;

 				      
    case 42                         : 
				      { k=i4_SHIFT_L;
					if (press)
					  sh|=1;
					else sh&=~1;
				      } break;
    case 54                         : 
				      { k=i4_SHIFT_R;
					if (press)
					  sh|=2;
					else sh&=~2;
				      } break;
  }

  svgalib_keyboard_instance.send_key(k,press);
}



