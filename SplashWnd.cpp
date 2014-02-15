/*! \file SplashWnd.cpp
\brief Класс, реализующий splash-окно в отдельном потоке. Без MFC.

Created 2006-06-08 by Kirill V. Lyadvinsky \n
Modified: 
2007-05-25 Добавлена дополнительная синхронизация. Поправлена работа с таймером. \n
2008-06-20 Добавлен вывод текста. Поддержка нескольких мониторов. Поддержка подгружаемых ресурсов (DLL) \n

* The contents of this file are subject to the terms of the Common Development and    \n
* Distribution License ("CDDL")(collectively, the "License"). You may not use this    \n
* file except in compliance with the License. You can obtain a copy of the CDDL at    \n
* http://www.opensource.org/licenses/cddl1.php.                                       \n

*/

#include <Windows.h>
#include <CommCtrl.h>
#include <GdiPlus.h>
#include <string>
#include <process.h>

#include "SplashWnd.h"
//#include "MemStream.h"

CSplashWnd::CSplashWnd( HWND hParent )
{
	m_hThread = NULL;
	m_pImage = NULL;
	m_hSplashWnd = NULL;
	m_ThreadId = 0;
	m_hProgressWnd = NULL;
	m_hEvent = NULL;
	m_hParentWnd = hParent;
}

CSplashWnd::~CSplashWnd()
{
	//Hide();
	//if (m_pImage) delete m_pImage;
}

void CSplashWnd::SetImage( Gdiplus::Image* pImage )
{
	if (m_pImage == NULL && pImage != NULL)
		m_pImage = pImage->Clone();
}

// void CSplashWnd::SetImage(HMODULE hModule, UINT nResourceID/*, DWORD dwLangLCID*/ )
// {
// 	if (m_pImage != NULL) return;
// 
// 	HRSRC hImage = FindResource( hModule, MAKEINTRESOURCE(nResourceID), L"IMAGE" );
// 	IMalloc* pMalloc = NULL;
// 	if (FAILED(CoGetMalloc(1, 	&pMalloc)))
// 	{
// 		if (pMalloc) pMalloc->Release();
// 		return;
// 	}
// 
// 	IMemStream* pImageStream = CreateStreamOnMalloc(pMalloc, 1,
// 		static_cast<LPBYTE>(LockResource(LoadResource(hModule, hImage))),
// 		SizeofResource(hModule, hImage), TRUE);
// 	pMalloc->Release();
// 
// 	if (pImageStream)
// 	{
// 		m_pImage = new Gdiplus::Image(pImageStream);
// 		pImageStream->Release();
// 	}
// }

void CSplashWnd::Show()
{
	if (m_hThread == NULL)
	{
		m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_hThread = (HANDLE)_beginthreadex(NULL, 0, SplashThreadProc, static_cast<LPVOID>(this), 0, &m_ThreadId);
		if (WaitForSingleObject(m_hEvent, 5000) == WAIT_TIMEOUT)
		{
			OutputDebugString("Error starting SplashThread\n");
		}
	}
	else
	{
		PostThreadMessage( m_ThreadId, WM_ACTIVATE, WA_CLICKACTIVE, 0 );
	}
}

void CSplashWnd::Hide()
{
	if (m_hThread)
	{
		PostThreadMessage(m_ThreadId, WM_QUIT, 0, 0);
		if ( WaitForSingleObject(m_hThread, 9000) == WAIT_TIMEOUT )
		{
			::TerminateThread( m_hThread, 2222 );
		}
		CloseHandle(m_hThread);
		CloseHandle(m_hEvent);
	}
	m_hThread = NULL;
}

