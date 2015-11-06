

;; Translation instructions :
;; Translate File/Edit/View/Window/Map/Terrain, etc.
;; Translate the first string (everything in quotes), do not translate the second string
;; pair

;;            Translate                 Don't Translate
;;            this colum                this colum
;----------------------------------------------------------------
(add_sub_menu "File"
              ("New"                    "File/New")
              -
              ("Save"                   "File/Save")
              ("Save As"                "File/Save As")
              ("Open"                   "File/Open")
              -
              ("Load DLL"               "File/Open DLL")
              -
              ("Exit"                   "File/Exit"))


(add_sub_menu "Edit"
              ("Undo"                   "Edit/Undo")
              ("Redo"                   "Edit/Redo")
              -
;              ("Cut"                    "Edit/Cut")
;              ("Copy"                   "Edit/Copy")
;              ("Paste"                  "Edit/Paste")
              ("Toggle Menu"            "Edit/Toggle Menu")
              -
              ("No Snap"                "Edit/No Snap")
              ("Snap Cell Center"       "Edit/Snap Cell Center")
              ("Snap Cell Origin"       "Edit/Snap Cell Origin"))


(add_sub_menu "View"
              ("Wireframe"              "View/Wireframe")
              ("Textured"               "View/Textured")
              ("Solid Colors"           "View/Solid")
              ("Toggle Texture Loading" "View/Toggle Texture Loading")
              -
              ("1 View"                 "View/1 View")
              ("4 Views"                "View/4 Views"))

(add_sub_menu "Tools"
              ("Scenes"                 "Tools/Scenes")
              ("Radar"                  "Tools/Radar")
              ("Profile"                "Tools/Profile")
              ("Debug"                  "Tools/Debug")
              ("AI"                     "Tools/AI")
              -
              ("Objects"                "Tools/Objects")
              ("Tiles"                  "Tools/Tiles"))


(add_sub_menu "Map"
              ("Simulate Tick"          "Map/Simulate Tick")
              ("Resize"                 "Map/Resize")
              ("Rotate 90"              "Map/Rotate 90")
              ("Change Sky"             "Map/Change Sky")
              ("SCM File"               "Map/SCM File")
              -
              ("Reload Map"             "reload_level")
              -
              ("Fog All"                "fog_map")
              ("UnFog All"              "unfog_map")
              -
              ("Dump Map (debug)"       "dump_level")
              ("Recalculate"            "Map/Recalculate")
              ("Delete Selected"        "Map/Delete Selected")
              -
              ("Edit Map Vars"          "edit_level_vars")
              ("Add Cloud Shadow"       "add_cloud_shadow")
              ("Remove Cloud Shadow"    "remove_cloud_shadow")
              )


(add_sub_menu "Objects"
              ("Select Game Pieces"     "Objects/Select Game Pieces")
              ("Select Similar"         "Objects/Select Similar")
              ("Floor Selected"         "Map/Floor Selected")
              ("Ceil Selected"          "Map/Ceil Selected")
              ("Drop Selected"          "Objects/Drop Selected")
              ("Full Health"            "full_health")
              ("Set Health"             "set_health")
              -
              ("Select Rest of Path"    "select_restof_path")
              ("Join Selected"          "join_path_ends")
              ("UnJoin Selected"        "unjoin_path_ends")
              ("Insert Path Node"       "insert_path_object") 
              -
              ("Toggle Path Display"    "toggle_show_list")
              ("Place Camera"           "place_camera")
              )


(add_sub_menu "Teams"
              ("No Tinting"             "tint_none")
              ("Tint team polys"        "tint_polys")
              ("Tint all polys"         "tint_all")
              -
              ("Nuetral Team"           "team_0")
              ("Green Team   (allies)"  "team_1")
              ("Red Team     (axis)"    "team_2")
              ("Blue Team    (axis)"    "team_3")
              ("Yellow Team  (axis)"    "team_4")
              -
              ("Edit Nuetral Team"      "edit_team_0")
              ("Edit Green"             "edit_team_1")
              ("Edit Red"               "edit_team_2")
              ("Edit Blue"              "edit_team_3")
              ("Edit Yellow"            "edit_team_4")


              )


(add_sub_menu "Terrain"
              ("Select All"             "Terrain/Select All")
              ("Flatten Selected"       "Terrain/Flatten Selected")
              ("Smooth Selected"        "Terrain/Smooth Selected")
                                                      
              ("Add Noise to Selected"  "Terrain/Add Noise to Selected")
;              ("Load Heights from map"   "Terrain/Load Heights from map")
              -
              ("Load Image Heightmap"   "Terrain/Load Image Heightmap")
              ("Save Image Heightmap"   "Terrain/Save Image Heightmap"))

(add_sub_menu "Sound"
              ("Pick Song"              "edit_music")
              ("Make Sfx List"          "list_sfx")
              -
              ("Music Volume Up"        "music_up")
              ("Music Volume Down"      "music_down"))

              
              

;; when adding editor functionality, the follow are available through the li system

;; (editor_undo int) 
;;; saves off undo information about sections of the map you are about to change
;;; if the sections are omitted, then the entire map is saved


;; (redraw_all)
;;; redraws all windows

;; (redraw)
;; redraws the current active window

;; (editor_changed)  
;;; if you changed the map and want the user to save before quiting

