// ********* Required for every application
error "Bad string!"


//  color_name                  red grn blue
//--------------------------------------------
window_active_bright           "220 220 200"
window_active_medium           "180 180 160"
window_active_dark             "160 160 130"

text_foreground                "0   0   0"
text_background                "150 150 150"
selected_text_foreground       "255 255 255"
selected_text_background       "0   128 128"


window_passive_bright          "180 180 180"
window_passive_medium          "150 150 150"
window_passive_dark            "100 100 100"

root_background_color          "150 150 150"


menu_foreground                "0   0   0"
menu_background                "128 128 128"
selected_menu_foreground       "255 255 255"
selected_menu_background        "0  0   128"

drag_bar_gradiant_active_start "0   200 200"
drag_bar_gradiant_active_end   "0   0   0"
drag_bar_gradiant_start        "128 128 128"
drag_bar_gradiant_end          "0   0   0"


//-------------------------------------------

window_manager = MWM

i4_resources_dir=resource
// ******** ICONS

close_icon resource/closicon.tga
up_icon    resource/upicon.pcx
down_icon  resource/downicon.pcx
left_icon  resource/lefticon.pcx
right_icon resource/rigticon.pcx
plus_icon  resource/plus.pcx
minus_icon resource/minus.pcx
background_bitmap resource/bground.tga
ok_icon           resource/ok.tga
cancel_icon       resource/cancel.tga


// ******** FONTS
small_font resource/helvetica_8.tga


// ******** CURSORS
normal_cursor      "resource/normcurs.pcx 0 1 0"
text_cursor        "resource/textcurs.pcx 0 3 7"

load_failed "Failed to load %S"
no_key      "No such keyname '%s' (%S)"
modifier_in_use "Cannot assign key %s, modifier already in use in this context"

button_delay = 250
button_repeat = 50
double_click = 300

prof_win_title "Profile"
expected       "Error, expected : "
status         "Progress"

get_savename_dialog = 
"[down x+(5) y+(20) 
 text('     Enter filename to save as')
 %p=text_input(400 '%S')
 x+(130) y+(10)
 [right button(text('  Ok  ') user_ev(%p %d))
        button(text('Cancel') user_ev(%p %d))]
]"

cancel_operation "CANCEL"
