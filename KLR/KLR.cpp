// KLR.cpp : Defines the entry point for the application.
//
#pragma comment(lib, "dwmapi.lib") 

#include "stdafx.h"
#include "KLR.h"
#include <dwmapi.h> 

#define KeyboardLayoutList std::vector<KeyboardLayout>
#define MAX_LOADSTRING 100
#define REGPATH_KEYBOARD_LAYOUT_NAME /*HKEY_LOCAL_MACHINE*/L"System\\CurrentControlSet\\Control\\keyboard layouts\\"
#define REGVAL_KEYBOARD_LAYOUT_NAME L"Layout Text"
#define SETTINGS_INI_FILE_NAME L"config.ini"
#define DEFAULT_DELIMITER L" "

void UpdateState();
BOOL InsertListViewItems(HWND hWndListView, int cItems);
BOOL InitListViewColumns(HWND hWndListView, int C_COLUMNS);
BOOL InsertListViewItems(HWND hWndListView, int nIcon, LPTSTR pszText);
std::wstring IniRead(std::wstring filename, std::wstring section, std::wstring key);
bool IniWrite(std::wstring filename, std::wstring section, std::wstring key, std::wstring data);

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
static HWND hList = NULL;						// Main List View Control

/* Temporary Variables Declarations */
LVCOLUMN LvCol; // Make Column struct for ListView
LVITEM LvItem;  // ListView Item struct
static bool bUpdateListView = true;

struct KeyboardLayout{
	HKL handle;
	std::wstring name;
};

KeyboardLayoutList klList(10);
KeyboardLayoutList klListOld(10);

bool SaveKeyboardlayoutList(KeyboardLayoutList save);
KeyboardLayoutList LoadKeyboardlayoutList();


inline bool operator==(const KeyboardLayout& lhs, const KeyboardLayout& rhs){
	return lhs.handle == rhs.handle;
}


// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

//TODO: Move this to another place
template<typename T>
std::vector<T>
split(const T & str, const T & delimiters) {
	std::vector<T> v;
	T::size_type start = 0;
	auto pos = str.find_first_of(delimiters, start);
	while (pos != T::npos) {
		if (pos != start) // ignore empty tokens
			v.emplace_back(str, start, pos - start);
		start = pos + 1;
		pos = str.find_first_of(delimiters, start);
	}
	if (start < str.length()) // ignore trailing delimiter
		v.emplace_back(str, start, str.length() - start); // add what's left of the string
	return v;
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	
	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_KLR, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_KLR));
	
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_KLR));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName = 0;// MAKEINTRESOURCE(IDC_KLR);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 320, 640, NULL, NULL, hInstance, NULL);
	//char tmp[8];
	//HKL buff[32];
	// GetKeyboardLayoutList(8, &buff[0]);
	// UnloadKeyboardLayout(buff[3]);

	INITCOMMONCONTROLSEX icex;           // Structure for control initialization.
	icex.dwICC = ICC_LISTVIEW_CLASSES;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	BOOL bRet = InitCommonControlsEx(&icex);

	RECT rcClient;
	GetClientRect(hWnd, &rcClient);
	hList = CreateWindowEx(0L,
		WC_LISTVIEW,
		L"",
		WS_VISIBLE | WS_CHILD | LVS_LIST | LVS_EDITLABELS,
		0, 0,
		rcClient.right - rcClient.left,
		rcClient.bottom - rcClient.top,
		hWnd,
		NULL,
		hInst,
		NULL);

	ListView_SetBkColor(hList, CLR_NONE);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_CREATE:
	{
		MARGINS margins = { -1, -1, -1, -1 };
		HRESULT hr = DwmExtendFrameIntoClientArea(hWnd, &margins);
		UINT WINAPI SetTimer(HWND hwnd, UINT idTimer, UINT uTimeout, TIMERPROC tmprc);
		int nTimerID = SetTimer(hWnd, 123456 /*!TODO*/, 1000 /*!TODO*/, NULL);
		
		break;
	}
	case WM_TIMER:

		if (wParam == 123456){
			UpdateState();
		}
		break;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		/*
		Graphics g(hdc);
		Rect paintRect(0, 0, rc.right, rc.bottom);
		// Создаем временный буфер
		Bitmap backBuffer(rc.right, rc.bottom, &g);
		Graphics temp(&backBuffer);
		// Рисуем в буфер
		PaintBackground(temp, paintRect);
		PaintFlower(temp, paintRect);
		PaintBatterfly(temp, paintRect);
		// Переносим на экран
		g.DrawImage(&backBuffer, 0, 0, 0, 0,
		rc.right, rc.bottom, UnitPixel);*/

		//DrawThemeTextEx(m_hTheme, dcMem, 0, 0, CT2CW(szTime), -1, uFormat, rcText2, &dto);


		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
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



std::wstring HexToString(int data){
	std::wstringstream ws;
	ws << std::hex << data;
	return ws.str();
}

std::wstring HexToString(int data, unsigned int size){
	std::wstring result = HexToString(data);
	if (result.length() <= size) {
		for (int i = size - result.length(); i > 0; --i)
			result = L'0' + result;
	}
	else {
		result.erase(size); // WARNING: Not tested
	}
	return result;
}

