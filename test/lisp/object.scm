
/*
(def_container object_settings   nil      # not derived from anything

 (speed           (type float)    (default 0.12)      (range 0.01 0.5))
 (accel           (type float)    (default 0.045)     (range 0.01 0.5))
 (turn_speed      (type degree)   (default 0.2)       (range 1 360))
 (cost            (type int)      (default 200))
 (health          (type int)      (default 20)        (range 1 maxint))
 (damage          (type int)      (default 3)         (range 0 maxint))
 (fire_delay      (type int)      (default 10)        (range 0 maxint))
 (build_image     (type image))
 (build_time      (type int)      (default 20)        (range 0 maxint))
 
(set obj_model_dir "x:/jc/gdata/build/%s.gmod")

(def_object 
  

(def_container 




(varibles


 (global_id       (type int)       (default 0) (range 0..100))
 (construct_type  (type g1_object) (default "peon_tank") 
                  (range "peon_tank" "missle_tank" "eletric_car"))
 (path            (type g1_path)) 

)


(int global_id 0)
(float x)
(float y)


  w32 global_id;             // unique to the object, used to reference via networking

  g1_object_type id;         // this is the object's 'type', i.e. all peon tanks have the same id

  i4_float x,y;              // location in game map in game coordinates
  i4_float lx,ly;            // last tick position (use to interpolate frames)

  i4_float h;                // height above (or below ground)
  i4_float lh;               // last tick height

  i4_float theta;            // facing direction in the x-y game plane
  i4_float ltheta;           // last tick theta (yaw)

  i4_float pitch;            // pitch (y-z)
  i4_float lpitch;           // last tick pitch

  i4_float roll;             // roll (x-z)
  i4_float lroll;            // last tick roll
*/
