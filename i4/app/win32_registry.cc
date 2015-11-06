/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include <windows.h>
#include "app/registry.hh"


i4_bool i4_get_registry(i4_registry_type type, 
                        char *path, 
                        char *key_name,
                        char *buffer, int buf_length)
{
  HKEY key;

  if (RegOpenKeyEx(type==I4_REGISTRY_MACHINE ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                   path,
                   0,
                   KEY_READ,
                   &key)==ERROR_SUCCESS)
  {
    for (int i=0; 1; i++)
    {
      char name[256];
      DWORD name_size=256, type;
      DWORD data_size=buf_length;
     


      if (RegEnumValue(key, i, name, &name_size, 0, 
                     &type, 
                     (LPBYTE)buffer, 
                     &data_size)==ERROR_SUCCESS)
      {
        if (strcmp(name, key_name)==0)
        {
          RegCloseKey(key);
          return i4_T;
        }
      }
      else
      {
        RegCloseKey(key);
        return i4_F;
      }
    }
  }
  return i4_F;
}


i4_bool i4_set_registry(i4_registry_type type, 
                        char *path, 
                        char *key_name,
                        char *buffer)
{
  HKEY key;

  if (RegCreateKey(type==I4_REGISTRY_MACHINE ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                 path,
                 &key)==ERROR_SUCCESS)
  {
    RegSetValueEx(key, key_name, 0, REG_SZ, (w8 *)buffer, strlen(buffer)+1);
    RegCloseKey(key);
    return i4_T;
  }
  
  return i4_F;
}

