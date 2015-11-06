# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=plugin - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to plugin - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "plugin - Win32 Release" && "$(CFG)" != "plugin - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "plugin.mak" CFG="plugin - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "plugin - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "plugin - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "plugin - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\plugin.dll"

CLEAN : 
	-@erase ".\Release\plugin.dll"
	-@erase ".\Release\util.obj"
	-@erase ".\Release\maxcomm.obj"
	-@erase ".\Release\crkutil.obj"
	-@erase ".\Release\win_file.obj"
	-@erase ".\Release\get_dir.obj"
	-@erase ".\Release\buf_file.obj"
	-@erase ".\Release\async.obj"
	-@erase ".\Release\file.obj"
	-@erase ".\Release\malloc.obj"
	-@erase ".\Release\growheap.obj"
	-@erase ".\Release\bmanage.obj"
	-@erase ".\Release\error.obj"
	-@erase ".\Release\threads.obj"
	-@erase ".\Release\win32_timedev.obj"
	-@erase ".\Release\string.obj"
	-@erase ".\Release\init.obj"
	-@erase ".\Release\plugin.lib"
	-@erase ".\Release\plugin.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\i4" /I "..\golg" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "..\i4" /I "..\golg" /D "WIN32" /D "NDEBUG"\
 /D "_WINDOWS" /Fp"$(INTDIR)/plugin.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/plugin.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/plugin.pdb" /machine:I386 /out:"$(OUTDIR)/plugin.dll"\
 /implib:"$(OUTDIR)/plugin.lib" 
LINK32_OBJS= \
	".\Release\util.obj" \
	".\Release\maxcomm.obj" \
	".\Release\crkutil.obj" \
	".\Release\win_file.obj" \
	".\Release\get_dir.obj" \
	".\Release\buf_file.obj" \
	".\Release\async.obj" \
	".\Release\file.obj" \
	".\Release\malloc.obj" \
	".\Release\growheap.obj" \
	".\Release\bmanage.obj" \
	".\Release\error.obj" \
	".\Release\threads.obj" \
	".\Release\win32_timedev.obj" \
	".\Release\string.obj" \
	".\Release\init.obj"

"$(OUTDIR)\plugin.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "plugin - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\plugin.dll"

CLEAN : 
	-@erase ".\Debug\plugin.dll"
	-@erase ".\Debug\util.obj"
	-@erase ".\Debug\maxcomm.obj"
	-@erase ".\Debug\crkutil.obj"
	-@erase ".\Debug\win_file.obj"
	-@erase ".\Debug\get_dir.obj"
	-@erase ".\Debug\buf_file.obj"
	-@erase ".\Debug\async.obj"
	-@erase ".\Debug\file.obj"
	-@erase ".\Debug\malloc.obj"
	-@erase ".\Debug\growheap.obj"
	-@erase ".\Debug\bmanage.obj"
	-@erase ".\Debug\error.obj"
	-@erase ".\Debug\threads.obj"
	-@erase ".\Debug\win32_timedev.obj"
	-@erase ".\Debug\string.obj"
	-@erase ".\Debug\init.obj"
	-@erase ".\Debug\plugin.ilk"
	-@erase ".\Debug\plugin.lib"
	-@erase ".\Debug\plugin.exp"
	-@erase ".\Debug\plugin.pdb"
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\i4" /I "..\golg" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\i4" /I "..\golg" /D "WIN32" /D\
 "_DEBUG" /D "_WINDOWS" /Fp"$(INTDIR)/plugin.pch" /YX /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/plugin.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/plugin.pdb" /debug /machine:I386 /out:"$(OUTDIR)/plugin.dll"\
 /implib:"$(OUTDIR)/plugin.lib" 
LINK32_OBJS= \
	".\Debug\util.obj" \
	".\Debug\maxcomm.obj" \
	".\Debug\crkutil.obj" \
	".\Debug\win_file.obj" \
	".\Debug\get_dir.obj" \
	".\Debug\buf_file.obj" \
	".\Debug\async.obj" \
	".\Debug\file.obj" \
	".\Debug\malloc.obj" \
	".\Debug\growheap.obj" \
	".\Debug\bmanage.obj" \
	".\Debug\error.obj" \
	".\Debug\threads.obj" \
	".\Debug\win32_timedev.obj" \
	".\Debug\string.obj" \
	".\Debug\init.obj"

"$(OUTDIR)\plugin.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "plugin - Win32 Release"
# Name "plugin - Win32 Debug"

!IF  "$(CFG)" == "plugin - Win32 Release"

!ELSEIF  "$(CFG)" == "plugin - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\util.cc
DEP_CPP_UTIL_=\
	".\crkutil.hh"\
	".\maxcomm.hh"\
	
