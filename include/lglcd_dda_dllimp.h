/*
lglcd_dda_dll Logitech LCD Direct Device Access
copyright (c) 2007-2008 by delphifreak
mailto: delphifreak[at]fantasymail[dot]de

This unit provides direct access to the Logitech G15 LCD Device, and nearly
all other functions of the keyboard.

Read the ReadMe for a documentation of the functions.
*/

#if !defined(LGLCD_DDA_DLLIMP_H)
#define LGLCD_DDA_DLLIMP_H

#include <windows.h>

//Keyboard versions
const DWORD LGLCD_DDA_G15CLASSIC = 1;
const DWORD LGLCD_DDA_G15REFRESH = 2;

//constants from lglcd.h
const DWORD LGLCD_BMP_WIDTH = 160;
const DWORD LGLCD_BMP_HEIGHT = 43;

//constants for m-button lights
const BYTE mbM1 = 0x01;
const BYTE mbM2 = 0x02;
const BYTE mbM3 = 0x04;
const BYTE mbMR = 0x08;

//constants for g-key callback function
const DWORD VK_LG_G1   = 0x00000001;
const DWORD VK_LG_G2   = 0x00000002;
const DWORD VK_LG_G3   = 0x00000004;
const DWORD VK_LG_G4   = 0x00000008;
const DWORD VK_LG_G5   = 0x00000010;
const DWORD VK_LG_G6   = 0x00000020;
const DWORD VK_LG_G7   = 0x00000040;
const DWORD VK_LG_G8   = 0x00000080;
const DWORD VK_LG_G9   = 0x00000100;
const DWORD VK_LG_G10  = 0x00000200;
const DWORD VK_LG_G11  = 0x00000400;
const DWORD VK_LG_G12  = 0x00000800;
const DWORD VK_LG_G13  = 0x00001000;
const DWORD VK_LG_G14  = 0x00002000;
const DWORD VK_LG_G15  = 0x00004000;
const DWORD VK_LG_G16  = 0x00008000;
const DWORD VK_LG_G17  = 0x00010000;
const DWORD VK_LG_G18  = 0x00020000;

const DWORD VK_LG_M1   = 0x00040000;
const DWORD VK_LG_M2   = 0x00080000;
const DWORD VK_LG_M3   = 0x00100000;
const DWORD VK_LG_MR   = 0x00200000;

const DWORD VK_LG_DB0  = 0x00400000;
const DWORD VK_LG_DB1  = 0x00800000;
const DWORD VK_LG_DB2  = 0x01000000;
const DWORD VK_LG_DB3  = 0x02000000;
const DWORD VK_LG_DB4  = 0x04000000;

const DWORD VK_LG_BULB = 0x08000000;

//constants for media-key callback function
const BYTE VK_LG_MEDIA_NEXT_TRACK = 0x01;
const BYTE VK_LG_MEDIA_PREV_TRACK = 0x02;
const BYTE VK_LG_MEDIA_STOP       = 0x04;
const BYTE VK_LG_MEDIA_PLAY_PAUSE = 0x08;
const BYTE VK_LG_VOLUME_MUTE      = 0x10;
const BYTE VK_LG_VOLUME_UP        = 0x20;
const BYTE VK_LG_VOLUME_DOWN      = 0x40;

//constants for keyboard-key callback function (modifiers)
const BYTE VK_LG_KB_MOD_LCTRL     = 0x01;
const BYTE VK_LG_KB_MOD_LSHIFT    = 0x02;
const BYTE VK_LG_KB_MOD_MENU      = 0x04;
const BYTE VK_LG_KB_MOD_LWIN      = 0x08;
const BYTE VK_LG_KB_MOD_RCTRL     = 0x10;
const BYTE VK_LG_KB_MOD_RSHIFT    = 0x20;
const BYTE VK_LG_KB_MOD_ALTERNATE = 0x40;
const BYTE VK_LG_KB_MOD_RWIN      = 0x80;

const WORD VK_LG_KB_GAMESWITCH_ON  = 0x0100;
const WORD VK_LG_KB_GAMESWITCH_OFF = 0x0200;

//key-callback function prototypes
typedef void (__cdecl *GKeyCallbackFunction)(DWORD Keys, BOOL togglebit);
typedef void (__cdecl *MediaKeyCallbackFunction)(BYTE Keys);
typedef void (__cdecl *KeyboardKeyCallbackFunction)(WORD Modifier, BYTE Keys[6]);

//lcddevice pointer
typedef void* LCDDEVICE;

//displaydata[y][x] struct, every byte defines a column of 8 pixels;
//the lowest bit is responsible for the pixel on top of the column.
//[x] represents the column: 0 is left, (LGLCD_BMP_WIDTH-1) is right,
//[y] represents the row: 0 is top, (LGLCD_BMP_HEIGHT / 8) is bottom.
//Hint: the last row is not completely visible!
typedef struct {
  BYTE data[(LGLCD_BMP_HEIGHT / 8) + 1][LGLCD_BMP_WIDTH];
} DISPLAYDATA;

typedef struct {
  DWORD keys;
  BOOL togglebit;
} GKEYS;

typedef struct {
  WORD modifier;
  BYTE keys[6];
} KEYBOARDKEYS;

#ifdef __cplusplus
  extern "C" {
#endif

#define lglcd_dda(rt) __declspec(dllimport) rt __cdecl

lglcd_dda(LCDDEVICE) LgOpenLcdDevice(DWORD Index, BOOL Exclusive, DWORD* KeybdVer);
lglcd_dda(void) LgFreeLcdDevice(LCDDEVICE Device);

lglcd_dda(BOOL) LgSetDisplayLight(LCDDEVICE Device, BYTE Intensity);
lglcd_dda(BOOL) LgGetDisplayLight(LCDDEVICE Device, BYTE* Intensity);

lglcd_dda(BOOL) LgSetKeyboardLight(LCDDEVICE Device, BYTE Intensity);
lglcd_dda(BOOL) LgGetKeyboardLight(LCDDEVICE Device, BYTE* Intensity);

lglcd_dda(BOOL) LgSetMLights(LCDDEVICE Device, BYTE MButtons);
lglcd_dda(BOOL) LgGetMLights(LCDDEVICE Device, BYTE* MButtons);

lglcd_dda(BOOL) LgSetContrast(LCDDEVICE Device, BYTE Contrast);

lglcd_dda(BOOL) LgSetGKeyCallbackFunction(LCDDEVICE Device, GKeyCallbackFunction CallbackFunction);
lglcd_dda(BOOL) LgSetMediaKeyCallbackFunction(LCDDEVICE Device, MediaKeyCallbackFunction CallbackFunction);
lglcd_dda(BOOL) LgSetKeyboardKeyCallbackFunction(LCDDEVICE Device, KeyboardKeyCallbackFunction CallbackFunction);

lglcd_dda(BOOL) LgSetDisplayData(LCDDEVICE Device, DISPLAYDATA* DisplayData);

lglcd_dda(BOOL) LgPollGKeys(LCDDEVICE Device, GKEYS* Keys);
lglcd_dda(BOOL) LgPollKeyboardKeys(LCDDEVICE Device, KEYBOARDKEYS* Keys);
lglcd_dda(BOOL) LgPollMediaKeys(LCDDEVICE Device, BYTE* Keys);

lglcd_dda(BOOL) LgGetLcdDeviceDisplayName(LCDDEVICE Device, PWCHAR* Name);

#ifdef __cplusplus
}
#endif

#endif