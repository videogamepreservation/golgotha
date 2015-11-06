# Key name                         Command                      Contexts
#----------------------------------------------------------------------
(def_key  "Escape"                 "quit_demo"                  action strategy follow)
(def_key  "Left"                   "Move Left"                  action follow strategy)
(def_key  "Right"                  "Move Right"                 action follow strategy)
(def_key  "Up"                     "Move Forward"               action follow strategy)
(def_key  "Down"                   "Move Backward"              action follow strategy)
(def_key  "Del"                    "Move Left"                  action follow strategy)
(def_key  "PageDown"               "Move Right"                 action follow strategy)       
(def_key  "Home"                   "Move Forward"               action follow strategy)       
(def_key  "End"                    "Move Backward"              action follow strategy)       

(def_key  "Space"                  "Main Gun"                   action follow)
(def_key  "Pad 0"                  "Main Gun"                   action follow)
(def_key  "Enter"                  "Missiles"                   action follow)
(def_key  "Pad ."                  "Chain Gun"                  action follow)                
(def_key  "Pad ."                  "Nothing" 	                  strategy)                
(def_key  "Ctrl"                   "Chain Gun"                  action follow)
(def_key  "Ctrl"                   "Nothing"                    strategy)

(def_key  ","                      "Strafe Left"                action follow)                
(def_key  "Insert"                 "Strafe Left"                action follow)                
(def_key  "."                      "Strafe Right"               action follow)                
(def_key  "PageUp"                 "Strafe Right"               action follow)                
              
(def_key  "Tab"                    "strategy_toggle"            follow strategy action)
(def_key  "Backspace"              "strategy_toggle"            follow strategy action)

(def_key  "Z"                      "Zoom In"                    follow)
(def_key  "X"                      "Zoom Out"                   follow)
(def_key  "C"                      "Zoom Up"                    follow)
(def_key  "V"                      "Zoom Down"                  follow)
(def_key  "B"                      "Zoom Left"                  follow)
(def_key  "N"                      "Zoom Right"                 follow)

(def_key  "P"                      "Pause"                      follow action strategy)

(def_key  "\\"                     "Strafe Mode"                action follow)                
                                                                                       
(def_key  "Alt+F4"                 "File/Exit"                  editor action strategy follow)
(def_key  "["                      "music_down"                 action strategy follow editor)
(def_key  "]"                      "music_up"                   action strategy follow editor)

(def_key  "0"                      "upgrade_level_0"            action strategy follow)
(def_key  "1"                      "upgrade_level_1"            action strategy follow)
(def_key  "2"                      "upgrade_level_2"            action strategy follow)

(def_key  "-"                      "shrink_screen"              action strategy follow)
(def_key  "="                      "grow_screen"                action strategy follow)
(def_key  "m"                      "toggle_follow_mode"         action follow)
                                                                                       
# Gifts to trey                    command                                                     
#--------------------------------------------------------------------                 
(def_key  "'"                      "Move Forward"               action follow strategy)
(def_key  "/"                      "Move Backward"              action follow strategy)
(def_key  "Shift"                  "Main Gun"                   action follow strategy)       
                                                                                       
                                                                                       
#Build Keys                         command                                                     
#--------------------------------------------------------------------                 

(def_key  "0"                      "team_0")
(def_key  "1"                      "team_1")
(def_key  "2"                      "team_2")
(def_key  "3"                      "team_3")
(def_key  "4"                      "team_4")


(def_key  "Alt+0"                  "tint_none")
(def_key  "Alt+1"                  "tint_polys")
(def_key  "Alt+2"                  "tint_all")

(def_key  "`"                      "Build Blank"                action follow strategy)       
(def_key  "Q"                      "Build stank"                action follow strategy)       
(def_key  "W"                      "Build peon_tank"            action follow strategy)       
(def_key  "E"                      "Build trike"                action follow strategy)       
(def_key  "R"                      "Build rocket_tank"          action follow strategy)       
(def_key  "T"                      "Build electric_car"         action follow strategy)       
(def_key  "Y"                      "Build helicopter"           action follow strategy)       
(def_key  "U"                      "Build jet"                  action follow strategy)       
(def_key  "I"                      "Build engineer"             action follow strategy)       
                                                                                       
(def_key  "A"                      "Build electric_car"         action follow strategy)       
(def_key  "S"                      "Build helicopter"           action follow strategy)       
(def_key  "D"                      "Build jet"                  action follow strategy)       
(def_key  "F"                      "Build engineer"             action follow strategy)       
                                                                                       

