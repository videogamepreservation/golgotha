default_map_width 75
default_map_height 75

//************************************* TRANSLATE ALL TEXT in between " " ****************
cmd_new         "New"
cmd_save        "Save"
cmd_saveas      "Save As"
cmd_open        "Open"
cmd_open_dll    "Load DLL"
cmd_exit        "Exit"

cmd_undo        "Undo"
cmd_redo        "Redo"
cmd_cut         "Cut"
cmd_copy        "Copy"
cmd_paste       "Paste"
cmd_toggle      "Toggle menu"
cmd_no_snap     "No Snap"
cmd_snap_center "Snap Cell Center"
cmd_snap_origin "Snap Cell Origin"
cmd_drop_objs   "Drop Selected Objects"

cmd_:_bld       "Toggle Buildings"
cmd_:_grd       "Toggle Ground"
cmd_:_obj       "Toggle Objects"
cmd_:_spr       "Toggle Sprites"
cmd_:_clr       "Toggle Clear"

cmd_:_1v        "1 View"
cmd_:_4v        "4 View"
cmd_:_wo        "Objects"
cmd_:_ws        "Scenes"
cmd_:radar      "Radar"
cmd_:_pf        "Profile"
cmd_:_debug     "Debug"
cmd_:_path      "AI Window"
cmd_wiref       "Wireframe"
cmd_textr       "Textured"
cmd_:_tilep     "Tile Picker"


cmd_:_lg        "Recalc Lighting"
cmd_:_sv        "Select All Heights"
cmd_tick        "Advance game 1 tick"
cmd_resz        "Resize map"
cmd_ssky        "Change sky"                 
cmd_res_file    "Set Resource File"
cmd_lhbmp       "Load Height Bitmap"         
cmd_shbmp       "Save Height Bitmap"         
cmd_fogall      "Fog out all map"            
cmd_fognone     "Unfog all map"              
cmd_editplyr    "Edit Player Info"


cmd_flatn       "Flatten selected"      
cmd_smooth      "Smooth selected"      
cmd_noise       "Add noise to selected"      
cmd_mgtn        "Load Terrain from map"      

file            "File"                 
edit            "Edit"                 
view            "View"                 
window          "Window"               
map             "Map"                  
terrain         "Terrain"              
object          "Object"

cmd_floor_objs  "Floor selected"
cmd_ciel_objs   "Ciel selected"

radar_title         "Radar"
object_pick_title   "Objects"          
scene_win_title     "Scenes"           
ai_window_title     "AI Window"        
resize_title        "Level Size"
new_title           "New Level"
resize_cur_fmt      "Current Level Size (width x height) %d x %d" 
set_sky_title       "Change level sky"   
tile_pick_title     "Tiles"               tiles are square ares with textures
debug_title         "Debug"              
set_snap_title      "Set Snap"           
terrain_noise_title "Add Noise"           (noise = randomness)

select_team_help "Select Current Team"       
delete_scene_help "Delete Scene"             
tp_grow_help "Make objects bigger"           
tp_shrink_help "Make objects smaller"        



CAMERA_help      "Camera path editing mode"  
CAMERA           0
  cADD_CAMERA    11
  cADD_CAMERA_help "Add Camera Path points"  
  cADD_TARGET    12
  cADD_TARGET_help "Add Target Path points"  // continue translating until XXXXXXX
  cADD_OBJECT    13
  cADD_OBJECT_help "Add Object Path points"
  cMODIFY        9
  cMODIFY_help   "Move/Delete points"
  cSELECT        10
  cSELECT_help   "Select points"
  cROTATE        7
  cROTATE_help   "Rotate camera"
  cZOOM          8
  cZOOM_help     "Zoom camera"

