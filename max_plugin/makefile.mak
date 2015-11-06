LTMP=/tmp
DOSTMP=c:/tmp
MACTMP=duh:tmp:
MacLinks=mac_links:
MSDEV=c:/msdev
all: c:/tmp/p1_16.dll c:/tmp/p1o_16.dll

# {{{ CL Variables

INCLUDE_p10=\
  /I "..\i4\inc"\
  /I "..\i4"\
  /I "..\golg"\
  /I "..\render"\
  /I "sdk_inc"

DEFINES_p10=\
  /D "WIN32"\
  /D "_WINDOWS"\
  /D "__MSC__"\
  /D "I4_SCREEN_DEPTH=16"\
  /D "DEBUG"

CPP=cl
CPPFLAGS_p10=/nologo /MD /Zi /Od

# }}} #############################################################

LIBS_p10=\
  $(MSDEV)/lib/kernel32.lib\
  $(MSDEV)/lib/user32.lib\
  $(MSDEV)/lib/gdi32.lib\
  $(MSDEV)/lib/comdlg32.lib\
  $(MSDEV)/lib/oldnames.lib\
  $(MSDEV)/lib/comctl32.lib\
  $(MSDEV)/lib/msvcrt.lib\
  $(MSDEV)/lib/winmm.lib\
  c:/maxsdk/lib/GEOM.LIB\
  c:/maxsdk/lib/UTIL.LIB\
  c:/maxsdk/lib/CORE.LIB

LFLAGS_p10=/DEBUG /NODEFAULTLIB

RES=rc
RESFLAGS=/l 0x409 /d "_DEBUG" /i "$(MSDEV)/mfc/include"

c:/tmp/23f88346.obj: ..\i4\loaders\dir_load.cc ..\i4\loaders\dir_load.hh ..\i4\file\file.hh ..\i4\inc\arch.hh ..\i4\inc\isllist.hh ..\i4\string\string.hh ..\i4\init\init.hh ..\i4\memory\growheap.hh ..\i4\memory\malloc.hh ..\i4\memory\bmanage.hh ..\i4\memory\lalloc.hh ..\i4\error\error.hh ..\i4\file\open.hh ..\i4\memory\array.hh ..\i4\checksum\checksum.hh 
	$(CPP) $(CPPFLAGS_p10) $(INCLUDE_p10) $(DEFINES_p10) /c /Tp "..\i4\loaders\dir_load.cc" /Fo"c:/tmp/23f88346.obj"

c:/tmp/52dbcd55.obj: ..\i4\loaders\dir_save.cc ..\i4\loaders\dir_save.hh ..\i4\file\file.hh ..\i4\inc\arch.hh ..\i4\inc\isllist.hh ..\i4\string\string.hh ..\i4\init\init.hh ..\i4\memory\growheap.hh ..\i4\memory\malloc.hh ..\i4\memory\bmanage.hh ..\i4\memory\lalloc.hh ..\i4\error\error.hh ..\i4\file\open.hh ..\i4\memory\array.hh ..\i4\checksum\checksum.hh 
	$(CPP) $(CPPFLAGS_p10) $(INCLUDE_p10) $(DEFINES_p10) /c /Tp "..\i4\loaders\dir_save.cc" /Fo"c:/tmp/52dbcd55.obj"

c:/tmp/f2117cc4.obj: ..\i4\checksum\checksum.cc ..\i4\inc\arch.hh 
	$(CPP) $(CPPFLAGS_p10) $(INCLUDE_p10) $(DEFINES_p10) /c /Tp "..\i4\checksum\checksum.cc" /Fo"c:/tmp/f2117cc4.obj"

c:/tmp/81088d5e.obj: ..\i4\file\file.cc ..\i4\file\file.hh ..\i4\inc\arch.hh ..\i4\inc\isllist.hh ..\i4\string\string.hh ..\i4\init\init.hh ..\i4\memory\growheap.hh ..\i4\memory\malloc.hh ..\i4\memory\bmanage.hh ..\i4\memory\lalloc.hh ..\i4\error\error.hh ..\i4\file\open.hh ..\i4\time\profile.hh ..\i4\time\win_prof.hh ..\i4\time\linux_prof.hh 
	$(CPP) $(CPPFLAGS_p10) $(INCLUDE_p10) $(DEFINES_p10) /c /Tp "..\i4\file\file.cc" /Fo"c:/tmp/81088d5e.obj"