NODEP_CPP_UTIL_=\
	".\obj3d.hh"\
	".\file\file.hh"\
	".\string\string.hh"\
	

"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\maxcomm.cc
DEP_CPP_MAXCO=\
	".\maxcomm.hh"\
	
NODEP_CPP_MAXCO=\
	".\string\string.hh"\
	

"$(INTDIR)\maxcomm.obj" : $(SOURCE) $(DEP_CPP_MAXCO) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\crkutil.cc
DEP_CPP_CRKUT=\
	".\crkutil.hh"\
	

"$(INTDIR)\crkutil.obj" : $(SOURCE) $(DEP_CPP_CRKUT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=\jc\code\i4\file\win32\win_file.cc
DEP_CPP_WIN_F=\
	{$(INCLUDE)}"\sys\types.h"\
	{$(INCLUDE)}"\sys\stat.h"\
	
NODEP_CPP_WIN_F=\
	"..\i4\file\win32\file\file.hh"\
	"..\i4\file\win32\memory\malloc.hh"\
	"..\i4\file\win32\error\error.hh"\
	"..\i4\file\win32\time\profile.hh"\
	"..\i4\file\win32\file\get_dir.hh"\
	"..\i4\file\win32\file\buf_file.hh"\
	"..\i4\file\win32\file\async.hh"\
	

"$(INTDIR)\win_file.obj" : $(SOURCE) $(DEP_CPP_WIN_F) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\jc\code\i4\file\win32\get_dir.cc
NODEP_CPP_GET_D=\
	"..\i4\file\win32\string\string.hh"\
	"..\i4\file\win32\arch.hh"\
	

"$(INTDIR)\get_dir.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\jc\code\i4\file\buf_file.cc
NODEP_CPP_BUF_F=\
	"..\i4\file\file\buf_file.hh"\
	"..\i4\file\memory\malloc.hh"\
	

"$(INTDIR)\buf_file.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\jc\code\i4\file\async.cc
NODEP_CPP_ASYNC=\
	"..\i4\file\file\async.hh"\
	"..\i4\file\threads\threads.hh"\
	"..\i4\file\time\time.hh"\
	

"$(INTDIR)\async.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\jc\code\i4\file\file.cc
NODEP_CPP_FILE_=\
	"..\i4\file\file\file.hh"\
	"..\i4\file\memory\malloc.hh"\
	"..\i4\file\error\error.hh"\
	"..\i4\file\time\profile.hh"\
	

"$(INTDIR)\file.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\jc\code\i4\memory\malloc.cc
NODEP_CPP_MALLO=\
	"..\i4\memory\arch.hh"\
	"..\i4\memory\memory\malloc.hh"\
	"..\i4\memory\error\error.hh"\
	"..\i4\memory\threads\critical.hh"\
	"..\i4\memory\memory\bmanage.hh"\
	

"$(INTDIR)\malloc.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\jc\code\i4\memory\growheap.cc
NODEP_CPP_GROWH=\
	"..\i4\memory\memory\growheap.hh"\
	"..\i4\memory\error\error.hh"\
	

"$(INTDIR)\growheap.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\jc\code\i4\memory\bmanage.cc
NODEP_CPP_BMANA=\
	"..\i4\memory\memory\bmanage.hh"\
	"..\i4\memory\error\error.hh"\
	

"$(INTDIR)\bmanage.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\jc\code\i4\error\error.cc
NODEP_CPP_ERROR=\
	"..\i4\error\error\error.hh"\
	

"$(INTDIR)\error.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\jc\code\i4\threads\win32\threads.cc
NODEP_CPP_THREA=\
	"..\i4\threads\win32\threads\threads.hh"\
	"..\i4\threads\win32\threads\critical.hh"\
	"..\i4\threads\win32\error\error.hh"\
	

"$(INTDIR)\threads.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\jc\code\i4\time\win32_timedev.cc
NODEP_CPP_WIN32=\
	"..\i4\time\arch.hh"\
	"..\i4\time\time\time.hh"\
	"..\i4\time\time\timedev.hh"\
	"..\i4\time\device\kernel.hh"\
	"..\i4\time\init\init.hh"\
	

"$(INTDIR)\win32_timedev.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\jc\code\i4\string\string.cc
NODEP_CPP_STRIN=\
	"..\i4\string\string\string.hh"\
	"..\i4\string\file\file.hh"\
	"..\i4\string\memory\malloc.hh"\
	"..\i4\string\arch.hh"\
	"..\i4\string\error\error.hh"\
	

"$(INTDIR)\string.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\jc\code\i4\init\init.cc
NODEP_CPP_INIT_=\
	"..\i4\init\init\init.hh"\
	"..\i4\init\error\error.hh"\
	

"$(INTDIR)\init.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