OBJECT_help      "Object editing mode"
OBJECT           1
  oADD           13
  oADD_help      "Add new objects"
  oSELECT        10
  oSELECT_help   "Select objects"
  oMOVE          9
  oMOVE_help     "Select/move/delete objects"
  oOBJECT_ROTATE 2
  oOBJECT_ROTATE_help "Spin object"
  oROTATE        7
  oROTATE_help   "Camera rotate (also SHIFT arrow keys)"
  oZOOM          8
  oZOOM_help     "Camera zoom"

TILE_help        "Terrain editing mode"
TILE             5
  tPLACE         5
  tPLACE_help    "Place single terrain tiles"
  tFILL          14
  tFILL_help     "Flood fill terrain tile"
  tROTATE        7
  tROTATE_help   "Rotate camera"
  tZOOM          8
  tZOOM_help     "Zoom camera"
  tHEIGHT        15
  tHEIGHT_help   "Raise/lower terrain vertices - use [] keys"

LIGHT_help       "Ambient Light edit mode"
LIGHT            16
  lROTATE        7
  lROTATE_help   "Rotate camera and light direction"
  lZOOM          8
  lZOOM_help     "Zoom camera"
  lGDARKEN        37
  lGDARKEN_help   "Turn down global light"
  lGBRIGHTEN      38
  lGBRIGHTEN_help "Turn up global light"
  lDDARKEN        39
  lDDARKEN_help   "Turn down directional light"
  lDBRIGHTEN      40
  lDBRIGHTEN_help "Turn up directional light"
  lAMBIENT       18
  lAMBIENT_help  "Set ambient light direction (must have a view selected)"


AI_help          "AI/path mode"
AI               34
  aROTATE        7
  aROTATE_help   "Rotate camera and light direction"
  aZOOM          8
  aZOOM_help     "Zoom camera"
  aSELECT        10
  aSELECT_help   "Select/delete control points"
  aMOVE          9
  aMOVE_help     "Select/move/delete control points"
  aCREATE        35
  aCREATE_help   "Add control points"
  aRECALC        36
  aRECALC_help   "Recalculate AI info"


// path window icons
//   numbers reference bitmaps
path_tool_win_icons = 
{ 
  25 "Set start point for test path"        // translate all of these
  26 "Set destination point for test path"
  27 "Add/Delete strategic points"
  27 "Load strategic points for takeover pads, etc"
  28 "Show graph / blockage for low grade vehicles (peon tanks, etc.)"
  29 "Show graph / blockage for normal grade vehicles (stock troops)"
  30 "Show graph / blockage for high speed vehicles (supertank, electric cars)"
  31 "Show graph / blockage for flying vehicles (jet, heli)"
  32 "Test Path using 1x1 convoy"
  33 "Test Path using 3x3 convoy"
  27 "Recalculate strategy regions and paths"
}


new      "Add New"           
new_scene "New Scene"        

e_left res/backward.bmp
e_left_help                   = "Go back one frame"         

e_right res/forward.bmp
e_right_help                  = "Go forward one frame"      

e_play res/play.bmp
e_play_help                   = "Play movie"                

e_fforward res/fforward.bmp
e_fforward_help               = "Fast forward to end"       

e_rewind res/rewind.bmp
e_rewind_help                 = "Rewind to Start"           

delete_icon res/delete.bmp
delete_icon_help              = "Delete this scene"         

delete_title "Delete?"               
delete_message "Delete this scene?"  
yes "Yes"                            
no  "No "                            

red red
green green
blue blue
brightness brightness

edit_object "Edit Object" 
edit_time  "Edit Time"   

sec Sec
msec mSec


merge_ter_title "Merge Terrain into current map" 
merge_ter_mask_name "Golgotha Level"     
merge_ter_start_dir "."
merge_ter_file_mask "*.level"     // don't translate

open_title "Load Level"                  
open_mask_name "Golgotha Level"          
open_start_dir "."
open_file_mask "*.level"        // don't translate

open_dll_title       "Load DLL"                  
open_dll_mask_name   "Dynamically Loaded Library"          
open_dll_file_mask   "*.dll"     // don't translate