c:/tmp/2ab10b1a.obj: ..\i4\file\win32\win_file.cc ..\i4\file\file.hh ..\i4\inc\arch.hh ..\i4\inc\isllist.hh ..\i4\string\string.hh ..\i4\init\init.hh ..\i4\memory\growheap.hh ..\i4\memory\malloc.hh ..\i4\memory\bmanage.hh ..\i4\memory\lalloc.hh ..\i4\error\error.hh ..\i4\file\open.hh ..\i4\time\profile.hh ..\i4\time\win_prof.hh ..\i4\time\linux_prof.hh ..\i4\file\get_dir.hh ..\i4\file\buf_file.hh ..\i4\file\async.hh ..\i4\memory\que.hh ..\i4\threads\signal.hh ..\i4\threads\linux\pthread.h ..\i4\threads\critical.hh 
	$(CPP) $(CPPFLAGS_p10) $(INCLUDE_p10) $(DEFINES_p10) /c /Tp "..\i4\file\win32\win_file.cc" /Fo"c:/tmp/2ab10b1a.obj"

c:/tmp/ec261cdc.obj: ..\i4\file\async.cc ..\i4\file\async.hh ..\i4\memory\que.hh ..\i4\inc\arch.hh ..\i4\threads\signal.hh ..\i4\threads\linux\pthread.h ..\i4\threads\critical.hh ..\i4\init\init.hh ..\i4\file\file.hh ..\i4\inc\isllist.hh ..\i4\string\string.hh ..\i4\memory\growheap.hh ..\i4\memory\malloc.hh ..\i4\memory\bmanage.hh ..\i4\memory\lalloc.hh ..\i4\error\error.hh ..\i4\file\open.hh ..\i4\threads\threads.hh ..\i4\time\time.hh 
	$(CPP) $(CPPFLAGS_p10) $(INCLUDE_p10) $(DEFINES_p10) /c /Tp "..\i4\file\async.cc" /Fo"c:/tmp/ec261cdc.obj"

c:/tmp/4960b186.obj: ..\i4\init\init.cc ..\i4\init\init.hh ..\i4\inc\arch.hh ..\i4\error\error.hh 
	$(CPP) $(CPPFLAGS_p10) $(INCLUDE_p10) $(DEFINES_p10) /c /Tp "..\i4\init\init.cc" /Fo"c:/tmp/4960b186.obj"

c:/tmp/a7da6c2f.obj: ..\i4\memory\malloc.cc ..\i4\inc\arch.hh ..\i4\memory\malloc.hh ..\i4\memory\bmanage.hh ..\i4\error\error.hh ..\i4\threads\critical.hh 
	$(CPP) $(CPPFLAGS_p10) $(INCLUDE_p10) $(DEFINES_p10) /c /Tp "..\i4\memory\malloc.cc" /Fo"c:/tmp/a7da6c2f.obj"

c:/tmp/fd41af82.obj: ..\i4\memory\bmanage.cc ..\i4\memory\bmanage.hh ..\i4\inc\arch.hh ..\i4\error\error.hh 
	$(CPP) $(CPPFLAGS_p10) $(INCLUDE_p10) $(DEFINES_p10) /c /Tp "..\i4\memory\bmanage.cc" /Fo"c:/tmp/fd41af82.obj"

c:/tmp/58a03d14.obj: ..\i4\memory\growheap.cc ..\i4\memory\growheap.hh ..\i4\memory\malloc.hh ..\i4\inc\arch.hh ..\i4\memory\bmanage.hh ..\i4\inc\isllist.hh ..\i4\error\error.hh 
	$(CPP) $(CPPFLAGS_p10) $(INCLUDE_p10) $(DEFINES_p10) /c /Tp "..\i4\memory\growheap.cc" /Fo"c:/tmp/58a03d14.obj"

