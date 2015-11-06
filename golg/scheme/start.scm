(setf normal_unlimited '(list_box normal unlimited))


(def_class player_vars nil
  (num_points        0)
  (num_stank_deaths  0)
  (num_stank_lives   3)

  (income_multipler  1)       ; computer players get an advantage
  (income_rate       0)
  (money             10000)

  (continue          0)       ; after stank dies
  (team_flag "team_flag")
)

(setf radar_darkness 1.0)   ; 0-1 scales the birghtness of the radar map
(setf editor_pan_accel 0.05)
(setf editor_rotate_accel 0.01)  ; in radians



; **************************** Border frame config *******************************
(setf strategy_camera_border 7)


  
; defines which weapons go with which upgrade levels and the border image to use
; weapons are defined in scheme/balance.scm
(setf default_frame "bitmaps/stank/def_frame.jpg")
(setf upgrade_levels
;     '(("bitmaps/stank/frame_level_0.jpg" guided_missile heavy_rocket  bolt kevlar)))
     '(("bitmaps/stank/frame_level_0.jpg" b120mm heavy_rocket    chain_gun kevlar)
       ("bitmaps/stank/frame_level_1.jpg" acid   vortex_missile  chain_gun reactive)
       ("bitmaps/stank/frame_level_2.jpg" napalm nuke_missile    chain_gun titanium)))

(setf upgrade_colors 
      '(0xffffff     ; no stank - white
        0x005fff     ; cyan - level 0
        0xff00       ; green - level 1 (acid)
        0xff0000))    ; red  - level 2  (fire)

; locations relative to status bar top left graphic
(setf action_mode_locations
      '((276   10)  ; lives
        (-10   500) ; money
        (92    34)  ; main
        (341   37)  ; missiles
        (369   6)   ; chain
        (92    6))) ; health

; locations relative to strategy-bar top left graphic
(setf strategy_mode_locations
      '((7 172)     ; lives
        (99 126)    ; money
        (37 270)    ; main
        (100 299)   ; missiles
        (101 218)   ; chain
        (37  218))) ; health


; *************************** Preferences ******************************
(setf team_tinting       nil)
(setf team_icons         nil)
(setf allow_follow_mode  nil)


;(set_default_ai "ai_builder")
(set_default_ai "ai_jim")


(setf watch_camera_accel 0.01)
(setf watch_camera_max_speed  0.3)
(setf watch_camera_turn_speed 0.04)

(setf font "bitmaps/golgotha_font.tga")


(def_class music_class nil
  (songs  "none" '(list_box 

                   ; ************ music list ***********
                   "music/splash_screen_opus_22khz.wav"

                   "music/roselyn_Chapel_scotland_ii.wav"
                   "music/rome_italy.wav"


;                   "music/frankfurt_germany_22khz.wav"
;                   "music/norway_nephelim_battle_22khz.wav"
;                   "music/rome_italy_22khz.wav"
;                   "music/roselyn_chapel_scotland_ii_22khz.wav"
;                   "music/naples_italy.wav"
;                   "music/turin_italy_22khz.wav"
;                   "music/vienna_austria_22khz.wav"
                   ; ************************************
                   ))
  (volume 63))

(setf music (new music_class))

(setf enemy_buildable  '(peon_tank bomb_truck engineer))
(setf player_buildable '(peon_tank bomb_truck stank engineer))
    


(load "scheme/classes.scm")

;(set_default_level "/u/ddt/src/dlev/d.level")


(setf mouse_left_button "Main Gun")
(setf mouse_right_button "Chain Gun")




