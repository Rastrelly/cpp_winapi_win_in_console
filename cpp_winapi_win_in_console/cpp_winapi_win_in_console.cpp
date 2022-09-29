//based off: https://cplusplus.com/forum/windows/219154/
//additional thanks for very good pointers to you-know-who-you-are :D

#include <iostream>
#include <windows.h>
#include <thread>
#include <string>
#include <atomic>

#define BUTTON1 101

typedef TCHAR TCHARARRAY[_MAX_INT_DIG];

HWND hwnd; //our window
HWND label1, button1;  //control elements

UINT updMsg = 0; //ID of our message

//using a shared pointer as a counter for thread safety
class counter 
{
private: int n;
public: 
	counter() { n = 0; } //counstructor restes counter
	void inc() { n ++; } //n++
	void inc(int val) { n+=val; } //polymorphic n+= 
	int getn() { return n; } //get our counter value
};

std::shared_ptr<counter> n = std::make_shared<counter>();

//use atmoic type bool for thread safety as an exit flag
std::atomic<bool> leave=false; 

//message processor callback
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

//type conversion (int to wchar)
void intToWChar(int val, TCHARARRAY &wc_data)
{
	_itow_s(val, wc_data, 10);
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
	std::cout << "Input a:\n";
	int a = 0;
	while ((!leave) && (std::cin >> a))
	{		
		n->inc(a);

		SendMessage(hwnd, updMsg,a,NULL);

		std::cout << "Input a:\n";
	}
	return 0;
}

//main func
int main()
{
	//registering our message ID to send messages from console thread to 
	//window thread
	updMsg = ::RegisterWindowMessage(L"WM_UPDATE_DATA");
	std::cout << "Mess ID = " << updMsg << std::endl;

	//starting threads
	std::thread conThreadObj(conThread);
	std::thread winThreadObj(winThread);

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
		TCHARARRAY data;
		intToWChar(n->getn(), data);
		SetWindowText(label1, (LPCWSTR)data);
		std::cout << "Got data update: " << (int)wparam << std::endl;
	}

	switch (message)
	{
	case WM_COMMAND:
	{
		if (wparam == BUTTON1) //NOT THREAD SAFE
		{
			TCHARARRAY data;
			intToWChar(n->getn(), data);
			SetWindowText(label1, (LPCWSTR)data);
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