c:/tmp/abc12eab.obj: ..\i4\file\win32\get_dir.cc ..\i4\string\string.hh ..\i4\inc\arch.hh ..\i4\init\init.hh ..\i4\memory\growheap.hh ..\i4\memory\malloc.hh ..\i4\memory\bmanage.hh ..\i4\inc\isllist.hh ..\i4\memory\lalloc.hh ..\i4\error\error.hh ..\i4\file\open.hh 
	$(CPP) $(CPPFLAGS_p10) $(INCLUDE_p10) $(DEFINES_p10) /c /Tp "..\i4\file\win32\get_dir.cc" /Fo"c:/tmp/abc12eab.obj"

c:/tmp/2f450bfa.obj: ..\i4\file\buf_file.cc ..\i4\file\buf_file.hh ..\i4\file\file.hh ..\i4\inc\arch.hh ..\i4\inc\isllist.hh ..\i4\string\string.hh ..\i4\init\init.hh ..\i4\memory\growheap.hh ..\i4\memory\malloc.hh ..\i4\memory\bmanage.hh ..\i4\memory\lalloc.hh ..\i4\error\error.hh ..\i4\file\open.hh 
	$(CPP) $(CPPFLAGS_p10) $(INCLUDE_p10) $(DEFINES_p10) /c /Tp "..\i4\file\buf_file.cc" /Fo"c:/tmp/2f450bfa.obj"

c:/tmp/5c5da04c.obj: ..\i4\string\string.cc ..\i4\string\string.hh ..\i4\inc\arch.hh ..\i4\init\init.hh ..\i4\memory\growheap.hh ..\i4\memory\malloc.hh ..\i4\memory\bmanage.hh ..\i4\inc\isllist.hh ..\i4\memory\lalloc.hh ..\i4\error\error.hh ..\i4\file\open.hh ..\i4\file\file.hh 
	$(CPP) $(CPPFLAGS_p10) $(INCLUDE_p10) $(DEFINES_p10) /c /Tp "..\i4\string\string.cc" /Fo"c:/tmp/5c5da04c.obj"

c:/tmp/85424b72.obj: ..\i4\error\error.cc ..\i4\error\error.hh 
	$(CPP) $(CPPFLAGS_p10) $(INCLUDE_p10) $(DEFINES_p10) /c /Tp "..\i4\error\error.cc" /Fo"c:/tmp/85424b72.obj"

c:/tmp/6a1cdb03.obj: ..\i4\threads\win32\threads.cc ..\i4\threads\threads.hh ..\i4\inc\arch.hh ..\i4\threads\critical.hh ..\i4\error\error.hh 
	$(CPP) $(CPPFLAGS_p10) $(INCLUDE_p10) $(DEFINES_p10) /c /Tp "..\i4\threads\win32\threads.cc" /Fo"c:/tmp/6a1cdb03.obj"

c:/tmp/08f31729.obj: ..\i4\time\win_time.cc ..\i4\inc\arch.hh ..\i4\time\time.hh 
	$(CPP) $(CPPFLAGS_p10) $(INCLUDE_p10) $(DEFINES_p10) /c /Tp "..\i4\time\win_time.cc" /Fo"c:/tmp/08f31729.obj"

c:/tmp/b123a4be.obj: ..\i4\time\profile.cc ..\i4\error\error.hh ..\i4\time\profile.hh ..\i4\inc\arch.hh ..\i4\time\win_prof.hh ..\i4\time\linux_prof.hh 
	$(CPP) $(CPPFLAGS_p10) $(INCLUDE_p10) $(DEFINES_p10) /c /Tp "..\i4\time\profile.cc" /Fo"c:/tmp/b123a4be.obj"