unsigned int __stdcall CSplashWnd::SplashThreadProc( void* lpParameter )
{
	CSplashWnd* pThis = static_cast<CSplashWnd*>(lpParameter);
	if (pThis->m_pImage == NULL) return 0;

	// Register your unique class name
	WNDCLASS wndcls = {0};

	wndcls.style = CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = SplashWndProc; 
	wndcls.hInstance = GetModuleHandle(NULL);
	wndcls.hCursor = LoadCursor(NULL, IDC_APPSTARTING);
	wndcls.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndcls.lpszClassName = "SplashWnd";
	wndcls.hIcon = LoadIcon(wndcls.hInstance, IDI_APPLICATION);

	if (!RegisterClass(&wndcls))
	{
		if (GetLastError() != 0x00000582) // already registered)
		{
			OutputDebugString("Unable to register class SplashWnd\n");
			return 0;
		}
	}

  // try to find monitor where mouse was last time
  POINT point = { 0 };
  MONITORINFO mi = { sizeof(MONITORINFO), 0 };
  HMONITOR hMonitor = 0;
  RECT rcArea = { 0 };

  ::GetCursorPos( &point );
  hMonitor = ::MonitorFromPoint( point, MONITOR_DEFAULTTONEAREST );
  if ( ::GetMonitorInfo( hMonitor, &mi ) )
  {
    rcArea.left = ( mi.rcMonitor.right + mi.rcMonitor.left - static_cast<long>(pThis->m_pImage->GetWidth()) ) /2;
    rcArea.top = ( mi.rcMonitor.top + mi.rcMonitor.bottom - static_cast<long>(pThis->m_pImage->GetHeight()) ) /2;
  }
  else
  {
    SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcArea, NULL);
    rcArea.left = (rcArea.right + rcArea.left - pThis->m_pImage->GetWidth())/2;
    rcArea.top = (rcArea.top + rcArea.bottom - pThis->m_pImage->GetHeight())/2;
  }  

	//
	pThis->m_hSplashWnd = CreateWindowEx(pThis->m_WindowName.length()?0:WS_EX_TOOLWINDOW, "SplashWnd", pThis->m_WindowName.c_str(), 
		WS_CLIPCHILDREN|WS_POPUP, rcArea.left, rcArea.top, pThis->m_pImage->GetWidth(), pThis->m_pImage->GetHeight(), 
		pThis->m_hParentWnd, NULL, wndcls.hInstance, NULL);
	if (!pThis->m_hSplashWnd)
	{
		OutputDebugString("Unable to create SplashWnd\n");
		return 0;
	}
  
	SetWindowLongPtr(pThis->m_hSplashWnd, GWL_USERDATA, reinterpret_cast<LONG_PTR>(pThis) );
	ShowWindow(pThis->m_hSplashWnd, SW_SHOWNOACTIVATE);

	MSG msg;
	BOOL bRet;
	LONG timerCount = 0;

	PeekMessage(&msg, NULL, 0, 0, 0); // invoke creating message queue
	SetEvent(pThis->m_hEvent);

	while ((bRet = GetMessage( &msg, NULL, 0, 0 )) != 0)
	{ 
		if (msg.message == WM_QUIT) break;
		if (msg.message == PBM_SETPOS)
		{
			KillTimer(NULL, pThis->m_TimerId);
			SendMessage(pThis->m_hSplashWnd, PBM_SETPOS, msg.wParam, msg.lParam);
			continue;
		}
		if (msg.message == PBM_SETSTEP)
		{
			SendMessage(pThis->m_hSplashWnd, PBM_SETPOS, LOWORD(msg.wParam), 0); // initiate progress bar creation
			SendMessage(pThis->m_hProgressWnd, PBM_SETSTEP, (HIWORD(msg.wParam)-LOWORD(msg.wParam))/msg.lParam, 0);
			timerCount = static_cast<LONG>(msg.lParam);
			pThis->m_TimerId = SetTimer(NULL, 0, 1000, NULL);
			continue;
		}
		if (msg.message == WM_TIMER && msg.wParam == pThis->m_TimerId)
		{
			SendMessage(pThis->m_hProgressWnd, PBM_STEPIT, 0, 0);
			timerCount--;
			if (timerCount <= 0) {
				timerCount =0;
				KillTimer(NULL, pThis->m_TimerId);
        Sleep(0);
			}
			continue;
		}
		if (msg.message == PBM_SETBARCOLOR)
		{
			if (!IsWindow(pThis->m_hProgressWnd)) {
				SendMessage(pThis->m_hSplashWnd, PBM_SETPOS, 0, 0); // initiate progress bar creation
			}
			SendMessage(pThis->m_hProgressWnd, PBM_SETBARCOLOR, msg.wParam, msg.lParam);
			continue;
		}

		if (bRet == -1)
		{
			// handle the error and possibly exit
		}
		else
		{
			TranslateMessage(&msg); 
			DispatchMessage(&msg);
		}
	}
	DestroyWindow(pThis->m_hSplashWnd);

	return 0;
}