std::wstring HKL2KLName(HKL _in){
	std::wstring result = L"Unknown";
	std::wstring wsPath = REGPATH_KEYBOARD_LAYOUT_NAME;
	
	/* TODO: REWRITE THIS UGLY CODE ^_^ */
	std::wstring wsKLName = HexToString((DWORD)_in, 8);
	if ((wsKLName.c_str()[0] == wsKLName.c_str()[4]) &&
		(wsKLName.c_str()[1] == wsKLName.c_str()[5]) &&
		(wsKLName.c_str()[2] == wsKLName.c_str()[6]) &&
		(wsKLName.c_str()[3] == wsKLName.c_str()[7]))
		wsPath += L"0000" + wsKLName.erase(4); // Maybe wrong, but i don't find any other solution
	else
		wsPath += wsKLName;

	HKEY hKey;
	LONG lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wsPath.c_str(), 0, KEY_READ, &hKey);
	if (lRes != 0) return result;

	DWORD pType = 0;
	BYTE pData = 0;
	DWORD pcbData = 0;
	lRes = RegQueryValueEx(hKey, REGVAL_KEYBOARD_LAYOUT_NAME, 0, &pType, 0, &pcbData);
	BYTE *bData = new BYTE[pcbData];
	lRes = RegQueryValueEx(hKey, REGVAL_KEYBOARD_LAYOUT_NAME, 0, &pType, bData, &pcbData); // Read Keyboard Layout name from reg
	if (lRes != 0)	return result;

	result.clear();
	for (unsigned int i = 0; i < pcbData; i+=2 ) result += bData[i];

	return result;
}

void UpdateListView(){
	InitListViewColumns(hList, 1);
	for (auto &klItem : klList){
		InsertListViewItems(hList, 0, &klItem.name[0]);
		klItem.name;
	}

	SaveKeyboardlayoutList(klList);
	LoadKeyboardlayoutList();
}

void UpdateState(){

	klListOld = klList;

	/* Get HKL List */
	int nListCount = GetKeyboardLayoutList(0, NULL);
	if (!nListCount) return;
	HKL *varbuff = new HKL[nListCount];
	if (!varbuff) return;
	GetKeyboardLayoutList(nListCount, varbuff);

	klList.clear();
	KeyboardLayout klItem;
	for (int i = 0; i < nListCount; ++i){
		klItem.handle = varbuff[i];
		klItem.name = HKL2KLName(klItem.handle);
		klList.insert(klList.end(), klItem);
	}
	delete[] varbuff;	
	if (klListOld != klList)
		UpdateListView();
}

BOOL InsertListViewItems(HWND hWndListView, int cItems)
{
	LVITEM lvI;
	lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
	lvI.iSubItem = 0;
	lvI.state = 0;
	lvI.stateMask = 0;
	lvI.pszText = LPSTR_TEXTCALLBACK; // sends an LVN_GETDISPINFO message
	
	for (int index = 0; index < cItems; index++)
	{
		lvI.iItem = index;
		lvI.iImage = index;

		if (ListView_InsertItem(hWndListView, &lvI) == -1)
			return FALSE;
	}
	return TRUE;
}

BOOL InsertListViewItems(HWND hWndListView, int nIcon, LPTSTR pszText)
{
	LVITEM lvI;
	lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
	lvI.iSubItem = 0;
	lvI.state = 0;
	lvI.stateMask = 0;
	lvI.pszText = pszText;//LPSTR_TEXTCALLBACK;
	lvI.iItem = ListView_GetItemCount(hWndListView);
	lvI.iImage = nIcon;
	if (ListView_InsertItem(hWndListView, &lvI) == -1)
		return FALSE;
	return TRUE;
}

BOOL InitListViewColumns(HWND hWndListView, int C_COLUMNS)
{
	TCHAR szText[256] = L"header";     // temporary buffer 
	LVCOLUMN lvc;
	int iCol;

	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	for (iCol = 0; iCol < C_COLUMNS; iCol++)
	{
		lvc.iSubItem = iCol;
		lvc.pszText = szText;
		lvc.cx = 100;     // width of column in pixels

		if (iCol < 2)
			lvc.fmt = LVCFMT_LEFT;  // left-aligned column
		else
			lvc.fmt = LVCFMT_RIGHT; // right-aligned column                                 

		if (ListView_InsertColumn(hWndListView, iCol, &lvc) == -1)
			return FALSE;
	}
	return TRUE;
}


std::wstring IniRead(std::wstring filename, std::wstring section, std::wstring key){
	wchar_t *out = new wchar_t[512];
	GetPrivateProfileString(section.c_str(), key.c_str(), NULL, out, 512, filename.c_str());
	std::wstring result(out);
	delete out;
	return result;
}

bool IniWrite(std::wstring filename, std::wstring section, std::wstring key, std::wstring data){
	return WritePrivateProfileString(section.c_str(), key.c_str(), data.c_str(), filename.c_str());
}

bool SaveKeyboardlayoutList(KeyboardLayoutList save){
	std::wstring wsData;
	for (auto &klItem : save){
		wsData += DEFAULT_DELIMITER + std::to_wstring((unsigned int)klItem.handle);
	}

	return IniWrite(SETTINGS_INI_FILE_NAME, L"OPTIONS", L"KBLList", wsData);
};

KeyboardLayoutList LoadKeyboardlayoutList(){
	KeyboardLayoutList result(10);
	std::wstring wsData;
	wsData = IniRead(SETTINGS_INI_FILE_NAME, L"OPTIONS", L"KBLList");
	
	std::vector<std::wstring> v = split<std::wstring>(wsData, DEFAULT_DELIMITER);

	result.clear();
	for (std::wstring &vItem : v){
		KeyboardLayout klItem;
		klItem.handle = (HKL)_wtoi64(vItem.c_str());
		klItem.name = HKL2KLName(klItem.handle);
		result.insert(result.end(), klItem);
	}
	
	return result;
};