c:/tmp/a58a542d.obj: max_object.cc max_object.hh ..\golg\obj3d.hh ..\i4\inc\arch.hh ..\i4\math\point.hh ..\i4\math\vector.hh ..\i4\math\num_type.hh ..\render\r1_vert.hh ..\golg\g1_vert.hh ..\render\tex_id.hh ..\golg\load3d.hh ..\i4\memory\growheap.hh ..\i4\memory\malloc.hh ..\i4\memory\bmanage.hh ..\i4\inc\isllist.hh ..\i4\string\string.hh ..\i4\init\init.hh ..\i4\memory\lalloc.hh ..\i4\error\error.hh ..\i4\file\open.hh ..\i4\memory\array.hh ..\golg\saver.hh ..\i4\file\file.hh ..\i4\memory\growarry.hh ..\golg\path.hh ..\golg\reference.hh ..\i4\loaders\dir_load.hh ..\i4\loaders\dir_save.hh ..\golg\saver_id.hh 
	$(CPP) $(CPPFLAGS_p10) $(INCLUDE_p10) $(DEFINES_p10) /c /Tp "max_object.cc" /Fo"c:/tmp/a58a542d.obj"

c:/tmp/ac8a9f03.obj: maxcomm.cc maxcomm.hh ..\i4\string\string.hh ..\i4\inc\arch.hh ..\i4\init\init.hh ..\i4\memory\growheap.hh ..\i4\memory\malloc.hh ..\i4\memory\bmanage.hh ..\i4\inc\isllist.hh ..\i4\memory\lalloc.hh ..\i4\error\error.hh ..\i4\file\open.hh 
	$(CPP) $(CPPFLAGS_p10) $(INCLUDE_p10) $(DEFINES_p10) /c /Tp "maxcomm.cc" /Fo"c:/tmp/ac8a9f03.obj"

c:/tmp/9b8d61cf.obj: util.cc crkutil.hh crkutilr.hh ..\golg\obj3d.hh ..\i4\inc\arch.hh ..\i4\math\point.hh ..\i4\math\vector.hh ..\i4\math\num_type.hh ..\render\r1_vert.hh ..\golg\g1_vert.hh ..\render\tex_id.hh maxcomm.hh ..\i4\string\string.hh ..\i4\init\init.hh ..\i4\memory\growheap.hh ..\i4\memory\malloc.hh ..\i4\memory\bmanage.hh ..\i4\inc\isllist.hh ..\i4\memory\lalloc.hh ..\i4\error\error.hh ..\i4\file\open.hh ..\i4\file\file.hh resource.hh max_object.hh ..\golg\load3d.hh ..\i4\memory\array.hh ..\golg\saver.hh ..\i4\memory\growarry.hh ..\golg\path.hh ..\golg\reference.hh ..\i4\loaders\dir_load.hh ..\i4\loaders\dir_save.hh 
	$(CPP) $(CPPFLAGS_p10) $(INCLUDE_p10) $(DEFINES_p10) /c /Tp "util.cc" /Fo"c:/tmp/9b8d61cf.obj"

c:/tmp/51a8f00f.obj: crkutil.cc crkutil.hh crkutilr.hh 
	$(CPP) $(CPPFLAGS_p10) $(INCLUDE_p10) $(DEFINES_p10) /c /Tp "crkutil.cc" /Fo"c:/tmp/51a8f00f.obj"

c:/tmp/a65f8392.res: plugin.rc
	$(RES) $(RESFLAGS) /fo"c:/tmp/a65f8392.res" "plugin.rc"

DEP_p10=\
  c:/tmp/23f88346.obj\
  c:/tmp/52dbcd55.obj\
  c:/tmp/f2117cc4.obj\
  c:/tmp/81088d5e.obj\
  c:/tmp/2ab10b1a.obj\
  c:/tmp/ec261cdc.obj\
  c:/tmp/4960b186.obj\
  c:/tmp/a7da6c2f.obj\
  c:/tmp/fd41af82.obj\
  c:/tmp/58a03d14.obj\
  c:/tmp/abc12eab.obj\
  c:/tmp/2f450bfa.obj\
  c:/tmp/5c5da04c.obj\
  c:/tmp/85424b72.obj\
  c:/tmp/6a1cdb03.obj\
  c:/tmp/08f31729.obj\
  c:/tmp/b123a4be.obj\
  c:/tmp/a58a542d.obj\
  c:/tmp/ac8a9f03.obj\
  c:/tmp/9b8d61cf.obj\
  c:/tmp/51a8f00f.obj\
  c:/tmp/a65f8392.res