LRESULT CALLBACK CSplashWnd::SplashWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	CSplashWnd* pInstance = reinterpret_cast<CSplashWnd*>(GetWindowLongPtr(hwnd, GWL_USERDATA));
	if (pInstance == NULL)
	{
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

  switch (uMsg)
	{
		case WM_PAINT:
		{
			if (pInstance->m_pImage)
			{
				Gdiplus::Graphics gdip(hwnd);
				gdip.DrawImage(pInstance->m_pImage, 0, 0, pInstance->m_pImage->GetWidth(), pInstance->m_pImage->GetHeight());

				if ( pInstance->m_ProgressMsg.size() > 0 )
				{
					Gdiplus::Font msgFont( L"Tahoma", 16, Gdiplus::UnitPixel );
					Gdiplus::SolidBrush msgBrush( static_cast<DWORD>(Gdiplus::Color::White) );
					WCHAR* buffer = new WCHAR[512];
					MultiByteToWideChar(CP_ACP, 0, pInstance->m_ProgressMsg.c_str(), -1, buffer, 512);
					gdip.DrawString( buffer, -1, &msgFont, Gdiplus::PointF(50.0f, pInstance->m_pImage->GetHeight()-55.0f), &msgBrush );
				}
			}
			ValidateRect(hwnd, NULL);
			return 0;
		} break;
		case PBM_SETPOS:
		{
			// Обновление progress bar
			if (!IsWindow(pInstance->m_hProgressWnd)) // Создание, если ещё не создан
			{
				RECT client;
				GetClientRect(hwnd, &client);
				pInstance->m_hProgressWnd = CreateWindow(PROGRESS_CLASS, NULL, WS_CHILD|WS_VISIBLE,
					4, client.bottom-20, client.right-8, 16, hwnd, NULL, GetModuleHandle(NULL), NULL);
				SendMessage(pInstance->m_hProgressWnd, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
			}
			SendMessage(pInstance->m_hProgressWnd, PBM_SETPOS, wParam, 0);

			// Обновление сообщения
			const std::string* msg = reinterpret_cast<std::string*>( lParam );
			if ( msg && pInstance->m_ProgressMsg != *msg )
			{
				pInstance->m_ProgressMsg = *msg;
				delete msg;
        SendMessage( pInstance->m_hSplashWnd, WM_PAINT, 0, 0 );
			}
			return 0;
		} break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CSplashWnd::SetWindowName( const char* windowName )
{
	m_WindowName = windowName;
}

void CSplashWnd::SetProgress( UINT procent )
{
  SetProgress( procent, static_cast<char*>(NULL) );
}

void CSplashWnd::SetProgress( UINT procent, const char* msg )
{
	std::string* tempmsg = new std::string( msg );
	PostThreadMessage( m_ThreadId, PBM_SETPOS, procent, reinterpret_cast<LPARAM>(tempmsg) );
}

void CSplashWnd::SetProgress( UINT procent, UINT nResourceID, HMODULE hModule )
{
	char* msg;
	int len = ::LoadString( hModule, nResourceID, reinterpret_cast<char*>(&msg), 0 );

	std::string* tempmsg = new std::string( msg, len );
	PostThreadMessage( m_ThreadId, PBM_SETPOS, procent, reinterpret_cast<LPARAM>(tempmsg) );
}

void CSplashWnd::SetAutoProgress( UINT from, UINT to, UINT seconds )
{
	PostThreadMessage(m_ThreadId, PBM_SETSTEP, MAKEWPARAM(from, to), seconds);
}

void CSplashWnd::SetProgressBarColor( COLORREF color )
{
	PostThreadMessage(m_ThreadId, PBM_SETBARCOLOR, 0, color);
}
