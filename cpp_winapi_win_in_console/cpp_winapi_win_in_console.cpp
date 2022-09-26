//based off: https://cplusplus.com/forum/windows/219154/

#include <iostream>
#include <windows.h>
#include <thread>
#include <string>

#define BUTTON1 101

using namespace std;

HWND hwnd; //our window
HWND label1, button1;  //control elements

UINT updMsg = 0; //ID of our message

int n=0;
bool leave=false;

//message processor callback
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

//type conversion (int to wchar)
LPCWSTR inttowchar(int val)
{
	wchar_t buf[2048];
	swprintf(buf, 2048, L"%d", val);
	return (LPCWSTR)buf;
}

//window thread
int winThread()
{
	WNDCLASS ourWindow = { 0 };
	ourWindow.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	ourWindow.hCursor = LoadCursor(NULL, IDC_ARROW);
	ourWindow.hInstance = NULL;
	ourWindow.lpfnWndProc = WndProc;
	ourWindow.lpszClassName = L"Window in Console"; //L symbol is
													//generally equal to 
													//(LPCWSTR)"Window in Console"
	ourWindow.style = CS_HREDRAW | CS_VREDRAW;
	if (!RegisterClass(&ourWindow))
		MessageBox(NULL, L"Could not register class", L"Error", MB_OK);
	hwnd = CreateWindow(ourWindow.lpszClassName,
		L"Window in Console",
		WS_SYSMENU | WS_MINIMIZEBOX | WS_CAPTION,
		900,
		80,
		640,
		480,
		NULL,
		NULL,
		NULL,
		NULL);

	label1 = CreateWindowEx(
		WS_EX_LEFT, L"STATIC", L"0",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER,
		10, 15, 200, 30,
		hwnd, NULL, NULL, NULL);
	button1 = CreateWindowEx(
		WS_EX_LEFT, L"BUTTON", L"Update",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER,
		10, 50, 200, 30,
		hwnd, (HMENU)BUTTON1, NULL, NULL);

	ShowWindow(hwnd, SW_RESTORE);

	//main loop for message processor
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	DeleteObject(hwnd); //doing it just in case
	return msg.wParam;
}

//console thread
int conThread()
{
	while (true)
	{
		int a = 0;
		cout << "Input a:\n";
		cin >> a;
		n += a;
		SendMessage(hwnd, updMsg,a,NULL);
		if (leave) break;
	}
	return 0;
}

//main func
int main()
{
	//registering our message ID to send messages from console thread to 
	//window thread
	updMsg = ::RegisterWindowMessage(L"WM_UPDATE_DATA");
	cout << "Mess ID = " << updMsg << endl;

	//starting threads
	thread conThreadObj(conThread);
	thread winThreadObj(winThread);

	//ending threads (threads are looped, so join() will
	//work when both threads are finished)
	conThreadObj.join();
	winThreadObj.join();

	return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{

	if (message==updMsg) //TREAD SAFE
	{
		LPCWSTR str_n = inttowchar(n);
		SetWindowText(label1, str_n);
		cout << "Got data update: " << (int)wparam << endl;
	}

	switch (message)
	{
	case WM_COMMAND:
	{
		if (wparam == BUTTON1) //NOT THREAD SAFE
		{
			LPCWSTR str_n = inttowchar(n);
			SetWindowText(label1, str_n);
		}
		break;
	}

	//program exit clauses	
	case WM_DESTROY:
		leave = true;
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, message, wparam, lparam);
	}
	return 0;
}