c:/tmp/p1_16.dll: $(DEP_p10)
	link /DLL  /def:"plugin.def"  $(LFLAGS_p10) /OUT:"c:/tmp/p1_16.dll" $(LIBS_p10) @make_p10.lnk

# {{{ CL Variables

INCLUDE_p12=\
  /I "..\i4\inc"\
  /I "..\i4"\
  /I "..\golg"\
  /I "..\render"\
  /I "sdk_inc"

DEFINES_p12=\
  /D "WIN32"\
  /D "_WINDOWS"\
  /D "__MSC__"\
  /D "I4_SCREEN_DEPTH=16"

CPP=cl
CPPFLAGS_p12=/nologo /MD /Zi /Ox

# }}} #############################################################

LIBS_p12=\
  $(MSDEV)/lib/kernel32.lib\
  $(MSDEV)/lib/user32.lib\
  $(MSDEV)/lib/gdi32.lib\
  $(MSDEV)/lib/comdlg32.lib\
  $(MSDEV)/lib/oldnames.lib\
  $(MSDEV)/lib/comctl32.lib\
  $(MSDEV)/lib/msvcrt.lib\
  $(MSDEV)/lib/winmm.lib\
  c:/maxsdk/lib/GEOM.LIB\
  c:/maxsdk/lib/UTIL.LIB\
  c:/maxsdk/lib/CORE.LIB

LFLAGS_p12=/NODEFAULTLIB /DEBUG

RES=rc
RESFLAGS=/l 0x409 /d "_DEBUG" /i "$(MSDEV)/mfc/include"

c:/tmp/1f8fbfb5.obj: ..\i4\loaders\dir_load.cc ..\i4\loaders\dir_load.hh ..\i4\file\file.hh ..\i4\inc\arch.hh ..\i4\inc\isllist.hh ..\i4\string\string.hh ..\i4\init\init.hh ..\i4\memory\growheap.hh ..\i4\memory\malloc.hh ..\i4\memory\bmanage.hh ..\i4\memory\lalloc.hh ..\i4\error\error.hh ..\i4\file\open.hh ..\i4\memory\array.hh ..\i4\checksum\checksum.hh 
	$(CPP) $(CPPFLAGS_p12) $(INCLUDE_p12) $(DEFINES_p12) /c /Tp "..\i4\loaders\dir_load.cc" /Fo"c:/tmp/1f8fbfb5.obj"

c:/tmp/4e7209c4.obj: ..\i4\loaders\dir_save.cc ..\i4\loaders\dir_save.hh ..\i4\file\file.hh ..\i4\inc\arch.hh ..\i4\inc\isllist.hh ..\i4\string\string.hh ..\i4\init\init.hh ..\i4\memory\growheap.hh ..\i4\memory\malloc.hh ..\i4\memory\bmanage.hh ..\i4\memory\lalloc.hh ..\i4\error\error.hh ..\i4\file\open.hh ..\i4\memory\array.hh ..\i4\checksum\checksum.hh 
	$(CPP) $(CPPFLAGS_p12) $(INCLUDE_p12) $(DEFINES_p12) /c /Tp "..\i4\loaders\dir_save.cc" /Fo"c:/tmp/4e7209c4.obj"

c:/tmp/30532733.obj: ..\i4\checksum\checksum.cc ..\i4\inc\arch.hh 
	$(CPP) $(CPPFLAGS_p12) $(INCLUDE_p12) $(DEFINES_p12) /c /Tp "..\i4\checksum\checksum.cc" /Fo"c:/tmp/30532733.obj"

c:/tmp/1b16c0cd.obj: ..\i4\file\file.cc ..\i4\file\file.hh ..\i4\inc\arch.hh ..\i4\inc\isllist.hh ..\i4\string\string.hh ..\i4\init\init.hh ..\i4\memory\growheap.hh ..\i4\memory\malloc.hh ..\i4\memory\bmanage.hh ..\i4\memory\lalloc.hh ..\i4\error\error.hh ..\i4\file\open.hh ..\i4\time\profile.hh ..\i4\time\win_prof.hh ..\i4\time\linux_prof.hh 
	$(CPP) $(CPPFLAGS_p12) $(INCLUDE_p12) $(DEFINES_p12) /c /Tp "..\i4\file\file.cc" /Fo"c:/tmp/1b16c0cd.obj"

