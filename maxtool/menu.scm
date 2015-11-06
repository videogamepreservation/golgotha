(add_sub_menu "File"
              ("Open"     "open_model")
              ("Save"     "save_model")
              ("SaveAs"   "saveas_model")
              -
              ("Quit"     "quit"))


(add_sub_menu "Edit"
              ("Undo"                  "undo")
              ("Redo"                  "redo")
              -
              ("Recenter View"         "recenter")
              ("Navigate"              "navigate")
              ("Pan"                   "pan")
              ("Translate Mode"        "translate_point")
              -
              ("Update Model Textures" "update_textures")
              ("Update All Textures"   "update_all_textures")
              ("Join Coords"           "join_coords"))


(add_sub_menu "Selection"
              ("Select None"           "select_none")
              ("Select Similar"        "select_similar")
              ("Drag Select"           "drag_select")

              )


(add_sub_menu "Animation"
              ("Advance"  "frame_advance")
              ("Back"     "frame_back")
              ("Rewind"   "frame_rewind")
              ("Add"      "frame_add")
              ("Remove"   "frame_remove")
              ("Animate"  "toggle_animation")
              )


(add_sub_menu "Faces"
              ("Delete Faces"          "delete_sel")
              -
              ("Swap Face Numbers"     "swap_polynums")
              ("Edit Special"          "edit_special")
              ("Flip Normal"           "flip_normal_sel")
              ("Default Coordinates"   "default_coords")
              ("Tint Selected"         "tint_sel")
              ("Untint Selected"       "untint_sel")
              -
              ("Distribute Texture"    "distribute_sel")
              ("Rotate Textures"       "rotate_texture_sel")
              ("Reverse Textures"      "reverse_texture_sel")
              ("Animate Textures"      "animate_texture_sel")
              ("Pan Textures"          "pan_texture_sel")
              -
              ("Dump Polys to SCM"     "dump_polys")
              )
              
              

(add_sub_menu "Options"
              ("No tint"         "no_tint")
              ("Tint Team 1"     "team_1")
              ("Tint Team 2"     "team_2")
              ("Tint Team 3"     "team_3")
              ("Tint Team 4"     "team_4")
              -
              ("Shading"         "toggle_shading")
              -
              ("Toggle Axis"     "axis_toggle")
              ("Wire Frame"      "wireframe")
              -
              ("Toggle Texture Names" "toggle_names")
              ("Toggle Face Numbers" "toggle_numbers")
              ("Toggle Vertex Numbers" "toggle_vnumbers")
              )

(add_sub_menu "Background"
              ("Black"           "back_black")
              ("Red"             "back_red")
              ("White"           "back_white")
              ("Blue"            "back_blue")
              ("Dark Blue"       "back_darkblue")
              ("Green"           "back_green"))

