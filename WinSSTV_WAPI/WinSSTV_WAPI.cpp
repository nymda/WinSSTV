#include <iostream>
#include <dinput.h>
#include <tchar.h>
#include <CommCtrl.h>
#include <windows.h>

#include "framework.h"
#include "WinSSTV_WAPI.h"

#include "modes.h"
#include "wav.h"
#include "SSTV.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define MAX_LOADSTRING 100

#pragma comment( lib, "comctl32.lib" )
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.19041.1110' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

HWND Button = 0;
HWND hWnd = 0;

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

//RGB buffer
SSTV::rgb* imgData = 0;
vec2 imgSize = { 0, 0 };
HBITMAP hBitmap = 0;
HBITMAP hOldBitmap = 0;
HDC hdc = 0;
HDC hdcMem = 0;
BITMAP bm = {};
HINSTANCE hI = 0;
PAINTSTRUCT ps = {};
RECT rect = { 0, 0 };
RECT rc = { 0, 0 };

vec2 dispImgPos = { 5, 6 };
vec2 dispImgSize = { 320, 240 };

void createConsole() {
	AllocConsole();
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);
	freopen_s(&f, "CONOUT$", "w", stderr);
	freopen_s(&f, "CONIN$", "r", stdin);
}

HBITMAP hbitmapFromRGBArray(SSTV::rgb* data, vec2 size) {
    BITMAPINFOHEADER bmih;
    bmih.biSize = sizeof(BITMAPINFOHEADER);
    bmih.biWidth = size.X;
    bmih.biHeight = -size.Y;
    bmih.biPlanes = 1;
    bmih.biBitCount = 24;
    bmih.biCompression = BI_RGB;
    bmih.biSizeImage = 0;
    bmih.biXPelsPerMeter = 10;
    bmih.biYPelsPerMeter = 10;
    bmih.biClrUsed = 0;
    bmih.biClrImportant = 0;

    BITMAPINFO dbmi;
    ZeroMemory(&dbmi, sizeof(dbmi));
    dbmi.bmiHeader = bmih;
    dbmi.bmiColors->rgbBlue = 0;
    dbmi.bmiColors->rgbGreen = 0;
    dbmi.bmiColors->rgbRed = 0;
    dbmi.bmiColors->rgbReserved = 0;
    unsigned char* bits = 0;

	HDC hdc = GetDC(hWnd);
	HBITMAP hbitmap = CreateDIBSection(hdc, &dbmi, DIB_RGB_COLORS, (void**)&bits, NULL, 0);
	if (!hbitmap || hbitmap == INVALID_HANDLE_VALUE) {
		printf_s("Error: CreateDIBSection failed with error %d\r", GetLastError());
	}

    int BytesPerLine = size.X * 3;
    if (BytesPerLine % 4 != 0) { BytesPerLine += 4 - BytesPerLine % 4; }
	for (int y = 0; y < size.Y; y++) {
        PBYTE line = bits;
		for (int x = 0; x < size.X; x++) {
            line[0] = data[y * size.X + x].b;
            line[1] = data[y * size.X + x].g;
            line[2] = data[y * size.X + x].r;
            line += 3;
		}
        bits += BytesPerLine;
	}

	ReleaseDC(hWnd, hdc);

	return hbitmap;
}

HBITMAP loadImageFile(HWND callerHwnd) {
    imgData = 0;
    imgSize = { 0, 0 };
    hBitmap = 0;
    hOldBitmap = 0;
    hdc = 0;
    hdcMem = 0;
    bm = {};
    hI = 0;
    ps = {};
    rect = { 0, 0 };
    rc = { 0, 0 };
    
    OPENFILENAME ofn = { 0 };
    TCHAR szFile[260] = { 0 };

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = _T("Images\0*.*\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE && ofn.lpstrFile)
    {
        if (imgData) { free(imgData); }
        char convertedStrBuffer[128];
        int imgChannels = 0;
        wcstombs_s(0, convertedStrBuffer, ofn.lpstrFile, 128);
        imgData = (SSTV::rgb*)stbi_load(convertedStrBuffer, &imgSize.X, &imgSize.Y, &imgChannels, 3);
		printf_s("Image loaded: X: %i, Y: %i, Channels: %i\n", imgSize.X, imgSize.Y, imgChannels);
        if (imgData) {
            printf_s("Processing\n");
            if (hBitmap) {
                DeleteObject(hBitmap);
            }
            
            hBitmap = hbitmapFromRGBArray(imgData, imgSize);
            printf_s("HBitmap: %p\n", hBitmap);
            GetObject(hBitmap, sizeof(BITMAP), &bm);
            hdc = GetDC(hWnd);
            hdcMem = CreateCompatibleDC(hdc);
            hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);
        }
        else {
            printf_s("Image load failed\n");
        }
        return 0;
    }
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINSSTVWAPI, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINSSTVWAPI));
    MSG msg;

    // Main message loop:
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

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINSSTVWAPI));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINSSTVWAPI);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX & ~WS_MAXIMIZEBOX, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

void initUI(HWND parent) {    
    Button = CreateWindowW(L"Button", L"Open", WS_VISIBLE | WS_CHILD | WS_BORDER, 5 + 5 + dispImgSize.X, 5, 80, 25, parent, (HMENU)1, NULL, NULL);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_STANDARD_CLASSES;
	InitCommonControlsEx(&icex);
    
    switch (message)
    {  
        case WM_COMMAND:
            {
                if (LOWORD(wParam) == 1) {
                    loadImageFile(hWnd);
					RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
                }
                break;
            }
       
            case WM_CREATE: 
            {
                createConsole();
                initUI(hWnd);
                
                HFONT defFont;
				defFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
				SendMessage(Button, WM_SETFONT, (WPARAM)defFont, MAKELPARAM(TRUE, 0));
                break;
            }

            case WM_PAINT:
            {
                hdc = BeginPaint(hWnd, &ps);
                
                //overlay image with stretching to fit the window 
                GetClientRect(hWnd, &rect);
                SetStretchBltMode(hdc, STRETCH_HALFTONE);

                COLORREF color = RGB(255, 0, 0);
                HPEN pen = CreatePen(0, 1, color);
                SelectObject(hdc, pen);
                Rectangle(hdc, dispImgPos.X - 1, dispImgPos.Y - 1, dispImgSize.X + dispImgPos.X + 1, dispImgSize.Y + dispImgPos.Y + 1);
                
                StretchBlt(hdc, dispImgPos.X, dispImgPos.Y, dispImgSize.X, dispImgSize.Y, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);


  

				//BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);
                
                EndPaint(hWnd, &ps);
                
                break;
            }

            
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