c:/tmp/a9969489.obj: ..\i4\file\win32\win_file.cc ..\i4\file\file.hh ..\i4\inc\arch.hh ..\i4\inc\isllist.hh ..\i4\string\string.hh ..\i4\init\init.hh ..\i4\memory\growheap.hh ..\i4\memory\malloc.hh ..\i4\memory\bmanage.hh ..\i4\memory\lalloc.hh ..\i4\error\error.hh ..\i4\file\open.hh ..\i4\time\profile.hh ..\i4\time\win_prof.hh ..\i4\time\linux_prof.hh ..\i4\file\get_dir.hh ..\i4\file\buf_file.hh ..\i4\file\async.hh ..\i4\memory\que.hh ..\i4\threads\signal.hh ..\i4\threads\linux\pthread.h ..\i4\threads\critical.hh 
	$(CPP) $(CPPFLAGS_p12) $(INCLUDE_p12) $(DEFINES_p12) /c /Tp "..\i4\file\win32\win_file.cc" /Fo"c:/tmp/a9969489.obj"

c:/tmp/36d6be4b.obj: ..\i4\file\async.cc ..\i4\file\async.hh ..\i4\memory\que.hh ..\i4\inc\arch.hh ..\i4\threads\signal.hh ..\i4\threads\linux\pthread.h ..\i4\threads\critical.hh ..\i4\init\init.hh ..\i4\file\file.hh ..\i4\inc\isllist.hh ..\i4\string\string.hh ..\i4\memory\growheap.hh ..\i4\memory\malloc.hh ..\i4\memory\bmanage.hh ..\i4\memory\lalloc.hh ..\i4\error\error.hh ..\i4\file\open.hh ..\i4\threads\threads.hh ..\i4\time\time.hh 
	$(CPP) $(CPPFLAGS_p12) $(INCLUDE_p12) $(DEFINES_p12) /c /Tp "..\i4\file\async.cc" /Fo"c:/tmp/36d6be4b.obj"

c:/tmp/e36ee4f5.obj: ..\i4\init\init.cc ..\i4\init\init.hh ..\i4\inc\arch.hh ..\i4\error\error.hh 
	$(CPP) $(CPPFLAGS_p12) $(INCLUDE_p12) $(DEFINES_p12) /c /Tp "..\i4\init\init.cc" /Fo"c:/tmp/e36ee4f5.obj"

c:/tmp/230a5b9e.obj: ..\i4\memory\malloc.cc ..\i4\inc\arch.hh ..\i4\memory\malloc.hh ..\i4\memory\bmanage.hh ..\i4\error\error.hh ..\i4\threads\critical.hh 
	$(CPP) $(CPPFLAGS_p12) $(INCLUDE_p12) $(DEFINES_p12) /c /Tp "..\i4\memory\malloc.cc" /Fo"c:/tmp/230a5b9e.obj"

c:/tmp/07cf0df1.obj: ..\i4\memory\bmanage.cc ..\i4\memory\bmanage.hh ..\i4\inc\arch.hh ..\i4\error\error.hh 
	$(CPP) $(CPPFLAGS_p12) $(INCLUDE_p12) $(DEFINES_p12) /c /Tp "..\i4\memory\bmanage.cc" /Fo"c:/tmp/07cf0df1.obj"

c:/tmp/bdfb0a83.obj: ..\i4\memory\growheap.cc ..\i4\memory\growheap.hh ..\i4\memory\malloc.hh ..\i4\inc\arch.hh ..\i4\memory\bmanage.hh ..\i4\inc\isllist.hh ..\i4\error\error.hh 
	$(CPP) $(CPPFLAGS_p12) $(INCLUDE_p12) $(DEFINES_p12) /c /Tp "..\i4\memory\growheap.cc" /Fo"c:/tmp/bdfb0a83.obj"

