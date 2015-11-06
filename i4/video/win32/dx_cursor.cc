/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

//#define INITGUID
#include "video/win32/dx_cursor.hh"
#include "video/win32/win32_input.hh"
#include "image/image.hh"
#include "image/color.hh"
#include "area/rectlist.hh"
#include "main/win_main.hh"
#include <process.h>
#include "threads/threads.hh"

extern void dx_error(sw32 result);


void dx_threaded_mouse_class::thread()
{
  DIDEVICEOBJECTDATA od;

  //set_active(i4_T);
  
  SetCursorPos(0,0);

  while (!stop)
  {    
    DWORD dwElements = 1;
   
    WaitForSingleObject(direct_input_mouse_event, INFINITE);

    acquire_lock.lock();
    
    sw32    temp_mouse_x = current_mouse_x;
    sw32    temp_mouse_y = current_mouse_y;
    
    i4_bool asdf = i4_F;

    while (dwElements)
    {
      direct_input_mouse_device->Acquire();

      HRESULT hr = direct_input_mouse_device->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), 
                                                            &od,
                                                            &dwElements, 0);      
      if (active && dwElements)
      {
        switch (od.dwOfs)
        {          
          case DIMOFS_X: 
          {            
            sw32 new_x = temp_mouse_x + od.dwData;
  
            if (new_x < clip_x1)
            {
              new_x = clip_x1;
            }
            else
            if (new_x >= clip_x2)
            {
              new_x = clip_x2-1;
            }

            temp_mouse_x = new_x;
          } break;

          case DIMOFS_Y:
          {
            sw32 new_y = temp_mouse_y + od.dwData;

            if (new_y < clip_y1)
            {
              new_y = clip_y1;
            }
            else
            if (new_y >= clip_y2)
            {
              new_y = clip_y2-1;
            }

            temp_mouse_y = new_y;
          } break;
          
          case DIMOFS_BUTTON0:
          {
            if (input)
            {
              if (od.dwData & 0x80)              
              {
                asdf = i4_T;
                input->update_mouse_buttons(i4_mouse_button_event_class::LEFT,
                                            win32_input_class::DOWN);
              }
              else
                input->update_mouse_buttons(i4_mouse_button_event_class::LEFT,
                                            win32_input_class::UP);
            }
          } break;

          case DIMOFS_BUTTON1:
          {
            if (input)
            {
              if (od.dwData & 0x80)              
                input->update_mouse_buttons(i4_mouse_button_event_class::RIGHT,
                                            win32_input_class::DOWN);
              else
                input->update_mouse_buttons(i4_mouse_button_event_class::RIGHT,
                                            win32_input_class::UP);
            }
          } break;
          
          case DIMOFS_BUTTON2:
          {
            if (input)
            {
              if (od.dwData & 0x80)              
                input->update_mouse_buttons(i4_mouse_button_event_class::CENTER,
                                            win32_input_class::DOWN);
              else
                input->update_mouse_buttons(i4_mouse_button_event_class::CENTER,
                                            win32_input_class::UP);
            }
          } break;
        }
      }
    }
    
    if (active)
    {
      if (temp_mouse_x != current_mouse_x || temp_mouse_y != current_mouse_y)
      {
        if (input)
          input->update_mouse_movement(temp_mouse_x,temp_mouse_y);
      
        if (!position_locked)
        {
          current_mouse_x = temp_mouse_x;
          current_mouse_y = temp_mouse_y;
        }
          
        draw_lock.lock();
        remove();
        display();
        draw_lock.unlock();
      }      
    }
    else
      direct_input_mouse_device->Unacquire();

    if (asdf)
    {
      i4_warning("hi");
    }
    acquire_lock.unlock();
  }

  set_active(i4_F);

  stop = i4_F;
}


void directx_mouse_thread_start(void *arg)
{
  ((dx_threaded_mouse_class *)arg)->thread();
  _endthread();
}



void dx_threaded_mouse_class::uninit_mouse()
{
  if (direct_input_driver)
  {
    direct_input_driver->Release();
    direct_input_driver = 0;
  }

  if (direct_input_mouse_device)
  {
    direct_input_mouse_device->Release();
    direct_input_mouse_device = 0;
  }

  if (direct_input_mouse_event)
  {
    CloseHandle(direct_input_mouse_event);
    direct_input_mouse_event = 0;
  }
}


void dx_threaded_mouse_class::init_mouse(HWND hwnd)
{
  HRESULT hr;

  hr = DirectInputCreate(i4_win32_instance, DIRECTINPUT_VERSION, &direct_input_driver, NULL);
  if (hr !=DI_OK) 
    i4_error("DirectInputCreate");

  hr = direct_input_driver->CreateDevice(GUID_SysMouse, &direct_input_mouse_device, NULL);
  if (hr !=DI_OK) 
    i4_error("CreateDevice(SysMouse)");

  hr = direct_input_mouse_device->SetDataFormat(&c_dfDIMouse);
  if (hr !=DI_OK) 
    i4_error("SetDataFormat(SysMouse, dfDIMouse)");


  hr = direct_input_mouse_device->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
  if (hr !=DI_OK) 
    i4_error("SetCooperativeLevel(SysMouse)");

  direct_input_mouse_event = CreateEvent(0, 0, 0, 0);
  if (direct_input_mouse_event == NULL) 
    i4_error("CreateEvent");


  hr = direct_input_mouse_device->SetEventNotification(direct_input_mouse_event);
  if (hr !=DI_OK) 
    i4_error("SetEventNotification(SysMouse)");

  
  /*
   *  Set the buffer size to DINPUT_BUFFERSIZE elements.
   *  The buffer size is a DWORD property associated with the device.
   */

  DIPROPDWORD dipdw =
  {
    {
      sizeof(DIPROPDWORD),        // diph.dwSize
      sizeof(DIPROPHEADER),       // diph.dwHeaderSize
      0,                          // diph.dwObj
      DIPH_DEVICE,                // diph.dwHow
    },
    16                            // dwData
  };
      
  hr = direct_input_mouse_device->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);      
  if (hr !=DI_OK)
    i4_error("Set buffer size(SysMouse)");
  
}

dx_threaded_mouse_class::dx_threaded_mouse_class(const i4_pal *screen_pal,
                                                 HWND   window_handle,
                                                 sw32 clip_x1, sw32 clip_y1,
                                                 sw32 clip_x2, sw32 clip_y2)
    : window_handle(window_handle),
      clip_x1(clip_x1), clip_y1(clip_y1), 
      clip_x2(clip_x2), clip_y2(clip_y2)      
{
  
  init_mouse(window_handle);
  
  cursor.pict=0;
  position_locked = i4_F;

  visible=i4_F;
  stop=i4_F;

  current_mouse_x=0;
  current_mouse_y=0; 

  _beginthread(directx_mouse_thread_start, thread_stack_size, this);
}

void dx_threaded_mouse_class::set_active(i4_bool act)
{
  acquire_lock.lock();

  active = act;

  if (active)
    direct_input_mouse_device->Acquire();
  else
    direct_input_mouse_device->Unacquire();
  
  acquire_lock.unlock();
}

dx_threaded_mouse_class::~dx_threaded_mouse_class()
{
  stop = i4_T;
  while (stop) 
    Sleep(0);

  if (cursor.pict)
  {
    delete cursor.pict;
    cursor.pict = 0;
  }
  
  uninit_mouse();
}


