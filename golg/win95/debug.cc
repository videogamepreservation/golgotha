/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include <windows.h>
#include <process.h>
#include <stdio.h>

class debug_start
{
public:
  debug_start() 
  {
    PROCESS_INFORMATION piProcInfo; 
    STARTUPINFO siStartInfo; 
    
    /* Set up members of STARTUPINFO structure. */ 
    
    siStartInfo.cb = sizeof(STARTUPINFO); 
    siStartInfo.lpReserved = NULL; 
    siStartInfo.lpReserved2 = NULL; 
    siStartInfo.cbReserved2 = 0; 
    siStartInfo.lpDesktop = NULL; 
    siStartInfo.dwFlags = 0; 
    
  /* Create the child process. */ 
  
    char cmd[200];
    sprintf(cmd,"f:\\jc\\code\\w95_db\\dbug\\debug\\dbug.exe %d", _getpid());
    CreateProcess(NULL, 
                  cmd,
                  NULL,          /* process security attributes        */ 
                  NULL,          /* primary thread security attributes */ 
                  TRUE,          /* handles are inherited              */ 
                  0,             /* creation flags                     */ 
                  NULL,          /* use parent's environment           */ 
                  NULL,          /* use parent's current directory     */ 
                  &siStartInfo,  /* STARTUPINFO pointer                */ 
                  &piProcInfo);  /* receives PROCESS_INFORMATION       */ 
    Sleep(5000);
  }
} debug_start_instance;