c:/tmp/451d481a.obj: ..\i4\file\win32\get_dir.cc ..\i4\string\string.hh ..\i4\inc\arch.hh ..\i4\init\init.hh ..\i4\memory\growheap.hh ..\i4\memory\malloc.hh ..\i4\memory\bmanage.hh ..\i4\inc\isllist.hh ..\i4\memory\lalloc.hh ..\i4\error\error.hh ..\i4\file\open.hh 
	$(CPP) $(CPPFLAGS_p12) $(INCLUDE_p12) $(DEFINES_p12) /c /Tp "..\i4\file\win32\get_dir.cc" /Fo"c:/tmp/451d481a.obj"

c:/tmp/ab75fa69.obj: ..\i4\file\buf_file.cc ..\i4\file\buf_file.hh ..\i4\file\file.hh ..\i4\inc\arch.hh ..\i4\inc\isllist.hh ..\i4\string\string.hh ..\i4\init\init.hh ..\i4\memory\growheap.hh ..\i4\memory\malloc.hh ..\i4\memory\bmanage.hh ..\i4\memory\lalloc.hh ..\i4\error\error.hh ..\i4\file\open.hh 
	$(CPP) $(CPPFLAGS_p12) $(INCLUDE_p12) $(DEFINES_p12) /c /Tp "..\i4\file\buf_file.cc" /Fo"c:/tmp/ab75fa69.obj"

c:/tmp/d88d8fbb.obj: ..\i4\string\string.cc ..\i4\string\string.hh ..\i4\inc\arch.hh ..\i4\init\init.hh ..\i4\memory\growheap.hh ..\i4\memory\malloc.hh ..\i4\memory\bmanage.hh ..\i4\inc\isllist.hh ..\i4\memory\lalloc.hh ..\i4\error\error.hh ..\i4\file\open.hh ..\i4\file\file.hh 
	$(CPP) $(CPPFLAGS_p12) $(INCLUDE_p12) $(DEFINES_p12) /c /Tp "..\i4\string\string.cc" /Fo"c:/tmp/d88d8fbb.obj"

c:/tmp/90035ce1.obj: ..\i4\error\error.cc ..\i4\error\error.hh 
	$(CPP) $(CPPFLAGS_p12) $(INCLUDE_p12) $(DEFINES_p12) /c /Tp "..\i4\error\error.cc" /Fo"c:/tmp/90035ce1.obj"

c:/tmp/0a604272.obj: ..\i4\threads\win32\threads.cc ..\i4\threads\threads.hh ..\i4\inc\arch.hh ..\i4\threads\critical.hh ..\i4\error\error.hh 
	$(CPP) $(CPPFLAGS_p12) $(INCLUDE_p12) $(DEFINES_p12) /c /Tp "..\i4\threads\win32\threads.cc" /Fo"c:/tmp/0a604272.obj"

c:/tmp/84230698.obj: ..\i4\time\win_time.cc ..\i4\inc\arch.hh ..\i4\time\time.hh 
	$(CPP) $(CPPFLAGS_p12) $(INCLUDE_p12) $(DEFINES_p12) /c /Tp "..\i4\time\win_time.cc" /Fo"c:/tmp/84230698.obj"

c:/tmp/fd64242d.obj: ..\i4\time\profile.cc ..\i4\error\error.hh ..\i4\time\profile.hh ..\i4\inc\arch.hh ..\i4\time\win_prof.hh ..\i4\time\linux_prof.hh 
	$(CPP) $(CPPFLAGS_p12) $(INCLUDE_p12) $(DEFINES_p12) /c /Tp "..\i4\time\profile.cc" /Fo"c:/tmp/fd64242d.obj"

c:/tmp/a1ef5c9c.obj: max_object.cc max_object.hh ..\golg\obj3d.hh ..\i4\inc\arch.hh ..\i4\math\point.hh ..\i4\math\vector.hh ..\i4\math\num_type.hh ..\render\r1_vert.hh ..\golg\g1_vert.hh ..\render\tex_id.hh ..\golg\load3d.hh ..\i4\memory\growheap.hh ..\i4\memory\malloc.hh ..\i4\memory\bmanage.hh ..\i4\inc\isllist.hh ..\i4\string\string.hh ..\i4\init\init.hh ..\i4\memory\lalloc.hh ..\i4\error\error.hh ..\i4\file\open.hh ..\i4\memory\array.hh ..\golg\saver.hh ..\i4\file\file.hh ..\i4\memory\growarry.hh ..\golg\path.hh ..\golg\reference.hh ..\i4\loaders\dir_load.hh ..\i4\loaders\dir_save.hh ..\golg\saver_id.hh 
	$(CPP) $(CPPFLAGS_p12) $(INCLUDE_p12) $(DEFINES_p12) /c /Tp "max_object.cc" /Fo"c:/tmp/a1ef5c9c.obj"

