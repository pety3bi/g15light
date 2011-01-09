// G15Light.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "G15Light.h"

void get_lights()
{
   EnterCriticalSection(&cs);

   BYTE bDisplay = 0;
   LgGetDisplayLight(Device, &bDisplay);
   if( !bDisabled && bDisplay != bLastDisplay )
      bLastDisplay = bDisplay;

   BYTE bKeyboard = 0;
   LgGetKeyboardLight(Device, &bKeyboard);
   if( !bDisabled && bKeyboard != bLastKeyboard )
      bLastKeyboard = bKeyboard;

   LeaveCriticalSection(&cs);
}

void set_lights(const bool enable = true )
{
   EnterCriticalSection(&cs);

   if( enable )
   {
      LgSetDisplayLight(Device, bLastDisplay);
      LgSetKeyboardLight(Device, bLastKeyboard);
      bDisabled = FALSE;
   }
   else
   {
      LgSetDisplayLight(Device, (BYTE)0);
      LgSetKeyboardLight(Device, (BYTE)0);
      bDisabled = TRUE;
   }

   LeaveCriticalSection(&cs);
}

void __cdecl GKeyCF(DWORD Keys, BOOL togglebit)
{
   if( Keys == VK_LG_BULB )
      get_lights();
}

DWORD WINAPI LightThreadProc( void* pParameter )
{
   bool *bThreadFlag = reinterpret_cast<bool *>( pParameter );

   DWORD KeybdVer = 0;
   Device = LgOpenLcdDevice(0, FALSE, &KeybdVer); //Get handle to the LCD device
   if( Device == NULL )
   {
      return -1;
   }

   /*
   switch( KeybdVer )
   {
   case LGLCD_DDA_G15CLASSIC:
      cout << "G15 Classic found." << endl;
      break;
   case LGLCD_DDA_G15REFRESH:
      cout << "G15 Refresh found. Some functions are not yet implemented for this version (-> ReadMe.txt)!" << endl;
      break;
   }
   */

   InitializeCriticalSection(&cs);

   LASTINPUTINFO lif;
   lif.cbSize = sizeof(LASTINPUTINFO);
   DWORD tickCount, idleCount;

   // Register the callback functions for key-events
   LgSetGKeyCallbackFunction(Device, GKeyCF);

   get_lights();

   while( *bThreadFlag )
   {
      GetLastInputInfo(&lif);
      tickCount = GetTickCount();
      idleCount = (tickCount - lif.dwTime) / 1000;

      //std::cout << "Idle time: " << idleCount << " seconds." << std::endl;

      if( idleCount >= timeout )
         set_lights( false );
      else
         set_lights( true );

      // Sleep for some milliseconds before the next output.
      Sleep( 50 );
   }

   LgSetGKeyCallbackFunction(Device, NULL);

   set_lights( true );

   LgFreeLcdDevice(Device);
   Device = NULL;

   DeleteCriticalSection(&cs);

   return 0;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

#ifndef _DEBUG
   HANDLE h = OpenMutex( MUTEX_ALL_ACCESS, FALSE, _T( "C9B27CD4-9AE9-46FA-A39B-F7125E1D1553" ) );
   if( h != NULL )
   {
      // приложение уже запущено, вываливаемся
      CloseHandle( h );
      MessageBox( 0, _T( "Application already started" ), _T( "G15Light" ), MB_OK | MB_ICONWARNING );
      return -1;
   }
#endif

   HANDLE m_hMutexHandle = CreateMutex( NULL, TRUE, _T( "C9B27CD4-9AE9-46FA-A39B-F7125E1D1553" ) );

	MSG msg;
   HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_G15LIGHT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

   if( !LoadSettings() )
      return FALSE;

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
		return FALSE;

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_G15LIGHT));

   if( input ) {
      bThreadFlag = true;
      hThread = CreateThread( NULL, 0, LightThreadProc, &bThreadFlag, 0L, NULL );
   }

   // Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

   if( input ) {
      bThreadFlag = false;
      // wait for thread to exit
      WaitForSingleObject( hThread, 5000 );
   }

   CloseHandle( m_hMutexHandle );

	return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style         = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc   = WndProc;
	wcex.cbClsExtra    = 0;
	wcex.cbWndExtra    = 0;
	wcex.hInstance     = hInstance;
	wcex.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_G15LIGHT));
	wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName  = MAKEINTRESOURCE(IDC_G15LIGHT);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm       = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_G15LIGHT));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   UNREFERENCED_PARAMETER(nCmdShow);

   // Store instance handle in our global variable
   hInst = hInstance;

   HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
   if( !hWnd )
      return FALSE;

   SetForegroundWindow(hWnd);

   if( !UpdateWindow(hWnd) )
      return FALSE;

   return TRUE;
}

