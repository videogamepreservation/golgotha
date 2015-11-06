rem Please edit make\build_i4_make to reflect your msdev path (if not c:\msdev)
rem Also edit i4\project.i4 to reflect your msdev path 

cd make
call build_i4_make
cd ..\golg
..\make\i4_make
cd ..\maxtool
..\make\i4_make
cd ..\test\cloud_test
..\..\make\i4_make debug
cd ..\g_install
..\..\make\i4_make debug
cd ..\lisp
..\..\make\i4_make debug
cd ..\spin_cube
..\..\make\i4_make debug
cd ..\web_crawl
..\..\make\i4_make debug
cd ..\font_test
..\..\make\i4_make debug
cd ..\ray_tracer
..\..\make\i4_make debug
cd ..\thread
..\..\make\i4_make debug
cd ..\..