c:/tmp/22245a72.obj: maxcomm.cc maxcomm.hh ..\i4\string\string.hh ..\i4\inc\arch.hh ..\i4\init\init.hh ..\i4\memory\growheap.hh ..\i4\memory\malloc.hh ..\i4\memory\bmanage.hh ..\i4\inc\isllist.hh ..\i4\memory\lalloc.hh ..\i4\error\error.hh ..\i4\file\open.hh 
	$(CPP) $(CPPFLAGS_p12) $(INCLUDE_p12) $(DEFINES_p12) /c /Tp "maxcomm.cc" /Fo"c:/tmp/22245a72.obj"

c:/tmp/0543cf3e.obj: util.cc crkutil.hh crkutilr.hh ..\golg\obj3d.hh ..\i4\inc\arch.hh ..\i4\math\point.hh ..\i4\math\vector.hh ..\i4\math\num_type.hh ..\render\r1_vert.hh ..\golg\g1_vert.hh ..\render\tex_id.hh maxcomm.hh ..\i4\string\string.hh ..\i4\init\init.hh ..\i4\memory\growheap.hh ..\i4\memory\malloc.hh ..\i4\memory\bmanage.hh ..\i4\inc\isllist.hh ..\i4\memory\lalloc.hh ..\i4\error\error.hh ..\i4\file\open.hh ..\i4\file\file.hh resource.hh max_object.hh ..\golg\load3d.hh ..\i4\memory\array.hh ..\golg\saver.hh ..\i4\memory\growarry.hh ..\golg\path.hh ..\golg\reference.hh ..\i4\loaders\dir_load.hh ..\i4\loaders\dir_save.hh 
	$(CPP) $(CPPFLAGS_p12) $(INCLUDE_p12) $(DEFINES_p12) /c /Tp "util.cc" /Fo"c:/tmp/0543cf3e.obj"

c:/tmp/c742ab7e.obj: crkutil.cc crkutil.hh crkutilr.hh 
	$(CPP) $(CPPFLAGS_p12) $(INCLUDE_p12) $(DEFINES_p12) /c /Tp "crkutil.cc" /Fo"c:/tmp/c742ab7e.obj"

c:/tmp/a65f8392.res: plugin.rc
	$(RES) $(RESFLAGS) /fo"c:/tmp/a65f8392.res" "plugin.rc"

DEP_p12=\
  c:/tmp/1f8fbfb5.obj\
  c:/tmp/4e7209c4.obj\
  c:/tmp/30532733.obj\
  c:/tmp/1b16c0cd.obj\
  c:/tmp/a9969489.obj\
  c:/tmp/36d6be4b.obj\
  c:/tmp/e36ee4f5.obj\
  c:/tmp/230a5b9e.obj\
  c:/tmp/07cf0df1.obj\
  c:/tmp/bdfb0a83.obj\
  c:/tmp/451d481a.obj\
  c:/tmp/ab75fa69.obj\
  c:/tmp/d88d8fbb.obj\
  c:/tmp/90035ce1.obj\
  c:/tmp/0a604272.obj\
  c:/tmp/84230698.obj\
  c:/tmp/fd64242d.obj\
  c:/tmp/a1ef5c9c.obj\
  c:/tmp/22245a72.obj\
  c:/tmp/0543cf3e.obj\
  c:/tmp/c742ab7e.obj\
  c:/tmp/a65f8392.res

c:/tmp/p1o_16.dll: $(DEP_p12)
	link /DLL  /def:"plugin.def"  $(LFLAGS_p12) /OUT:"c:/tmp/p1o_16.dll" $(LIBS_p12) @make_p12.lnk

