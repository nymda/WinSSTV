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
SSTV::vec2 imgSize = { 0, 0 };
HBITMAP hBitmap = 0;
HBITMAP hOldBitmap = 0;
HDC hdc = 0;
HDC hdcMem = 0;
BITMAP bm = {};
HINSTANCE hI = 0;
PAINTSTRUCT ps = {};
RECT rect = { 0, 0 };
RECT rc = { 0, 0 };

SSTV::vec2 dispImgPos = { 5, 6 };
SSTV::vec2 dispImgSize = { 320, 240 };

HBITMAP bmp240 = 0;
unsigned char* DIB240 = 0;
SSTV::simpleBitmap displayImage240 = {
    {320, 240},
    0
};

HBITMAP bmp256 = 0;
unsigned char* DIB256 = 0;
SSTV::simpleBitmap displayImage256 = {
    {320, 256},
    0
};

void createConsole() {
	AllocConsole();
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);
	freopen_s(&f, "CONOUT$", "w", stderr);
	freopen_s(&f, "CONIN$", "r", stdin);
}

void initGlbBitmaps() {

    HDC hdc;
    
	//sets up and allocates a DIB for the 320x240 image
    
    BITMAPINFOHEADER bmih240;
    bmih240.biSize = sizeof(BITMAPINFOHEADER);
    bmih240.biWidth = 320;
    bmih240.biHeight = -240;
    bmih240.biPlanes = 1;
    bmih240.biBitCount = 24;
    bmih240.biCompression = BI_RGB;
    bmih240.biSizeImage = 0;
    bmih240.biXPelsPerMeter = 10;
    bmih240.biYPelsPerMeter = 10;
    bmih240.biClrUsed = 0;
    bmih240.biClrImportant = 0;

    BITMAPINFO dbmi240;
    ZeroMemory(&dbmi240, sizeof(dbmi240));
    dbmi240.bmiHeader = bmih240;
    dbmi240.bmiColors->rgbBlue = 0;
    dbmi240.bmiColors->rgbGreen = 0;
    dbmi240.bmiColors->rgbRed = 0;
    dbmi240.bmiColors->rgbReserved = 0;

    hdc = GetDC(hWnd);
    bmp240 = CreateDIBSection(hdc, &dbmi240, DIB_RGB_COLORS, (void**)&DIB240, NULL, 0);
    if (!bmp240 || bmp240 == INVALID_HANDLE_VALUE) {
        printf_s("Error: CreateDIBSection for 240 failed with error %d\r", GetLastError());
    }
    
    //sets up and allocates a DIB for the 320x256 image

    BITMAPINFOHEADER bmih256;
    bmih256.biSize = sizeof(BITMAPINFOHEADER);
    bmih256.biWidth = 320;
    bmih256.biHeight = -256;
    bmih256.biPlanes = 1;
    bmih256.biBitCount = 24;
    bmih256.biCompression = BI_RGB;
    bmih256.biSizeImage = 0;
    bmih256.biXPelsPerMeter = 10;
    bmih256.biYPelsPerMeter = 10;
    bmih256.biClrUsed = 0;
    bmih256.biClrImportant = 0;

    BITMAPINFO dbmi256;
    ZeroMemory(&dbmi256, sizeof(dbmi256));
    dbmi256.bmiHeader = bmih256;
    dbmi256.bmiColors->rgbBlue = 0;
    dbmi256.bmiColors->rgbGreen = 0;
    dbmi256.bmiColors->rgbRed = 0;
    dbmi256.bmiColors->rgbReserved = 0;

    hdc = GetDC(hWnd);
    bmp240 = CreateDIBSection(hdc, &dbmi256, DIB_RGB_COLORS, (void**)&DIB256, NULL, 0);
    if (!bmp240 || bmp240 == INVALID_HANDLE_VALUE) {
        printf_s("Error: CreateDIBSection for 256 failed with error %d\r", GetLastError());
    }
}

void pushImageTo240Disp(SSTV::simpleBitmap* image) {
	if (image->size.X != 320 || image->size.Y != 240) {
		printf_s("Error: image size is not 320x240\r");
		return;
	}

    unsigned char* tmp = DIB240;
    
    int BytesPerLine = image->size.X * 3;
    if (BytesPerLine % 4 != 0) { BytesPerLine += 4 - BytesPerLine % 4; }
    for (int y = 0; y < image->size.Y; y++) {
        PBYTE line = tmp;
        for (int x = 0; x < image->size.X; x++) {
			line[0] = image->data[(y * 320 + x)].b;
			line[1] = image->data[(y * 320 + x)].g;
			line[2] = image->data[(y * 320 + x)].r;
            line += 3;
        }
        tmp += BytesPerLine;
    }

    GetObject(bmp240, sizeof(BITMAP), &bm);
    hdc = GetDC(hWnd);
    hdcMem = CreateCompatibleDC(hdc);
    hOldBitmap = (HBITMAP)SelectObject(hdcMem, bmp240);
    ReleaseDC(hWnd, hdc);
}

void pushImageTo256Disp(SSTV::simpleBitmap* image){
	if (image->size.X != 320 || image->size.Y != 256) {
		printf_s("Error: image size is not 320x256\r");
		return;
	}
    
    PBYTE imageWrite = DIB256;

    int BytesPerLine = image->size.X * 3;
    if (BytesPerLine % 4 != 0) { BytesPerLine += 4 - BytesPerLine % 4; }
    for (int y = 0; y < image->size.Y; y++) {
        PBYTE line = imageWrite;
        for (int x = 0; x < image->size.X; x++) {
            line[0] = image->data[y * image->size.X + x].b;
            line[1] = image->data[y * image->size.X + x].g;
            line[2] = image->data[y * image->size.X + x].r;
            line += 3;
        }
        imageWrite += BytesPerLine;
    }

    GetObject(bmp256, sizeof(BITMAP), &bm);
    hdc = GetDC(hWnd);
    hdcMem = CreateCompatibleDC(hdc);
    hOldBitmap = (HBITMAP)SelectObject(hdcMem, bmp256);
    ReleaseDC(hWnd, hdc);
}

HBITMAP hbitmapFromRGBArray(SSTV::rgb* data, SSTV::vec2 size) {
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

SSTV::simpleBitmap test = {
    {320, 240},
    0
};

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
                    //loadImageFile(hWnd);

                    //initGlbBitmaps();

                    for (int x = 0; x < (320 * 240); x++) {
                        test.data[x] = { (unsigned char)(rand() % 0xFF), (unsigned char)(rand() % 0xFF), (unsigned char)(rand() % 0xFF) };
                    }

                    printf_s("paint: %x\n", test.data[0].r);
                    pushImageTo240Disp(&test);
                    
					RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
                }
                break;
            }
       
            case WM_CREATE: 
            {
                createConsole();
                initGlbBitmaps();

				test.data = (SSTV::rgb*)malloc((320 * 240) * 3);
                
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

