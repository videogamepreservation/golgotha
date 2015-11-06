(def_class damager_vars nil
  (attach_pos  (new vector 0 0 0))
  (smoke_type  'acid '(list_box acid napalm))
  (ticks_to_think  1) ; after visible, think for 2 seconds
  (damage_per_tick 0)
  (person_being_damaged (new object_ref))
  (person_giving_damage (new object_ref))
)


(def_class takeover_pad_vars nil
  (trigger_objects  (new object_ref_list))
  (trigger_message  'on)
  (takeover_objects (new object_ref_list))
  (turret           (new object_ref)))

(def_class bank_vars nil
  (can_build  '(moneyplane))
  (ticks_till_next_deploy   0)
  (reset_time_in_ticks      50)
  (path_color               0)
  (start                    (new object_ref))
  (moneyplanes     1)
  (time            0)
  (income_time     50)
  (crates          0)
  (crate_value     2000)
  (crate_capacity  10)
  (crate                    (new object_ref)))

(def_class moneycrate_vars nil
  (vspeed          0.0)
  (crate_value     100))

(def_class moneyplane_vars nil
  (crate                    (new object_ref))
  (mode            0))

(def_class lawfirm_vars nil
                (income_rate        3000)    ; how much they are sueing for
                (commision           0.5)    ; lawyers get %50 of income from other players
                (counter             0)      ; ticks left till current law-suite   
                (reset_time          150)     ; ticks till next law-suite
                )
             

(def_class chain_gun_type_vars nil
  (texture_name              "blue_flare")
  (texture_size              0.2))


(def_class director_vars nil
  ; use an objects as the location to send people to.  Randly picks one
  (deploy_to            (new object_ref_list)) 

  (range_when_deployed  2.0)                ; when an object is this close, send it to destination
  (current_state       'on                  ; when off it will not deploy
                       '(list_box on off))
  (on_message          'on)                 ; when we get this message we turn on
  (off_message         'off)
  (who_to_send         'team '(list_box team enemy anyone))

  
  ; list of nearby objects we've already sent so we don't send them again,
  ; or if we turned off, then a list of objects to send when we turn on
  (nearby_objects              (new object_ref_list))

  ; objects should be sent next tick.
  (objects_to_send_next_tick    (new object_ref_list))
)

(def_class trigger_vars nil
  (range_when_activated   3.0     '(list_box 1.0 2.0 3.0 4.0 5.0 6.0))
  (range_when_deactivated 4.0     '(list_box 1.0 2.0 3.0 4.0 5.0 6.0))
  (who_can_trigger        'enemy '(list_box anyone team_mates enemy
                                   supertanks enemy_supertanks))
  (objects_to_trigger    (new object_ref_list))
  (send_on_trigger       'on)
  (send_on_untrigger     'off)
  (objects_in_range      (new object_ref_list))
  (current_state         'on  '(list_box on off))
)


(def_class garage_vars nil
  (can_build  '(electric_car peon_tank engineer 
                trike tank_buster rocket_tank bomb_truck bridger))
  (ticks_till_next_deploy    0)
  (reset_time_in_ticks      10)
  (path_color               0x7f7f7f)
  (selected_path_color      0xffffff)
  (start                    (new object_ref)))

(def_class airbase_vars nil
  (can_build  '(helicopter jet bomber))
  (ticks_till_next_deploy    0)
  (reset_time_in_ticks      10)
  (traffic_height           1.5)   ; height of all flying objects objects 
  (path_color               0x7f007f)
  (selected_path_color      0xff00ff)
  (start                    (new object_ref)))

(def_class mainbasepad_vars nil
  (can_build  '(stank))
  (ticks_till_next_deploy    0)
  (reset_time_in_ticks      10)
  (path_color               0x7f7f7f)
  (selected_path_color      0xffffff)
  (start                    (new object_ref)))

(def_class path_object_vars nil
  (warning_level      0   '(list_box 0 1 2 3 4 5 6 7 8 9))
  (bridgeable_spot    'no '(list_box no yes already_attached))    
  (active   'on       '(list_box on off))     ; is this node turned on?
  (controlled_objects (new object_ref_list))) ; objects taken over by engineers & other specials

(def_class secret_hider_vars nil
  (grab_height    'no '(list_box no yes))
  (grab_textures  'no '(list_box no yes)))

(def_class field_camera_vars nil
  (name  "unknown"))


(def_class bridger_vars nil
  (marker_attached_to (new object_ref))
)

(def_class guided_missile_vars nil
  (smoke_trail    (new object_ref))
  (who_fired_me   (new object_ref))
  (track_object   (new object_ref))
  (fuel           0.0)
  (velocity       (new vector)))

(def_class buster_rocket_vars nil
  (smoke_trail    (new object_ref))
  (who_fired_me   (new object_ref))
  (track_object   (new object_ref))
  (fuel           0.0)
  (velocity       (new vector)))

(def_class heavy_rocket_vars nil
  (smoke_trail    (new object_ref))
  (who_fired_me   (new object_ref))
  (track_object   (new object_ref))
  (fuel           0.0)
  (velocity       (new vector)))

(def_class vortex_missile_vars nil
  (smoke_trail    (new object_ref))
  (who_fired_me   (new object_ref))
  (track_object   (new object_ref))
  (fuel           0.0)
  (velocity       (new vector)))

(def_class nuke_missile_vars nil
  (smoke_trail    (new object_ref))
  (who_fired_me   (new object_ref))
  (track_object   (new object_ref))
  (fuel           0.0)
  (velocity       (new vector)))


(def_class crate_vars nil
  (type   'money   '(list_box health bullet missile chain_gun money))
  (amount 'small    '(list_box small large))
  (yvel    0.0)
  (ticks_left -1))

(def_class cloud_color nil
  (red   1.0)
  (green 1.0)
  (blue  1.0)
  (alpha 1.0)
)

(def_class level_vars nil
;  (bottom_cloud_layer    (new cloud_color 0.8 0.6 0.4 1.0))
;  (top_cloud_layer (new cloud_color 1.0 0.8 0.6 0.75)) 

  (bottom_cloud_layer    (new cloud_color 1.0 1.0 1.0 1.0))
  (top_cloud_layer (new cloud_color 0.8 0.8 0.8 0.75)) 
)