BOOL LoadSettings()
{
	HKEY hKey;
   LSTATUS lStatus;

   // Открываем основную ветку или создаем ее, если ее нет
   lStatus = RegOpenKey (HKEY_CURRENT_USER, _T( "Software\\G15Light" ), &hKey );
	if( lStatus != ERROR_SUCCESS )
	{
      lStatus = RegCreateKey( HKEY_CURRENT_USER, _T( "Software\\G15Light" ), &hKey );
		if( lStatus != ERROR_SUCCESS )
		{
			MessageBox( 0, _T( "Creating registry key failed" ), _T( "G15Light" ), MB_OK | MB_ICONERROR );
			return FALSE;
		}

      DWORD dwVal = 10;
      lStatus = RegSetValueEx( hKey, _T( "timeout" ), 0L, REG_DWORD, (CONST BYTE*) &dwVal, sizeof( DWORD ) );
      if( lStatus != ERROR_SUCCESS)
         MessageBox( 0, _T( "Creating initial registry value failed" ), _T( "G15Light" ), MB_OK | MB_ICONERROR );

      dwVal = 1;
      lStatus = RegSetValueEx( hKey, _T( "input" ), 0L, REG_DWORD, (CONST BYTE*) &dwVal, sizeof( DWORD ) );
      if( lStatus != ERROR_SUCCESS)
         MessageBox( 0, _T( "Creating initial registry value failed" ), _T( "G15Light" ), MB_OK | MB_ICONERROR );

      dwVal = 0;
      lStatus = RegSetValueEx( hKey, _T( "screensaver" ), 0L, REG_DWORD, (CONST BYTE*) &dwVal, sizeof( DWORD ) );
      if( lStatus != ERROR_SUCCESS)
         MessageBox( 0, _T( "Creating initial registry value failed" ), _T( "G15Light" ), MB_OK | MB_ICONERROR );

      dwVal = 0;
      lStatus = RegSetValueEx( hKey, _T( "timelight" ), 0L, REG_DWORD, (CONST BYTE*) &dwVal, sizeof( DWORD ) );
      if( lStatus != ERROR_SUCCESS)
         MessageBox( 0, _T( "Creating initial registry value failed" ), _T( "G15Light" ), MB_OK | MB_ICONERROR );

      dwVal = 0;
      lStatus = RegSetValueEx( hKey, _T( "time_from" ), 0L, REG_DWORD, (CONST BYTE*) &dwVal, sizeof( DWORD ) );
      if( lStatus != ERROR_SUCCESS)
         MessageBox( 0, _T( "Creating initial registry value failed" ), _T( "G15Light" ), MB_OK | MB_ICONERROR );

      dwVal = 0;
      lStatus = RegSetValueEx( hKey, _T( "time_to" ), 0L, REG_DWORD, (CONST BYTE*) &dwVal, sizeof( DWORD ) );
      if( lStatus != ERROR_SUCCESS)
         MessageBox( 0, _T( "Creating initial registry value failed" ), _T( "G15Light" ), MB_OK | MB_ICONERROR );
   }
   else
   {
  		DWORD dwType = REG_DWORD;
		DWORD dwSize = sizeof( DWORD );

		DWORD dwTimeout = 0;
      lStatus = RegQueryValueEx(hKey, _T( "timeout" ), NULL, &dwType, (LPBYTE)&dwTimeout, &dwSize );
		if( lStatus == ERROR_SUCCESS )
			timeout = dwTimeout;
		else
			MessageBox( 0, _T( "Reading registry key failed" ), _T( "G15Light" ), MB_OK | MB_ICONERROR );

		DWORD dwInput = 0;
      lStatus = RegQueryValueEx(hKey, _T( "input" ), NULL, &dwType, (LPBYTE)&dwInput, &dwSize );
		if( lStatus == ERROR_SUCCESS )
			input = dwInput;
		else
			MessageBox( 0, _T( "Reading registry key failed" ), _T( "G15Light" ), MB_OK | MB_ICONERROR );

		DWORD dwScreenSaver = 0;
      lStatus = RegQueryValueEx(hKey, _T( "screensaver" ), NULL, &dwType, (LPBYTE)&dwScreenSaver, &dwSize );
		if( lStatus == ERROR_SUCCESS )
			screensaver = dwScreenSaver;
		else
			MessageBox( 0, _T( "Reading registry key failed" ), _T( "G15Light" ), MB_OK | MB_ICONERROR );

		DWORD dwTimelight = 0;
      lStatus = RegQueryValueEx(hKey, _T( "timelight" ), NULL, &dwType, (LPBYTE)&dwTimelight, &dwSize );
		if( lStatus == ERROR_SUCCESS )
			timelight = dwTimelight;
		else
			MessageBox( 0, _T( "Reading registry key failed" ), _T( "G15Light" ), MB_OK | MB_ICONERROR );

/*
		DWORD dwTimeFrom = 0;
      lStatus = RegQueryValueEx(hKey, _T( "time_from" ), NULL, &dwType, (LPBYTE)&dwTimeFrom, &dwSize );
		if( lStatus == ERROR_SUCCESS )
			time_from = dwTimeFrom;
		else
			MessageBox( 0, _T( "Reading registry key failed" ), _T( "G15Light" ), MB_OK | MB_ICONERROR );

		DWORD dwTimeTo = 0;
      lStatus = RegQueryValueEx(hKey, _T( "time_to" ), NULL, &dwType, (LPBYTE)&dwTimeTo, &dwSize );
		if( lStatus == ERROR_SUCCESS )
			time_to = dwTimeTo;
		else
			MessageBox( 0, _T( "Reading registry key failed" ), _T( "G15Light" ), MB_OK | MB_ICONERROR );
*/
   }

   RegCloseKey( hKey );

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
      case IDM_SETTINGS:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_SETTINGSBOX), hWnd, Settings);
			break;
      case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

   case WM_CREATE:
      //ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
      nid.cbSize = sizeof(NOTIFYICONDATA);
      nid.hWnd = hWnd; //the hWnd and uID members allow the OS to uniquely identify your icon. One window (the hWnd) can have more than one icon, as long as they have unique uIDs.
      nid.uFlags = //some flags that determine the tray's behavior:
         NIF_ICON //we're adding an icon
         | NIF_MESSAGE //we want the tray to send a message to the window identified by hWnd when something happens to our icon (see uCallbackMesage member below).
         | NIF_TIP; //our icon has a tooltip.
      // this message must be handled in hwnd's window procedure. more info below.
      nid.uCallbackMessage = MSG_MINTRAYICON;

      nid.hIcon = (HICON)LoadImage(
         GetModuleHandle(NULL), //get the HINSTANCE to this program
         MAKEINTRESOURCE(IDI_G15LIGHT), //grab the icon out of our resource file
         IMAGE_ICON, //tells the versatile LoadImage function that we are loading an icon
         16, 16, //x and y values. we want a 16x16-pixel icon for the tray.
         0); //no flags necessary. these flags specify special behavior, such as loading the icon from a file instead of a resource. see source list below for MSDN docs on LoadImage.

      _tcscpy_s(nid.szTip, _T( "G15 Light" ) ); //this string cannot be longer than 64 characters including the NULL terminator (which is added by default to string literals).

      //There are some more members of the NOTIFYICONDATA struct that are for advanced features we aren't using. See sources below for MSDN docs if you want to use balloon tips (only Win2000/XP).

      //NIM_ADD=add an icon to the tray. Then I pass a pointer to the struct that we set up above. You should error-check this function (it returns a BOOL) but I didn't since this is just an example.
      Shell_NotifyIcon(NIM_ADD, &nid);
      break;

   case WM_CLOSE:
      DestroyWindow(hWnd);
      break;

	case WM_DESTROY:
      /*
      // the OS only needs the hwnd and id number to know which icon to delete.
      NOTIFYICONDATA nid = { 0 };
      nid.cbSize = sizeof(NOTIFYICONDATA);
      nid.hWnd = hWnd;
      nid.uID = ID_MINTRAYICON;
      Shell_NotifyIcon(NIM_DELETE, &nid);
      */
      Shell_NotifyIcon(NIM_DELETE, &nid);

		PostQuitMessage(0);
		break;

   // aha! something happened, and the tray has dutifully notified us of the event. WPARAM is the uID member of nid, and LPARAM
   // is one of the Window mouse messages (such as WM_LBUTTONUP).
   case MSG_MINTRAYICON:
      /*
      // if it's not the icon we planted, then go away.
      if (wParam != ID_MINTRAYICON)
         break;
      */

      /*
      if (lParam == WM_LBUTTONDOWN)
      {
         ShowWindow(hWnd, SW_SHOW);
         break;
      }
      */

      if (lParam == WM_RBUTTONUP)
      {
         // create our menu. If it fails the next few functions may produce segmentation faults.
         HMENU hMenu = CreatePopupMenu();
         if( hMenu )
         {
            AppendMenu(hMenu, MF_STRING, IDM_SETTINGS, _T( "Settings" ) );
            AppendMenu(hMenu, MF_STRING, IDM_ABOUT, _T( "About" ));
            AppendMenu(hMenu, MF_STRING, IDM_EXIT, _T( "Exit" ) );

            // even though the window is hidden, we must set it to the foreground window because of popup-menu peculiarities.
            // See the Remarks section of the MSDN page for TrackPopupMenu.
            SetForegroundWindow(hWnd);
            SetFocus(hWnd);

            // get the position of the mouse at the time the icon was clicked
            POINT pt;
			   if( !GetCursorPos(&pt) )
               MessageBox( 0, _T( "GetCursorPos failed" ), _T( "G15Light" ), MB_OK | MB_ICONERROR );

            if( !TrackPopupMenuEx(hMenu, TPM_LEFTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, pt.x, pt.y, hWnd, NULL) )
               MessageBox( 0, _T( "TrackPopupMenuEx failed" ), _T( "G15Light" ), MB_OK | MB_ICONERROR );

            if( !DestroyMenu( hMenu ) )
               MessageBox( 0, _T( "DestroyMenu failed" ), _T( "G15Light" ), MB_OK | MB_ICONERROR );
            hMenu = NULL;
         }

         // send benign message to window to make sure the menu goes away.
         SendMessage(hWnd, WM_NULL, 0, 0);
         break;
      }

      break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
      break;
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// Message handler for settings box.
INT_PTR CALLBACK Settings(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		EnableWindow( GetDlgItem( hDlg, IDC_AUTOSTART ), TRUE );

   	HKEY hKey;
      LSTATUS lStatus;
      lStatus = RegOpenKey( HKEY_CURRENT_USER, _T( "Software\\Microsoft\\Windows\\CurrentVersion\\Run" ), &hKey );
	   if( lStatus == ERROR_SUCCESS )
      {
     		DWORD dwType = REG_SZ;
   		DWORD dwSize = MAX_LOADSTRING;

   		TCHAR szAutoStart[MAX_LOADSTRING];
         lStatus = RegQueryValueEx(hKey, _T( "G15Light" ), NULL, &dwType, (LPBYTE)&szAutoStart, &dwSize );
		   if( lStatus == ERROR_SUCCESS )
   			CheckDlgButton( hDlg, IDC_AUTOSTART, BST_CHECKED );
		   else
            CheckDlgButton( hDlg, IDC_AUTOSTART, BST_UNCHECKED );

         RegCloseKey( hKey );
      }

      SetDlgItemInt( hDlg, IDC_EDIT_TIMEOUT, timeout, FALSE );
      //EnableWindow( GetDlgItem( hDlg, IDC_INPUT ), TRUE );
      if( input )
      {
         CheckDlgButton( hDlg, IDC_INPUT, BST_CHECKED );
         EnableWindow( GetDlgItem( hDlg, IDC_EDIT_TIMEOUT ), TRUE );
      }
      else
         CheckDlgButton( hDlg, IDC_INPUT, BST_UNCHECKED );

      //EnableWindow( GetDlgItem( hDlg, IDC_SCREENSAVER ), TRUE );
      if( screensaver )
         CheckDlgButton( hDlg, IDC_SCREENSAVER, BST_CHECKED );
      else
         CheckDlgButton( hDlg, IDC_SCREENSAVER, BST_UNCHECKED );


      //SetDlgItemText( hDlg, IDC_DATETIMEPICKER_FROM, szTimeFrom );
      //SetDlgItemText( hDlg, IDC_DATETIMEPICKER_TO, szTimeTo );

      //EnableWindow( GetDlgItem( hDlg, IDC_TIME ), TRUE );
      if( timelight )
      {
         CheckDlgButton( hDlg, IDC_TIME, BST_CHECKED );
         EnableWindow( GetDlgItem( hDlg, IDC_DATETIMEPICKER_FROM ), TRUE );
         EnableWindow( GetDlgItem( hDlg, IDC_DATETIMEPICKER_TO ), TRUE );
      }
      else
         CheckDlgButton( hDlg, IDC_TIME, BST_UNCHECKED );

		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_INPUT)
		{
         if( IsDlgButtonChecked( hDlg, IDC_INPUT ) == BST_CHECKED )
            EnableWindow( GetDlgItem( hDlg, IDC_EDIT_TIMEOUT ), TRUE );
         else
            EnableWindow( GetDlgItem( hDlg, IDC_EDIT_TIMEOUT ), FALSE );

			return (INT_PTR)TRUE;
		}

		if (LOWORD(wParam) == IDC_TIME)
		{
         if( IsDlgButtonChecked( hDlg, IDC_TIME ) == BST_CHECKED )
         {
            EnableWindow( GetDlgItem( hDlg, IDC_DATETIMEPICKER_FROM ), TRUE );
            EnableWindow( GetDlgItem( hDlg, IDC_DATETIMEPICKER_TO ), TRUE );
         }
         else
         {
            EnableWindow( GetDlgItem( hDlg, IDC_DATETIMEPICKER_FROM ), FALSE );
            EnableWindow( GetDlgItem( hDlg, IDC_DATETIMEPICKER_TO ), FALSE );
         }

			return (INT_PTR)TRUE;
		}

      if (LOWORD(wParam) == IDOK)
		{
         LSTATUS lStatus = ERROR_SUCCESS;
         if( IsDlgButtonChecked( hDlg, IDC_AUTOSTART ) == BST_CHECKED )
         {
            TCHAR szPath[MAX_LOADSTRING];
            DWORD bytes = GetModuleFileName(NULL, szPath, MAX_LOADSTRING);

         	HKEY hKey;
            lStatus = RegOpenKey( HKEY_CURRENT_USER, _T( "Software\\Microsoft\\Windows\\CurrentVersion\\Run" ), &hKey );
	         if( lStatus == ERROR_SUCCESS )
	         {
               lStatus = RegSetValueEx( hKey, _T( "G15Light" ), 0L, REG_SZ, (CONST BYTE*) szPath, MAX_LOADSTRING );
               if( lStatus != ERROR_SUCCESS )
                  MessageBox( 0, _T( "Autostart failed" ), _T( "G15Light" ), MB_OK | MB_ICONERROR );
               RegCloseKey( hKey );
            }
         }
         else
         {
            lStatus = RegDeleteKeyValue( HKEY_CURRENT_USER, _T( "Software\\Microsoft\\Windows\\CurrentVersion\\Run" ), _T( "G15Light" ) );
            if( lStatus != ERROR_SUCCESS )
               MessageBox( 0, _T( "Autostart failed" ), _T( "G15Light" ), MB_OK | MB_ICONERROR );
         }

         if( IsDlgButtonChecked( hDlg, IDC_INPUT ) == BST_CHECKED )
            input = true;
         else
            input = false;

         if( IsDlgButtonChecked( hDlg, IDC_SCREENSAVER ) == BST_CHECKED )
            screensaver = true;
         else
            screensaver = false;

         if( IsDlgButtonChecked( hDlg, IDC_TIME ) == BST_CHECKED )
            timelight = true;
         else
            timelight = false;

         BOOL lpTranslated = FALSE;
         timeout = GetDlgItemInt( hDlg, IDC_EDIT_TIMEOUT, &lpTranslated, FALSE );

         //time_from = GetDlgItemInt( hDlg, IDC_DATETIMEPICKER_FROM, &lpTranslated, FALSE );
         //time_to   = GetDlgItemInt( hDlg, IDC_DATETIMEPICKER_TO, &lpTranslated, FALSE );


         //GetDlgItemText( hDlg, IDC_DATETIMEPICKER_FROM, szTimeFrom, MAX_LOADSTRING );
         //GetDlgItemText( hDlg, IDC_DATETIMEPICKER_TO, szTimeFrom, MAX_LOADSTRING );

/*
         unsigned int h, m, s;
         _stscanf_s(szTimeFrom, _T( "%d:%d:%d" ), &h, &m, &s);
         
         time_t rawtime = 0;
         struct tm timeinfo;
         //errno_t err = localtime_s( &timeinfo, &rawtime );
         errno_t err = gmtime_s( &timeinfo, &rawtime );

         timeinfo.tm_hour = h;
         timeinfo.tm_min  = m;
         timeinfo.tm_sec  = s;

         //timeinfo.tm_isdst = -1;

         rawtime = mktime ( &timeinfo );
         //mbstowcs( szTimeFrom, ctime(&rawtime), MAX_LOADSTRING );
         //localtime_s( &timeinfo, &rawtime );
*/

      	HKEY hKey;
         lStatus = RegOpenKey (HKEY_CURRENT_USER, _T( "Software\\G15Light" ), &hKey );
	      if( lStatus == ERROR_SUCCESS )
	      {
            DWORD dwVal = timeout;
            lStatus = RegSetValueEx( hKey, _T( "timeout" ), 0L, REG_DWORD, (CONST BYTE*) &dwVal, sizeof( DWORD ) );
            if( lStatus != ERROR_SUCCESS)
               MessageBox( 0, _T( "Store registry value failed" ), _T( "G15Light" ), MB_OK | MB_ICONERROR );

            dwVal = input;
            lStatus = RegSetValueEx( hKey, _T( "input" ), 0L, REG_DWORD, (CONST BYTE*) &dwVal, sizeof( DWORD ) );
            if( lStatus != ERROR_SUCCESS)
               MessageBox( 0, _T( "Store registry value failed" ), _T( "G15Light" ), MB_OK | MB_ICONERROR );

            dwVal = screensaver;
            lStatus = RegSetValueEx( hKey, _T( "screensaver" ), 0L, REG_DWORD, (CONST BYTE*) &dwVal, sizeof( DWORD ) );
            if( lStatus != ERROR_SUCCESS)
               MessageBox( 0, _T( "Store registry value failed" ), _T( "G15Light" ), MB_OK | MB_ICONERROR );

            dwVal = timelight;
            lStatus = RegSetValueEx( hKey, _T( "timelight" ), 0L, REG_DWORD, (CONST BYTE*) &dwVal, sizeof( DWORD ) );
            if( lStatus != ERROR_SUCCESS)
               MessageBox( 0, _T( "Store registry value failed" ), _T( "G15Light" ), MB_OK | MB_ICONERROR );

/*
            dwVal = time_from;
            lStatus = RegSetValueEx( hKey, _T( "time_from" ), 0L, REG_DWORD, (CONST BYTE*) &dwVal, sizeof( DWORD ) );
            if( lStatus != ERROR_SUCCESS)
               MessageBox( 0, _T( "Store registry value failed" ), _T( "G15Light" ), MB_OK | MB_ICONERROR );

            dwVal = time_to;
            lStatus = RegSetValueEx( hKey, _T( "time_to" ), 0L, REG_DWORD, (CONST BYTE*) &dwVal, sizeof( DWORD ) );
            if( lStatus != ERROR_SUCCESS)
               MessageBox( 0, _T( "Store registry value failed" ), _T( "G15Light" ), MB_OK | MB_ICONERROR );
*/

            RegCloseKey( hKey );
         }

			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}

      if (LOWORD(wParam) == IDCANCEL)
      {
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
      }

		break;
	}
	return (INT_PTR)FALSE;
}