undo_file "undo/undo_%d.dat"    // don't translate
undo_dir  "undo"                // don't translate
redo_file "undo/redo_%d.dat"    // don't translate


get_song_title "Change WAV for Level"     
get_song_start_dir "songs"      // don't translate
get_song_file_mask "*.wav"      // don't translate
get_song_mask_name "WAV file"             


resize_dialog = 
"
[right
  y+(15)
 [down 
  x+(5)
  up_deco(200 70 [down x+(15) text('Current Size') 
                   x+(20) y+(15)
                   text('Width  : %d') y+(6) text('Height : %d')])
  y+(15)
  up_deco(200 70 [down x+(15) text('New Size')
                   x+(20) y+(10)
                   [right text('Width  (%d..%d) : ') %p=text_input(70 '%d')]
                   y+(15)
                   [right text('Height (%d..%d) : ') %p=text_input(70 '%d')]])

  y+(15)
  up_deco(200 80 [down x+(15) text('Orientation') 
                   x+(20) y+(10)
                   butbox(4 [down
                           [right 
                            button(text(' ') user_ev(%p %d))
                            button(text(' ') user_ev(%p %d))
                            button(text(' ') user_ev(%p %d))]
                           [right 
                            button(text(' ') user_ev(%p %d))
                            button(text(' ') user_ev(%p %d))
                            button(text(' ') user_ev(%p %d))]
                           [right 
                            button(text(' ') user_ev(%p %d))
                            button(text(' ') user_ev(%p %d))
                            button(text(' ') user_ev(%p %d))]])])]
 x+(25)
 [down
  button(text('  Ok  ') user_ev(%p %d))
  y+(2)
  button(text('Cancel') user_ev(%p %d))]]

"

set_sky_dialog =
"
[down
 y+(15)
 text('Current sky : %s') 
 y+(15)
 [right text('Sky ') %p=text_input(70 '%s')]

 y+(15)
 x+(10)
 [right
  button(text('  Ok  ') user_ev(%p %d))
  x+(2)
  button(text('Cancel') user_ev(%p %d))]]"

bad_map_w_h_dialog = 
"
[down x+(5) y+(20) 
 text('Bad width or height (%d X %d)')
 text('Dimensions must be between %d & %d') 
 x+(130) y+(10)
 button(text(' Ok ') user_ev(%p %d))
]"

bad_w_h_title = "Invalid W / H"

undo_dir  undo
undo_name %d.undo

load_height_map_title "Load Terrain Height Image"
load_height_start_dir "."
load_height_file_mask "*.tga"
load_height_mask_name "Targa File"

couldn't_load_image_title "Unknown image type"
couldn't_load_image_dialog "
[down x+(5) y+(20) 
 text('Could not load image (%S)')
 text('Unrecognized format') 
 x+(130) y+(10)
 button(text(' Ok ') user_ev(%p %d))
]"

load_height_bad_size_title "Bad image size"
load_height_bad_size_dialog "
[down x+(5) y+(20) 
 text('Dimensions of image do not match map')
 text('Image (%S) %dx%d, map verts=%dx%d')
 x+(130) y+(10)
 button(text(' Ok ') user_ev(%p %d))
]"


save_height_map_title "Save Terrain Height Image"
save_height_start_dir "."
save_height_file_mask "*.tga"
save_height_mask_name "Targa File"
save_height_default_name "%S.tga"

save_height_bad_file_tile "Could not create"
save_height_bad_file_dialog
"[down x+(5) y+(20) 
 text('Could not create file %S')
 x+(130) y+(10)
 button(text(' Ok ') user_ev(%p %d))
]"

loading_terrain_bitmap = "Loading terrain bitmap"
applying_terrain_map = "Applying terrain map"
applying_smooth = "Smoothing terrain"
applying_flatten = "Flattening terrain"
applying_noise = "Applyiong Noise to terrain"
"moving vertices" = "Moving vertices"

