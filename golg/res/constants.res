lock_cheat 0
disable_a3d 1
disable_sound 0

default_cursor     "bitmaps/cursors/default_%s%d.tga 16646822  0 0"
select_cursor      "bitmaps/cursors/select_%s%d.tga 16646822   8 8"
move_cursor        "bitmaps/cursors/move_%s%d.tga 16646822     8 8"
target_cursor      "bitmaps/cursors/attack_%s%d.tga 16646822   8 8"
follow_cursor      "bitmaps/cursors/follow_%s%d.tga 16646822   8 8"
x_cursor           "bitmaps/cursors/x_%s%d.bmp 16646822        8 8"
takeover_cursor    "bitmaps/cursors/takeover_%s%d.tga 16646822 8 8"

gravity                        0.05  // universal gravity
damping_friction               0.2   // speed damping friction

recoil_factor  0.3
recoil_gravity 0.05
bullet_speed   1.5

sink_rate                      0.01
water_damage                   2

player_turn_speed              0.15
strafe_turn_speed              0.2
player_accel                   0.05
player_strafe_accel            0.08
player_max_speed               0.2
player_max_strafe_speed        0.2
player_stop_friction           0.60
slow_camera_speed              0.10
max_camera_off                 0.5
player_roll_speed              0.1
player_roll_max                0.5
player_turret_radius            0.6
player_top_attach              { 0.0 0.0 0.0 }
player_fire_slop_angle         5.0


small_health_added             50
large_health_added             200

small_bullets_added            20
large_bullets_added            100

small_chain_added              20
large_chain_added              100

small_missiles_added           1
large_missiles_added           2

small_money_added              1000
large_money_added              10000


supergun_muzzle_attach         { -0.2 0.0 0.75}

turret_muzzle_attach           { 0.0 0.0 0.15}
turret_top_attach              { 0.0 0.0 0.0}

repairer_tip_attach            { 1.5 0.0 0.6}
repairer_boom_offset           1.5


health_window =     {  93   6 272 25 }
chain_gun_window =  { 369   6 548 25 }
main_gun_window =   {  94   34 305 53 }
missile_window =    { 341   37 520 51 }

options_window =    { 286    25 316 57 }

radar_window =      { 545     4 635  63 }   // small radar in action mode
big_radar_window =  { 406     0 635  240 }  // radar in strategy_mode


pov_window     =   {   0    0  639 419 }    // main 3d view
small_pov_window = {   0    0  400 479 }    // 3d view in strategy mode

build_buttons  =    { 404   236 564 358 }

lives          =    { 315   431 346 444 }

mouse_scroll   =    { 5       5  5   5 }    // amount of area on
balance        =    { 295     5  395 14 }   // left,top,right & bottom


rocket_tank_top_attach         { -0.0821 0.0 0.1482}

lod_switch_dist                  100
lod_disappear_dist               1600
skimpy_details_dist              25    // beyond this distance things lights/explosions don't show

visual_radii_low                 15 
visual_radii_med                 20
visual_radii_high                25
 
camera_dist                    -0.23
camera_angle                   0.0

startegy_camera_angle          1.2
startegy_camera_dist           4
strategy_camera_turn_angle     1.57

follow_camera_height           2.0
follow_camera_dist             3.0
follow_camera_rotation         0.0


// locations of field in the net graphic
net_hostname_x                 34
net_hostname_y                 395

net_username_x                 480
net_username_y                 183

net_found_x1                   480
net_found_x2                   608
net_found_y                    295

net_start_x                    30
net_start_y                    45

net_find_port                  15740
net_udp_port                   15741

// locations of compass
compass_x                      -50
compass_y                      5

// locations & sizes for pad status displays
stat_pad_start_x               -30
stat_pad_start_y               5
stat_pad_size_x                32
stat_pad_size_y                32
stat_pad_step_x                -32
stat_pad_step_y                32

supertank_turret_damage         450
supertank_machine_gun_damage    5
supertank_guided_missile_damage 400

verybiggun_damage                45

action_strategy_ticks_to_switch 8   // 10 = 1 second


images = { "bitmaps/letter-0.bmp"
           "bitmaps/letter-1.bmp"  
           "bitmaps/letter-2.bmp"  
           "bitmaps/letter-3.bmp"  
           "bitmaps/letter-4.bmp"  
           "bitmaps/letter-5.bmp"  
           "bitmaps/letter-6.bmp"  
           "bitmaps/letter-7.bmp"  
           "bitmaps/letter-8.bmp"  
           "bitmaps/letter-9.bmp"  
           "bitmaps/letter-dollar.bmp"  
           "bitmaps/letter-comma.bmp"  

           "bitmaps/options/open.tga"
           "bitmaps/options/visibility_close.tga"
           "bitmaps/options/visibility_medium.tga"
           "bitmaps/options/visibility_far.tga"
           "bitmaps/options/sound_normal.bmp"
           "bitmaps/options/sound_3d.bmp"
           "bitmaps/options/shadows_off.tga"        
           "bitmaps/options/shadows_on.tga"
           "bitmaps/options/interlace_on.tga"       
           "bitmaps/options/doublepixel_on.tga"
           "bitmaps/options/doublepixel_off.tga"
           "bitmaps/options/gamespeed.tga"
        }
        