# Debugging keys
# -----------------------------------------------------------------
(def_key  "O"                      "Toggle Stats"                action follow strategy)

(def_key  "F1"                     "Show Help"                   action follow strategy)
(def_key  "F2"                     "Screen Shot"                 action follow strategy editor)

(def_key  "F3"                     "play_demo"                   action follow strategy)
(def_key  "F4"                     "record_toggle"               action follow strategy)

(def_key  "F5"                     "Map/Change Sky"              editor follow strategy action)
(def_key  "F6"                     "Edit/Toggle Menu"            action follow strategy editor)
(def_key  "F7"                     "Tools/Profile"               action follow strategy editor)
(def_key  "F8"                     "Cheat Menu"                  action follow strategy editor)
(def_key  "F9"                     "edit_music"                  action follow strategy editor)
(def_key  "F12"                    "File/Save As"                editor)                       




# Editor keys
# ------------------------------------------------------------------

(def_key  "Left"                   "Pan Left"                   editor)
(def_key  "Right"                  "Pan Right"                  editor)
(def_key  "Up"                     "Pan Forward"                editor)
(def_key  "Down"                   "Pan Backward"               editor)

#(def_key  "Del"                    "Pan Left"                   editor)
#(def_key  "PageDown"               "Pan Right"                  editor)
#(def_key  "Home"                   "Pan Forward"                editor)
#(def_key  "End"                    "Pan Backward"               editor)

(def_key  "Del"                    "Map/Delete Selected"        editor)
(def_key  "Shift+Left"             "Rotate Left"                editor)
(def_key  "Shift+Right"            "Rotate Right"               editor)
(def_key  "Shift+Up"               "Pan Up"                     editor)
(def_key  "Shift+Down"             "Pan Down"                   editor)
(def_key  "Alt+Up"                 "Rotate Up"                  editor)
(def_key  "Alt+Down"               "Rotate Down"                editor)

(def_key  "Ctrl+N"                 "File/New"                   editor)                       
(def_key  "Ctrl+S"                 "File/Save"                  editor)                       

(def_key  "Ctrl+D"                 "File/Open DLL"              editor)                       

                                                                                       
(def_key  "Ctrl+Z"                 "Edit/Undo"                  editor)                       
(def_key  "Ctrl+A"                 "Edit/Redo"                  editor)
(def_key  "Ctrl+X"                 "Edit/Cut"                   editor)
(def_key  "Ctrl+C"                 "Edit/Copy"                  editor)
(def_key  "Ctrl+V"                 "Edit/Paste"                 editor)


(def_key  "Alt+1"                  "View/1 View"                editor)  
(def_key  "Alt+4"                  "View/4 Views"               editor)  
(def_key  "Ctrl+6"                 "View/Wireframe"             editor)
(def_key  "Ctrl+7"                 "View/Textured"              editor)

(def_key  "Ctrl+F9"                "View/Toggle Wireframe"      editor)
(def_key  "Ctrl+F10"               "View/Toggle Textured"       editor)


(def_key  "Alt+O"                  "Tools/Objects"              editor)
(def_key  "Alt+T"                  "Tools/Tiles"                editor)
(def_key  "Alt+R"                  "Tools/Radar"                editor)

(def_key  "Ctrl+T"                 "Map/Simulate Tick"          editor)

(def_key  "Ctrl+Q"                 "full_reload_level"          editor)
(def_key  "Ctrl+R"                 "reload_level"               editor)


(def_key  "Alt+L"             "Terrain/Recalc Light"       editor)
(def_key  "Alt+M"             "Terrain/Merge"              editor)
(def_key  "Alt+F"             "Terrain/Flatten Selected"   editor)
(def_key  "Alt+S"             "Terrain/Smooth Selected"    editor)
(def_key  "Alt+N"             "Terrain/Add Noise to Selected" editor)

(def_key  "Ctrl+O"                 "Objects/Select Game Pieces" editor)                       


(def_key  "Alt+Space"              "edit_selected"              editor)
(def_key  "Alt+X"                  "add_link"                   editor)
(def_key  "Alt+Z"                  "remove_link"                editor)

(def_key  "Alt+`"                  "read-eval"                  editor follow strategy action)
(def_key  ","                      "move_selected_down"         editor)
(def_key  "."                      "move_selected_up"           editor)

(def_key  "Alt+["                  "fix_previous_link"          editor)
(def_key  "Alt+]"                  "fix_forward_link"           editor)
(def_key  "Alt+P"                  "fix_path_link"              editor)