set_snap_dialog =
"[down x+(5) y+(20)
 [right text('Vertex snap      (1..128)') %p=text_input(50 '%d')] 
 y+(20)
 [right text('Object move snap (0..2)  ') %p=text_input(50 '%2.2f')] 
 y+(20)
 [right
  button(text('  Ok  ') user_ev(%p %d))
  x+(2)
  button(text('Cancel') user_ev(%p %d))]]"


bad_vert_snap_dialog =
"[down x+(5) y+(20) 
 text('Bad value for Vertex Snap (%d)')
 text('Value should be between 1 & 128')
 x+(130) y+(10)
 button(text(' Ok ') user_ev(%p %d))
]"


bad_object_snap_dialog =
"[down x+(5) y+(20) 
 text('Bad value for Object Snap (%2.2f)')
 text('Value should be between 0 & 2')
 x+(130) y+(10)
 button(text(' Ok ') user_ev(%p %d))
]"

terrain_noise_dialog =
"
[down x+(5) y+(20)
 [right text('Noise maximum    (1..64)')  %p=text_input(50 '%d')] 
 y+(20)
 [right
  button(text('  Ok  ') user_ev(%p %d))
  x+(2)
  button(text('Cancel') user_ev(%p %d))]]
]
"


sfx_obj_dialog = 
"
[down x+(5) y+(20)     

 [right text('Filename ') %p=text_input(230 '%S') x+(10) button(text('Browse') user_ev(%p %d))]
y+(8)
 [right text('Max Volume (0..%d)                      ') %p=text_input(50 '%d')]
y+(8)
 [right text('Max Hearable Distance                   ') %p=text_input(50 '%d')]
y+(8)
 [right text('Restart Delay in ticks (%d per second)  ') %p=text_input(50 '%d')]
y+(8)
 [right text('Additional Random Restart Delay in ticks') %p=text_input(50 '%d')]
y+(8)
 [right text('Current Delay in ticks                  ') %p=text_input(50 '%d')]

 y+(20)
 [right
  button(text('  Ok  ') user_ev(%p %d))
  x+(2)
  button(text('Cancel') user_ev(%p %d))]]
]
"


bad_volume_dialog
"[down x+(5) y+(20) 
 text('Bad Max Volume value %d : should be (0..%d)')
 x+(130) y+(10)
 button(text(' Ok ') user_ev(%p %d))
]"



get_res_title "Change Resource file for Level"     
get_res_start_dir "."             // don't translate
get_res_file_mask "*.scm"      // don't translate
get_res_mask_name "SCHEME file"             


new_level_dialog = 
"
[down x+(5) y+(20)     

 [right text('Level Name/Filename ') %p=text_input(230 'test.level')]
y+(8)
 [right text('Map Width (%d..%d)                      ') %p=text_input(50 '%d')]
y+(8)
 [right text('Max Height (%d..%d)                     ') %p=text_input(50 '%d')]
y+(8)

 y+(20)
 [right
  button(text('  Ok  ') user_ev(%p %d))
  x+(2)
  button(text('Cancel') user_ev(%p %d))]]
]
"


change_res_title = "Change Resource File"
change_res_dialog = 
"
[down x+(5) y+(20)
 text('Current Level Name : %S')
 y+(20)
 [right text('Resource File       ') %p=text_input(230 '%S') x+(10) button(text('Browse') user_ev(%p %d))]
 y+(20)
 [right
  button(text('Change & Reload Level') user_ev(%p %d))
  x+(2)
  button(text('Cancel') user_ev(%p %d))]]
]
"

