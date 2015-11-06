/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef G1_MEESAGE_ID_HH
#define G1_MEESAGE_ID_HH

#include "device/event.hh"

class g1_object_controller_class;
class g1_movie_flow_class;


// g1 application user_message space message id's
enum 
{ 
  G1_NO_MESSAGE,
  G1_MAIN_MENU,
  G1_SERVER_MENU,
  G1_NETWORK_MENU,
  G1_CLIENT_JOINED_MENU,
  G1_HELP_SCREEN,
  G1_PLOT_SCREEN,
  G1_START_HELP_SCREEN,

  G1_RETURN_TO_GAME,
  G1_OPTIONS,
  G1_YOU_LOSE,
  G1_YOU_WIN,

  G1_START_NEW_GAME,
  G1_CONTINUE,
  G1_ESCAPE,

  G1_MAP_CHANGED,
  G1_GET_ROOT_IMAGE,
  G1_GET_CURRENT_MOVIE,
  G1_REDRAW_LATER,
  G1_PLAY_MOVIE,
  G1_STOP_MOVIE,
  G1_TMERGE_FILE_OPEN_CANCEL,

  G1_INTERLACE_PIXEL,
  G1_DOUBLE_PIXEL,
  G1_NORMAL_PIXEL,

  G1_EDITOR_LOAD_TERRAIN_HEIGHT_OK,
  G1_EDITOR_SAVE_TERRAIN_HEIGHT_OK,
  G1_EDITOR_NO_OP,

  G1_EDITOR_SET_SKY_OK,

  G1_TMERGE_FILE_OPEN_OK,
  G1_FILE_OPEN_OK,
  G1_FILE_OPEN_CANCEL,
  G1_FILE_SAVEAS_OK,
  G1_OPEN_DLL_OK,
  G1_EDITOR_RESIZE_PLACE_LT,
  G1_EDITOR_RESIZE_PLACE_CT,
  G1_EDITOR_RESIZE_PLACE_RT,
  G1_EDITOR_RESIZE_PLACE_LC,
  G1_EDITOR_RESIZE_PLACE_CC,
  G1_EDITOR_RESIZE_PLACE_RC,
  G1_EDITOR_RESIZE_PLACE_BL,
  G1_EDITOR_RESIZE_PLACE_BC,
  G1_EDITOR_RESIZE_PLACE_BR,
  G1_EDITOR_RESIZE_MAP_OK,
  G1_EDITOR_NEW_OK,
  G1_EDITOR_RES_SET_NAME,
  G1_EDITOR_RES_BROWSE,
  G1_EDITOR_CHANGE_RES_OK,
  G1_EDITOR_CHANGE_RES_RELOAD_OK,

  G1_EDITOR_MODAL_BOX_CANCEL,
  G1_EDITOR_MODAL_CLOSED,
  G1_EDITOR_PROFILE_CLOSED,
  G1_EDITOR_DEBUG_CLOSED,

  G1_EDITOR_SCENE_WINDOW_CLOSED,
  G1_EDITOR_AI_WINDOW_CLOSED,
  G1_EDITOR_CREATE_OBJECT,
  G1_EDITOR_SET_SNAP_OK,
  G1_EDITOR_TERRAIN_NOISE_OK,

  G1_QUIT
};


class g1_get_root_image_event : public i4_user_message_event_class
{
public:
  i4_image_class *result;
  g1_get_root_image_event() : i4_user_message_event_class(G1_GET_ROOT_IMAGE) { result=0; }
  virtual i4_event  *copy() { return new g1_get_root_image_event; }  
  virtual dispatch_time when()  { return NOW; }  
  char *name() { return "get_root_image"; }
} ;


class g1_movie_flow_class;
class g1_get_current_movie_event : public i4_user_message_event_class
{
public:
  g1_movie_flow_class *mflow;
  g1_get_current_movie_event() : i4_user_message_event_class(G1_GET_CURRENT_MOVIE), mflow(0) {} 
  virtual i4_event  *copy() { return 0; }  
  virtual dispatch_time when()  { return NOW; }  
};

#endif
