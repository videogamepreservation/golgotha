
(def_class color nil
  (red 255)
  (green 255)
  (blue 255))

(def_class options nil
  (pixel                 (new color))
  (render_mode           'single_pixel      '(list_box single_pixel double_pixel interlaced))    
  (view_distance         'near              '(list_box near mid far))
  (sound_volume          63.0               )
  (ambient_and_music     'on                '(list_box on off))
  (ambient_volume        63                 )
  (shadows               T                  )
  (sound                 '_2d               '(list_box off _2d _3d))
  )
  

(setf p (new options))
