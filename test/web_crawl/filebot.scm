; make sure this directory exsist!
(setf save_path "e:\\mp3\\")

; pages to start traversing from
(add_page "http://personal.nbnet.nb.ca/jkeller/MP3.HTM")
(add_page "http://www.geocities.com/Area51/Vault/8821/mp3.html")
(add_page "http://personal.nbnet.nb.ca/jkeller/MP3.HTM")
(add_page "http://hem2.passagen.se/lethagen/Prodigy/")
(add_page "http://thebigsite.com/index.html")
(add_page "http://www.wgn.net/~eburrow/mp3.html")

; add thread takes x & y offset for window position
(add_thread 0   20)  (add_thread 320   20)  
(add_thread 0   70)  (add_thread 320   70)
(add_thread 0  120)  (add_thread 320  120)
(add_thread 0  170)  (add_thread 320  170)
(add_thread 0  220)  (add_thread 320  220)
(add_thread 0  270)  (add_thread 320  270)
(add_thread 0  320)  (add_thread 320  320)
(add_thread 0  370)  (add_thread 320  370)
(add_thread 0  420)


(setf process_extensions  '(".html" ".htm"))
(setf save_extensions     '(".mp3"))

(setf include_sub_strings '(
                            (".html")    ; follow any html page
                            (".htm")   
                            (".com")   
                            (".net")   
                            (".edu")   
                            (".htm")   
                            (".mp3"  
                             "rem."
                             "weezer"
                             "ADAM_SANDLER" "adamsandler"
                             "u2"
                             "willsmith"  "will_smith"
                             "offspring"
                             "sarah_mclachlan" "sarahmclachlan"
                             "No_Doubt" "nodoubt"
                             "Notorious"  "puffdaddy"
                             "james_bond" "jamesbond"
                             "mariah_carey" "Mariahcarey"
                             "matchbox20"
                             "pinkfloyd"
                             "aerosmith"

                             "batman" "movie"  "jurpark" "indianajones" "indiana_jones"

                             "Robert_Miles"
                             "No_Doubt"
                             "depeche"
                             "queen"
                             "simpsons"
                             "simon"
                             
                             "CHUMBAWAMBA"
                             "SirenPossession"
                             "Spice"
                             "Karma Police"  "karampolice"
                             "prodigy"
                             )))

                           

; don't process any page/file that has these sub strings
(setf exclude_sub_strings '("toriamos"     ; enough already!
                            "jacky1"       ; this guy has crap
                            "mailto"
                            "www.mp3.com"  ; they don't have any that I can tell of
                            ))

; don't save files smaller than this  (700k)
(setf save_min_size 700000)

; load up our history of previously saved files
(load "saved_files.scm")

; load up history of visited urls
(load "visited_urls.scm")


; timeout depends on how much data we have already, if we we a lot then
; give a little longer to respond so we have less chance of wasting time

(setf timeouts '((100000000 1000)    ; 
                 (1000000  500)  
                 (500000   360)                     
                 (200000   120)
                 (100000    60)
                 (50000     45)     
                 (10000     30)    ; if we have 10K give them 30 seconds to respond
                 ))  
