lglcd_dda_dll Logitech LCD Direct Device Access
copyright (c) 2007-2008 by delphifreak
mailto: delphifreak[at]fantasymail[dot]de

version: v0.3.1

This unit provides direct access to the Logitech G15 LCD Device, and nearly
all other functions of the keyboard.

Headerfiles for Delphi and C/C++ are provided.

All functions should be called with the calling convention cdecl!

Important Hint:
  The "new G15" or "G15 Refresh" as it is sometimes called is since v0.3 partially
  supported. Unsupported functions are LgSetKeyboardKeyCallbackFunction and
  LgPollKeyboardKeys, because Logitech changed a few things in the new version.
  I don't have such a new G15, so I want to thank "Obstbaum" from g15-applets.de
  for his help concerning the new keyboard support.

Function documentation:
  LgOpenLcdDevice:
    Returns a handle to the first LCD device found in the target-system.
    Index is reserved for later identification of multiple LCD devices.
    Set Exclusive to true if you want exclusive access to your keyboard. This will
      only work if no other program is already accessing the device already.
      If a valid device is returned, no other program will be able to access the
      device until you free it again.
    KeybdVer specifies the keyboard version. It will be set to LGLCD_DDA_G15CLASSIC
      for the Classic G15 and to LGLCD_DDA_G15REFRESH for the G15 Refresh.
    If this function fails it returns a null-pointer.
    Release the handle with LgFreeLcdDevice.

  LgFreeLcdDevice:
    Frees the handle of a LCD device, resets all callbacks.
    Device is the LCD device-pointer to free, returned by LgOpenLcdDevice.

  LgSetDisplayLight/LgGetDisplayLight:
    Sets/Gets the state of the display background light.
    Device is the LCD device-pointer returned by LgOpenLcdDevice.
    Intensity specifies the light intensity. Possible values are 0, 1 and 2    

  LgSetKeyboardLight/LgGetKeyboardLight:
    Sets/Gets the state of the keyboard background light.
    Device is the LCD device-pointer returned by LgOpenLcdDevice.
    Intensity specifies the light intensity. Possible values are 0, 1 and 2    

  LgSetMLights/LgGetMLights:
    Sets/Gets the state of the m-keys light. (The keys above the g-keys)
    Device is the LCD device-pointer returned by LgOpenLcdDevice.
    MButtons is a bitmask of mbM1, mbM2, mbM3 and mbMR (or'ed together).

  LgSetContrast:
    Sets the displays contrast. Getting the contrast value is not possible (yet?).
    Device is the LCD device-pointer returned by LgOpenLcdDevice.
    Contrast can be a value between 0 and 63 (0x3F). At 0 everything is "white",
      at 63 everything is "black". Default values are between 20 and 30.

  LgSetGKeyCallbackFunction/LgSetMediaKeyCallbackFunction/LgSetKeyboardKeyCallbackFunction:
    Sets the callback function for the g-keys and some other ones. 
    Device is the LCD device-pointer returned by LgOpenLcdDevice.
    CallbackFunction should be set to a pointer to a GKeyCallbackFunction/
      MediaKeyCallbackFunction/KeyboardKeyCallbackFunction.
      Setting CallbackFunction to 0 will clear the callback.
    This function creates a thread.

  LgSetDisplayData:
    Device is the LCD device-pointer returned by LgOpenLcdDevice.
    DisplayData[y][x] is an array of byte with 2 dimensions.
      Every byte defines a column of 8 pixels, the lowest bit is responsible for
      the pixel on top of the column. [x] represents the column: 0 is left, 
      159 is right, [y] represents the row: 0 is top, 5 is bottom.
      Hint: the last row is not completely visible!
      
  LgPollGKeys/LgPollKeyboardKeys/LgPollMediaKeys (since v0.2):
    Polls keystates
    Device is the LCD device-pointer returned by LgOpenLcdDevice.
    The second parameter (or the parameters in the structure of the second parameter)
      has/have the same meaning as the parameter(s) in the CallbackFunctions.

  LgGetLcdDeviceDisplayName (since v0.3):
    Returns a pointer to the DeviceDisplayName that is valid until LgFreeLcdDevice is called.
    Device is the LCD device-pointer returned by LgOpenLcdDevice.
    Name should point to a pointer where the pointer to the string should be stored.
    Hint: The string should make it possible to distinguish between the "classic" g15 and the
      "refresh" g15. The "classic" g15 will return "Logitech Gaming Keyboard" while the new
      one returns "G15 Gaming Keyboard".

  Callback Functions:
    All callback functions are called in own threads. Remember this when writing programms that
    need synchronization!
    The calling convention of all callback functions is cdecl (since v0.3; before it was stdcall)
    
    GKeyCallbackFunction:
      Keys is a bitmask of the constants declared in the headers (they are or'ed together):
        VK_LG_G1 - VK_LG_G18: The g-keys.
        VK_LG_M1 - VK_LG_MR: The profile keys and the macro-key.
        VK_LG_DB0 - VK_LG_DB4: The 5 keys below the display, starting with the round one.
        VK_LG_BULB: The "bulb"-key that controls the background light
      togglebit should change after every keypress, but don't rely on it.
      
    MediaKeyCallbackFunction:
      Keys is a bitmask of the constants declared in the headers (they are or'ed together):
        VK_LG_MEDIA_(...): The media keys.
        VK_LG_VOLUME_(...): The volume keys.
        
    KeyboardKeyCallbackFunction:
      Modifier is a bitmask of the constants declared in the headers (they are or'ed together):
        VK_LG_KB_MOD_(...): The modifier-keys (e.g. shift, ctrl) and the state of the
          Win-Key-Disabler-Switch (VK_LG_KB_GAMESWITCH)
      Keys is a array of byte containing 6 key codes:
        6 "normal" keys can be pressed at once. They are listed in the Keys-array in the sequence
        they were pressed, from 0 to 5. They are not (yet) declared in the headers, but I'm sure,
        that you will find your keycodes yourself ;) .