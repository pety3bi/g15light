#pragma once

#include "resource.h"

#include <iostream>
#include <sstream>
#include <cstdio>
#include <ctime>

#include "ShellAPI.h"
//#include "windowsx.h"

#include "lglcd_dda_dllimp.h"
using namespace std;

#define MAX_LOADSTRING 100
#define MSG_MINTRAYICON WM_USER+1

// Global Variables:
HINSTANCE hInst;                      // current instance
TCHAR szTitle[MAX_LOADSTRING];        // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];  // the main window class name

// Tray icon
NOTIFYICONDATA nid = { 0 };

LCDDEVICE Device = NULL;

CRITICAL_SECTION cs;

bool bThreadFlag = false;
HANDLE hThread   = NULL;

// LCD data
BYTE bLastDisplay = 0;
BYTE bLastKeyboard = 0;
BOOL bDisabled = FALSE;

// Settings
BOOL  input       = FALSE;
BOOL  screensaver = FALSE;
BOOL  timelight   = FALSE;
DWORD timeout     = 10;
//DWORD time_from   = 0;
//DWORD time_to     = 0;
TCHAR szTimeFrom[MAX_LOADSTRING] = _T( "00:00:00" );
TCHAR szTimeTo[MAX_LOADSTRING] = _T( "00:00:00" );


// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
BOOL				LoadSettings();
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	Settings(HWND, UINT, WPARAM, LPARAM);
