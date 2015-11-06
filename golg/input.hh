/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_INPUT_HH
#define G1_INPUT_HH


#include "device/device.hh"
#include "device/keys.hh"
#include "player_type.hh"
#include "time/time.hh"

// This class will gather global key presses and store only those relavant to the game.
// This should also allow playstation input to be inserted at this level.

// Ussage :

// if (g1_input.active(g1_input_class::LEFT))
//   move_to_the_left();


class i4_key_matchup_class;

class g1_input_class : public i4_event_handler_class
{
  i4_key_matchup_class *matchup;
public: 
  enum in_type
  { LEFT =0,
    RIGHT, 
    UP, 
    DOWN,
    BUTTON_1,
    BUTTON_2,
    BUTTON_3,
    STRAFE_LEFT,
    STRAFE_RIGHT,
    STRAFE_MODIFIER,
    ZOOM_IN,
    ZOOM_OUT,
    ZOOM_UP,
    ZOOM_DOWN,
    ZOOM_LEFT,
    ZOOM_RIGHT,
    NUM_STANK_CONTROLS,
  };

  enum input_state { NOT_PRESSED, 
                     PRESSED, 
                     PRESSED_AND_RELEASED };  // this state is so we don't miss a button between
                                              // frames
private:

  w8 in[NUM_STANK_CONTROLS];

  i4_time_class time_down[NUM_STANK_CONTROLS];   // time the key was pressed
  w32 time_qued[NUM_STANK_CONTROLS];             // milli secs of unprocessed time

  sw8 keys[I4_NUM_KEYS];

  g1_player_type command_player_num;
  i4_bool esc, cont;
  
  enum {CHEATCODE_BUFFERSIZE = 16};

  char cheat_code_buffer[CHEATCODE_BUFFERSIZE]; //keeps a buffer of the last 16 keys for whatever reason

public:
  enum { MOUSE_SCROLL_LEFT=1,
         MOUSE_SCROLL_RIGHT=2,
         MOUSE_SCROLL_UP=4,
         MOUSE_SCROLL_DOWN=8 };

  int mouse_scroll_flags;
  i4_bool key_pressed;

  virtual void receive_event(i4_event *ev);

  void que_keys(i4_time_class cur_time);

  void commands_are_for_player(g1_player_type pl) { command_player_num=pl; }

  void acknowledge();  // called by game after it has examined all the key states
                       // g1_input then changes all PRESSED_AND_RELEASED to NOT_PRESSED

  void init();

  char *grab_cheat_keys()  { return cheat_code_buffer; }
  void  clear_cheat_keys() { memset(cheat_code_buffer,0,CHEATCODE_BUFFERSIZE); }

  w32 deque_time(in_type t);

  i4_bool strafe_active() { return (i4_bool)(in[STRAFE_MODIFIER]); }
  i4_bool strafe_left()   { return (i4_bool)(in[STRAFE_LEFT]  || (in[LEFT] && strafe_active())); }  
  i4_bool strafe_right()  { return (i4_bool)(in[STRAFE_RIGHT] || (in[RIGHT] && strafe_active())); }
  
  i4_bool left()       { return (i4_bool)(in[LEFT] && !strafe_active()); }
  i4_bool right()      { return (i4_bool)(in[RIGHT] && !strafe_active()); }
  i4_bool up()         { return in[UP]; }
  i4_bool down()       { return in[DOWN]; }
  i4_bool button_1()   { return in[BUTTON_1]; }
  i4_bool button_2()   { return in[BUTTON_2]; }
  i4_bool button_3()   { return in[BUTTON_3]; }

  i4_bool zoom_in()    { return in[ZOOM_IN]; }
  i4_bool zoom_out()   { return in[ZOOM_OUT]; }
  i4_bool zoom_up()    { return in[ZOOM_UP]; }
  i4_bool zoom_down()  { return in[ZOOM_DOWN]; }
  i4_bool zoom_left()  { return in[ZOOM_LEFT]; }
  i4_bool zoom_right() { return in[ZOOM_RIGHT]; }

  i4_bool esc_pressed() { return esc; }
  void reset_esc() { esc=i4_F; }

  void uninit();
  char *name() { return "g1_input"; }
};

extern g1_input_class g1_input;

#endif