// new_level_dialog = "
// (arrange_top_down
//  (arrange_left_right (text \"Level Name\") (text_input 70 (param) (param)))
//  (rectangle 1 15)
//  (arrange_left_right (text \"Resource File\") (text_input 70 (param) (param)))
//  (rectangle 1 20)
//  (arrange_left_right (text \"Width\") (text_input 70 (param) (param)))
//  (rectangle 1 15)
//  (arrange_left_right (text \"Height\") (text_input 70 (param) (param)))
//  (rectangle 1 30)
//  (arrange_left_right (rectangle 20 1) 
//                      (button (text \"  Ok  \") (user_event (param) (param)))
//                      (button (text \"Cancel\") (user_event (param) (param))))
// )
// "



screen_shot_filename "shot.tga"
pick_pass            "bitmaps/pick1.tga"
pick_act             "bitmaps/pick2.tga"
click_to_active      "Click to active this view"

e_icons = {
       bitmaps/editor/camera.bmp       // 0
       bitmaps/editor/object.bmp       // 1
       bitmaps/editor/spin.bmp         // 2
       bitmaps/editor/frame.bmp        // 3
       bitmaps/editor/play.bmp         // 4
       bitmaps/editor/tile.bmp         // 5
       bitmaps/editor/frame.pcx.bmp    // 6
       bitmaps/editor/rotate.bmp       // 7
       bitmaps/editor/zoom.bmp         // 8
       bitmaps/editor/move.bmp         // 9
       bitmaps/editor/select.bmp       // 10
       bitmaps/editor/camera_add.bmp   // 11
       bitmaps/editor/target.bmp       // 12
       bitmaps/editor/obj_add.bmp      // 13
       bitmaps/editor/fill.bmp         // 14
       bitmaps/editor/height.bmp       // 15
       bitmaps/editor/light.bmp        // 16
       bitmaps/editor/lcreate.bmp      // 17
       bitmaps/editor/lambient.bmp     // 18
       bitmaps/editor/mirror.bmp       // 19
       bitmaps/editor/grow.bmp         // 20
       bitmaps/editor/shrink.bmp       // 21

       bitmaps/editor/begin.tga        // 22
       bitmaps/editor/dest.tga         // 23
       bitmaps/editor/critical.tga     // 24

       bitmaps/editor/start.jpg        // 25
       bitmaps/editor/end.jpg          // 26
       bitmaps/editor/stop.jpg         // 27
       bitmaps/editor/grade1.jpg       // 28
       bitmaps/editor/grade2.jpg       // 29
       bitmaps/editor/grade3.jpg       // 30
       bitmaps/editor/grade4.jpg       // 31
       bitmaps/editor/size1.jpg        // 32
       bitmaps/editor/size3.jpg        // 33

       bitmaps/editor/ai.bmp           // 34
       bitmaps/editor/waypoint_add.bmp // 35
       bitmaps/editor/ai_recalc.bmp    // 36
        
       bitmaps/editor/ambient-lower_button.bmp   // 37
       bitmaps/editor/ambient-higher_button.bmp   // 38

       bitmaps/editor/directional-lower_button.bmp   // 39
       bitmaps/editor/directional-higher_button.bmp   // 40

}


// tile picker icons
tp_rotate        2
tp_rotate_help   "Rotate tile 90 degrees"
tp_mirror        19
tp_mirror_help   "Mirror tile across X axis"
tp_grow          20
tp_grow_help     "Enlarge tile size"
tp_shrink        21
tp_shrink_help   "Shrink tile size"


path_start       22
path_dest        23
path_critical    24

0 0

//               #    start     length   name
scene_info_fmt "%3d  %3d:%2d   %3d:%2d   %S"

frame_format "%d"
time_format "%d:%02d"
next         "next"
not_sel       "NA"
no_next      "NA"
null_string  ""
sec_fmt "%d"
msec_fmt "%02d"

change_link_help "Link to the currently selected object(s)"
change_link "Set Link"

add_links "Add"
del_links "Del"
clear_links "Clear"


add_links_help "Link currently selected objects"
del_links_help "Unlink currently selected objects"
clear_links_help "Clear all links"
