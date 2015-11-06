//                                                                                 
//                      turn                                 fire   fire
//              speed   speed  accel  cost   health  damage  delay  range
//////////////////////////////////////////////////////////////////////////////////////////////////

object_defaults = "
electric_car    0.08    0.2    0.030   800     50      3      0      4
peon_tank       0.08    0.20   0.030   300     30     10     10      5
helicopter      0.08    0.09   0.030  1000    150     30     30      6
jet             0.08    0.09   0.030  1500     30     20     20      6
engineer        0.08    0.2    0.030   300     35      0      0      6
trike           0.08    0.2    0.030   500    100    200      0      6
tank_buster     0.08    0.2    0.030   800     40     30     20      6
rocket_tank     0.08    0.2    0.030   800     40     30     20      6
verybiggun        0     0.1    0         0    200     45     15      6
turret            0     0.1    0         0    200     25      7      6
supergun          0     0.01   0         0   1200   1000     30      6
guided_missile  0.8     0.1    0.030     0    100      0      0      0
buster_rocket   0.9     0.1    0.025     0    100      0      0      0
nuc_missile     0.9     0.1    0.025     0    100      0      0      0
base_launcher     0     0.2    0         0      0   1000     20      6
repairer        0.1     0.05   0         0      0     20      5      3  


// some supertank constants are in constants.res named player_XXXX
stank           0.3     0.2    0.1   10000    1000    10      5       6     

"

