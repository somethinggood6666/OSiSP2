// // При включении SDKDDKVer.h будет задана самая новая из доступных платформ Windows.
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN             
#define IDC_LISTVIEW 1111
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <commctrl.h>
#include "resource.h"
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "ComCtl32.Lib")

#define MAX_LOADSTRING 100
#define ROW_DEF_HEIGHT 10

//#define MAKEINTRESOURCEL(i) (LPTSTR) ((DWORD) ((WORD) (i)))

HINSTANCE hInst;                                // текущий экземпляр
HWND      hWndLV;
int       lvColNum;
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
HWND                CreateListView(HWND hWndParent, UINT uId);
int                 SetListViewColumns(HWND hWndLV, int colNum);
BOOL                AddListViewItems(HWND hWndLV, int colNum, int textMaxLen, WCHAR* item);
void                DrawItem(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
HWND                AddListView(HWND hWndParent);
LONG                GetMaxHeight(HWND hWnd);
    
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_OSISP2, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // выполнение инициализации приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_OSISP2));

    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

// регистрация класса главного онка
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_OSISP2));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = nullptr;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}


// создание и вывод главного окна
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance;

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

// обработка сообщений в главном окне
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    WORD width;
    WORD height;
    int cx;
    LPMEASUREITEMSTRUCT measure;

    switch (message)
    {
    case WM_CREATE:
        hWndLV = AddListView(hWnd);
        break;
    case WM_SIZE: //перерасчет размеров окна
        width = LOWORD(lParam);
        height = HIWORD(lParam);

        cx = width / lvColNum;

        for (int i = 0; i < lvColNum; i++)
            ListView_SetColumnWidth(hWndLV, i, cx); // пересчет размера колонок, чтобы они были во всю ширину окна

        MoveWindow(hWndLV, 0, 0, width, height, TRUE);
        break;
    case WM_DRAWITEM:
        DrawItem(hWnd, message, wParam, lParam);
        break;
    case WM_MEASUREITEM:
        measure = (LPMEASUREITEMSTRUCT)lParam;
        height = GetMaxHeight(hWnd);
        measure->itemHeight = height;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LONG GetMaxHeight(HWND hWnd) {
    HDC hdc = GetDC(hWnd);
    int m = 4;
    WCHAR s[256];
    LPWSTR ss = reinterpret_cast<WCHAR*>(s);
    RECT rc;
    int count = ListView_GetItemCount(hWndLV);
    LONG res = 10;

    for (int index = 0; index < count; ++index)
        for (int col = 0; col < lvColNum; ++col)
        {
            ListView_GetItemText(hWndLV, index, col, ss, 256);

            ListView_GetSubItemRect(hWndLV, index, col, LVIR_LABEL, &rc);
            rc.left += m;
            rc.top += m;
            rc.bottom -= m;
            rc.right -= m;
            DrawText(hdc, ss, -1, &rc, DT_WORDBREAK | DT_LEFT | DT_CALCRECT);

            if (res < rc.bottom - rc.top + 1 + m * 2)
                res = rc.bottom - rc.top + 1 + m * 2;
        }

    ReleaseDC(hWnd, hdc);
    return res;
}

void DrawItem(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;

    if (lpdis->CtlID != IDC_LISTVIEW)
        return;

    HDC hdc = lpdis->hDC;
    int index = lpdis->itemID;
    int m = 2;
    WCHAR text[256];
    LPWSTR textCasted = reinterpret_cast<WCHAR*>(text);
    RECT rc;

    for (int col = 0; col < lvColNum; ++col)
    {
        ListView_GetItemText(hWndLV, index, col, textCasted, 256);

        ListView_GetSubItemRect(hWndLV, index, col, LVIR_LABEL, &rc);
        rc.left += m;
        rc.top += m;
        rc.bottom -= m;
        rc.right -= m;
        DrawText(hdc, textCasted, -1, &rc, DT_WORDBREAK | DT_LEFT);
    }
}

LPCSTR convertToLPTSTR(DWORD d) {
    std::ostringstream oss;
    oss << "0x" << std::hex << std::setw(8) << std::right << std::setfill('0') << d;
    LPCSTR pc = oss.str().c_str();
    return pc;
}

HWND AddListView(HWND hWnd) {
    //const WCHAR* colNumStr = MAKEINTRESOURCE(IDS_COLUMNS);
    int const colNum = 3;
    lvColNum = colNum;
    int const itemNum = 3;
    int const textMaxLen = 256; 
    // 222 - 226 -> аналог макроса makeintresource(i)
    // его напрямую нельзя использовать, потому что
    // msdn гласит, что результат функции может быть использован 
    // только в качестве параметра в других функциях
    WORD t1 = ((WORD)(104));  
    DWORD t2 = ((DWORD)(t1));
    LPCSTR t3 = convertToLPTSTR(t2);
    int wchars_num = MultiByteToWideChar(CP_UTF8, 0, t3, -1, NULL, 0);
    WCHAR* str = new WCHAR[wchars_num];

    MultiByteToWideChar(CP_UTF8, 0, t3, -1, str, wchars_num);

   // const WCHAR* str = (LPTSTR)((DWORD)((WORD)(104))); 
   // const WCHAR* str = t3;
    WCHAR items[colNum][256][textMaxLen];
    int j = 0;
    int c = 0;
    int s = wcslen(str);
    for (int i = 0; i < c; i++) {
        if (str[i] != L',')
            items[c % colNum][c / colNum][j] = str[i];
        else
        {
            j = 0;
            c++;
        }
    }

    HWND hWndLV;
    if ((hWndLV = CreateListView(hWnd, IDC_LISTVIEW)) == NULL)
    {
        MessageBox(NULL, L"Невозможно создать элемент ListView", L"Ошибка", MB_OK); //L for unicode
    }
    else
    {
        SetListViewColumns(hWndLV, colNum);
        AddListViewItems(hWndLV, colNum, textMaxLen, reinterpret_cast<WCHAR*>(items[0]));
        AddListViewItems(hWndLV, colNum, textMaxLen, reinterpret_cast<WCHAR*>(items[1]));
        AddListViewItems(hWndLV, colNum, textMaxLen, reinterpret_cast<WCHAR*>(items[2]));
        ShowWindow(hWndLV, SW_SHOWDEFAULT);

        MoveWindow(hWndLV, 0, 0, 0, 0, TRUE);
    }

    return hWndLV;
}

BOOL AddListViewItems(HWND hWndLV, int colNum, int textMaxLen, WCHAR* items)
{
    int iLastIndex = ListView_GetItemCount(hWndLV);

    LVITEM lvi;
    lvi.mask = LVIF_TEXT;
    lvi.cchTextMax = textMaxLen;
    lvi.iItem = iLastIndex;
    lvi.pszText = &items[0];
    lvi.iSubItem = 0;

    if (ListView_InsertItem(hWndLV, &lvi) == -1)
        return FALSE;
    for (int i = 1; i < colNum; i++)
        ListView_SetItemText(hWndLV, iLastIndex, i, &items[i * textMaxLen]);

    return TRUE;
}

int SetListViewColumns(HWND hWndLV, int colNum)
{
    RECT rcl;
    GetClientRect(hWndLV, &rcl);

    int index = -1;

    LVCOLUMN lvc;
    lvc.mask = LVCF_TEXT | LVCF_WIDTH;
    lvc.cx = (rcl.right - rcl.left) / colNum;
    lvc.cchTextMax = 0;

    for (int i = 0; i < colNum; i++)
    {
        lvc.pszText = nullptr;
        index = ListView_InsertColumn(hWndLV, i, &lvc);
        if (index == -1) break;
    }

    return index;
}

HWND CreateListView(HWND hWndParent, UINT uId)
{
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    RECT rcl;
    GetClientRect(hWndParent, &rcl);

    HWND hWndLV = CreateWindow(WC_LISTVIEW, L"",
        WS_CHILD | LVS_REPORT | LVS_NOCOLUMNHEADER | LVS_OWNERDRAWFIXED,
        0, 0, rcl.right - rcl.left, rcl.bottom - rcl.top,
        hWndParent, (HMENU)uId, GetModuleHandle(NULL), NULL);

    ListView_SetExtendedListViewStyleEx(hWndLV, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    return hWndLV;
}
