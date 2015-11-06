default_xres 640
default_yres 480

res_name_format = "%s_name = \"%S\"\n"
array_format1 = "%s_array = {\n"
array_format2 = "\t%S\n"
array_format3 = "} \n\n"


i4_resources = "i4_res"
g1_resources = "res"

#include "resource/i4.res"
#include "g1.res"

name_format = "%s"
slot_name = "\\\\.\\mailslot\\maxtool"
fly_name = "c:/tmp/fly.gmod"

texture "x:/kevin/mortership/toptank6.tga"

screen_shot_filename "maxtshot.tga"

frame_name "Frame #%d"

model_resource_file "x:/crack/golgotha/models.res"
buildings Buildings
objects   Objects
sprites   Sprites

warn_title "Warnings..."
loaded_model "Loaded model '%S'"

warning_window_fore "127 127 0"
warning_window_back "0 0 128"
no_load_2 "Could not load the models %S and %S"
poly_mismatch "Cannot insert animation : polygon counts differ"
vertex_mismatch "Cannot insert animation : vertex counts differ"
write_open_fail "Could not open file %S for writing"
no_animations "Object %S has no animations, please add some"
cannot_rename_obj "Cannot rename object %S to %S because file already exsist"
no_write_resource "Cannot write to resource fle %S"

player_number_buttons = { "Team 1" "Team 2" "Team 3" "Team 4" "default" "quit"}
m1_tcalc  "Recalc Obj Texts"
m1_recalc "Recalc textures"
m1_quit   "Quit"
m1_dir    "Create tdir"
m1_next   "Next"
m1_last   "Last"
m1_recenter   "Recenter"
m1_copy_update "Update Textures"
m1_axis   "Axis"
m1_wireframe "Wire frame"

back_bitmap "x:/crack/golgotha/bitmaps/darkcracked2.tga"
default_animation_name "still"

obj_missing    "Object model missing : %S"

updating_obj   "Updating object textures  : %S"
generating_texture_list "Generating Texture List"
copying_textures "Copying Textures"
updating_textures "Updating all textures that need updating"
toggle_z_write "Toggle z write"

default_tga_dir x:/crack/golgotha/textures

model_stats "Model %S has %d faces and %d verts"


