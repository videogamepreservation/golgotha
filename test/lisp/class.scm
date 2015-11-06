(def_class base_class nil
  (base_var 'holy_cow_man))

(def_class color nil
  (red 255)
  (green 255)
  (blue 255))

(print (new color 128 0 5))


(def_class test_class base_class
  (x 4)
  (y 5)
  (z 6)
  (my_symbol 'hello_there)
  (my_string "string")
)

(setf test (new test_class))
